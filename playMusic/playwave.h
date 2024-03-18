#ifndef PLAYWAVE_H
#define PLAYWAVE_H

/* PCMデフォルト設定 */
#ifdef PCM_DEFAULT

#define DEF_CHANNEL 2
#define DEF_FS 48000
#define DEF_BITPERSAMPLE 16
#define WAVE_FORMAT_PCM 1
#define BUF_SAMPLES 1024

#endif

/* ChunkID 定義 */
const char ID_RIFF[4] = "RIFF";
const char ID_WAVE[4] = "WAVE";
const char ID_FMT[4] = "fmt ";
const char ID_DATA[4] = "data";

/* PCM情報格納用構造体 */
typedef struct {
 uint16_t wFormatTag; // format type
 uint16_t nChannels; // number of channels (1:mono, 2:stereo)
 uint32_t nSamplesPerSec; // sample rate
 uint32_t nAvgBytesPerSec; // for buffer estimation
 uint16_t nBlockAlign; // block size of data
 uint16_t wBitsPerSample; // number of bits per sample of mono data
 uint16_t cbSize; // extra information
} WAVEFORMATEX;

/* CHUNK */
typedef struct {
 char ID[4]; // Chunk ID
 uint32_t Size; // Chunk size;
} CHUNK;

/* WAVE ファイルのヘッダ部分を解析、必要な情報を構造体に入れる
 * ・fp は DATA の先頭位置にポイントされる
 * ・戻り値は、成功時：データChunk のサイズ、失敗時：-1
 */
static int readWavHeader(FILE *fp, WAVEFORMATEX *wf)
{
 char FormatTag[4];
 CHUNK Chunk;
 int ret = -1;
 int reSize;

 /* Read RIFF Chunk*/
 if((fread(&Chunk, sizeof(Chunk), 1, fp) != 1) ||
 (strncmp(Chunk.ID, ID_RIFF, 4) != 0)) {
	 printf("file is not RIFF Format \n");
	 goto RET;
 }

 /* Read Wave */
 if((fread(FormatTag, 1, 4, fp) != 4) ||
 (strncmp(FormatTag, ID_WAVE, 4) != 0)) {
	 printf("file is not Wave file\n");
	 goto RET;
 }

 /* Read Sub Chunk (Expect FMT, DATA) */
 while(fread(&Chunk, sizeof(Chunk), 1, fp) == 1) {
	 if(strncmp(Chunk.ID, ID_FMT, 4) == 0) {
	 /* 小さい方に合せる(cbSize をケアするため) */
	 reSize = (sizeof(WAVEFORMATEX) < Chunk.Size) ? sizeof(WAVEFORMATEX) : Chunk.Size;
	 fread(wf, reSize, 1, fp);
		 if(wf->wFormatTag != WAVE_FORMAT_PCM) {
			 printf("Input file is not PCM\n");
			 goto RET;
		 }
	 } else if(strncmp(Chunk.ID, ID_DATA, 4) == 0) {
		 /* DATA Chunk を見つけたらそのサイズを返す */
		 ret = Chunk.Size;
		 break;
	 }
	 else {
		 /* 知らない Chunk は読み飛ばす */
		 fseek(fp, Chunk.Size, SEEK_CUR);
		 continue;
	 }
 };

RET:
 return ret;
}

int countData(FILE *fp) {
}
#endif
