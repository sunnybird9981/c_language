/* 曲当てゲームのためのプログラム */
// 実際には、第1引数に再生ファイルを入力する必要があるので、
// 自分で曲当てゲームを遊ぶ場合は工夫する必要あり
// 第1引数に再生ファイル名が必要
// 第2引数に、※  曲の再生時間が必要(単位は秒)
// 題3引数に、※  曲のフェードアウト開始時点の指定が必要(曲終了のfadeout秒前からフェードアウトする)
// ※ : 第2,3引数をデフォルトにしたい場合はどちらも-1を引数に入力する

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include "../include/wave.h"
#include "../include/file.h"

// 曲をduraion秒流す
#define DEFAULT_DURATION 5
// 曲終了FADEOUT秒前から音がフェードアウト
#define DEFAULT_FADEOUT 4 //seconds


// wavファイル読み込み開始位置をdatabaseから読み取る関数
int read_startpoint(FILE *, FILE_NAME *);


int main(int argc, char *argv[]) {
	int i;
	short data;
	FILE *fp, *fpdb;
	FILE_NAME wavefile = {0};

	WAVE_FORMAT waveFormat = {0};
	MONO_PCM pcm;

	if(argc != 4) {
		printf("error : few or too many arguments\n");
		exit(0);
	}

	/*
	// 再生するファイルを決定
	wavefile.name = get_rand_file("./");
	wavefile.length = (int8_t)cnt_strlen(wavefile.name);

	while(strcmp(wavefile.name + wavefile.length - 3, "wav") || 
	!strcmp(wavefile.name, "a.wav")) {
		wavefile.name = get_rand_file("./");
		wavefile.length = (int8_t)cnt_strlen(wavefile.name);
	}
	*/


	// 再生するファイル
	wavefile.name = argv[1];
	wavefile.length = (int8_t)cnt_strlen(wavefile.name);
	

	//printf("wavefile : %s\n", wavefile.name);
	fp = fopen(wavefile.name, "rb");


	if(!fp) {
		printf("file open error\n");
	} else {


		// チャンクの読み込み
		readFormat(&waveFormat, wavefile.name);


		// pcm設定
		pcm.fs = waveFormat.fmtChunk.samplesPerSec;
		pcm.bits = waveFormat.fmtChunk.bitsPerSample;
		pcm.length = waveFormat.dataChunk.chunkSize;
		

		// dataチャンクまで読み飛ばす
		int startpt_datachunk = 0;
		fseek(fp, 36, SEEK_SET);
		startpt_datachunk += 36;
		CHUNK Chunk = {0};
		while(fread(&Chunk, sizeof(CHUNK), 1, fp)) {
			if(strncmp(Chunk.ID, "data", 4)) {
				startpt_datachunk += sizeof(Chunk);
				fseek(fp, Chunk.size, SEEK_CUR);
			}	else
				break;
		}


		// 曲の再生位置の決定
		short duration;
		duration = atoi(argv[2]);
		if(duration < 0) {
			duration = DEFAULT_DURATION;
		}

		int start, fin;
		int duration_bytes;
		fpdb = fopen("database", "r");
		if(!fpdb) {
			printf("no database\n");
			start = 0;
		} else {
			start = read_startpoint(fpdb, &wavefile);
			start = (start < pcm.length) ? start : pcm.length;
		}

		if(start < startpt_datachunk) {
			start = startpt_datachunk;
		}

		fseek(fp, start, SEEK_CUR);    /* ファイル位置指示子の修正 */

		duration_bytes = duration * waveFormat.fmtChunk.bytesPerSec;
		fin = (start + duration_bytes < pcm.length) ? start + duration_bytes : pcm.length;
		pcm.length = fin - start;
		pcm.s = calloc(pcm.length, sizeof(float));


		// チャンク情報の修正
		waveFormat.riffChunk.chunkSize = 36 + pcm.length * 2;
		waveFormat.dataChunk.chunkSize = pcm.length * 2;


		/* 音データの読み込み,加工,書き込み */
		short fadeout_sec;

		fadeout_sec = atoi(argv[3]);
		if(fadeout_sec < 0) {
			fadeout_sec = DEFAULT_FADEOUT;
		}

		unsigned int fadeoutpt;
		uint16_t amp, amp_max;
		uint8_t bytesPerSample;
		bytesPerSample = waveFormat.fmtChunk.bitsPerSample / 8;

		fadeoutpt = ((fin - start) > fadeout_sec * pcm.fs * pcm.bits / 8) ? 
			fin - start - fadeout_sec * pcm.fs * pcm.bits / 8: fin - start;
		amp_max = fadeout_sec * pcm.fs * pcm.bits / 80000;
		amp_max *= amp_max;
    // 音データの読み込み,加工
		for(i = 0; i < fin - start; i++) {
			fread(&data, bytesPerSample, 1, fp);
			if(fadeoutpt < i) {
				amp = (fin - start - i) / 10000;
				amp *= amp;
				pcm.s[i] = data * amp / amp_max / GROUND;
			} else {
				pcm.s[i] = data / GROUND;
			}
		}
    // 音データの書き込み
		waveWrite(&waveFormat, &pcm, "/home/cometalow/command/mfile/intro.wav");

		free(pcm.s);
		if(fpdb)
			fclose(fpdb);
	}

	fclose(fp);


	return 0;
}


int read_startpoint(FILE *fp, FILE_NAME *wavefile) {
	int start;
	char *buff;
	
	buff = (char *)malloc((wavefile->length) + 2);
	
	while(fgets(buff, wavefile->length + 2, fp) != NULL) {
		// databaseファイルでスタート位置が設定されていた場合は反映
		if(buff[0] == '#') {
			if(!strncmp(buff + 1, wavefile->name, wavefile->length)) {
				while(strchr(buff, '\n') == NULL)
					fgets(buff, wavefile->length + 2, fp);

				fgets(buff, wavefile->length + 2, fp);
				start = atoi(buff);
				free(buff);

				return start;
			}
		}
	}

	free(buff);


	return 0;
}
