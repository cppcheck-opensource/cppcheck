/*
 * Shared test case for Y2038 addon compiler flag testing
 *
 * This file tests various compiler flag scenarios:
 * - Proper Y2038 configuration: -D_TIME_BITS=64 -D_FILE_OFFSET_BITS=64 -D_USE_TIME_BITS64
 * - Incorrect _TIME_BITS value: -D_TIME_BITS=32
 * - Incomplete configuration: -D_USE_TIME_BITS64 (without _TIME_BITS)
 *
 * The same source code is used for all scenarios - differentiation happens
 * through the compiler flags passed to cppcheck during dump creation.
 */

#include <time.h>

int main(int argc, char **argv)
{
    time_t current_time;
    struct timespec ts;

    current_time = time(NULL);
    clock_gettime(CLOCK_REALTIME, &ts);

    return 0;
}

