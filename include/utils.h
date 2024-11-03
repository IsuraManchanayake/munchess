#pragma once

#include <time.h>

#ifndef min
#define min(a, b) ((a) < (b) ? (a) : (b));
#endif
#ifndef max
#define max(a, b) ((a) < (b) ? (b) : (a));
#endif

#if (defined(__GNUC__) || defined(__clang__)) && (__STDC_VERSION__ >= 201710L)
#define UCHAR_ENUM enum: uint8_t
#define ENUM_BITS(Type, name, bits) Type name : bits
#else
#define UCHAR_ENUM enum
#define ENUM_BITS(Type, name, bits) uint8_t name : bits
#endif

#define debugzu(x) (printf("%s = %zu\n", #x, (x)))
#define debugi(x) (printf("%s = %d\n", #x, (x)))
#define debugc(x) (printf("%s = %c\n", #x, (x)))
#define debugs(x) (printf("%s = %s\n", #x, (x)))

unsigned rand_lim(unsigned limit);

int rand_range(int a, int b);

time_t time_now(void);

char* read_file(const char* path);

void error_exit(int status);

#if __STDC_VERSION__ < 202311L
char* strndup(const char* str, size_t n);
#endif
