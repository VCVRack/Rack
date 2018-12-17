#include "engine/Wire.hpp"


namespace rack {


void Wire::step() {
	// Copy output to input
	float value = outputModule->outputs[outputId].value;
	inputModule->inputs[inputId].value = value;
}


} // namespace rack
