#include "Bidoo.hpp"
#include "dsp/digital.hpp"
#include "BidooComponents.hpp"
#include "osdialog.h"
#define DR_WAV_IMPLEMENTATION
#include "dep/dr_wav/dr_wav.h"
#include <vector>
#include "cmath"
#include <iomanip> // setprecision
#include <sstream> // stringstream

using namespace std;

namespace rack_plugin_Bidoo {

struct OUAIVE : Module {
	enum ParamIds {
		NB_SLICES_PARAM,
		TRIG_MODE_PARAM,
		READ_MODE_PARAM,
		SPEED_PARAM,
		NUM_PARAMS
	};
	enum InputIds {
		GATE_INPUT,
		POS_INPUT,
		NB_SLICES_INPUT,
		READ_MODE_INPUT,
		SPEED_INPUT,
		NUM_INPUTS
	};
	enum OutputIds {
		OUTL_OUTPUT,
		OUTR_OUTPUT,
		NUM_OUTPUTS
	};
	enum LightIds {
		NUM_LIGHTS
	};

	bool play = false;
	string lastPath;
	unsigned int channels;
  unsigned int sampleRate;
  drwav_uint64 totalSampleCount;
	float* pSampleData;
	float samplePos = 0.0f;
	vector<double> displayBuffL;
	vector<double> displayBuffR;
	string fileDesc;
	bool fileLoaded = false;
	int trigMode = 0; // 0 trig 1 gate, 2 sliced
	int sliceIndex = -1;
	int sliceLength = 0;
	int nbSlices = 1;
	int readMode = 0; // 0 formward, 1 backward, 2 repeat
	float speed;
	string displayParams = "";
	string displayReadMode = "";
	string displaySlices = "";
	string displaySpeed;
	SchmittTrigger playTrigger;
	SchmittTrigger trigModeTrigger;
	SchmittTrigger readModeTrigger;
	std::mutex mylock;


	OUAIVE() : Module(NUM_PARAMS, NUM_INPUTS, NUM_OUTPUTS, NUM_LIGHTS) {	}

	void step() override;

	void loadSample(std::string path);

	// persistence

	json_t *toJson() override {
		json_t *rootJ = json_object();
		// lastPath
		json_object_set_new(rootJ, "lastPath", json_string(lastPath.c_str()));
		json_object_set_new(rootJ, "trigMode", json_integer(trigMode));
		json_object_set_new(rootJ, "readMode", json_integer(readMode));
		return rootJ;
	}

	void fromJson(json_t *rootJ) override {
		// lastPath
		json_t *lastPathJ = json_object_get(rootJ, "lastPath");
		if (lastPathJ) {
			lastPath = json_string_value(lastPathJ);
			loadSample(lastPath);
		}
		json_t *trigModeJ = json_object_get(rootJ, "trigMode");
		if (trigModeJ) {
			trigMode = json_integer_value(trigModeJ);
		}
		json_t *readModeJ = json_object_get(rootJ, "readMode");
		if (readModeJ) {
			readMode = json_integer_value(readModeJ);
		}
	}
};

void OUAIVE::loadSample(std::string path) {
	mylock.lock();
	fileLoaded = false;
	drwav_free(pSampleData);
  pSampleData = drwav_open_and_read_file_f32(path.c_str(), &channels, &sampleRate, &totalSampleCount);
  if (pSampleData == NULL) {
      fileLoaded = false;
  }
	else {
		vector<double>().swap(displayBuffL);
		vector<double>().swap(displayBuffR);
		for (unsigned int i=0; i < totalSampleCount; i = i + floor(totalSampleCount/125)) {
			displayBuffL.push_back(pSampleData[i]);
			if (channels == 2)
				displayBuffR.push_back(pSampleData[i+1]);
		}
		fileDesc = (stringFilename(path)).substr(0,20) + ((stringFilename(path)).length() >=20  ? "...\n" :  "\n");
		fileDesc += std::to_string(sampleRate) + " Hz\n";
		fileLoaded = true;
	}
	mylock.unlock();
}

void OUAIVE::step() {
	if (trigModeTrigger.process(params[TRIG_MODE_PARAM].value)) {
		trigMode = (((int)trigMode + 1) % 3);
	}
	if (inputs[READ_MODE_INPUT].active) {
		readMode = round(rescale(inputs[READ_MODE_INPUT].value, 0.0f,10.0f,0.0f,2.0f));
	} else if (readModeTrigger.process(params[READ_MODE_PARAM].value + inputs[READ_MODE_INPUT].value)) {
		readMode = (((int)readMode + 1) % 3);
	}
	nbSlices = clamp(roundl(params[NB_SLICES_PARAM].value + inputs[NB_SLICES_INPUT].value), 1, 128);
	speed = clamp(params[SPEED_PARAM].value + inputs[SPEED_INPUT].value, 0.2f, 10.0f);
	stringstream stream;
	stream << fixed << setprecision(1) << speed;
	string s = stream.str();
	displaySpeed = "x" + s;
	if (trigMode == 0) {
		displayParams = "TRIG";
		if (readMode == 0) {
			displayReadMode = "►";
		} else if (readMode == 2) {
			displayReadMode = "►►";
		}
		else {
			displayReadMode = "◄";
		}
	}	else if (trigMode == 1) {
		displayParams = "GATE";
	} else if (trigMode == 2) {
		displayParams = "SLICE ";
		displaySlices = "|" + std::to_string(nbSlices) + "|";
		if (readMode == 0) {
			displayReadMode = "►";
		} else if (readMode == 2) {
			displayReadMode = "►►";
		}
		else {
			displayReadMode = "◄";
		}
	}


	if (fileLoaded) {
		sliceLength = clamp(totalSampleCount / nbSlices, 1, totalSampleCount);

		if ((trigMode == 0) && (playTrigger.process(inputs[GATE_INPUT].value))) {
			play = true;
			samplePos = clamp((int)(inputs[POS_INPUT].value * totalSampleCount / 10), 0 , totalSampleCount - 1);
		}	else if (trigMode == 1) {
			play = (inputs[GATE_INPUT].value > 0);
			samplePos = clamp((int)(inputs[POS_INPUT].value * totalSampleCount / 10), 0 , totalSampleCount - 1);
		} else if ((trigMode == 2) && (playTrigger.process(inputs[GATE_INPUT].value))) {
			play = true;
			if (inputs[POS_INPUT].active)
				sliceIndex = clamp((int)(inputs[POS_INPUT].value * nbSlices / 10), 0, nbSlices);
			 else
				sliceIndex = (sliceIndex+1)%nbSlices;
			if (readMode != 1)
				samplePos = clamp(sliceIndex*sliceLength, 0, totalSampleCount);
			else
				samplePos = clamp((sliceIndex + 1) * sliceLength - 1, 0 , totalSampleCount);
		}

		if ((play) && (samplePos>=0) && (samplePos < totalSampleCount)) {
			mylock.lock();
			//calulate outputs
			if (channels == 1) {
				outputs[OUTL_OUTPUT].value = 10.0f * pSampleData[(unsigned int)floor(samplePos)];
				outputs[OUTR_OUTPUT].value = 10.0f * pSampleData[(unsigned int)floor(samplePos)];
			}
			else if (channels == 2) {
				if (outputs[OUTL_OUTPUT].active && outputs[OUTR_OUTPUT].active) {
					outputs[OUTL_OUTPUT].value = 10.0f * pSampleData[(unsigned int)floor(samplePos)];
					outputs[OUTR_OUTPUT].value = 10.0f * pSampleData[(unsigned int)floor(samplePos)+1];
				}
				else {
					outputs[OUTL_OUTPUT].value = 10.0f * (pSampleData[(unsigned int)floor(samplePos)] + pSampleData[(unsigned int)floor(samplePos)+1]) / 2;
					outputs[OUTR_OUTPUT].value = 10.0f * (pSampleData[(unsigned int)floor(samplePos)] + pSampleData[(unsigned int)floor(samplePos)+1]) / 2;
				}
			}
			mylock.unlock();

			//shift samplePos
			if (trigMode == 0) {
				if (readMode != 1)
					samplePos = samplePos + speed * channels;
				else
					samplePos = samplePos - speed * channels;
				//manage eof readMode
				if ((readMode == 0) && (samplePos >= totalSampleCount))
						play = false;
				else if ((readMode == 1) && (samplePos <=0))
						play = false;
				else if ((readMode == 2) && (samplePos >= totalSampleCount))
					samplePos = clamp((int)(inputs[POS_INPUT].value * totalSampleCount / 10), 0 , totalSampleCount -1);
			}
			else if (trigMode == 2)
			{
				if (readMode != 1)
					samplePos = samplePos + speed * channels;
				else
					samplePos = samplePos - speed * channels;
				//update diplay slices
				displaySlices = "|" + std::to_string(nbSlices) + "|";
				//manage eof readMode
				if ((readMode == 0) && ((samplePos >= (sliceIndex+1) * sliceLength) || (samplePos >= totalSampleCount)))
						play = false;
				if ((readMode == 1) && ((samplePos <= (sliceIndex) * sliceLength) || (samplePos <= 0)))
						play = false;
				if ((readMode == 2) && ((samplePos >= (sliceIndex+1) * sliceLength) || (samplePos >= totalSampleCount)))
					samplePos = clamp(sliceIndex*sliceLength, 0 , totalSampleCount);
			}
		}
		else if (samplePos == totalSampleCount)
			play = false;
	}
}

struct OUAIVEDisplay : TransparentWidget {
	OUAIVE *module;
	int frame = 0;
	shared_ptr<Font> font;
	string displayParams;

	OUAIVEDisplay() {
		font = Font::load(assetPlugin(plugin, "res/DejaVuSansMono.ttf"));
	}

	void draw(NVGcontext *vg) override {
		nvgFontSize(vg, 12);
		nvgFontFaceId(vg, font->handle);
		nvgStrokeWidth(vg, 1);
		nvgTextLetterSpacing(vg, -2);
		nvgFillColor(vg, YELLOW_BIDOO);
		nvgTextBox(vg, 5, 3,120, module->fileDesc.c_str(), NULL);

		nvgFontSize(vg, 14);
		nvgFillColor(vg, YELLOW_BIDOO);
		nvgTextBox(vg, 5, 55,40, module->displayParams.c_str(), NULL);
		if (module->trigMode != 1)
			nvgTextBox(vg, 95, 55,30, module->displaySpeed.c_str(), NULL);

		if (module->trigMode == 0) {
			nvgTextBox(vg, 45, 55,20, module->displayReadMode.c_str(), NULL);
		}

		if (module->trigMode == 2) {
			nvgTextBox(vg, 45, 55,20, module->displayReadMode.c_str(), NULL);
			nvgTextBox(vg, 62, 55,40, module->displaySlices.c_str(), NULL);
		}


		if (module->fileLoaded) {
				// Draw play line
				nvgStrokeColor(vg, LIGHTBLUE_BIDOO);
				{
					nvgBeginPath(vg);
					nvgStrokeWidth(vg, 2);
					nvgMoveTo(vg, (int)(module->samplePos * 125 / module->totalSampleCount) , 70);
					nvgLineTo(vg, (int)(module->samplePos * 125 / module->totalSampleCount) , 150);
					nvgClosePath(vg);
				}
				nvgStroke(vg);

				if (module->channels == 1) {
					// Draw ref line
					nvgStrokeColor(vg, nvgRGBA(0xff, 0xff, 0xff, 0x30));
					nvgStrokeWidth(vg, 1);
					{
						nvgBeginPath(vg);
						nvgMoveTo(vg, 0, 110);
						nvgLineTo(vg, 130, 110);
						nvgClosePath(vg);
					}
					nvgStroke(vg);

					// Draw waveform
					nvgStrokeColor(vg, PINK_BIDOO);
					nvgSave(vg);
					Rect b = Rect(Vec(0, 70), Vec(125, 80));
					nvgScissor(vg, b.pos.x, b.pos.y, b.size.x, b.size.y);
					nvgBeginPath(vg);
					for (unsigned int i = 0; i < module->displayBuffL.size(); i++) {
						float x, y;
						x = (float)i / (module->displayBuffL.size() - 1.0f);
						y = module->displayBuffL[i] / 2.0f + 0.5f;
						Vec p;
						p.x = b.pos.x + b.size.x * x;
						p.y = b.pos.y + b.size.y * (1.0f - y);
						if (i == 0)
							nvgMoveTo(vg, p.x, p.y);
						else
							nvgLineTo(vg, p.x, p.y);
					}
					nvgLineCap(vg, NVG_ROUND);
					nvgMiterLimit(vg, 2.0);
					nvgStrokeWidth(vg, 1);
					nvgGlobalCompositeOperation(vg, NVG_LIGHTER);
					nvgStroke(vg);
					nvgResetScissor(vg);
					nvgRestore(vg);
				}
				else {
					// Draw ref line
					nvgStrokeColor(vg, nvgRGBA(0xff, 0xff, 0xff, 0x30));
					{
						nvgBeginPath(vg);
						nvgMoveTo(vg, 0, 90);
						nvgLineTo(vg, 130, 90);
						nvgMoveTo(vg, 0, 130);
						nvgLineTo(vg, 130, 130);
						nvgClosePath(vg);
					}
					nvgStroke(vg);

					// Draw waveform
					nvgStrokeColor(vg, PINK_BIDOO);
					nvgSave(vg);
					Rect b = Rect(Vec(0, 70), Vec(125, 40));
					nvgScissor(vg, b.pos.x, b.pos.y, b.size.x, b.size.y);
					nvgBeginPath(vg);
					for (unsigned int i = 0; i < module->displayBuffL.size(); i++) {
						float x, y;
						x = (float)i / (module->displayBuffL.size() - 1.0f);
						y = module->displayBuffL[i] / 2.0f + 0.5f;
						Vec p;
						p.x = b.pos.x + b.size.x * x;
						p.y = b.pos.y + b.size.y * (1.0f - y);
						if (i == 0)
							nvgMoveTo(vg, p.x, p.y);
						else
							nvgLineTo(vg, p.x, p.y);
					}
					nvgLineCap(vg, NVG_ROUND);
					nvgMiterLimit(vg, 2.0);
					nvgStrokeWidth(vg, 1);
					nvgGlobalCompositeOperation(vg, NVG_LIGHTER);
					nvgStroke(vg);

					b = Rect(Vec(0, 110), Vec(125, 40));
					nvgScissor(vg, b.pos.x, b.pos.y, b.size.x, b.size.y);
					nvgBeginPath(vg);
					for (unsigned int i = 0; i < module->displayBuffR.size(); i++) {
						float x, y;
						x = (float)i / (module->displayBuffR.size() - 1.0f);
						y = module->displayBuffR[i] / 2.0f + 0.5f;
						Vec p;
						p.x = b.pos.x + b.size.x * x;
						p.y = b.pos.y + b.size.y * (1.0f - y);
						if (i == 0)
							nvgMoveTo(vg, p.x, p.y);
						else
							nvgLineTo(vg, p.x, p.y);
					}
					nvgLineCap(vg, NVG_ROUND);
					nvgMiterLimit(vg, 2.0f);
					nvgStrokeWidth(vg, 1);
					nvgGlobalCompositeOperation(vg, NVG_LIGHTER);
					nvgStroke(vg);
					nvgResetScissor(vg);
					nvgRestore(vg);
				}

			//draw slices
			if (module->trigMode == 2) {
				for (int i = 1; i < module->nbSlices; i++) {
					nvgStrokeColor(vg, YELLOW_BIDOO);
					{
						nvgBeginPath(vg);
						nvgStrokeWidth(vg, 1);
						nvgMoveTo(vg, (int)(i * module->sliceLength * 125 / module->totalSampleCount) , 70);
						nvgLineTo(vg, (int)(i * module->sliceLength * 125 / module->totalSampleCount) , 150);
						nvgClosePath(vg);
					}
					nvgStroke(vg);
				}
			}
		}
	}
};

struct OUAIVEWidget : ModuleWidget {
	Menu *createContextMenu() override;

	OUAIVEWidget(OUAIVE *module) : ModuleWidget(module) {
		setPanel(SVG::load(assetPlugin(plugin, "res/OUAIVE.svg")));

		addChild(Widget::create<ScrewSilver>(Vec(RACK_GRID_WIDTH, 0)));
		addChild(Widget::create<ScrewSilver>(Vec(box.size.x - 2 * RACK_GRID_WIDTH, 0)));
		addChild(Widget::create<ScrewSilver>(Vec(RACK_GRID_WIDTH, RACK_GRID_HEIGHT - RACK_GRID_WIDTH)));
		addChild(Widget::create<ScrewSilver>(Vec(box.size.x - 2 * RACK_GRID_WIDTH, RACK_GRID_HEIGHT - RACK_GRID_WIDTH)));

		{
			OUAIVEDisplay *display = new OUAIVEDisplay();
			display->module = module;
			display->box.pos = Vec(5, 40);
			display->box.size = Vec(130, 250);
			addChild(display);
		}

		static const float portX0[4] = {34, 67, 101};

		addParam(ParamWidget::create<BlueCKD6>(Vec(portX0[0]-25, 215), module, OUAIVE::TRIG_MODE_PARAM, 0.0, 2.0, 0.0));

		addParam(ParamWidget::create<BlueCKD6>(Vec(portX0[1]-14, 215), module, OUAIVE::READ_MODE_PARAM, 0.0, 2.0, 0.0));
		addInput(Port::create<TinyPJ301MPort>(Vec(portX0[2]+5, 222), Port::INPUT, module, OUAIVE::READ_MODE_INPUT));

		addParam(ParamWidget::create<BidooBlueTrimpot>(Vec(portX0[1]-9, 250), module, OUAIVE::NB_SLICES_PARAM, 1.0, 128.01, 1.0));
		addInput(Port::create<TinyPJ301MPort>(Vec(portX0[2]+5, 252), Port::INPUT, module, OUAIVE::NB_SLICES_INPUT));

		addParam(ParamWidget::create<BidooBlueTrimpot>(Vec(portX0[1]-9, 275), module, OUAIVE::SPEED_PARAM, -0.05, 10, 1.0));
		addInput(Port::create<TinyPJ301MPort>(Vec(portX0[2]+5, 277), Port::INPUT, module, OUAIVE::SPEED_INPUT));

		addInput(Port::create<PJ301MPort>(Vec(portX0[0]-25, 321), Port::INPUT, module, OUAIVE::GATE_INPUT));
		addInput(Port::create<PJ301MPort>(Vec(portX0[1]-19, 321), Port::INPUT, module, OUAIVE::POS_INPUT));
		addOutput(Port::create<TinyPJ301MPort>(Vec(portX0[2]-13, 331), Port::OUTPUT, module, OUAIVE::OUTL_OUTPUT));
		addOutput(Port::create<TinyPJ301MPort>(Vec(portX0[2]+11, 331), Port::OUTPUT, module, OUAIVE::OUTR_OUTPUT));
	}
};

struct OUAIVEItem : MenuItem {
	OUAIVE *ouaive;
	void onAction(EventAction &e) override {

		std::string dir = ouaive->lastPath.empty() ? assetLocal("") : stringDirectory(ouaive->lastPath);
		char *path = osdialog_file(OSDIALOG_OPEN, dir.c_str(), NULL, NULL);
		if (path) {
			ouaive->play = false;
			ouaive->fileLoaded = false;
			ouaive->loadSample(path);
			ouaive->samplePos = 0;
			ouaive->lastPath = path;
			ouaive->sliceIndex = -1;
			free(path);
		}
	}
};

Menu *OUAIVEWidget::createContextMenu() {
	Menu *menu = ModuleWidget::createContextMenu();

	MenuLabel *spacerLabel = new MenuLabel();
	menu->addChild(spacerLabel);

	OUAIVE *ouaive = dynamic_cast<OUAIVE*>(module);
	assert(ouaive);

	OUAIVEItem *sampleItem = new OUAIVEItem();
	sampleItem->text = "Load sample";
	sampleItem->ouaive = ouaive;
	menu->addChild(sampleItem);

	return menu;
}

} // namespace rack_plugin_Bidoo

using namespace rack_plugin_Bidoo;

RACK_PLUGIN_MODEL_INIT(Bidoo, OUAIVE) {
   Model *modelOUAIVE = Model::create<OUAIVE, OUAIVEWidget>("Bidoo","OUAIve", "OUAIve player", SAMPLER_TAG, GRANULAR_TAG);
   return modelOUAIVE;
}
