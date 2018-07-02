#pragma once


#include "../dep/yac/yac.h"

namespace rack {


struct Global;
struct GlobalUI;
extern YAC_TLS Global *global;
extern YAC_TLS GlobalUI *global_ui;

struct KeyboardDriver;



} // namespace rack
