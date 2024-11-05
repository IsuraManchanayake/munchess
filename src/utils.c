#include <stdio.h>
#include <stdlib.h>
#include <assert.h>

#include "utils.h"
#include "defs.h"

#ifdef _WIN32
#define WIN32_LEAN_AND_MEAN
#include <windows.h>
#include <stdint.h>
#else
#include <sys/time.h>
#endif

unsigned rand_lim(unsigned limit) {
    unsigned divisor = RAND_MAX/(limit+1);
    unsigned retval;

    do { 
        retval = rand() / divisor;
    } while (retval > limit);

    return retval;
}

int rand_range(int a, int b) {
    assert(b >= a);
    return a + rand_lim(b - a);
}

time_t time_now(void) {
#ifdef _WIN32
    static uint64_t is_init = 0;
    static LARGE_INTEGER win_frequency;
    if (0 == is_init) {
        QueryPerformanceFrequency(&win_frequency);
        is_init = 1;
    }
    LARGE_INTEGER now;
    QueryPerformanceCounter(&now);
    return (uint64_t)((1000000 * now.QuadPart) / win_frequency.QuadPart);
#else
    struct timeval tv;
    time_t full_time;
    gettimeofday(&tv, NULL);
    full_time = tv.tv_sec * 1000000 + tv.tv_usec;
    return full_time;
#endif
}

char* read_file(const char* path) {
    FILE* fp = fopen(path, "rb");
    if (!fp) {
        return NULL;
    }
    fseek(fp, 0, SEEK_END);
    long len = ftell(fp);
    rewind(fp);
    char* buf = malloc(len + 1);
    if (fread(buf, len, 1, fp) != 1) {
        fclose(fp);
        free(buf);
        return NULL;
    }
    buf[len] = 0;
    fclose(fp);
    return buf;
}

void error_exit(int status) {
    exit(status);
}

#if __STDC_VERSION__ < 202311L
#include <string.h>

char* strndup(const char* str, size_t n) {
    size_t l = strlen(str);
    l = min(l, n);
    char* new = malloc(l + 1);
    if (new == NULL) {
        return NULL;
    }
    new[l] = '\0';
    return (char*)memcpy(new, str, l);
}
#endif
