#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include "../include/wave.h"
#include "../include/file.h"

/* 
this program apply compressor effect to wav files
*/

int main(int argc, char *argv[]) {
	FILE *fp;
	float time_frame = 0.010; //ms

	WAVE_FORMAT wave_format = {0};
	PCM pcm;
	COMPRESSOR comp = {0.0, 0.0, 0.0, 0.0};
	comp.threshold = 0.110;
	comp.ratio = 0.500;

	fp = fopen(argv[1], "rb");
	if(!fp) {
		printf("file open error\n");
		exit(0);
	}

	// read Chunks
	wave_readRiffChunk(fp, &wave_format);
	wave_readFmtChunk(fp, &wave_format);
	wave_readDataChunk(fp, &wave_format, &pcm);

	comp.windowSize = (int) (wave_format->fmtChunk.sampleRate * wave_format-fmtChunk.numChannels * time_frame);

	// apply compressor effect
	wave_rmsComp(&pcm, time_frame, &comp);
	// write wav file
	wave_writeWavFile(&wave_format, &pcm, "compressor.wav");

	free(pcm.s);
	fclose(fp);

	return 0;
}
