#include "Utilities.hpp"

uint32_t mwcRand(uint32_t& w, uint32_t& z) {
    z = 36969 * (z & 65535) + (z >> 16);
    w = 18000 * (w & 65535) + (w >> 16);
    return (z << 16) + w;
}
