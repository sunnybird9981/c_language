/* 読み込んだwavファイルをn倍速にして別ファイルに保存 */
// 第1引数にファイル名(path)、第２引数に何倍速にするかの値
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include "/home/cometalow/Scripts/C_Language/include/wave.h"

int main(int argc, char *argv[]) {
	int i;
	int times;
	int chunkSize;
	short data;
	float s;
	FILE *fp, *fpWrite;
	char chunkID[5] = {};

	WAVE_FORMAT waveFormat = {0};
	WAVE_FORMAT waveFormatWrite = {0};
	MONO_PCM pcm, pcmForWrite;

	fp = fopen(argv[1], "rb");
	times = (argv[2][0] - '0');

	if(!fp) {
		printf("file open error\n");
		exit(0);
	}

//riffチャンクの読み込み
	fread(waveFormat.riffChunk.chunkID, 1, 4, fp);
	fread(&waveFormat.riffChunk.chunkSize, 4, 1, fp);
	fread(waveFormat.riffChunk.chunkFormType, 1, 4, fp);

	printf("chunkID=%s, chunkSize=%ld, chunkFormType=%s\n\n",
	waveFormat.riffChunk.chunkID, waveFormat.riffChunk.chunkSize, waveFormat.riffChunk.chunkFormType);

//fmtチャンクの読み込み
	fread(waveFormat.fmtChunk.chunkID, 1, 4, fp);
	fread(&waveFormat.fmtChunk.chunkSize, 4, 1, fp);
	fread(&waveFormat.fmtChunk.waveFormatType, 2, 1, fp);
	fread(&waveFormat.fmtChunk.formatChannel, 2, 1, fp);
	fread(&waveFormat.fmtChunk.samplesPerSec, 4, 1, fp);
	fread(&waveFormat.fmtChunk.bytesPerSec, 4, 1, fp);
	fread(&waveFormat.fmtChunk.blockSize, 2, 1, fp);
	fread(&waveFormat.fmtChunk.bitsPerSample, 2, 1, fp);

	printf("chunkID=%s\n", waveFormat.fmtChunk.chunkID);
	printf("chunkSize=%ld\n", waveFormat.fmtChunk.chunkSize);
	printf("waveFormatType=%d\n", waveFormat.fmtChunk.waveFormatType);
	printf("formatChannel=%d\n", waveFormat.fmtChunk.formatChannel);
	printf("samplesPerSec=%ld\n", waveFormat.fmtChunk.samplesPerSec);
	printf("bytesPerSec=%ld\n", waveFormat.fmtChunk.bytesPerSec);
	printf("blockSize=%d\n", waveFormat.fmtChunk.blockSize);
	printf("bitsPerSample=%d\n\n", waveFormat.fmtChunk.bitsPerSample);

//オプションのチャンク及びdataチャンのを読み込み
	fread(chunkID, 1, 4, fp);
	fread(&chunkSize, 4, 1, fp);

	while(strcmp(chunkID, "data")) {
		if(!strcmp(chunkID, "LIST")) {
			fseek(fp, chunkSize, SEEK_CUR);
		}
		if(!strcmp(chunkID, "")){
			break;
		}
		fread(chunkID, 1, 4, fp);
		fread(&chunkSize, 4, 1, fp);
		printf("chunkID=%s\n", chunkID);
		printf("chunkSize=%d\n\n", chunkSize);
}

	for(i = 0; i < 4; i++) {
		waveFormat.dataChunk.chunkID[i] = chunkID[i];
	}
	waveFormat.dataChunk.chunkSize = chunkSize;

	printf("chunkID=%s\n", waveFormat.dataChunk.chunkID);
	printf("chunkSize=%ld\n\n\n", waveFormat.dataChunk.chunkSize);

	pcm.fs = waveFormat.fmtChunk.samplesPerSec;
	pcm.bits = waveFormat.fmtChunk.bitsPerSample;
	pcm.length = waveFormat.dataChunk.chunkSize / times;
//	pcm.s = calloc(pcm.length, sizeof(float));



//書き込み

	fpWrite = fopen("a.wav", "wb");

	if(!fpWrite) {
		printf("file open error");
		exit(0);
	}

	//RIFFチャンクの書き込み
	fwrite(waveFormat.riffChunk.chunkID, 1, 4, fpWrite);
	fwrite(&waveFormat.riffChunk.chunkSize, 4, 1, fpWrite);
	fwrite(waveFormat.riffChunk.chunkFormType, 1, 4, fpWrite);

	//fmtチャンクの書き込み
	fwrite(waveFormat.fmtChunk.chunkID, 1, 4, fpWrite);
	fwrite(&waveFormat.fmtChunk.chunkSize, 4, 1, fpWrite);
	fwrite(&waveFormat.fmtChunk.waveFormatType, 2, 1, fpWrite);
	fwrite(&waveFormat.fmtChunk.formatChannel, 2, 1, fpWrite);
	fwrite(&waveFormat.fmtChunk.samplesPerSec, 4, 1, fpWrite);
	fwrite(&waveFormat.fmtChunk.bytesPerSec, 4, 1, fpWrite);
	fwrite(&waveFormat.fmtChunk.blockSize, 2, 1, fpWrite);
	fwrite(&waveFormat.fmtChunk.bitsPerSample, 2, 1, fpWrite);

	fwrite(waveFormat.dataChunk.chunkID, 1, 4, fpWrite);
	fwrite(&waveFormat.dataChunk.chunkSize, 4, 1, fpWrite);

	int j;
  short bytesPerSample = pcm.bits / 8;
	for(i = 0; i < pcm.length / bytesPerSample; i++) {
		for(j = 0; j < times; j++) {
			fread(&data, bytesPerSample, 1, fp);
		}
		//printf("%d\n", data);
		fwrite(&data, bytesPerSample, 1, fpWrite);
	}

	fclose(fpWrite);
//	free(pcmForWrite.s);





//	free(pcm.s);
	fclose(fp);

	return 0;
}
