#include "Bidoo.hpp"
#include "dsp/digital.hpp"
#include "BidooComponents.hpp"
#include "osdialog.h"
#include <vector>
#include "cmath"
#include "dep/lodepng/lodepng.h"
#include "dep/filters/fftsynth.h"
#include "dsp/ringbuffer.hpp"
#include <algorithm>

#define FFT_SIZE 8192
#define STEPS 8
using namespace std;

namespace rack_plugin_Bidoo {

inline double fastPow(double a, double b) {
  union {
    double d;
    int x[2];
  } u = { a };
  u.x[1] = (int)(b * (u.x[1] - 1072632447) + 1072632447);
  u.x[0] = 0;
  return u.d;
}

struct EMILE : Module {
	enum ParamIds {
		SPEED_PARAM,
		CURVE_PARAM,
    GAIN_PARAM,
		NUM_PARAMS
	};
	enum InputIds {
		GATE_INPUT,
		CURVE_INPUT,
		NUM_INPUTS
	};
	enum OutputIds {
		OUT,
		NUM_OUTPUTS
	};
	enum LightIds {
		NUM_LIGHTS
	};

  FftSynth *synth;
	string lastPath;
	bool loading = false;
	std::vector<unsigned char> image;
  unsigned width = 0;
	unsigned height = 0;
	unsigned samplePos = 0;
	int delay = 0;
  bool play = false;
  SchmittTrigger playTrigger;
  float *magn;
  float *phas;
  DoubleRingBuffer<float,FFT_SIZE/STEPS> outBuffer;
  long fftSize2 = FFT_SIZE / 2;

	EMILE() : Module(NUM_PARAMS, NUM_INPUTS, NUM_OUTPUTS, NUM_LIGHTS) {
    synth = new FftSynth(FFT_SIZE, STEPS, engineGetSampleRate());
    magn = (float*)calloc(fftSize2,sizeof(float));
    phas = (float*)calloc(fftSize2,sizeof(float));
	}

  ~EMILE() {
		delete synth;
    free(magn);
		free(phas);
	}

	void step() override;

	void loadSample(std::string path);

	// persistence

	json_t *toJson() override {
		json_t *rootJ = json_object();
		// lastPath
		json_object_set_new(rootJ, "lastPath", json_string(lastPath.c_str()));
		return rootJ;
	}

	void fromJson(json_t *rootJ) override {
		// lastPath
		json_t *lastPathJ = json_object_get(rootJ, "lastPath");
		if (lastPathJ) {
			lastPath = json_string_value(lastPathJ);
			loadSample(lastPath);
		}
	}

};

void EMILE::loadSample(std::string path) {
	loading = true;
  image.clear();
	unsigned error = lodepng::decode(image, width, height, path, LCT_RGB);
	if(error != 0)
  {
    std::cout << "error " << error << ": " << lodepng_error_text(error) << std::endl;
		lastPath = "";
	}
  else {
    lastPath = path;
    samplePos = 0;
  }
	loading = false;
}

void EMILE::step() {

  if (playTrigger.process(inputs[GATE_INPUT].value)) {
    play = true;
    samplePos = 0;
  }
  memset(phas, 0, fftSize2*sizeof(float));
  memset(magn, 0, fftSize2*sizeof(float));

	if (play && !loading && (lastPath != "")) {
    if (outBuffer.size() == 0) {
      float iHeight = 1.0f/height;
      for (unsigned i = 0; i < height; i++) {
        float volume = (0.33f * image[samplePos * 3 + (height - i - 1) * width * 3] + 0.5f * image[samplePos * 3 + (height - i - 1) * width * 3 + 1] +  0.16f * image[samplePos * 3 + (height - i - 1) * width * 3 + 2]) / 255.0f;
  			if (volume > 0.0f) {
          float fact = fastPow(10.0f,i*params[CURVE_PARAM].value/height)*0.1f;
          size_t index = clamp((int)(i * fact * fftSize2 * iHeight * 0.5f),0,fftSize2/2);
          magn[index] = clamp(volume,0.0f,1.0f);
  			}
  		}

      synth->process(magn,phas,outBuffer.endData());
      outBuffer.endIncr(FFT_SIZE/STEPS);
    }

		outputs[OUT].value = params[GAIN_PARAM].value * clamp(*outBuffer.startData() * 10.0f,-10.0f,10.0f);
    outBuffer.startIncr(1);

		delay++;

		if (samplePos >= width) {
      samplePos = 0;
      play = false;
    }
		else {
			if (delay>params[SPEED_PARAM].value)
			{
				samplePos++;
				delay = 0;
			}
		}

	}
}

struct EMILEDisplay : OpaqueWidget {
	EMILE *module;
	shared_ptr<Font> font;
	const float width = 125.0f;
	const float height = 130.0f;
	string path = "";
	bool first = true;
	int img = 0;

	EMILEDisplay() {
		font = Font::load(assetPlugin(plugin, "res/DejaVuSansMono.ttf"));
	}

	void draw(NVGcontext *vg) override {
		if (!module->loading) {

			if (path != module->lastPath) {
				img = nvgCreateImage(vg, module->lastPath.c_str(), 0);
				path = module->lastPath;
			}

			nvgBeginPath(vg);
			if (module->width>0 && module->height>0)
				nvgScale(vg, width/module->width, height/module->height);
		 	NVGpaint imgPaint = nvgImagePattern(vg, 0, 0, module->width,module->height, 0, img, 1.0f);
		 	nvgRect(vg, 0, 0, module->width, module->height);
		 	nvgFillPaint(vg, imgPaint);
		 	nvgFill(vg);
			nvgClosePath(vg);
		}
	}
};

struct EMILEPositionDisplay : OpaqueWidget {
	EMILE *module;
	shared_ptr<Font> font;
	const float width = 125.0f;
	const float height = 130.0f;
	string path = "";
	bool first = true;
	int img = 0;

	EMILEPositionDisplay() {
		font = Font::load(assetPlugin(plugin, "res/DejaVuSansMono.ttf"));
	}

	void draw(NVGcontext *vg) override {
		if (!module->loading) {
			nvgStrokeColor(vg, LIGHTBLUE_BIDOO);
			{
				nvgBeginPath(vg);
				nvgStrokeWidth(vg, 2);
				if (module->image.size()>0) {
					nvgMoveTo(vg, (float)module->samplePos * width / module->width, 0);
					nvgLineTo(vg, (float)module->samplePos * width / module->width, height);
				}
				else {
					nvgMoveTo(vg, 0, 0);
					nvgLineTo(vg, 0, height);
				}
				nvgClosePath(vg);
			}
			nvgStroke(vg);
		}
	}
};

struct EMILEWidget : ModuleWidget {
	Menu *createContextMenu() override;

	EMILEWidget(EMILE *module) : ModuleWidget(module) {
		setPanel(SVG::load(assetPlugin(plugin, "res/EMILE.svg")));

		addChild(Widget::create<ScrewSilver>(Vec(RACK_GRID_WIDTH, 0)));
		addChild(Widget::create<ScrewSilver>(Vec(box.size.x - 2 * RACK_GRID_WIDTH, 0)));
		addChild(Widget::create<ScrewSilver>(Vec(RACK_GRID_WIDTH, RACK_GRID_HEIGHT - RACK_GRID_WIDTH)));
		addChild(Widget::create<ScrewSilver>(Vec(box.size.x - 2 * RACK_GRID_WIDTH, RACK_GRID_HEIGHT - RACK_GRID_WIDTH)));

		{
			EMILEDisplay *display = new EMILEDisplay();
			display->module = module;
			display->box.pos = Vec(5, 30);
			display->box.size = Vec(125, 130);
			addChild(display);
		}

		{
			EMILEPositionDisplay *display = new EMILEPositionDisplay();
			display->module = module;
			display->box.pos = Vec(5, 30);
			display->box.size = Vec(125, 130);
			addChild(display);
		}

		static const float portX0[4] = {34, 67, 101};

    addParam(ParamWidget::create<BidooBlueKnob>(Vec(portX0[1]-15, 180), module, EMILE::GAIN_PARAM, 0.1f, 10.0f, 1.0f));
		addParam(ParamWidget::create<BidooBlueKnob>(Vec(portX0[1]-15, 235), module, EMILE::CURVE_PARAM, 0.5f, 8.0f, 1.0f));
		addParam(ParamWidget::create<BidooBlueKnob>(Vec(portX0[1]-15, 290), module, EMILE::SPEED_PARAM, 200.0f, 1.0f, 50.0f));
		addInput(Port::create<PJ301MPort>(Vec(portX0[0]-25, 321), Port::INPUT, module, EMILE::GATE_INPUT));

		addOutput(Port::create<PJ301MPort>(Vec(portX0[2], 321), Port::OUTPUT, module, EMILE::OUT));
	}
};

struct EMILEItem : MenuItem {
	EMILE *emile;
	void onAction(EventAction &e) override {

		std::string dir = emile->lastPath.empty() ? assetLocal("") : stringDirectory(emile->lastPath);
		char *path = osdialog_file(OSDIALOG_OPEN, dir.c_str(), NULL, NULL);
		if (path) {
			emile->loadSample(path);
			free(path);
		}
	}
};

Menu *EMILEWidget::createContextMenu() {
	Menu *menu = ModuleWidget::createContextMenu();

	MenuLabel *spacerLabel = new MenuLabel();
	menu->addChild(spacerLabel);

	EMILE *emile = dynamic_cast<EMILE*>(module);
	assert(emile);

	EMILEItem *sampleItem = new EMILEItem();
	sampleItem->text = "Load image (png)";
	sampleItem->emile = emile;
	menu->addChild(sampleItem);

	return menu;
}

} // namespace rack_plugin_Bidoo

using namespace rack_plugin_Bidoo;

RACK_PLUGIN_MODEL_INIT(Bidoo, EMILE) {
   Model *modelEMILE = Model::create<EMILE, EMILEWidget>("Bidoo","EMILE", "EMILE png player", SAMPLER_TAG, GRANULAR_TAG);
   return modelEMILE;
}
