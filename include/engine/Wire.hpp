#pragma once
#include "common.hpp"
#include "engine/Module.hpp"


namespace rack {


struct Wire {
	int id = 0;
	Module *outputModule = NULL;
	int outputId;
	Module *inputModule = NULL;
	int inputId;
	void step();
};


} // namespace rack
