#include <stdio.h>
#include <stdlib.h>
#include <math.h>
#include "../include/wave.h"


int main(int argv, char **argc)
{
    MONO_PCM pcmForWrite, pcmForRead;
    int i;
    float amp, frequency, wave;

    pcmForWrite.fs = 8000;
    pcmForWrite.bits = 16;
    pcmForWrite.length = 32000;
    pcmForWrite.s = calloc(pcmForWrite.length, sizeof(float));

    amp = 1.0;

    frequency = 440.0;

    for(i = 0; i < pcmForWrite.length / 2; i++) {
        wave = amp * sin((2.0 * M_PI * 261.626 * i) / pcmForWrite.fs);
        wave += amp * sin((2.0 * M_PI * 329.628 * i) / pcmForWrite.fs);
        wave += amp * sin((2.0 * M_PI * 391.995 * i) / pcmForWrite.fs);
				wave /= 3;
				pcmForWrite.s[i] = (float)wave * ((pcmForWrite.length / 2) - i) / (pcmForWrite.length / 2);
    }
		//waveファイル書き込み
    monoWaveWrite(&pcmForWrite, "a.wav");
    free(pcmForWrite.s);

    monoWaveRead(&pcmForRead, "a.wav");
    for(i = 0; i < pcmForRead.length; i++)	{
//        printf("%d %f\n", i, pcmForRead.s[i]);
    }
		

free(pcmForRead.s);

    return 0;
}
