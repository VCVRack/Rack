#include "Bidoo.hpp"
#include "BidooComponents.hpp"
#include "dsp/digital.hpp"
#include <iomanip>
#include <sstream>
#include "window.hpp"

using namespace std;

namespace rack_plugin_Bidoo {

struct ACNE : Module {
	enum ParamIds {
	  COPY_PARAM,
		MAIN_OUT_GAIN_PARAM,
		RAMP_PARAM,
		MUTE_PARAM,
		SOLO_PARAM,
		SEND_MUTE_PARAM,
		TRACKLINK_PARAMS,
		OUT_MUTE_PARAMS = TRACKLINK_PARAMS + 8,
		IN_MUTE_PARAMS = OUT_MUTE_PARAMS + ACNE_NB_OUTS,
		IN_SOLO_PARAMS = IN_MUTE_PARAMS + ACNE_NB_TRACKS,
		SNAPSHOT_PARAMS = IN_SOLO_PARAMS + ACNE_NB_TRACKS,
		FADERS_PARAMS = SNAPSHOT_PARAMS + ACNE_NB_SNAPSHOTS,
		NUM_PARAMS = FADERS_PARAMS + (ACNE_NB_TRACKS * ACNE_NB_OUTS)
	};
	enum InputIds {
		SNAPSHOT_INPUT,
		TRACKS_INPUTS,
		NUM_INPUTS = TRACKS_INPUTS + ACNE_NB_TRACKS
	};
	enum OutputIds {
		TRACKS_OUTPUTS,
		NUM_OUTPUTS = TRACKS_OUTPUTS + ACNE_NB_OUTS
	};
	enum LightIds {
		COPY_LIGHT,
		TRACKLINK_LIGHTS,
		OUT_MUTE_LIGHTS = TRACKLINK_LIGHTS + 8,
		IN_MUTE_LIGHTS = OUT_MUTE_LIGHTS + ACNE_NB_OUTS,
		IN_SOLO_LIGHTS = IN_MUTE_LIGHTS + ACNE_NB_TRACKS,
		SNAPSHOT_LIGHTS = IN_SOLO_LIGHTS + ACNE_NB_TRACKS,
		NUM_LIGHTS = SNAPSHOT_LIGHTS + ACNE_NB_SNAPSHOTS
	};

	int currentSnapshot = 0;
	int previousSnapshot = 0;
	int copySnapshot = 0;
	bool copyState = false;
	float snapshots[ACNE_NB_SNAPSHOTS][ACNE_NB_OUTS][ACNE_NB_TRACKS] = {{{0.0f}}};
	bool  outMutes[ACNE_NB_OUTS] = {0};
	bool  inMutes[ACNE_NB_TRACKS] = {0};
	bool  inSolo[ACNE_NB_TRACKS] = {0};
	SchmittTrigger outMutesTriggers[ACNE_NB_OUTS];
	SchmittTrigger inMutesTriggers[ACNE_NB_TRACKS];
	SchmittTrigger inSoloTriggers[ACNE_NB_TRACKS];
	SchmittTrigger snapshotTriggers[ACNE_NB_SNAPSHOTS];
	SchmittTrigger muteTrigger;
	SchmittTrigger soloTrigger;
	int rampSteps = 0;
	int rampSize = 1;
	float rampedValue = 0.0;
	int version = 0;
	SchmittTrigger linksTriggers[8];
	bool links[8] = {0,0,0,0,0,0,0,0};

	ACNE() : Module(NUM_PARAMS, NUM_INPUTS, NUM_OUTPUTS, NUM_LIGHTS) {	}

	void step() override;

	json_t *toJson() override {
		json_t *rootJ = json_object();
		// scenes
		json_t *snapShotsJ = json_array();
		for (int i = 0; i < ACNE_NB_SNAPSHOTS; i++) {
			json_t *snapshotJ = json_array();
			for (int j = 0; j < ACNE_NB_OUTS; j++) {
				json_t *outJ = json_array();
				for (int k = 0 ; k < ACNE_NB_TRACKS; k ++) {
					json_t *controlJ = json_real(snapshots[i][j][k]);
					json_array_append_new(outJ, controlJ);
				}
				json_array_append_new(snapshotJ, outJ);
			}
			json_array_append_new(snapShotsJ, snapshotJ);
		}
		json_object_set_new(rootJ, "snapshots", snapShotsJ);

		for (int i = 0; i < 8; i++) {
			json_object_set_new(rootJ, ("link" + to_string(i)).c_str(), json_boolean(links[i]));
		}

		return rootJ;
	}

	void fromJson(json_t *rootJ) override {
		// scenes
		json_t *snapShotsJ = json_object_get(rootJ, "snapshots");
		if (snapShotsJ) {
			for (int i = 0; i < ACNE_NB_SNAPSHOTS; i++) {
				json_t *snapshotJ = json_array_get(snapShotsJ, i);
				if (snapshotJ) {
					for (int j = 0; j < ACNE_NB_OUTS; j++) {
						json_t *outJ = json_array_get(snapshotJ, j);
						if (outJ) {
							for (int k = 0; k < ACNE_NB_TRACKS; k++) {
								json_t *inJ = json_array_get(outJ, k);
								if (inJ) {
									snapshots[i][j][k] = json_number_value(inJ);
								}
							}
						}
					}
				}
			}
		}

		for (int i = 0; i < 8; i++) {
			json_t *linkJ = json_object_get(rootJ, ("link" + to_string(i)).c_str());
			if (linkJ)
				links[i] = json_is_true(linkJ);
		}
	}

	float getRampedValue(int i, int j) {
		if (rampSize>0) {
			return crossfade(snapshots[currentSnapshot][i][j],snapshots[previousSnapshot][i][j],(float)rampSteps/(float)rampSize);
		}
		else {
			return snapshots[currentSnapshot][i][j];
		}
	}

};

void ACNE::step() {
	rampSize = static_cast<int>(engineGetSampleRate()*params[RAMP_PARAM].value);

	if (inputs[SNAPSHOT_INPUT].active) {
		int newSnapshot = clamp((int)(inputs[SNAPSHOT_INPUT].value * 16 * 0.1f),0,ACNE_NB_SNAPSHOTS-1);
		if (currentSnapshot != newSnapshot) {
			previousSnapshot = currentSnapshot;
			currentSnapshot = newSnapshot;
			rampSteps = rampSize;
			version = (version + 1)%100;
		}
	}
	else {
		for (int i = 0; i < ACNE_NB_SNAPSHOTS; i++) {
			if (snapshotTriggers[i].process(params[SNAPSHOT_PARAMS + i].value)) {
				previousSnapshot = currentSnapshot;
				currentSnapshot = i;
				rampSteps = rampSize;
				version = (version + 1)%100;
			}
		}
	}

	for (int i = 0; i < ACNE_NB_OUTS; i++) {
		if (outMutesTriggers[i].process(params[OUT_MUTE_PARAMS + i].value)) {
			outMutes[i] = !outMutes[i];
		}
	}

	for (int i = 0; i < 8; i++) {
		if (linksTriggers[i].process(params[TRACKLINK_PARAMS + i].value))
			links[i] = !links[i];

		lights[TRACKLINK_LIGHTS + i].value = (links[i] == true) ? 1 : 0;
	}

	for (int i = 0; i < ACNE_NB_TRACKS; i++) {
		int linkIndex = i/2;
		int linkSwitch = i%2;
		if (inMutesTriggers[i].process(params[IN_MUTE_PARAMS + i].value)) {
			inMutes[i] = !inMutes[i];
			if (links[linkIndex]) {
				if (linkSwitch == 0)
					inMutes[i+1] = inMutes[i];
				else
					inMutes[i-1] = inMutes[i];
			}
		}
		if (inSoloTriggers[i].process(params[IN_SOLO_PARAMS + i].value)) {
			inSolo[i] = !inSolo[i];
			if (links[linkIndex]) {
				if (linkSwitch == 0)
					inSolo[i+1] = inSolo[i];
				else
					inSolo[i-1] = inSolo[i];
			}
		}
	}

	if (muteTrigger.process(params[MUTE_PARAM].value)) {
		for (int i = 0; i < ACNE_NB_TRACKS; i++) {
			inMutes[i] = false;
		}
	}

	if (muteTrigger.process(params[SEND_MUTE_PARAM].value)) {
		for (int i = 0; i < ACNE_NB_OUTS; i++) {
			outMutes[i] = false;
		}
	}

	if (soloTrigger.process(params[SOLO_PARAM].value)) {
		for (int i = 0; i < ACNE_NB_TRACKS; i++) {
			inSolo[i] = false;
		}
	}

	for (int i = 0; i < ACNE_NB_OUTS; i++) {
		outputs[TRACKS_OUTPUTS + i].value = 0.0f;
		if (!outMutes[i]) {
			int sum = 0;
			for (int s = 0; s < ACNE_NB_TRACKS; ++s) {
			  sum |= (inSolo[s] == true ? 1 : 0);
			}
			if (sum > 0) {
				for (int j = 0; j < ACNE_NB_TRACKS; j ++) {
					if ((inputs[TRACKS_INPUTS + j].active) && (inSolo[j])) {
						outputs[TRACKS_OUTPUTS + i].value += (getRampedValue(i,j) * 0.1f) * inputs[TRACKS_INPUTS + j].value * 30517578125e-15f;
					}
				}
			}
			else {
				for (int j = 0; j < ACNE_NB_TRACKS; j ++) {
					if ((inputs[TRACKS_INPUTS + j].active) && (!inMutes[j])) {
						if (rampSize>0) {
							rampedValue = crossfade(snapshots[currentSnapshot][i][j],snapshots[previousSnapshot][i][j],(float)rampSteps/(float)rampSize);
						}
						else {
							rampedValue = snapshots[currentSnapshot][i][j];
						}
						outputs[TRACKS_OUTPUTS + i].value += (getRampedValue(i,j) * 0.1f) * inputs[TRACKS_INPUTS + j].value * 30517578125e-15f;
					}
				}
			}
			outputs[TRACKS_OUTPUTS + i].value = outputs[TRACKS_OUTPUTS + i].value * 32768.0f;
		}
	}

	if (rampSteps > 0)
		rampSteps--;

	outputs[TRACKS_OUTPUTS].value = outputs[TRACKS_OUTPUTS].value * params[MAIN_OUT_GAIN_PARAM].value * 0.1f;
	outputs[TRACKS_OUTPUTS + 1].value = outputs[TRACKS_OUTPUTS + 1].value * params[MAIN_OUT_GAIN_PARAM].value * 0.1f;

	lights[COPY_LIGHT].value = (copyState == true) ? 1 : 0;
	for (int i = 0; i < ACNE_NB_OUTS; i++) {
		lights[OUT_MUTE_LIGHTS + i].value = (outMutes[i] == true) ? 1 : 0;
	}
	for (int i = 0; i < ACNE_NB_TRACKS; i++) {
		lights[IN_MUTE_LIGHTS + i].value = (inMutes[i] == true) ? 1 : 0;
		lights[IN_SOLO_LIGHTS + i].value = (inSolo[i] == true) ? 1 : 0;
	}
	for (int i = 0; i < ACNE_NB_SNAPSHOTS; i++) {
		lights[SNAPSHOT_LIGHTS + i].value = (i == currentSnapshot) ? 1 : 0;
	}
}

struct ACNEWidget : ModuleWidget {
	ParamWidget *faders[ACNE_NB_OUTS][ACNE_NB_TRACKS];
	void UpdateSnapshot(int snapshot);
	void step() override;
	int moduleVersion = 0;
	int frames=0;
	ACNEWidget(ACNE *module);
};

struct ACNETrimPot : BidooColoredTrimpot {
	void onChange(EventChange &e) override {
			ACNEWidget *parent = dynamic_cast<ACNEWidget*>(this->parent);
			ACNE *module = dynamic_cast<ACNE*>(this->module);
			if (parent && module) {
				module->snapshots[module->currentSnapshot][(int)((this->paramId - ACNE::FADERS_PARAMS) / ACNE_NB_TRACKS)][(this->paramId - ACNE::FADERS_PARAMS) % ACNE_NB_TRACKS] = value;
			}
			BidooColoredTrimpot::onChange(e);
	}

	virtual void onMouseDown(EventMouseDown &e) override {
		BidooColoredTrimpot::onMouseDown(e);
		ACNEWidget *parent = dynamic_cast<ACNEWidget*>(this->parent);
		ACNE *module = dynamic_cast<ACNE*>(this->module);
		if (parent && module) {
			if ((e.button == RACK_MOUSE_BUTTON_MIDDLE) || ((e.button == RACK_MOUSE_BUTTON_LEFT) && (windowIsShiftPressed()))) {
				this->setValue(10);
				module->snapshots[module->currentSnapshot][(int)((this->paramId - ACNE::FADERS_PARAMS) / ACNE_NB_TRACKS)][(this->paramId - ACNE::FADERS_PARAMS) % ACNE_NB_TRACKS] = 10;
			}
		}
	}
};

struct ACNEChoseSceneLedButton : LEDButton {
	void onMouseDown(EventMouseDown &e) override {
		ACNEWidget *parent = dynamic_cast<ACNEWidget*>(this->parent);
		ACNE *module = dynamic_cast<ACNE*>(this->module);
		if (parent && module) {
			module->currentSnapshot = this->paramId - ACNE::SNAPSHOT_PARAMS;
			parent->UpdateSnapshot(module->currentSnapshot);
		}
		LEDButton::onMouseDown(e);
	}
};

struct ACNECOPYPASTECKD6 : BlueCKD6 {
	void onMouseDown(EventMouseDown &e) override {
		ACNEWidget *parent = dynamic_cast<ACNEWidget*>(this->parent);
		ACNE *module = dynamic_cast<ACNE*>(this->module);
		if (parent && module) {
			if (!module->copyState) {
				module->copySnapshot = module->currentSnapshot;
				module->copyState = true;
			} else if ((module->copyState) && (module->copySnapshot != module->currentSnapshot)) {
				for (int i = 0; i < ACNE_NB_OUTS; i++) {
					for (int j = 0; j < ACNE_NB_TRACKS; j++) {
						module->snapshots[module->currentSnapshot][i][j] = module->snapshots[module->copySnapshot][i][j];
					}
				}
				parent->UpdateSnapshot(module->currentSnapshot);
				module->copyState = false;
			}
		}
		BlueCKD6::onMouseDown(e);
	}
};

ACNEWidget::ACNEWidget(ACNE *module) : ModuleWidget(module) {
	setPanel(SVG::load(assetPlugin(plugin, "res/ACNE.svg")));

	addChild(Widget::create<ScrewSilver>(Vec(RACK_GRID_WIDTH, 0)));
	addChild(Widget::create<ScrewSilver>(Vec(box.size.x - 2 * RACK_GRID_WIDTH, 0)));
	addChild(Widget::create<ScrewSilver>(Vec(RACK_GRID_WIDTH, RACK_GRID_HEIGHT - RACK_GRID_WIDTH)));
	addChild(Widget::create<ScrewSilver>(Vec(box.size.x - 2 * RACK_GRID_WIDTH, RACK_GRID_HEIGHT - RACK_GRID_WIDTH)));

	addParam(ParamWidget::create<BidooBlueKnob>(Vec(474.0f, 39.0f), module, ACNE::MAIN_OUT_GAIN_PARAM, 0.0f, 10.0f, 7.0f));

	addParam(ParamWidget::create<ACNECOPYPASTECKD6>(Vec(7.0f, 39.0f), module, ACNE::COPY_PARAM, 0.0f, 1.0f, 0.0f));
	addChild(ModuleLightWidget::create<SmallLight<GreenLight>>(Vec(18.0f, 28.0f), module, ACNE::COPY_LIGHT));

	addParam(ParamWidget::create<BidooBlueTrimpot>(Vec(432.0f, 28.0f), module, ACNE::RAMP_PARAM, 0.0f, 0.01f, 0.001f));

	addInput(Port::create<TinyPJ301MPort>(Vec(58.0f, 30.0f), Port::INPUT, module, ACNE::SNAPSHOT_INPUT));

	addParam(ParamWidget::create<MuteBtn>(Vec(2.0f, 293.0f), module, ACNE::SEND_MUTE_PARAM, 0.0f, 1.0f, 0.0f));
	addParam(ParamWidget::create<MuteBtn>(Vec(21.0f, 293.0f), module, ACNE::MUTE_PARAM, 0.0f, 1.0f, 0.0f));
	addParam(ParamWidget::create<SoloBtn>(Vec(11.0f, 314.0f), module, ACNE::SOLO_PARAM, 0.0f, 1.0f, 0.0f));

	for (int i = 0; i < ACNE_NB_OUTS; i++) {
		addOutput(Port::create<TinyPJ301MPort>(Vec(482.0f, 79.0f+i*27.0f),Port::OUTPUT, module, ACNE::TRACKS_OUTPUTS + i));

		addParam(ParamWidget::create<LEDButton>(Vec(10.0f, 77.0f+i*27.0f), module, ACNE::OUT_MUTE_PARAMS + i, 0.0f, 1.0f,  0.0f));
		addChild(ModuleLightWidget::create<SmallLight<RedLight>>(Vec(16.0f, 82.0f+i*27.0f), module, ACNE::OUT_MUTE_LIGHTS + i));
	}

	for (int i = 0; i < ACNE_NB_TRACKS; i++) {
		addParam(ParamWidget::create<ACNEChoseSceneLedButton>(Vec(43.0f+i*27.0f, 49.0f), module, ACNE::SNAPSHOT_PARAMS + i, 0.0f, 1.0f,  0.0f));
		addChild(ModuleLightWidget::create<SmallLight<BlueLight>>(Vec(49.0f+i*27.0f, 55.0f), module, ACNE::SNAPSHOT_LIGHTS + i));

		addInput(Port::create<TinyPJ301MPort>(Vec(45.0f+i*27.0f, 338.0f),Port::INPUT, module, ACNE::TRACKS_INPUTS + i));

		addParam(ParamWidget::create<LEDButton>(Vec(43.0f+i*27.0f, 292.0f), module, ACNE::IN_MUTE_PARAMS + i, 0.0f, 1.0f,  0.0f));
		addChild(ModuleLightWidget::create<SmallLight<RedLight>>(Vec(49.0f+i*27.0f, 297.0f), module, ACNE::IN_MUTE_LIGHTS + i));

		addParam(ParamWidget::create<LEDButton>(Vec(43.0f+i*27.0f, 314.0f), module, ACNE::IN_SOLO_PARAMS + i, 0.0f, 1.0f,  0.0f));
		addChild(ModuleLightWidget::create<SmallLight<GreenLight>>(Vec(49.0f+i*27.0f, 320.0f), module, ACNE::IN_SOLO_LIGHTS + i));
	}

	for (int i = 0; i < 8; i++) {
		addParam(ParamWidget::create<MiniLEDButton>(Vec(62.0f+i*54.0f, 309.0f), module, ACNE::TRACKLINK_PARAMS + i, 0.0f, 10.0f,  0.0f));
		addChild(ModuleLightWidget::create<SmallLight<BlueLight>>(Vec(62.0f+i*54.0f, 309.0f), module, ACNE::TRACKLINK_LIGHTS + i));
	}

	for (int i = 0; i < ACNE_NB_OUTS; i++) {
		for (int j = 0; j < ACNE_NB_TRACKS; j++) {
				faders[i][j] = ParamWidget::create<ACNETrimPot>(Vec(43.0f+j*27.0f, 77.0f+i*27.0f), module, ACNE::FADERS_PARAMS + j + i * (ACNE_NB_TRACKS), 0.0f, 10.0f, 0.0f);
				addParam(faders[i][j]);
		}
	}
	module->currentSnapshot = 0;
	UpdateSnapshot(module->currentSnapshot);
}


void ACNEWidget::UpdateSnapshot(int snapshot) {
	ACNE *module = dynamic_cast<ACNE*>(this->module);
	for (int i = 0; i < ACNE_NB_OUTS; i++) {
		for (int j = 0; j < ACNE_NB_TRACKS; j++) {
			if (faders[i][j]->value != module->snapshots[module->currentSnapshot][i][j])
				faders[i][j]->setValue(module->snapshots[module->currentSnapshot][i][j]);
		}
	}
}

void ACNEWidget::step() {
	frames++;
	if (frames>2){
		ACNE *module = dynamic_cast<ACNE*>(this->module);
		if (module->version != moduleVersion) {
			for (int i = 0; i < ACNE_NB_OUTS; i++) {
				for (int j = 0; j < ACNE_NB_TRACKS; j++) {
					if (faders[i][j]->value != module->snapshots[module->currentSnapshot][i][j])
						faders[i][j]->setValue(module->snapshots[module->currentSnapshot][i][j]);
				}
			}
			moduleVersion = module->version;
		}
		frames = 0;
	}
	ModuleWidget::step();
}

} // namespace rack_plugin_Bidoo

using namespace rack_plugin_Bidoo;

RACK_PLUGIN_MODEL_INIT(Bidoo, ACNE) {
   Model *modelACNE = Model::create<ACNE, ACNEWidget>("Bidoo", "ACnE", "ACnE mixer", MIXER_TAG);
   return modelACNE;
}
