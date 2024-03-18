#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include "../include/wave.h"

int main(int argc, char *argv[]) {
	SOUND_DATA sound_data = {0};
	WAVE_FORMAT wave_format = {0};

	FILE *fp_read;
	fp_read = fopen(argv[1], "rb");

	if(!fp_read) {
		printf("file open error\n");
		exit(0);
	}

	// read Chunks
	wave_readRiffChunk(fp_read, &wave_format.riffChunk);
	wave_readFmtChunk(fp_read, &wave_format.fmtChunk);
	wave_readDataChunk(fp_read, &wave_format, &sound_data);
	fclose(fp_read);

	FILE *fp_write;
	fp_write = fopen(argv[2], "wb");

	if(!fp_write) {
		printf("file open error : fp_write\n");
		exit(0);
	}

	wave_writeWavFile(fp_write, &wave_format, &sound_data);

	free(sound_data.s);
	fclose(fp_write);

	return 0;
}
