#!/bin/sh
COMPILED_PROGRAMS_DIR=$HOME/bin/compiledPrograms
ADJUST_VOLUME_PROGRAM=$COMPILED_PROGRAMS_DIR/adjust_volume

for f; do
  #compiledPrograms/adjust_volume "$f" -o ${f.wav};
	TMP_ADJUSTED_WAV_FILE=${f%.wav}_adjusted.wav
	ADJUSTED_M4A_FILE=${f%.wav}.m4a
  $ADJUST_VOLUME_PROGRAM $f $TMP_ADJUSTED_WAV_FILE 0.01 0.15;
	ffmpeg -i $TMP_ADJUSTED_WAV_FILE -b:a 320k $ADJUSTED_M4A_FILE
	rm $TMP_ADJUSTED_WAV_FILE
done
