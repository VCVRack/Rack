This is the code structure used in an Impromptu sequencer. The code excerpt shows how resets, clocks and run states are implemented. The following concepts should be visible in the code below:
* 1ms-clock-ignore-on-reset (and initialize / power-up)
* Clock muting when run is off
* Gate retriggering on reset

```c++
void initRun() {
	// reposition run head to first step
	// ...
}


void MyModule::onReset() override {
	// initialize sequencer variables
	// ...
	running = true;
	initRun();
	clockIgnoreOnReset = (long) (0.001f * engineGetSampleRate());// useful when Rack starts
}


void MyModule::fromJson(json_t *rootJ) override {
	// load sequencer variables
	// ...
	initRun();
}


void MyModule::step() override {
	// Run button
	if (runningTrigger.process(params[RUN_PARAM].value + inputs[RUNCV_INPUT].value)) {
		running = !running;
		if (running) {
			if (resetOnRun) {// this is an option offered in the right-click menu of the sequencer
				initRun();
				clockIgnoreOnReset = (long) (0.001f * engineGetSampleRate());
			}
		}
		// ...
	}

	// Process user interactions (buttons, knobs, etc)
	// ...
	
	// Clock
	if (running && clockIgnoreOnReset == 0) {// clock muting and 1ms-clock-ignore-on-reset
		if (clockTrigger.process(inputs[CLOCK_INPUT].value)) {
			// advance the sequencer
			// ...
		}
	}	
	
	// Reset
	if (resetTrigger.process(inputs[RESET_INPUT].value + params[RESET_PARAM].value)) {
		initRun();
		clockIgnoreOnReset = (long) (0.001f * engineGetSampleRate());
		clockTrigger.reset();
	}
	
	// Outputs
	outputs[CV_OUTPUT].value = cv;
	outputs[GATE_OUTPUT].value = (gate && (clockIgnoreOnReset == 0)) ? 10.f : 0.f;// gate retriggering on reset
			
	// Set the lights of the sequencer 
	// ...
			
	if (clockIgnoreOnReset > 0l)
		clockIgnoreOnReset--;
}// step()

```
