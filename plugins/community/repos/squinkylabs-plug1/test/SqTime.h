#pragma once
/**
 * @class SqTime
 * A simple high-res timer. Returns current seconds
 */


 /**
  * Windows version is based on QueryPerformanceFrequency
  * Typically this is accurate to 1/3 microsecond. Can be made more
  * accurate by tinkering with your bios
  */
#if defined(_MSC_VER) || defined(ARCH_WIN)
#include <Windows.h>
class SqTime
{
public:
    static double seconds()
    {
        LARGE_INTEGER t;
        if (frequency == 0) {

            QueryPerformanceFrequency(&t);
            frequency = double(t.QuadPart);
        }

        QueryPerformanceCounter(&t);
        int64_t n = t.QuadPart;
        return double(n) / frequency;
    }
private:
    static double frequency;
};
double SqTime::frequency = 0;

#else
#include <time.h>
class SqTime
{
public:
    static double seconds()
    {
        struct timespec ts;
        int x = clock_gettime(CLOCK_THREAD_CPUTIME_ID, &ts);
        assert(x == 0);

        // seconds = sec + nsec / 10**9
        return double(ts.tv_sec) + double(ts.tv_nsec) / (1000.0 * 1000.0 * 1000.0);
    }
};
#endif
