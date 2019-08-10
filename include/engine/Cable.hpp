#pragma once
#include <common.hpp>
#include <engine/Module.hpp>


namespace rack {
namespace engine {


struct Cable {
	int id = -1;
	Module* outputModule = NULL;
	int outputId;
	Module* inputModule = NULL;
	int inputId;
};


} // namespace engine
} // namespace rack
