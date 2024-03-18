#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include "../include/wave.h"

/*
	usage : program (input file) (output file) (window_sec) (target_level)
*/

int main(int argc, char *argv[]) {
	int window_size;
	SOUND_DATA sound_data = {0};
	WAVE_FORMAT wave_format = {0};

	float window_sec = atof(argv[3]);
	double target_level = atof(argv[4]);

	FILE *fp_read;
	fp_read = fopen(argv[1], "rb");
	if(!fp_read) {
		printf("file open error : fp_read\n");
		exit(0);
	}

	// read data
	wave_readRiffChunk(fp_read, &wave_format.riffChunk);
	wave_readFmtChunk(fp_read, &wave_format.fmtChunk);
	wave_readDataChunk(fp_read, &wave_format, &sound_data);
	fclose(fp_read);

	// write data
	FILE *fp_write;
	fp_write = fopen(argv[2], "wb");
	//printf("%s\n", argv[2]);
	if(!fp_write) {
		printf("file open error : fp_write\n");
		exit(0);
	}

	window_size = wave_format.fmtChunk.sampleRate * wave_format.fmtChunk.numChannels * window_sec;
	wave_adjustSoundData(&sound_data, window_size, target_level);
	wave_writeWavFile(fp_write, &wave_format, &sound_data);

	free(sound_data.s);
	fclose(fp_write);

	return 0;
}
