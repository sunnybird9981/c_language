#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include "../include/wave.h"

/* 
	this program apply compressor effect to wav files
	usage : compressor.out (input file) (output file) (threshold) (ratio) (attack time) (release time) (time_frame)
	threshold : 0 ~ 1
	ratio : 0 ~ 1
	attack : (sec)
	release : (sec)
	time_frame : how long a window is (sec)
*/

int main(int argc, char *argv[]) {
	FILE *fp_read, *fp_write;

	/* initialization */
	WAVE_FORMAT wave_format = {0};
	SOUND_DATA sound_data = {0};
	COMPRESSOR comp = {0};
	comp.threshold = atof(argv[3]);
	comp.ratio = atof(argv[4]);
	comp.attack = atof(argv[5]);
	comp.release = atof(argv[6]);
	comp.timeFrame = atof(argv[7]);

	fp_read = fopen(argv[1], "rb");
	if(!fp_read) {
		printf("file open error : %s\n", argv[1]);
		exit(0);
	}

	/* read Chunks */
	wave_readRiffChunk(fp_read, &wave_format.riffChunk);
	wave_readFmtChunk(fp_read, &wave_format.fmtChunk);
	wave_readDataChunk(fp_read, &wave_format, &sound_data);
	fclose(fp_read);

	fp_write = fopen(argv[2], "wb");
	if(!fp_write) {
		printf("file open error : %s\n", argv[2]);
		exit(0);
	}

	/* write data */
	wave_rmsComp(&wave_format, &comp, &sound_data); // apply compressor effect
	wave_writeWavFile(fp_write, &wave_format, &sound_data);

	free(sound_data.s);
	fclose(fp_write);

	return 0;
}
