#ifndef FILE_H
#define FILE_H

#include <stdio.h>
#include <stdlib.h>
#include <stdint.h>
#include <dirent.h>
#include <time.h>

typedef struct 
{
	//ファイル名
	char *name;
	//ファイルの文字数
	uint8_t length;
}FILE_NAME;


//引数にディレクトリのパスを渡すと、中のファイル,ディレクトリ名を出力する
void flist(char *);

unsigned int cnt_files_num(char *);

// パス先のディレクトリ内からランダムにファイルを選び、ファイル名を返す
char *get_rand_file(char *);

//文字列の文字数をカウントする
int8_t cnt_strlen(char *);

#endif
