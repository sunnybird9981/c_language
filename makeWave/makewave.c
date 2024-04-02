#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <math.h>
#include "../include/wave.h"

/*
write monaural sound file
usage : ./makewave.out
*/

int main(int argc, char *argv[]) {
	int freq;
	int sample_rate;
	short num_channels;
	float time;
	short bits_per_sample;
	SOUND_DATA sound_data = {0};

	/* initialization */
	freq = 440;
	sample_rate = 44100;
	num_channels = 1;
	time = 4.0;
	bits_per_sample = 16;
	
	/* calculate sound data */
	double rad = 0;
	double decay = 0;
	sound_data.numSamples = sample_rate * time * num_channels;
	sound_data.s = calloc(sound_data.numSamples, sizeof(double));
	for(int i = 0; i < sound_data.numSamples; i ++) {
		rad = 2.0 * M_PI * ((double)(i * freq) / (double)(sample_rate * num_channels));
		decay = 1.0 - (double)i / (double)sound_data.numSamples;
		sound_data.s[i] = sin(rad);
		//sound_data.s[i] = sin(rad) * decay;
		printf("%lf\n", sound_data.s[i]);
	}

	/* write sound data */

	FILE *fp_write;
	fp_write = fopen("makewave.wav", "wb");

	if(!fp_write) {
		printf("file open error : fp_write\n");
		exit(0);
	}

	wave_writeMonoWavFile(fp_write, sample_rate, bits_per_sample, &sound_data);

	free(sound_data.s);
	fclose(fp_write);

	return 0;
}
