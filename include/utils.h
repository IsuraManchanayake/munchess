#pragma once

#include <stdio.h>

#define debugzu(x) (printf("%s = %zu\n", #x, (x)));
#define debugc(x) (printf("%s = %c\n", #x, (x)));
#define debugs(x) (printf("%s = %s\n", #x, (x)));