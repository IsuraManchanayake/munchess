#pragma once

#include <stdio.h>
#include <stdlib.h>

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
