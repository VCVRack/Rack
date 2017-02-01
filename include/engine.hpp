#pragma once
#include <vector>


namespace rack {


struct Module {
	std::vector<float> params;
	/** Pointers to voltage values at each port
	If value is NULL, the input/output is disconnected
	*/
	std::vector<float*> inputs;
	std::vector<float*> outputs;
	/** For CPU usage */
	float cpuTime = 0.0;

	virtual ~Module() {}

	/** Advances the module by 1 audio frame with duration 1.0 / gSampleRate */
	virtual void step() {}
};

struct Wire {
	Module *outputModule = NULL;
	int outputId;
	Module *inputModule = NULL;
	int inputId;
	/** The voltage connected to input ports */
	float inputValue = 0.0;
	/** The voltage connected to output ports */
	float outputValue = 0.0;
};

void engineInit();
void engineDestroy();
/** Launches engine thread */
void engineStart();
void engineStop();
/** Does not transfer pointer ownership */
void engineAddModule(Module *module);
void engineRemoveModule(Module *module);
/** Does not transfer pointer ownership */
void engineAddWire(Wire *wire);
void engineRemoveWire(Wire *wire);
void engineSetParamSmooth(Module *module, int paramId, float value);

extern float gSampleRate;


} // namespace rack
