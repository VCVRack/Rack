#pragma once
#include "../common.hpp"
#include "blendish.h"

#define CHECKMARK_STRING "✔"
#define CHECKMARK(_cond) ((_cond) ? CHECKMARK_STRING : "")
