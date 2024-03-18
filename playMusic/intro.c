/* ALSA lib を使用して、WAVファイルを再生する */

/* PCMデフォルト設定 */
#define PCM_DEFAULT

//曲が流れる時間の長さ
#define TIME 200

#include <stdio.h>
#include <stdlib.h>
#include <stdint.h>
#include <dirent.h>
#include <alsa/asoundlib.h>
#include <termios.h>
#include <unistd.h>
#include <fcntl.h>
#include <time.h>
#include "playwave.h"


int main(void)
{
  /* 入力モードの切り替え */
  struct termios oldt, newt;
  int ch;
  int oldf;
  tcgetattr(STDIN_FILENO, &oldt);
  newt = oldt;
  newt.c_lflag &= ~(ICANON);
  tcsetattr(STDIN_FILENO, TCSANOW, &newt);
  oldf = fcntl(STDIN_FILENO, F_GETFL, 0);
  fcntl(STDIN_FILENO, F_SETFL, oldf | O_NONBLOCK);

  /* 出力デバイス */
  char *device = "default";
	char title[30];
  /* ソフトSRC有効無効設定 */
  unsigned int soft_resample = 1;
  /* ALSAのバッファ時間[msec] */
  const static unsigned int latency = 50000;
  /* PCM 情報 */
  WAVEFORMATEX wf = { WAVE_FORMAT_PCM, // PCM
  DEF_CHANNEL,
  DEF_FS,
  DEF_FS * DEF_CHANNEL * (DEF_BITPERSAMPLE/8),
  (DEF_BITPERSAMPLE/8) * DEF_CHANNEL,
  DEF_BITPERSAMPLE,
  0};
  /* 符号付き16bit */
  static snd_pcm_format_t format = SND_PCM_FORMAT_S16;

	int8_t flag = 0;
  int16_t *buffer = NULL;
  int dSize, reSize, ret, n, i, j;
	int start, fin, duration;
	int dataStartByte;
  short int cnum, fnum, id;
  FILE *fp = NULL;
	FILE *database = NULL;
  snd_pcm_t *hndl = NULL;

  char input, dirpath[] = "./";
  char **fname = NULL;
  DIR *dir;
  struct dirent *dp;

  fnum = 0;


  /* wavファイルの名前を取得 */


  /* ファイルに番号付け */
  dir = opendir(dirpath);
  if(dir == NULL){
   return 1;
  }

  /* ファイルの数を取得 */
  dp = readdir(dir);
  /* 拡張子の識別 */
  while(dp != NULL){
   for(i = 0; i < 256; i++){
     if(dp->d_name[i] == '\0'){
       break;
     }
   }
   if(i > 3 && dp->d_name[i-3] == 'w' && dp->d_name[i-2] == 'a' &&  dp->d_name[i-1] == 'v'){
     fnum ++;
   }
   dp = readdir(dir);
  }
  if(fnum == 0){
    printf("no wav file was found\n");
    goto End;
  }

  /* wavファイルの数だけchar型ポインタのためのメモリー領域を確保 */
  fname = (char **)malloc(sizeof(char *) * fnum);

  /* もう一度カレントディレクトリを読み込む  */
  closedir(dir);
  if(fnum == 0){
    goto End;
  }
  dir = opendir(dirpath);
  dp =  readdir(dir);

  for(i = 0; i < fnum; i++){
    /* 拡張子の識別 */
    while(dir != NULL){
    for(j = 0; j < 256; j++){
      if(dp->d_name[j] == '\0'){
        break;
      }
    }
    if(j > 3 && dp->d_name[j-3] == 'w' && dp->d_name[j-2] == 'a' &&  dp->d_name[j-1] == 'v'){
      break;
    }
    else{
      dp = readdir(dir);
    }
  }


   /* i番目のファイル名の文字数を取得 */
   for(cnum = 0; cnum < 256; cnum ++){
     if(dp->d_name[cnum] == '\0'){
       break;
     }
     cnum++;
   }

   /* i番目のファイル名を */
   fname[i] = (char *)malloc(sizeof(char) * cnum);
   for(j = 0; j < cnum; j++){
     fname[i][j] = dp->d_name[j];
   }
   dp = readdir(dir);
  }

  if(dir != NULL){
   closedir(dir);
  }

  /* WAVファイルの再生 */
	//フェードアウト用
	unsigned int amp;
	unsigned int amp_max;
	float amp_f;

  while(fnum > 0){
    /* 続行か中断かを判定 */
    while(input != EOF){
      if(input == 'q'){
        goto End;
      }
      input = getchar();
    }
    printf("if you want to close, please enter 'q'\n");
    sleep(2);
    input = getchar();
    while(input != EOF){
      if(input == 'q'){
        goto End;
      }
      input = getchar();
    }

    /* 再生するファイルを乱数で指定 */
    srand((unsigned)time(NULL));
    id = rand()%fnum;
    printf("\nid:%2d...%s\n",id ,fname[id]);

    /* WAVファイルを開く*/
    fp = fopen(fname[id], "rb");
    if (fp == NULL) {
      printf("Open error:%s\n", fname[id]);
      goto End;
    }

    /* WAVのヘッダーを解析する */
    dSize = readWavHeader(fp, &wf);
    if (dSize <= 0) {
      goto End;
    }

    /* PCMフォーマットの確認と情報出力を行う */
    printf("format : PCM, nChannels = %d, SamplePerSec = %d, BitsPerSample = %d\n",
    wf.nChannels, wf.nSamplesPerSec, wf.wBitsPerSample);

    /* バッファの用意 */
    buffer = malloc(BUF_SAMPLES * wf.nBlockAlign);
    if(buffer == NULL) {
      printf("malloc error\n");
      goto End;
    }

    /* 再生用PCMストリームを開く */
    ret = snd_pcm_open(&hndl, device, SND_PCM_STREAM_PLAYBACK, 0);
    if(ret != 0) {
      printf( "Unable to open PCM\n" );
      goto End;
    }

    /* フォーマット、バッファサイズ等各種パラメータを設定する */
    ret = snd_pcm_set_params( hndl, format, SND_PCM_ACCESS_RW_INTERLEAVED, wf.nChannels,
    wf.nSamplesPerSec, soft_resample, latency);
    if(ret != 0) {
      printf( "Unable to set format\n" );
      goto End;
    }

		/*
    srand((unsigned)time(NULL));
		duration = BUF_SAMPLES * wf.nBlockAlign * TIME;
		if(dSize > duration) {
			start = rand() % (dSize - duration);
			start = start - start % 2;
		} else {
			start = 0;
		}
		if(start + duration < dSize) {
			fin = start + duration;
		} else {
			fin = dSize;
		}
		*/
		printf("align = %d\n", wf.nBlockAlign);


		//データベースから、スタート位置を取得
		database = fopen("database", "r");
		//fseek(database, 0, SEEK_SET);
		while(flag == 0) {
			flag = 1;
			fgets(title, 30, database);
			for(i = 0; i < cnum - 1; i++) {
				printf("cnum = %d", cnum);
				printf("title -> %c, fname -> %c\n", title[i], fname[id][i]);
				if(title[i] != fname[id][i]) {
					flag = 0;
					break;
				}
			}
			if(title[0] == '\0') {
				printf("no database\n");
				goto End;
				break;
			}
		}

		//曲のスタート位置と終了位置を設定
		char str[10];
		fgets(str, 30, database);
		start = atoi(str);
		duration = BUF_SAMPLES * wf.nBlockAlign * TIME;
		if(start + duration < dSize) {
			fin = start + duration;
		} else {
			fin = dSize;
		}

		//デバッグ用スタート位置
		start = 0;

		fseek(fp, start, SEEK_CUR);

		//フェードアウト
		amp_max = (fin - start) / 10000;
		amp_max *= amp_max;


		int16_t data;
    for (n = start; n < fin; n += BUF_SAMPLES * wf.nBlockAlign) {

			//フェードアウト
			amp = (fin - n) / 10000;
			amp *= amp;
			amp_f = (float)(amp) / (float)(amp_max);
			/* PCMの読み込み */

			//デバッグ用の設定
			fread(buffer, wf.nBlockAlign, BUF_SAMPLES, fp);
			
			for(i = 0; i < BUF_SAMPLES; i++) {
	     	//fread(data, wf.nBlockAlign, 1, fp);
				//buffer[i] = data;
				//buffer[i] = data * amp_f;
				//printf("%d\n", data);
				buffer[i] *= 0;
				printf("%d\n", buffer[i]);
			}
		

			/* 中断判定 */
		 	input = getchar();
			if(input == 'q'){
				break;
			}

      /* PCMの書き込み */
      reSize = (n < BUF_SAMPLES) ? n : BUF_SAMPLES;
      ret = snd_pcm_writei(hndl, (const void*)buffer, reSize);
      /* バッファアンダーラン等が発生してストリームが停止した時は回復を試みる */
      if (ret < 0) {
        if(snd_pcm_recover(hndl, ret, 0 ) < 0 ) {
        printf( "Unable to recover Stream." );
        goto End;
        }
      }
    }

/*
		printf("文字を入力すると答えが流れます\n");
    while(input != EOF){
		duration, }
    ret = snd_pcm_set_params( hndl, format, SND_PCM_ACCESS_RW_INTERLEAVED, wf.nChannels,
    wf.nSamplesPerSec, soft_resample, latency);
    if(ret != 0) {
      printf( "Unable to set format\n" );
      goto End;
    }
			for (n = 0; n < dSize; n += BUF_SAMPLES * wf.nBlockAlign) {
				// PCMの読み込み

				fread(buffer, wf.nBlockAlign, BUF_SAMPLES, fp);

				// 中断判定 

				input = getchar();
				if(input == 'q'){
					break;
				}

				// PCMの書き込み

		printf("文字を入力すると答えが流れます\n");
				reSize = (n < BUF_SAMPLES) ? n : BUF_SAMPLES;
				ret = snd_pcm_writei(hndl, (const void*)buffer, reSize);
				// バッファアンダーラン等が発生してストリームが停止した時は回復を試みる

		printf("文字を入力すると答えが流れます\n");
				if (ret < 0) {
					if( snd_pcm_recover(hndl, ret, 0 ) < 0 ) {
					printf( "Unable to recover Stream." );
					goto End;
					}
				}
			}

    // データ出力が終わったため、たまっているPCMを出力する。
*/
    snd_pcm_drain(hndl);
		fclose(fp);
		sleep(15);
  }

  End:


  /* 入力モードを戻す　*/
  tcsetattr(STDIN_FILENO, TCSANOW, &oldt);
  fcntl(STDIN_FILENO, F_SETFL, oldf);


  /* ストリームを閉じる */
  if (hndl != NULL) {
  snd_pcm_close(hndl);
  }

  /* ファイルを閉じる */
  if (fp != NULL) {
  fclose(fp);
  }

  /* メモリの解放 */
  if (fname != NULL) {
  free(fname);
  fname = NULL;
  }
  if (buffer != NULL) {
  free(buffer);
  buffer = NULL;
  }

  if(dir != NULL){
  closedir(dir);
  }

	printf("\nsuccessfully fishished\n");
  return 0;
}
