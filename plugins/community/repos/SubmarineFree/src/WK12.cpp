#include "SubmarineFree.hpp"
#include <mutex>
#include "torpedo.hpp"
#include <fstream>
#include <cctype>
#include "UpdateRing.hpp"

namespace rack_plugin_SubmarineFree {

struct WK_Update {
	float offsets[12];
	int isDirty = false;
};

struct WK_Tuning {
	std::string name;
	float offsets[12];
};

std::vector<WK_Tuning> tunings;

int tuningsLoaded = false;

struct WK_Tunings {
	static void loadTuningsFromWK(const char *path);
	static void loadTuningsFromScala(const char /*Plugin*/ *_plugin);
	static void loadScalaFile(std::string path);
	static void loadTunings(const char /*Plugin*/ *_plugin) {
		if (tuningsLoaded)
			return;
		tuningsLoaded = true;
		loadTuningsFromWK(assetPlugin(_plugin, "WK_Custom.tunings").c_str());
		loadTuningsFromScala(_plugin);
	}
};

void WK_Tunings::loadTuningsFromWK(const char *path) {
	FILE *file = fopen(path, "r");
	if (!file) {
		return;
	}
	int defaultSize = tunings.size();
	
	json_error_t error;
	json_t *rootJ = json_loadf(file, 0, &error);
	if (rootJ) {
		int size = json_array_size(rootJ);
		for (int i = 0; i < size; i++) {
			json_t *j0 = json_array_get(rootJ, i);
			if (j0) {
				json_t *jname = json_object_get(j0, "name");
				if (jname) {
					json_t *joffsets = json_object_get(j0, "tunings");
					if (joffsets) {
						tunings.push_back(WK_Tuning());
						tunings[i + defaultSize].name.assign(json_string_value(jname));
						int tsize = json_array_size(joffsets);
						for (int j = 0; j < 12; j++) {
							if (j < tsize) {
								json_t *joffset = json_array_get(joffsets, j);
								if (joffset) {
									tunings[i + defaultSize].offsets[j] = json_number_value(joffset);
								}
							}
							else {
								tunings[i + defaultSize].offsets[j] = 0.0f;
							}
						}
					}	
				}
			}
		}
		json_decref(rootJ);
	}
	else {
		std::string message = stringf("SubmarineFree WK: JSON parsing error at %s %d:%d %s", error.source, error.line, error.column, error.text);
		warn(message.c_str());
	}
	fclose(file);
}

void WK_Tunings::loadScalaFile(std::string path) {
	std::ifstream fs{path, std::ios_base::in};
	if (fs) {
		std::vector<std::string> strings;
		while (!fs.eof()) {
			std::string line;
			getline(fs, line);
			int iscomment = false;
			for (unsigned int i = 0; i < line.size(); i++) {
				if (std::isspace(line[i]))
					continue;
				if (line[i] == '!') {
					iscomment = true;
					break;
				}
			}
			if (iscomment)
				continue;
			strings.push_back(std::string(line));
			if (strings.size() >= 14)
				break;
		}
		fs.close();
		if (strings.size() < 2) return;
		WK_Tuning tuning = { "", { 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0 } };
		tuning.name.assign(strings[0]);
		for (unsigned int i = 2; i < strings.size(); i++) {
			// remove leading whitespace
			while (strings[i].size() && std::isspace(strings[i][0]))
				strings[i].erase(0,1);
			std::string line;
			int decimal = false;
			int ratio = false;
			while (strings[i].size() && !std::isspace(strings[i][0])) {
				char c = strings[i][0];
				line.append(1,c);
				strings[i].erase(0,1);
				if (!std::isdigit(c) && (c != '/') && (c != '.')) {
					warn("SubmarineFree WK: Scala file format error in %s", stringFilename(path).c_str());
					return;
				}
				if (c == '.')
					decimal = true;
				if (c == '/' && !ratio)
					ratio = line.size();
				if (decimal && ratio) {
					warn("SubmarineFree WK: Scala file format error in %s", stringFilename(path).c_str());
					return;
				}
			}
			if (decimal) {
				try {
					float d = std::stof(line, nullptr);
					d -= (i-1) * 100.0;
					if ((d < -50.0) || (d > 50.0)) {
						warn("SubmarineFree WK: Scala file format error in %s", stringFilename(path).c_str());
						return;
					}
					tuning.offsets[(i-1)%12] = d;
				}
				catch (std::exception &err) {
					warn("SubmarineFree WK: Scala file format error in %s", stringFilename(path).c_str());
					return;
				}
			}
			else {
				if (ratio) {
					std::string num = line.substr(0,ratio);
					std::string denom = line.substr(ratio);
					try {
						int inum = std::stoi(num,nullptr);
						int idenom = std::stoi(denom, nullptr);
						if (!idenom) {
							warn("SubmarineFree WK: Scala file format error in %s", stringFilename(path).c_str());
							return;
						}
						float r = (1.0f * inum / idenom);  
						float d = 1200.0 * log2(r);
						d -= (i-1) * 100.0;
						if ((d < -50.0) || (d > 50.0)) {
							warn("SubmarineFree WK: Scala file format error in %s", stringFilename(path).c_str());
							return;
						}
						tuning.offsets[(i-1)%12] = d;
					}
					catch (std::exception &err) {
						warn("SubmarineFree WK: Scala file format error in %s", stringFilename(path).c_str());
						return;
					}
				}
				else {
					try {
						int inum = std::stoi(line, nullptr);
						float d = 1200.0 * log2(inum);
						d -= (i-1) * 100.0;
						if ((d < -50.0) || (d > 50.0)) {
							warn("SubmarineFree WK: Scala file format error in %s", stringFilename(path).c_str());
							return;
						}
						tuning.offsets[(i-1)%12] = d;
					}
					catch (std::exception &err) {
						warn("SubmarineFree WK: Scala file format error in %s", stringFilename(path).c_str());
						return;
					}
				}
			}
		}
		int index = tunings.size();
		tunings.push_back(WK_Tuning());
		tunings[index].name = tuning.name;
		for (int i = 0; i < 12; i++)
			tunings[index].offsets[i] = tuning.offsets[i];
		info("SubmarineFree WK: Loaded Scala file %s", tuning.name.c_str());
	}

}

void WK_Tunings::loadTuningsFromScala(const char /*Plugin*/ *_plugin) {
	std::vector<std::string> dirList = systemListEntries(assetPlugin(_plugin, "Scala"));
	for (auto entry : dirList) {
		if (systemIsDirectory(entry)) continue;
		if (stringExtension(entry).compare("scl")) continue;
		loadScalaFile(entry);
	}
}

struct WK_101;

struct WK101_InputPort : Torpedo::PatchInputPort {
	WK_101 *wkModule;
	WK101_InputPort(WK_101 *module, unsigned int portNum):PatchInputPort((Module *)module, portNum) { wkModule = module;};
	void received(std::string pluginName, std::string moduleName, json_t *rootJ) override;
};

struct WK_101 : Module {
	enum ParamIds {
		PARAM_1,
		PARAM_2,
		PARAM_3,
		PARAM_4,
		PARAM_5,
		PARAM_6,
		PARAM_7,
		PARAM_8,
		PARAM_9,
		PARAM_10,
		PARAM_11,
		PARAM_12,
		NUM_PARAMS
	};
	enum InputIds {
		INPUT_CV,
		INPUT_TOR,
		NUM_INPUTS
	};
	enum OutputIds {
		OUTPUT_CV,
		OUTPUT_TOR,
		NUM_OUTPUTS
	};
	enum LightIds {
		LIGHT_1,
		LIGHT_2,
		LIGHT_3,
		LIGHT_4,
		LIGHT_5,
		LIGHT_6,
		LIGHT_7,
		LIGHT_8,
		LIGHT_9,
		LIGHT_10,
		LIGHT_11,
		LIGHT_12,
		NUM_LIGHTS
	};
	float tunings[12];
	int toSend = 0;
	UpdateRing<WK_Update> updateRing;
	Torpedo::PatchOutputPort outPort = Torpedo::PatchOutputPort(this, OUTPUT_TOR);
	WK101_InputPort inPort = WK101_InputPort(this, INPUT_TOR);

	WK_101() : Module(NUM_PARAMS, NUM_INPUTS, NUM_OUTPUTS, NUM_LIGHTS) {outPort.size(5);}
	void step() override;
	void PrepareUpdate() {
		WK_Update *upd = updateRing.bg();
		for (int i = 0; i < 12; i++)
			upd->offsets[i] = tunings[i];
		upd->isDirty = true;
		updateRing.swap();
	}
};

void WK_101::step() {
	int quantized = floor((12.0f * inputs[INPUT_CV].value) + 0.5f);
	int note = (120 + quantized) % 12;
	outputs[OUTPUT_CV].value = (tunings[note] / 1200.0f) + (quantized / 12.0f);	
	for (int i = 0; i < 12; i++) 
		lights[LIGHT_1 + i].value = (note == i)?1.0f:0.0f;
	if (toSend && !outPort.isBusy()) {
		toSend = 0;
		json_t *rootJ = json_array();
		for (int i = 0; i < 12; i++)
			json_array_append_new(rootJ, json_real(tunings[i]));
		outPort.send(std::string(TOSTRING(SLUG)), std::string("WK"), rootJ);
	}
	outPort.process();
	inPort.process();

}

void WK101_InputPort::received(std::string pluginName, std::string moduleName, json_t *rootJ) {
	if (pluginName.compare(TOSTRING(SLUG))) return;
	if (moduleName.compare("WK")) return;
	float tunings[12];
	int size = json_array_size(rootJ);
	if (!size) return;
	if (size > 12)
		size = 12;
	for (int i = 0; i < size; i++) {
		json_t *j1 = json_array_get(rootJ, i);
		if (j1)
			tunings[i] = json_number_value(j1);
	}
	{
		//std::lock_guard<std::mutex> guard(wkModule->mtx);
		//wkModule->isDirty = true;
		WK_Update *upd = wkModule->updateRing.bg();
		for (int i = 0; i < 12; i++)
			upd->offsets[i] = tunings[i];
		upd->isDirty = true;
		wkModule->updateRing.swap();
	}
}

struct WK_Display : TransparentWidget {
	std::shared_ptr<Font> font;
	WK_101 *module;
	int index;
	char dspText[20];
	
	WK_Display() {
		font = Font::load(assetGlobal("res/fonts/DejaVuSans.ttf"));
	}

	void draw(NVGcontext *vg) override {
		float val = module->tunings[index];
		sprintf(dspText, "%+05.2f", val);
		nvgFontSize(vg, 14);
		nvgFontFaceId(vg, font->handle);
		nvgFillColor(vg, nvgRGBA(0x28, 0xb0, 0xf3, 0xff));
		nvgTextAlign(vg, NVG_ALIGN_CENTER);
		nvgText(vg, 30, 13, dspText, NULL);
	}
};

struct WK101_MenuItem : MenuItem {
	WK_101 *module;
	int index;
	void onAction(EventAction &e) override {
		for (int i = 0; i < 12; i++)
			module->tunings[i] = tunings[index].offsets[i];
		module->PrepareUpdate();
		module->toSend = true;
	}
};

struct WK_Param : MedKnob<LightKnob> {
	
	void onChange(EventChange &e) override {
		MedKnob<LightKnob>::onChange(e);
		WK_101 *module = dynamic_cast<WK_101 *>(this->module);
		module->tunings[paramId - WK_101::PARAM_1] = value;
		module->toSend = true;
	}
};

struct WK101 : ModuleWidget {
	WK_Param *widgets[12];
	WK101(WK_101 *module) : ModuleWidget(module) {
		setPanel(SVG::load(assetPlugin(plugin, "res/WK-101.svg")));

		addInput(Port::create<SilverPort>(Vec(4,29), Port::INPUT, module, WK_101::INPUT_CV));
		addOutput(Port::create<SilverPort>(Vec(43,29), Port::OUTPUT, module, WK_101::OUTPUT_CV));
		addInput(Port::create<BlackPort>(Vec(82,29), Port::INPUT, module, WK_101::INPUT_TOR));
		addOutput(Port::create<BlackPort>(Vec(121,29), Port::OUTPUT, module, WK_101::OUTPUT_TOR));

		for (int i = 0; i < 5; i++)
		{
			WK_Display *display = new WK_Display();
			display->module = module;
			display->index = i;
			display->box.pos = Vec(45, 79 + 21 * i);
			display->box.size = Vec(60, 20);
			addChild(display);
			widgets[i] = ParamWidget::create<WK_Param>(Vec(4 + 104 * (i%2),70 + 21 * i), module, WK_101::PARAM_1 + i, -50.0f, 50.0f, 0.0f);
			addParam(widgets[i]);
			addChild(ModuleLightWidget::create<TinyLight<BlueLight>>(Vec(21.5 + 104 * (i%2), 87.5 + 21 * i), module, WK_101::LIGHT_1 + i));
		}
		for (int i = 5; i < 12; i++)
		{
			WK_Display *display = new WK_Display();
			display->module = module;
			display->index = i;
			display->box.pos = Vec(45, 100 + 21 * i);
			display->box.size = Vec(60, 20);
			addChild(display);
			widgets[i] = ParamWidget::create<WK_Param>(Vec(108 - 104 * (i%2),91 + 21 * i), module, WK_101::PARAM_1 + i, -50.0f, 50.0f, 0.0f);
			addParam(widgets[i]);
			addChild(ModuleLightWidget::create<TinyLight<BlueLight>>(Vec(125.5 - 104 * (i%2), 108.5 + 21 * i), module, WK_101::LIGHT_1 + i));
		}
		WK_Tunings::loadTunings(plugin);
	}
	void appendContextMenu(Menu *menu) override;
	void step() override;
};

void WK101::appendContextMenu(Menu *menu) {
	WK_101 *module = dynamic_cast<WK_101 *>(this->module);
	menu->addChild(MenuEntry::create());
	for (unsigned int i = 0; i < tunings.size(); i++) { 
		WK101_MenuItem *m = MenuItem::create<WK101_MenuItem>(tunings[i].name.c_str());
		m->module = module;
		m->index = i;
		menu->addChild(m);
	}
}

void WK101::step() {
	float tunings[12];
	int isDirty = 0;
	WK_101 *module = dynamic_cast<WK_101 *>(this->module);
	{
		//std::lock_guard<std::mutex> guard(module->mtx);
		WK_Update *upd  = module->updateRing.fg();
		if (upd->isDirty) {
			for (int i = 0; i < 12; i++)
				tunings[i] = upd->offsets[i];
			upd->isDirty = false;
			isDirty = true;
		}
	}
	if (isDirty) {
		for (int i = 0; i < 12; i++) {
			if (widgets[i]->value != tunings[i])
				widgets[i]->setValue(tunings[i]);	
		}
	}

	ModuleWidget::step();
}

struct WK_205;

struct WK205_InputPort : Torpedo::PatchInputPort {
	WK_205 *wkModule;
	WK205_InputPort(WK_205 *module, unsigned int portNum):PatchInputPort((Module *)module, portNum) { wkModule = module;};
	void received(std::string pluginName, std::string moduleName, json_t *rootJ) override;
};

struct WK_205 : Module {
	static const int deviceCount = 5;
	enum ParamIds {
		NUM_PARAMS
	};
	enum InputIds {
		INPUT_CV_1,
		INPUT_CV_2,
		INPUT_CV_3,
		INPUT_CV_4,
		INPUT_CV_5,
		INPUT_TOR,
		NUM_INPUTS
	};
	enum OutputIds {
		OUTPUT_CV_1,
		OUTPUT_CV_2,
		OUTPUT_CV_3,
		OUTPUT_CV_4,
		OUTPUT_CV_5,
		NUM_OUTPUTS
	};
	enum LightIds {
		NUM_LIGHTS
	};
	float tunings[12];
	WK205_InputPort inPort = WK205_InputPort(this, INPUT_TOR);

	WK_205() : Module(NUM_PARAMS, NUM_INPUTS, NUM_OUTPUTS, NUM_LIGHTS) {}
	void step() override;
	json_t *toJson(void) override {
		json_t *rootJ = json_array();
		for (int i = 0; i < 12; i++)
			json_array_append_new(rootJ, json_real(tunings[i]));
		return rootJ;
	}
	void fromJson(json_t *rootJ) override {
		int size = json_array_size(rootJ);
		if (!size) return;
		if (size > 12)
			size = 12;
		for (int i = 0; i < size; i++) {
			json_t *j1 = json_array_get(rootJ, i);
			if (j1)
				tunings[i] = json_number_value(j1);
		}
	}
};

void WK_205::step() {
	for (int i = 0; i < deviceCount; i++) {
		int quantized = floor((12.0f * inputs[INPUT_CV_1 + i].value) + 0.5f);
		int note = (120 + quantized) % 12;
		outputs[OUTPUT_CV_1 + i].value = (tunings[note] / 1200.0f) + (quantized / 12.0f);	
	}
	inPort.process();
}

void WK205_InputPort::received(std::string pluginName, std::string moduleName, json_t *rootJ) {
	if (pluginName.compare(TOSTRING(SLUG))) return;
	if (moduleName.compare("WK")) return;
	int size = json_array_size(rootJ);
	if (!size) return;
	if (size > 12)
		size = 12;
	for (int i = 0; i < size; i++) {
		json_t *j1 = json_array_get(rootJ, i);
		if (j1)
			wkModule->tunings[i] = json_number_value(j1);
	}
}

struct WK205_MenuItem : MenuItem {
	WK_205 *module;
	int index;
	void onAction(EventAction &e) override {
		for (int i = 0; i < 12; i++)
			module->tunings[i] = tunings[index].offsets[i];
	}
};

struct WK205 : ModuleWidget {
	WK205(WK_205 *module) : ModuleWidget(module) {
		setPanel(SVG::load(assetPlugin(plugin, "res/WK-205.svg")));

		addInput(Port::create<BlackPort>(Vec(2.5,19), Port::INPUT, module, WK_205::INPUT_TOR));
		for (int i = 0; i < WK_205::deviceCount; i++) {
			addInput(Port::create<SilverPort>(Vec(2.5,63 + i * 60), Port::INPUT, module, WK_205::INPUT_CV_1 + i));
			addOutput(Port::create<SilverPort>(Vec(2.5,92 + i * 60), Port::OUTPUT, module, WK_205::OUTPUT_CV_1 + i));
		}

		WK_Tunings::loadTunings(plugin);
	}
	void appendContextMenu(Menu *menu) override;
};

void WK205::appendContextMenu(Menu *menu) {
	WK_205 *module = dynamic_cast<WK_205 *>(this->module);
	menu->addChild(MenuEntry::create());
	for (unsigned int i = 0; i < tunings.size(); i++) { 
		WK205_MenuItem *m = MenuItem::create<WK205_MenuItem>(tunings[i].name.c_str());
		m->module = module;
		m->index = i;
		menu->addChild(m);
	}
}

} // namespace rack_plugin_SubmarineFree

using namespace rack_plugin_SubmarineFree;

RACK_PLUGIN_MODEL_INIT(SubmarineFree, WK101) {
   Model *modelWK101 = Model::create<WK_101, WK101>("Submarine (Free)", "WK-101", "WK-101 Das Wohltemperierte Klavier", QUANTIZER_TAG, TUNER_TAG);
   return modelWK101;
}

RACK_PLUGIN_MODEL_INIT(SubmarineFree, WK205) {
   Model *modelWK205 = Model::create<WK_205, WK205>("Submarine (Free)", "WK-205", "WK-205 Das Wohltemperierte Klavier Nano", QUANTIZER_TAG, TUNER_TAG, MULTIPLE_TAG);
   return modelWK205;
}
