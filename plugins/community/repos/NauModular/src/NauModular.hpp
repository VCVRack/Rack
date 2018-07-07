#include "rack.hpp"

#include <time.h>
#include <iostream>

#ifdef ARCH_MAC
#include <mach/clock.h>
#include <mach/mach.h>
#include <mach/mach_time.h>
#include <mach/clock_types.h>
#endif

#ifdef ARCH_LIN
#include <string.h>
#endif

using namespace rack;

RACK_PLUGIN_DECLARE(NauModular);

#ifdef USE_VST2
#define plugin "NauModular"
#endif // USE_VST2

namespace NauModular{
/*
#ifdef TARGET_OSX
   static clock_serv_t cs;
#endif
    static void init();
    float getTimef();
    static void getMonotonicTime(uint64_t & seconds, uint64_t & nanos);
*/
};
