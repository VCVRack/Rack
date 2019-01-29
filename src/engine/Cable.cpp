#include "engine/Cable.hpp"


namespace rack {
namespace engine {


void Cable::step() {
	Output *output = &outputModule->outputs[outputId];
	Input *input = &inputModule->inputs[inputId];
	// Match number of polyphonic channels to output port
	input->channels = output->channels;
	// Copy voltages from output to input
	for (int i = 0; i < output->channels; i++) {
		input->voltages[i] = output->voltages[i];
	}
}


} // namespace engine
} // namespace rack
