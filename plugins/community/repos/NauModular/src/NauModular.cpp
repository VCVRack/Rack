#include "NauModular.hpp"

RACK_PLUGIN_MODEL_DECLARE(NauModular, Tension);
RACK_PLUGIN_MODEL_DECLARE(NauModular, Function);
RACK_PLUGIN_MODEL_DECLARE(NauModular, Perlin);
RACK_PLUGIN_MODEL_DECLARE(NauModular, S_h_it);
RACK_PLUGIN_MODEL_DECLARE(NauModular, BitHammer);
RACK_PLUGIN_MODEL_DECLARE(NauModular, Osc);

RACK_PLUGIN_INIT(NauModular) {
   RACK_PLUGIN_INIT_ID();

   RACK_PLUGIN_INIT_WEBSITE("http://naus3a.github.io/NauModular");
   RACK_PLUGIN_INIT_MANUAL("http://naus3a.github.io/NauModular");

   RACK_PLUGIN_MODEL_ADD(NauModular, Tension);
   RACK_PLUGIN_MODEL_ADD(NauModular, Function);
   RACK_PLUGIN_MODEL_ADD(NauModular, Perlin);
   RACK_PLUGIN_MODEL_ADD(NauModular, S_h_it);
   RACK_PLUGIN_MODEL_ADD(NauModular, BitHammer);
   RACK_PLUGIN_MODEL_ADD(NauModular, Osc);
}
/*
void NauModular::init(){
#if defined(TARGET_OSX)
    host_get_clock_service(mach_host_self(), SYSTEM_CLOCK, &NauModular::cs);
#endif
}

void NauModular::getMonotonicTime(uint64_t & seconds, uint64_t & nanos){
#if defined(TARGET_OSX)
    mach_timespec_t now;
    clock_get_time(cs, &now);
    seconds = now.tv_sec;
    nanos = now.tv_nsec;
#elif defined(TARGET_LINUX)
    struct timespec now;
    clock_gettime(CLOCK_MONOTONIC, &now);
    seconds = now.tv_sec;
    nanos = now.tv_nsec;
#elif defined(TARGET_WINDOWS)
    LARGE_INTEGER freq;
	LARGE_INTEGER counter;
	QueryPerformanceFrequency(&freq);
	QueryPerformanceCounter(&counter);
	seconds = counter.QuadPart/freq.QuadPart;
	nanos = (counter.QuadPart % freq.QuadPart)*1000000000/freq.QuadPart;
#else
    struct timeval now;
	gettimeofday( &now, nullptr );
	seconds = now.tv_sec;
	nanos = now.tv_usec * 1000;
#endif
}

float NauModular::getTimef(){
    uint64_t seconds;
    uint64_t nanos;
    NauModular::getMonotonicTime(seconds, nanos);
    float timef = seconds + ((long long)(nanos))/1000000000.;
    return timef;
}
*/


