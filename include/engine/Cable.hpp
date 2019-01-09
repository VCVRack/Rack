#pragma once
#include "common.hpp"
#include "engine/Module.hpp"


namespace rack {


struct Cable {
	int id = 0;
	Module *outputModule = NULL;
	int outputId;
	Module *inputModule = NULL;
	int inputId;
	void step();
};


} // namespace rack
