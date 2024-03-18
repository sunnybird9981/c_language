#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include "../include/wave.h"

int main(int argc, char *argv[]) {
	SOUND_DATA sound_data = {0};
	WAVE_FORMAT wave_format = {0};

	FILE *fp;
	fp = fopen(argv[1], "rb");

	if(!fp) {
		printf("file open error\n");
		exit(0);
	}

	// read Chunks
	wave_readRiffChunk(fp, &wave_format.riffChunk);
	wave_readFmtChunk(fp, &wave_format.fmtChunk);
	wave_readDataChunk(fp, &wave_format, &sound_data);

	wave_printWaveFormatInfo(&wave_format);

	free(sound_data.s);
	fclose(fp);

	return 0;
}
