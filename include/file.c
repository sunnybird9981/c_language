#include <stdio.h>
#include <stdlib.h>
#include <stdint.h>
#include <dirent.h>
#include <time.h>

//引数にディレクトリのパスを渡すと、ディレクトリ内のファイル,ディレクトリ名を出力する
void flist(char *path) {
  DIR *dir;
  struct dirent *dp;

  dir = opendir(path);
  if(dir == NULL) {
    printf("no file or directory is found\n");
  }

  dp = readdir(dir);
  while(dp != NULL) {
    printf("%s\n",dp->d_name);
    dp = readdir(dir);
  }

  if(dir != NULL) {
    closedir(dir);
  }
}


unsigned int cnt_files_num(char *path) {
	unsigned int cnt = 0;
	DIR *dir;
	struct dirent *dp;

	dir = opendir(path);
	if(dir == NULL) {
		printf("no file or directory is found\n");
		closedir(dir);
		return 0;
	}

	dp = readdir(dir);
	while(dp != NULL) {
		dp = readdir(dir);
		cnt++;
	}

	closedir(dir);
	return cnt;
}

// パス先のディレクトリからランダムにファイルを選び、ファイル名を返す
char *get_rand_file(char *path) {
	int rand_num;
	int file_num;
	DIR *dir;
	struct dirent *dp;

	dir = opendir(path);
	file_num = cnt_files_num(path);

	srand(time(NULL));
	rand_num = rand() % file_num + 1;

	int i;
	for(i = 0; i < rand_num; i++) {
		dp = readdir(dir);
	}
	closedir(dir);

	return dp->d_name;
}

//文字列の文字数を返す関数
int cnt_strlen(char *str) {
	int len;

	for(len = 0; len < 65535; len++) {
		if(str[len] == '\0')
			return len;
	}

	return -1;
}
