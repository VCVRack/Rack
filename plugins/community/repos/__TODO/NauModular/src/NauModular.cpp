#include "NauModular.hpp"

Plugin * plugin;

void init(rack::Plugin * p){
    //NauModular::init();

    plugin = p;
    p->slug = TOSTRING(SLUG);
    p->version = TOSTRING(VERSION);
    p->website = "http://naus3a.github.io/NauModular";
    p->manual = "http://naus3a.github.io/NauModular";

    p->addModel(modelTension);
    p->addModel(modelFunction);
    p->addModel(modelPerlin);
	p->addModel(modelS_h_it);
	p->addModel(modelBitHammer);
    p->addModel(modelOsc);
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


