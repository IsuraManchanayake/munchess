#pragma once

#include <time.h>
#include <stdio.h>

unsigned rand_lim(unsigned limit);

int rand_range(int a, int b);

time_t time_now(void);

char* read_file(const char* path);

void error_exit(int status);

#define READ_LINE_CHUNK_SIZE 2048
char* read_line(FILE* fp);

#if __STDC_VERSION__ < 202311L
char* strndup(const char* str, size_t n);
#endif
