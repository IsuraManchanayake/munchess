#pragma once

#include <time.h>
#include <stdio.h>
#include <stdint.h>

#ifdef _WIN32
#include <windows.h>
#define getpid GetCurrentProcessId
#else
#include <unistd.h>
#include <sys/types.h>
#endif

unsigned rand_lim(unsigned limit);

int rand_range(int a, int b);

time_t time_now(void);

char *read_file(const char *path);

void error_exit(int status);

uint8_t next_piece_idx(uint64_t bb);

#define READ_LINE_CHUNK_SIZE 2048
char *read_line(FILE *fp);

#if __STDC_VERSION__ < 202311L
char *strndup(const char *str, size_t n);
#endif
