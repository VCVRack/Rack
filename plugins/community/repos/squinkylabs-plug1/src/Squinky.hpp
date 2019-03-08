#include "rack.hpp"

#ifdef __V1
#include "component.hpp"
#else
#include "componentlibrary.hpp"
#endif
//extern rack::Plugin* plugin;

#ifndef __V1
//#define _SEQ        // just for test
#endif


#define _FUN        // works with 1.0
#define _LFN
#define _FORMANTS
#define _SHAPER
#define _CHB
#define _GRAY
//#define _CH10

#ifndef __V1
    #define _EV3
    #define _CHB
    #define _CHBG
    #define _LFN
    #define _COLORS
    #define _GRAY
    #define _SUPER
    #define _GROWLER
    #define _FORMANTS
    #define _TBOOST
    #define _BOOTY
    #define _TREM
#endif


using namespace rack;

RACK_PLUGIN_DECLARE(squinkylabs_plug1);

#ifdef USE_VST2
#define plugin "squinkylabs-plug1"
#endif // USE_VST2

