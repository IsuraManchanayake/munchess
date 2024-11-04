#pragma once

#include <time.h>

unsigned rand_lim(unsigned limit);

int rand_range(int a, int b);

time_t time_now(void);

char* read_file(const char* path);

void error_exit(int status);

#if __STDC_VERSION__ < 202311L
char* strndup(const char* str, size_t n);
#endif
