#include "rack.hpp"

using namespace rack;

RACK_PLUGIN_DECLARE(LOGinstruments);

#ifdef USE_VST2
#define plugin "LOGinstruments"
#endif // USE_VST2

template <typename T> int sign(T val) {
    return (T(0) < val) - (val < T(0));
}


////////////////////
// Additional GUI stuff
////////////////////

#ifdef DEBUG
  #define dbgPrint(a) printf a
#else
  #define dbgPrint(a) (void)0
#endif
