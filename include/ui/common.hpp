#pragma once
#include "../common.hpp"
#include <nanovg.h>
#include <blendish.h>

#define CHECKMARK_STRING "✔"
#define CHECKMARK(_cond) ((_cond) ? CHECKMARK_STRING : "")
