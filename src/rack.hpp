#pragma once

#include <stdlib.h>
#include <set>
#include <vector>

// TODO Find a clean way to make this a variable
#define SAMPLE_RATE 44100


struct Wire;

struct Module {
	std::vector<float> params;
	// Pointers to voltage values at each port
	// If value is NULL, the input/output is disconnected
	std::vector<float*> inputs;
	std::vector<float*> outputs;

	virtual ~Module() {}

	// Always called on each sample frame before calling getOutput()
	virtual void step() {}
};


struct Wire {
	Module *outputModule = NULL;
	int outputId;
	Module *inputModule = NULL;
	int inputId;
	// The voltage which is pointed to by module inputs/outputs
	float value = 0.0;
};
