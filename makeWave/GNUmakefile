# Makefile for mkwave.c

makewave: ../include/wave.c makewave.c
	gcc -Wall -o makewave ../include/wave.c makewave.c -lm

copy: ../include/wave.c copy.c
	gcc -Wall ../include/wave.c copy.c -lm -o copy.out

print_waveinfo: ../include/wave.c ../include/file.c print_waveinfo.c
	gcc -Wall ../include/wave.c print_waveinfo.c -lm -o print_waveinfo.out

intro: ../include/wave.c ../include/file.c intro.c
	gcc -Wall ../include/wave.c ../include/file.c intro.c -lm

adjust_volume: ../include/wave.c ../include/file.c adjust_volume.c
	gcc -Wall ../include/wave.c ../include/file.c adjust_volume.c -lm -o adjust_volume.out

compressor: ../include/wave.c ../include/file.c compressor.c
	gcc -Wall ../include/wave.c ../include/file.c compressor.c -lm -o compressor.out

doublespeed: ../include/wave.c
	gcc -Wall ../include/wave.c doublespeed.c -lm
