#include "engine/Cable.hpp"


namespace rack {


void Cable::step() {
	Output *output = &outputModule->outputs[outputId];
	Input *input = &inputModule->inputs[inputId];
	// Match number of polyphonic channels to output port
	input->channels = output->channels;
	// Copy values from output to input
	for (int i = 0; i < output->channels; i++) {
		input->values[i] = output->values[i];
	}
}


} // namespace rack
