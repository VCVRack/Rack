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
#include "window.hpp"

using namespace std;

namespace rack_plugin_Bidoo {

struct OUAIVE : Module {
	enum ParamIds {
		NB_SLICES_PARAM,
		TRIG_MODE_PARAM,
		READ_MODE_PARAM,
		SPEED_PARAM,
		CVSLICES_PARAM,
		CVSPEED_PARAM,
		NUM_PARAMS
	};
	enum InputIds {
		GATE_INPUT,
		POS_INPUT,
		NB_SLICES_INPUT,
		READ_MODE_INPUT,
		SPEED_INPUT,
		POS_RESET_INPUT,
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
	unsigned int channels;
  unsigned int sampleRate;
  drwav_uint64 totalSampleCount;

	float samplePos = 0.0f;
	vector<vector<float>> playBuffer;
	string lastPath;
	string waveFileName;
	string waveExtension;
	bool loading = false;
	int trigMode = 0; // 0 trig 1 gate, 2 sliced
	int sliceIndex = -1;
	int sliceLength = 0;
	int nbSlices = 1;
	int readMode = 0; // 0 formward, 1 backward, 2 repeat
	float speed;
	SchmittTrigger playTrigger;
	SchmittTrigger trigModeTrigger;
	SchmittTrigger readModeTrigger;
	SchmittTrigger posResetTrigger;
	std::mutex mylock;


	OUAIVE() : Module(NUM_PARAMS, NUM_INPUTS, NUM_OUTPUTS, NUM_LIGHTS) {
		playBuffer.resize(2);
		playBuffer[0].resize(0);
		playBuffer[1].resize(0);
	}

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
	loading = true;
	unsigned int c;
  unsigned int sr;
  drwav_uint64 sc;
	float* pSampleData;
  pSampleData = drwav_open_and_read_file_f32(path.c_str(), &c, &sr, &sc);
  if (pSampleData != NULL)  {
		lastPath = path;
		waveFileName = stringFilename(path);
		waveExtension = stringExtension(path);
		channels = c;
		sampleRate = sr;
		playBuffer[0].clear();
		playBuffer[1].clear();
		for (unsigned int i=0; i < sc; i = i + c) {
			playBuffer[0].push_back(pSampleData[i]);
			if (channels == 2)
				playBuffer[1].push_back((float)pSampleData[i+1]);
			else
				playBuffer[1].push_back((float)pSampleData[i]);
		}
		totalSampleCount = playBuffer[0].size();
		drwav_free(pSampleData);
	}
	loading = false;
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
	nbSlices = clamp(roundl(params[NB_SLICES_PARAM].value + params[CVSLICES_PARAM].value * inputs[NB_SLICES_INPUT].value), 1, 128);
	speed = clamp(params[SPEED_PARAM].value + params[CVSPEED_PARAM].value * inputs[SPEED_INPUT].value, 0.2f, 10.0f);

	if (!loading) {
		sliceLength = clamp(totalSampleCount / nbSlices, 1, totalSampleCount);

		if ((trigMode == 0) && (playTrigger.process(inputs[GATE_INPUT].value))) {
			play = true;
			if (inputs[POS_INPUT].active)
				samplePos = clamp((int)(inputs[POS_INPUT].value * totalSampleCount * 0.1f), 0 , totalSampleCount - 1);
			else {
				if (readMode != 1)
					samplePos = 0;
				else
					samplePos = totalSampleCount - 1;
			}
		}	else if (trigMode == 1) {
			play = (inputs[GATE_INPUT].value > 0);
			samplePos = clamp((int)(inputs[POS_INPUT].value * totalSampleCount * 0.1f), 0 , totalSampleCount - 1);
		} else if ((trigMode == 2) && (playTrigger.process(inputs[GATE_INPUT].value))) {
			play = true;
			if (inputs[POS_INPUT].active)
				sliceIndex = clamp((int)(inputs[POS_INPUT].value * nbSlices * 0.1f), 0, nbSlices);
			 else
				sliceIndex = (sliceIndex+1)%nbSlices;
			if (readMode != 1)
				samplePos = clamp(sliceIndex*sliceLength, 0, totalSampleCount);
			else
				samplePos = clamp((sliceIndex + 1) * sliceLength - 1, 0 , totalSampleCount);
		}

		if (posResetTrigger.process(inputs[POS_RESET_INPUT].value)) {
			sliceIndex = 0;
			samplePos = 0;
		}

		if ((!loading) && (play) && (samplePos>=0) && (samplePos < totalSampleCount)) {
			if (channels == 1) {
				outputs[OUTL_OUTPUT].value = 5.0f * playBuffer[0][floor(samplePos)];
				outputs[OUTR_OUTPUT].value = 5.0f * playBuffer[0][floor(samplePos)];
			}
			else if (channels == 2) {
				if (outputs[OUTL_OUTPUT].active && outputs[OUTR_OUTPUT].active) {
					outputs[OUTL_OUTPUT].value = 5.0f * playBuffer[0][floor(samplePos)];
					outputs[OUTR_OUTPUT].value = 5.0f * playBuffer[1][floor(samplePos)];
				}
				else {
					outputs[OUTL_OUTPUT].value = 5.0f * (playBuffer[0][floor(samplePos)] + playBuffer[1][floor(samplePos)]);
					outputs[OUTR_OUTPUT].value = 5.0f * (playBuffer[0][floor(samplePos)] + playBuffer[1][floor(samplePos)]);
				}
			}

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
					samplePos = clamp((int)(inputs[POS_INPUT].value * totalSampleCount * 0.1f), 0 , totalSampleCount -1);
			}
			else if (trigMode == 2)
			{
				if (readMode != 1)
					samplePos = samplePos + speed * channels;
				else
					samplePos = samplePos - speed * channels;

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

struct OUAIVEDisplay : OpaqueWidget {
	OUAIVE *module;
	shared_ptr<Font> font;
	const float width = 125.0f;
	const float height = 50.0f;
	float zoomWidth = 125.0f;
	float zoomLeftAnchor = 0.0f;
	int refIdx = 0;
	float refX = 0.0f;

	OUAIVEDisplay() {
		font = Font::load(assetPlugin(plugin, "res/DejaVuSansMono.ttf"));
	}

	void onDragStart(EventDragStart &e) override {
		windowCursorLock();
		OpaqueWidget::onDragStart(e);
	}

	void onDragMove(EventDragMove &e) override {
		float zoom = 1.0f;
		if (e.mouseRel.y > 0.0f) {
			zoom = 1.0f/(windowIsShiftPressed() ? 2.0f : 1.1f);
		}
		else if (e.mouseRel.y < 0.0f) {
			zoom = windowIsShiftPressed() ? 2.0f : 1.1f;
		}
		zoomWidth = clamp(zoomWidth*zoom,width,zoomWidth*(windowIsShiftPressed() ? 2.0f : 1.1f));
		zoomLeftAnchor = clamp(refX - (refX - zoomLeftAnchor)*zoom + e.mouseRel.x, width - zoomWidth,0.0f);
		OpaqueWidget::onDragMove(e);
	}

	void onDragEnd(EventDragEnd &e) override {
		windowCursorUnlock();
		OpaqueWidget::onDragEnd(e);
	}

	void draw(NVGcontext *vg) override {
		module->mylock.lock();
		std::vector<float> vL(module->playBuffer[0]);
		std::vector<float> vR(module->playBuffer[1]);
		module->mylock.unlock();
		size_t nbSample = vL.size();

		nvgFontSize(vg, 14);
		nvgFillColor(vg, YELLOW_BIDOO);

		string trigMode = "";
		string slices = "";
		if (module->trigMode == 0) {
			trigMode = "TRIG ";
		}
		else if (module->trigMode==1)	{
			trigMode = "GATE ";
		}
		else {
			trigMode = "SLICE ";
			slices = "|" + to_string(module->nbSlices) + "|";
		}

		nvgTextBox(vg, 3, -15, 40, trigMode.c_str(), NULL);
		nvgTextBox(vg, 59, -15, 40, slices.c_str(), NULL);

		string readMode = "";
		if (module->readMode == 0) {
			readMode = "►";
		}
		else if (module->readMode == 2) {
			readMode = "►►";
		}
		else {
			readMode = "◄";
		}

		nvgTextBox(vg, 40, -15, 40, readMode.c_str(), NULL);

		stringstream stream;
		stream << fixed << setprecision(1) << module->speed;
		string s = stream.str();
		string speed = "x" + s;

		nvgTextBox(vg, 90, -15, 40, speed.c_str(), NULL);

		//Draw play line
		if ((module->play) && (!module->loading)) {
			nvgStrokeColor(vg, LIGHTBLUE_BIDOO);
			{
				nvgBeginPath(vg);
				nvgStrokeWidth(vg, 2);
				if (module->totalSampleCount>0) {
					nvgMoveTo(vg, module->samplePos * zoomWidth / nbSample + zoomLeftAnchor, 0);
					nvgLineTo(vg, module->samplePos * zoomWidth / nbSample + zoomLeftAnchor, 2 * height+10);
				}
				else {
					nvgMoveTo(vg, 0, 0);
					nvgLineTo(vg, 0, 2 * height+10);
				}
				nvgClosePath(vg);
			}
			nvgStroke(vg);
		}

		//Draw ref line
		nvgStrokeColor(vg, nvgRGBA(0xff, 0xff, 0xff, 0x30));
		nvgStrokeWidth(vg, 1);
		{
			nvgBeginPath(vg);
			nvgMoveTo(vg, 0, height * 0.5f);
			nvgLineTo(vg, width, height * 0.5f);
			nvgClosePath(vg);
		}
		nvgStroke(vg);

		nvgStrokeColor(vg, nvgRGBA(0xff, 0xff, 0xff, 0x30));
		nvgStrokeWidth(vg, 1);
		{
			nvgBeginPath(vg);
			nvgMoveTo(vg, 0, 3*height * 0.5f + 10);
			nvgLineTo(vg, width, 3*height * 0.5f + 10);
			nvgClosePath(vg);
		}
		nvgStroke(vg);

		if ((!module->loading) && (vL.size()>0)) {
			//Draw waveform
			nvgStrokeColor(vg, PINK_BIDOO);
			nvgSave(vg);
			Rect b = Rect(Vec(zoomLeftAnchor, 0), Vec(zoomWidth, height));
			nvgScissor(vg, 0, b.pos.y, width, height);
			nvgBeginPath(vg);
			for (size_t i = 0; i < vL.size(); i++) {
				float x, y;
				x = (float)i/vL.size();
				y = vL[i] / 2.0f + 0.5f;
				Vec p;
				p.x = b.pos.x + b.size.x * x;
				p.y = b.pos.y + b.size.y * (1.0f - y);
				if (i == 0) {
					nvgMoveTo(vg, p.x, p.y);
				}
				else {
					nvgLineTo(vg, p.x, p.y);
				}
			}
			nvgLineCap(vg, NVG_MITER);
			nvgStrokeWidth(vg, 1);
			nvgGlobalCompositeOperation(vg, NVG_LIGHTER);
			nvgStroke(vg);

			b = Rect(Vec(zoomLeftAnchor, height+10), Vec(zoomWidth, height));
			nvgScissor(vg, 0, b.pos.y, width, height);
			nvgBeginPath(vg);
			for (size_t i = 0; i < vR.size(); i++) {
				float x, y;
				x = (float)i/vR.size();
				y = vR[i] / 2.0f + 0.5f;
				Vec p;
				p.x = b.pos.x + b.size.x * x;
				p.y = b.pos.y + b.size.y * (1.0f - y);
				if (i == 0)
					nvgMoveTo(vg, p.x, p.y);
				else
					nvgLineTo(vg, p.x, p.y);
			}
			nvgLineCap(vg, NVG_MITER);
			nvgStrokeWidth(vg, 1);
			nvgGlobalCompositeOperation(vg, NVG_LIGHTER);
			nvgStroke(vg);
			nvgResetScissor(vg);

			//draw slices

			if (module->trigMode == 2) {
				nvgScissor(vg, 0, 0, width, 2*height+10);
				for (int i = 1; i < module->nbSlices; i++) {
					nvgStrokeColor(vg, YELLOW_BIDOO);
					nvgStrokeWidth(vg, 1);
					{
						nvgBeginPath(vg);
						nvgMoveTo(vg, (int)(i * module->sliceLength * zoomWidth / nbSample + zoomLeftAnchor) , 0);
						nvgLineTo(vg, (int)(i * module->sliceLength * zoomWidth / nbSample + zoomLeftAnchor) , 2*height+10);
						nvgClosePath(vg);
					}
					nvgStroke(vg);
				}
				nvgResetScissor(vg);
			}

			nvgRestore(vg);
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
			display->box.pos = Vec(5, 70);
			display->box.size = Vec(125, 110);
			addChild(display);
		}

		static const float portX0[4] = {34, 67, 101};

		addInput(Port::create<TinyPJ301MPort>(Vec(10, 18), Port::INPUT, module, OUAIVE::POS_RESET_INPUT));

		addParam(ParamWidget::create<BlueCKD6>(Vec(portX0[0]-25, 215), module, OUAIVE::TRIG_MODE_PARAM, 0.0, 2.0, 0.0));

		addParam(ParamWidget::create<BlueCKD6>(Vec(portX0[1]-14, 215), module, OUAIVE::READ_MODE_PARAM, 0.0, 2.0, 0.0));
		addInput(Port::create<TinyPJ301MPort>(Vec(portX0[2]+5, 222), Port::INPUT, module, OUAIVE::READ_MODE_INPUT));

		addParam(ParamWidget::create<BidooBlueTrimpot>(Vec(portX0[1]-9, 250), module, OUAIVE::NB_SLICES_PARAM, 1.0, 128.01, 1.0));
		addParam(ParamWidget::create<BidooBlueTrimpot>(Vec(portX0[1]+15, 250), module, OUAIVE::CVSLICES_PARAM, -1.0f, 1.0f, 0.0f));
		addInput(Port::create<TinyPJ301MPort>(Vec(portX0[2]+5, 252), Port::INPUT, module, OUAIVE::NB_SLICES_INPUT));

		addParam(ParamWidget::create<BidooBlueTrimpot>(Vec(portX0[1]-9, 275), module, OUAIVE::SPEED_PARAM, -0.05, 10, 1.0));
		addParam(ParamWidget::create<BidooBlueTrimpot>(Vec(portX0[1]+15, 275), module, OUAIVE::CVSPEED_PARAM, -1.0f, 1.0f, 0.0f));
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
