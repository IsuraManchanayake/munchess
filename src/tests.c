#include "tests.h"
#include "common.h"
#include "board.h"

#include <stdio.h>


void test() {
    test_wrapper(test_common);
    test_wrapper(test_board);
}
