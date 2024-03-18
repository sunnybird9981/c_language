#include <string.h>
#include <stdlib.h>
#include <string.h>
#include <stdint.h>
#include <math.h>
#include "wave.h"


// define ChunkID
const char ID_RIFF[5] = "RIFF\0";
const char ID_WAVE[5] = "WAVE\0";
const char ID_FMT[5] = "fmt \0";
const char ID_DATA[5] = "data\0";


// moves file pointer to the start of the chunk.
void wave_setReadingPoint(FILE *fp, const char *targeted_chunk_ID) {
	char chunk_ID[5] = {};
	uint32_t chunk_size = 0;

	while(fread(chunk_ID, 1, 4, fp)) {
		// if the point is not the start of the target chunk, moves reading point to the next chunk.
		if(strncmp(chunk_ID, targeted_chunk_ID, 4) != 0) {
			fread(&chunk_size, 4, 1, fp);
			fseek(fp, chunk_size, SEEK_CUR);
		// if the point is the start of the target chunk, finish this function.
		} else {
			break;
		}
	}
}


// read RIFF chunk
void wave_readRiffChunk(FILE *fp, RIFF_CHUNK *riff_chunk) {
	// read RIFF chunk
	wave_setReadingPoint(fp, ID_RIFF);
	strcpy(riff_chunk->chunkID, ID_RIFF);
	fread(&riff_chunk->chunkSize, 4, 1, fp);
	fread(riff_chunk->chunkFormType, 1, 4, fp);
}


// read fmt chunk
void wave_readFmtChunk(FILE *fp, FMT_CHUNK *fmt_chunk) {
	// read fmt chunk
	fseek(fp, 12, SEEK_SET);
	wave_setReadingPoint(fp, ID_FMT);
	strcpy(fmt_chunk->chunkID, ID_FMT);
	fread(&fmt_chunk->chunkSize, 4, 1, fp);
	fread(&fmt_chunk->audioFormat, 2, 1, fp);
	fread(&fmt_chunk->numChannels, 2, 1, fp);
	fread(&fmt_chunk->sampleRate, 4, 1, fp);
	fread(&fmt_chunk->byteRate, 4, 1, fp);
	fread(&fmt_chunk->blockAlign, 2, 1, fp);
	fread(&fmt_chunk->bitsPerSample, 2, 1, fp);
}


void wave_getDataRange(SOUND_DATA *sound_data, short bytes_per_sample) {
	sound_data->max = pow(2, bytes_per_sample);
	sound_data->min = 0;
	sound_data->ground = sound_data->max / 2;
}


void wave_readSoundData1Byte(FILE *fp, WAVE_FORMAT *wave_format, SOUND_DATA *sound_data) {
	uint8_t data;
	sound_data->num_samples = wave_format->dataChunk.chunkSize / 1;
	sound_data->s = calloc(sound_data->num_samples, sizeof(double));

	for(int i = 0; i < sound_data->num_samples; i++) {
		fread(&data, 1, 1, fp);
		sound_data->s[i] = (double)data / (double)sound_data->ground;
	}
}


void wave_readSoundData2Byte(FILE *fp, WAVE_FORMAT *wave_format, SOUND_DATA *sound_data) {
	int16_t data;
	sound_data->num_samples = wave_format->dataChunk.chunkSize / 2;
	sound_data->s = calloc(sound_data->num_samples, sizeof(double));

	for(int i = 0; i < sound_data->num_samples; i++) {
		fread(&data, 2, 1, fp);
		sound_data->s[i] = (double)data / (double)sound_data->ground;
	}
}


void wave_readSoundData4Byte(FILE *fp, WAVE_FORMAT *wave_format, SOUND_DATA *sound_data) {
	int32_t data;
	sound_data->num_samples = wave_format->dataChunk.chunkSize / 4;
	sound_data->s = calloc(sound_data->num_samples, sizeof(double));

	for(int i = 0; i < sound_data->num_samples; i++) {
		fread(&data, 4, 1, fp);
		sound_data->s[i] = (double)data / (double)sound_data->ground;
	}
}


void wave_readSoundData(FILE *fp, WAVE_FORMAT *wave_format, SOUND_DATA *sound_data) {
	short bytes_per_sample = (wave_format->fmtChunk.bitsPerSample / 8);

	wave_getDataRange(sound_data, wave_format->fmtChunk.bitsPerSample);
	switch(bytes_per_sample) {
		case 1:
			wave_readSoundData1Byte(fp, wave_format, sound_data);
			break;
		case 2:
			wave_readSoundData2Byte(fp, wave_format, sound_data);
			break;
		case 4:
			wave_readSoundData4Byte(fp, wave_format, sound_data);
			break;
		default:
			printf("error : wave_readSoundData\nunsupported bytes_per_sample\n");
			exit(0);
	}
}


// read data chunk
void wave_readDataChunk(FILE *fp, WAVE_FORMAT *wave_format, SOUND_DATA *sound_data) {
	// read data chunk
	fseek(fp, 12, SEEK_SET);
	wave_setReadingPoint(fp, ID_DATA);
	strcpy(wave_format->dataChunk.chunkID, ID_DATA);
	fread(&wave_format->dataChunk.chunkSize, 4, 1, fp);

	// read sound data;
	wave_readSoundData(fp, wave_format, sound_data);
}


void wave_applyComp(COMPRESSOR *comp, double *s, int start_pos, int end_pos, double cur_vol) {
	double amp;

	amp = (cur_vol - comp->threshold) * comp->ratio + comp->threshold;
	printf("amp : %f\n", amp);
	for(int i = start_pos; i < end_pos + 1; i++) {
		s[i] = s[i] / cur_vol * amp;
	}
}


double wave_calcRms(double *s, int start_pos, int end_pos) {
	double sum = 0;
	double rms;

	for(int i = start_pos; i < end_pos + 1; i++) {
		sum += fabs(s[i]);
	}
	rms = sum / (end_pos - start_pos + 1);

	return rms;
}


double wave_getMinRms(SOUND_DATA *sound_data, int window_size) {
	int window_start_pos = 0;
	int window_end_pos = 0;
	double rms = 0;
	double min_rms = 1.0;

	for(window_start_pos = 0; window_start_pos < sound_data->num_samples; window_start_pos += window_size) {
		window_end_pos = (window_start_pos + window_size < sound_data->num_samples - 1) ? window_end_pos + window_size : sound_data->num_samples;
		rms = wave_calcRms(sound_data->s, window_start_pos, window_end_pos);
		if(rms < min_rms) {
			min_rms = rms;
		}
	}

	return min_rms;
}


double wave_getMaxRms(SOUND_DATA *sound_data, int window_size) {
	int window_start_pos = 0;
	int window_end_pos = 0;
	double rms = 0;
	double max_rms = 0;

	for(window_start_pos = 0; window_start_pos < sound_data->num_samples; window_start_pos += window_size) {
		window_end_pos = (window_start_pos + window_size < sound_data->num_samples - 1) ? window_end_pos + window_size : sound_data->num_samples - 1;
		rms = wave_calcRms(sound_data->s, window_start_pos, window_end_pos);
		if(rms > max_rms) {
			max_rms = rms;
		}
	}

	return max_rms;
}


void wave_peakComp(WAVE_FORMAT *wave_format, COMPRESSOR *comp, double *s) {
}


void wave_rmsComp(WAVE_FORMAT *wave_format, COMPRESSOR *comp, double *s) {
	int window_start_pos = 0;
	int window_end_pos = 0;
	double rms = 0;

	for(window_start_pos = 0; window_start_pos < wave_format->dataChunk.chunkSize; window_start_pos += comp->windowSize) {
		window_end_pos = (window_start_pos + comp->windowSize < wave_format->dataChunk.chunkSize - 1) ? window_end_pos + comp->windowSize : wave_format->dataChunk.chunkSize - 1;
		rms = wave_calcRms(s, window_start_pos, window_end_pos);

		if(rms > comp->threshold) {
			wave_applyComp(comp, s, window_start_pos, window_end_pos, rms);
		}
	}
}


double wave_getMaxAbsValue(SOUND_DATA *sound_data) {
	double abs_value = 0;
	double max_abs_value = 0;

	int i;
	for(i = 0; i < sound_data->num_samples; i++) {
		abs_value = fabs(sound_data->s[i]);
		if(abs_value > max_abs_value) {
			max_abs_value = abs_value;
		}
	}

	return max_abs_value;
}


void wave_multSoundData(SOUND_DATA *sound_data, double mult) {
	int i;
	for(i = 0; i < sound_data->num_samples; i++) {
		sound_data->s[i] *= mult;
	}
}


void wave_adjustSoundData(SOUND_DATA *sound_data, int window_size, double target_level) {
	double max_abs_value = 0;
	double max_rms = 0;
	double amp = 0;

	max_abs_value = wave_getMaxAbsValue(sound_data);
	max_rms = wave_getMaxRms(sound_data, window_size);

	amp = 1.0 / max_rms * target_level;
	if(max_abs_value * amp < 1.0) {
		wave_multSoundData(sound_data, amp);
	} else {
		printf("error : wave_adjustSoundData\namp is too large\n");
		exit(0);
	}
}


void wave_writeSoundData1Byte(FILE *fp, WAVE_FORMAT *wave_format, SOUND_DATA *sound_data) {
	uint8_t datai;
	double datalf;

	for(int i = 0; i < sound_data->num_samples; i++) {
		datalf = ((sound_data->s[i] + 1.0) / 2) * (sound_data->max - 1);
		if(datalf > sound_data->max - 1) {
			printf("error : wave_writeSoundData1Byte\nnegative over flow\n");
			exit(1);
		}
		else if(datalf < sound_data->min) {
			printf("error : wave_writeSoundData1Byte\npositive over flow\n");
			exit(1);
		} else {
		datai = (uint8_t)(datalf + 0.5) - sound_data->ground;
		fwrite(&datai, 1, 1, fp);
		}
	}
}


void wave_writeSoundData2Byte(FILE *fp, WAVE_FORMAT *wave_format, SOUND_DATA *sound_data) {
	int16_t datai;
	double datalf;

	for(int i = 0; i < sound_data->num_samples; i++) {
		datalf = ((sound_data->s[i] + 1.0) / 2) * (sound_data->max - 1);
		if(datalf > sound_data->max - 1) {
			printf("error : wave_writeSoundData2Byte\nnegative over flow\n");
			exit(1);
		}
		else if(datalf < sound_data->min) {
			printf("error : wave_writeSoundData1Byte\npositive over flow\n");
			exit(1);
		} else {
		datai = (int16_t)(datalf + 0.5) - sound_data->ground;
		fwrite(&datai, 2, 1, fp);
		}
	}
}


void wave_writeSoundData4Byte(FILE *fp, WAVE_FORMAT *wave_format, SOUND_DATA *sound_data) {
	int32_t datai;
	double datalf;

	for(int i = 0; i < sound_data->num_samples; i++) {
		datalf = ((sound_data->s[i] + 1.0) / 2) * (sound_data->max - 1);
		if(datalf > sound_data->max - 1) {
			printf("error : wave_writeSoundData4Byte\nnegative over flow\n");
			exit(1);
		}
		else if(datalf < sound_data->min) {
			printf("error : wave_writeSoundData1Byte\npositive over flow\n");
			exit(1);
		} else {
		datai = (int32_t)(datalf + 0.5) - sound_data->ground;
		fwrite(&datai, 4, 1, fp);
		}
	}
}


void wave_writeSoundData(FILE *fp, WAVE_FORMAT *wave_format, SOUND_DATA *sound_data) {
	short bytes_per_sample = (wave_format->fmtChunk.bitsPerSample / 8);

	switch(bytes_per_sample) {
		case 1:
			wave_writeSoundData1Byte(fp, wave_format, sound_data);
			break;
		case 2:
			wave_writeSoundData2Byte(fp, wave_format, sound_data);
			break;
		case 4:
			wave_writeSoundData4Byte(fp, wave_format, sound_data);
			break;
		default:
			printf("error : wave_writeSoundData\nunsupported bytes_per_sample\n");
			exit(0);
	}
}


void wave_writeWavFile(FILE *fp, WAVE_FORMAT *wave_format, SOUND_DATA *sound_data) {
	// RIFF chunk
	fwrite(wave_format->riffChunk.chunkID, 1, 4, fp);
	fwrite(&wave_format->riffChunk.chunkSize, 4, 1, fp);
	fwrite(wave_format->riffChunk.chunkFormType, 1, 4, fp);

	// fmt chunk
	wave_format->fmtChunk.chunkSize = 16;
	fwrite(wave_format->fmtChunk.chunkID, 1, 4, fp);
	fwrite(&wave_format->fmtChunk.chunkSize, 4, 1, fp);
	fwrite(&wave_format->fmtChunk.audioFormat, 2, 1, fp);
	fwrite(&wave_format->fmtChunk.numChannels, 2, 1, fp);
	fwrite(&wave_format->fmtChunk.sampleRate, 4, 1, fp);
	fwrite(&wave_format->fmtChunk.byteRate, 4, 1, fp);
	fwrite(&wave_format->fmtChunk.blockAlign, 2, 1, fp);
	fwrite(&wave_format->fmtChunk.bitsPerSample, 2, 1, fp);

	//data chunk
	fwrite(wave_format->dataChunk.chunkID, 1, 4, fp);
	fwrite(&wave_format->dataChunk.chunkSize, 4, 1, fp);
	wave_writeSoundData(fp, wave_format, sound_data);
}


//	print header information
void wave_printWaveFormatInfo(WAVE_FORMAT *wave_format) {
	printf("/*   RiffChunk   */");
	printf("\nriffChunk.chunkID : %s", wave_format->riffChunk.chunkID);
	printf("\nchunkSize\n : %d", wave_format->riffChunk.chunkSize);
	printf("\nchunkFormType : %s", wave_format->riffChunk.chunkFormType);

	printf("\n\n/*  fmtChunk	 */");
	printf("\nchunk_ID : %s", wave_format->fmtChunk.chunkID);
	printf("\nchunkSize : %d", wave_format->fmtChunk.chunkSize);
	printf("\naudioFormat : %d", wave_format->fmtChunk.audioFormat);
	printf("\nnumChannels : %d", wave_format->fmtChunk.numChannels);
	printf("\nsampleRate : %d", wave_format->fmtChunk.sampleRate);
	printf("\nbyteRate : %d", wave_format->fmtChunk.byteRate);
	printf("\nblockAlign : %d", wave_format->fmtChunk.blockAlign);
	printf("\nbitsPerSample : %d", wave_format->fmtChunk.bitsPerSample);

	printf("\n\n/*   dataChunk   */");
	printf("\nchunk_ID : %s", wave_format->dataChunk.chunkID);
	printf("\nchunkSize : %d\n", wave_format->dataChunk.chunkSize);
}
