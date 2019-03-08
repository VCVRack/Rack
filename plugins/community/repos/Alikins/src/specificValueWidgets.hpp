#include "rack.hpp"

namespace rack_plugin_Alikins {

struct PurpleTrimpot : Trimpot {
	Module *module;
    bool initialized = false;
    PurpleTrimpot();
    void step() override;
    void reset() override;
    void randomize() override;
};

PurpleTrimpot::PurpleTrimpot() : Trimpot() {
    setSVG(SVG::load(assetPlugin(plugin, "res/PurpleTrimpot.svg")));
    shadow->blurRadius = 0.0;
    shadow->opacity = 0.10;
    shadow->box.pos = Vec(0.0, box.size.y * 0.05);
}


// FIXME: if we are getting moving inputs and we are hovering
//        over the trimpot, we kind of jitter arround.
// maybe run this via an onChange()?
void PurpleTrimpot::step() {
	// debug("paramId=%d this->initialized: %d initialized: %d this->value: %f value: %f param.value: %f",
     // paramId,  this->initialized, initialized, this->value, value, module->params[paramId].value);

    if (this->value != module->params[paramId].value) {
		if (this != RACK_PLUGIN_UI_HOVERED_WIDGET && this->initialized) {
			// this->value = module->params[paramId].value;
			setValue(module->params[paramId].value);
		} else {
			module->params[paramId].value = this->value;
            this->initialized |= true;
		}
		EventChange e;
		onChange(e);
	}

	Trimpot::step();
}

void PurpleTrimpot::reset() {
    this->initialized = false;
    Trimpot::reset();
    }

void PurpleTrimpot::randomize() {
    reset();
    setValue(rescale(randomUniform(), 0.0f, 1.0f, minValue, maxValue));
}

} // namespace rack_plugin_Alikins
