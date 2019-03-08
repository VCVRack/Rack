#include "rack.hpp"

using namespace rack;

const int ACNE_NB_TRACKS = 16;
const int ACNE_NB_OUTS = 8;
const int ACNE_NB_SNAPSHOTS = 16;

const NVGcolor BLUE_BIDOO = nvgRGBA(42, 87, 117, 255);
const NVGcolor LIGHTBLUE_BIDOO = nvgRGBA(45, 114, 143, 255);
const NVGcolor RED_BIDOO = nvgRGBA(205, 31, 0, 255);
const NVGcolor YELLOW_BIDOO = nvgRGBA(255, 233, 0, 255);
const NVGcolor YELLOW_BIDOO_LIGHT = nvgRGBA(255, 233, 0, 25);
const NVGcolor SAND_BIDOO = nvgRGBA(230, 220, 191, 255);
const NVGcolor ORANGE_BIDOO = nvgRGBA(228, 87, 46, 255);
const NVGcolor PINK_BIDOO = nvgRGBA(164, 3, 111, 255);
const NVGcolor GREEN_BIDOO = nvgRGBA(2, 195, 154, 255);

RACK_PLUGIN_DECLARE(Bidoo);

#ifdef USE_VST2
#define plugin "Bidoo"
#endif // USE_VST2
