#pragma once

#include <stdio.h>
#include <stdlib.h>
#include <sys/time.h>

#define debugzu(x) (printf("%s = %zu\n", #x, (x)))
#define debugi(x) (printf("%s = %d\n", #x, (x)))
#define debugc(x) (printf("%s = %c\n", #x, (x)))
#define debugs(x) (printf("%s = %s\n", #x, (x)))

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
  struct timeval tv;
  time_t full_time;
  gettimeofday(&tv, NULL);
  full_time = tv.tv_sec * 1000000 + tv.tv_usec;
  return full_time;
}
