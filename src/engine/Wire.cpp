#include "engine/Wire.hpp"


namespace rack {


void Wire::step() {
	Output *output = &outputModule->outputs[outputId];
	Input *input = &inputModule->inputs[inputId];
	// Match number of polyphonic channels to output port
	input->numChannels = output->numChannels;
	// Copy values from output to input
	for (int i = 0; i < output->numChannels; i++) {
		input->values[i] = output->values[i];
	}
}


} // namespace rack
