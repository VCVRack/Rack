#pragma once
#include "common.hpp"
#include "engine.hpp"


namespace rack {


/** A Component which has control over a Param */
struct ParamWidget : Component, QuantityWidget {
	int paramId;
	/** Used to momentarily disable value randomization
	To permanently disable or change randomization behavior, override the randomize() method instead of changing this.
	*/
	bool randomizable = true;
	/** Apply per-sample smoothing in the engine */
	bool smooth = false;

	json_t *toJson();
	void fromJson(json_t *rootJ);
	virtual void reset();
	virtual void randomize();
	void onButton(event::Button &e) override;
	void onChange(event::Change &e) override;
};


} // namespace rack
