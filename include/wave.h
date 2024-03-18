#ifndef WAVE_H
#define WAVE_H

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <stdint.h>

#define GROUND 32768.0
#define MAX 65536.0
#define MIN 0.0

extern const char ID_RIFF[5];
extern const char ID_WAVE[5];
extern const char ID_FMT[5];
extern const char ID_DATA[5];

// range of sound data value
typedef struct {
	double *s;
	int num_samples;
	unsigned long ground;
	unsigned long max;
	short min;
} SOUND_DATA;

// RIFF chunk
typedef struct {
	char chunkID[4];
	int chunkSize;
	char chunkFormType[4];
} RIFF_CHUNK;

// fmt chunk
typedef struct {
	char chunkID[4];
	int chunkSize;
	short audioFormat;
	short numChannels;
	int sampleRate;
	int byteRate; // sampleRate * numChannels * bitsPerSample / 8
	short blockAlign; // numChannels * bitsPerSample / 8 : the number of bytes for one sample including all channels.
	short bitsPerSample;
} FMT_CHUNK;

/// data chunk
typedef struct {
	char chunkID[4];
	int chunkSize; // (number of samples) * numChannels * bitsPerSample / 8 : the number of bytes in the data
	short data;
} DATA_CHUNK;

typedef struct {
	RIFF_CHUNK riffChunk;
	FMT_CHUNK fmtChunk;
	DATA_CHUNK dataChunk;
} WAVE_FORMAT;

typedef struct {
	float threshold;
	float ratio;
	float attack;
	float release;
	int windowSize;
} COMPRESSOR;

void wave_setReadingPoint(FILE *, const char *);
void wave_readRiffChunk(FILE *, RIFF_CHUNK *);
void wave_readFmtChunk(FILE *, FMT_CHUNK *);
void wave_getDataRange(SOUND_DATA *, short);
void wave_readSoundData1Byte(FILE *, WAVE_FORMAT *, SOUND_DATA *);
void wave_readSoundData2Byte(FILE *, WAVE_FORMAT *, SOUND_DATA *);
void wave_readSoundData4Byte(FILE *, WAVE_FORMAT *, SOUND_DATA *);
void wave_readSoundData(FILE *, WAVE_FORMAT *, SOUND_DATA *);
void wave_readDataChunk(FILE *, WAVE_FORMAT *, SOUND_DATA *);
void wave_applyComp(COMPRESSOR *, double *, int, int, double);
double wave_calcRms(double *, int, int);
double wave_getMinRms(SOUND_DATA *, int);
double wave_getMaxRms(SOUND_DATA *, int);
void wave_peakComp(WAVE_FORMAT *, COMPRESSOR *, double *);
void wave_rmsComp(WAVE_FORMAT *, COMPRESSOR *, double *);
double wave_getMaxAbsValue(SOUND_DATA *);
void wave_multSoundData(SOUND_DATA *, double);
void wave_adjustSoundData(SOUND_DATA *, int, double);
void wave_writeSoundData1Byte(FILE *, WAVE_FORMAT *, SOUND_DATA *);
void wave_writeSoundData2Byte(FILE *, WAVE_FORMAT *, SOUND_DATA *);
void wave_writeSoundData4Byte(FILE *, WAVE_FORMAT *, SOUND_DATA *);
void wave_writeSoundData(FILE *, WAVE_FORMAT *, SOUND_DATA *);
void wave_writeWavFile(FILE *, WAVE_FORMAT *, SOUND_DATA *);
void wave_printWaveFormatInfo(WAVE_FORMAT *);

#endif
