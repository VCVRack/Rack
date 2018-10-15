#pragma once
#include "common.hpp"


namespace rack {


struct Module;


/** A Widget that exists on a Panel and interacts with a Module */
struct Component : OpaqueWidget {
	Module *module = NULL;
};


} // namespace rack
