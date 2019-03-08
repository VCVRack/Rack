#include "Bidoo.hpp"
#include "dsp/resampler.hpp"
#include "dsp/filter.hpp"
//#include "dep/pffft/pffft.h"
#include "pffft.h"
#include "dep/filters/fftanalysis.h"
#include <iostream>
#include <vector>
#include <algorithm>
#include <limits>
#include "dsp/ringbuffer.hpp"

using namespace std;

namespace rack_plugin_Bidoo {

const int N = 4096;

struct GARCON : Module {
	enum ParamIds {
		NUM_PARAMS
	};
	enum InputIds {
		INPUT,
		NUM_INPUTS
	};
	enum OutputIds {
		// OUT_OUTPUT,
		NUM_OUTPUTS
	};
	enum LightIds {
		NUM_LIGHTS
	};

	FfftAnalysis *processor;
	vector<vector<float>> fft;
	DoubleRingBuffer<float,N> in_Buffer;
	std::mutex mylock;

	GARCON() : Module(NUM_PARAMS, NUM_INPUTS, NUM_OUTPUTS, NUM_LIGHTS) {
		processor = new FfftAnalysis(N, 4, engineGetSampleRate());
	}

	~GARCON() {
		delete processor;
	}

	void step() override;
};

void GARCON::step() {
	in_Buffer.push(inputs[INPUT].value/10.0f);
	if (in_Buffer.full()) {
		processor->process(in_Buffer.startData(), &fft, &mylock);
		in_Buffer.clear();
	}
}

struct GARCONDisplay : OpaqueWidget {
	GARCON *module;
	shared_ptr<Font> font;
	const float width = 130.0f;
	const float height = 256.0f;
	float threshold = 5.0f;

	GARCONDisplay() {
		font = Font::load(assetPlugin(plugin, "res/DejaVuSansMono.ttf"));
	}

	NVGcolor getColor(float f) {
		if (f <= threshold)
			return nvgRGBA(0, 0, (int)(f * 255 / threshold), 255);
		else if (f <= (2 * threshold) )
			return nvgRGBA(0, (int)((f - threshold) * 255 / threshold), 255, 255);
		else
			return nvgRGBA((int)((f - 2 * threshold) * 255 / threshold), 255, 255, 255);
	}

	void draw(NVGcontext *vg) override {
			module->mylock.lock();
			vector<vector<float>> tmp(module->fft);
			module->mylock.unlock();

			if (tmp.size()>0) {
				for (size_t i = 0; i < width; i++) {
					if (i < tmp.size()) {
						if (tmp[i].size()>0) {
							float iHeith =  1.0f / height;
							for (size_t j = 0; j < height; j++) {
								nvgBeginPath(vg);
								float index = (height - j) * iHeith * (height - j) * iHeith * tmp[i].size();
								nvgStrokeColor(vg, getColor(interpolateLinear(&tmp[i][0], index)));
								nvgMoveTo(vg, i, j);
								nvgLineTo(vg, i, j + 1);
								nvgLineCap(vg, NVG_MITER);
								nvgClosePath(vg);
								nvgStrokeWidth(vg, 1);
								nvgStroke(vg);
							}
						}
					}
				}
			}
		}
};


struct GARCONWidget : ModuleWidget {
	GARCONWidget(GARCON *module);
};

GARCONWidget::GARCONWidget(GARCON *module) : ModuleWidget(module) {
	setPanel(SVG::load(assetPlugin(plugin, "res/GARCON.svg")));

	addChild(Widget::create<ScrewSilver>(Vec(15, 0)));
	addChild(Widget::create<ScrewSilver>(Vec(box.size.x-30, 0)));
	addChild(Widget::create<ScrewSilver>(Vec(15, 365)));
	addChild(Widget::create<ScrewSilver>(Vec(box.size.x-30, 365)));

	{
		GARCONDisplay *display = new GARCONDisplay();
		display->module = module;
		display->box.pos = Vec(10, 28);
		display->box.size = Vec(130, 256);
		addChild(display);
	}


	addInput(Port::create<PJ301MPort>(Vec(11, 330), Port::INPUT, module, GARCON::INPUT));

}

} // namespace rack_plugin_Bidoo

using namespace rack_plugin_Bidoo;

RACK_PLUGIN_MODEL_INIT(Bidoo, GARCON) {
   Model *modelGARCON = Model::create<GARCON, GARCONWidget>("Bidoo", "Garçon", "Garçon fft display", VISUAL_TAG);
   return modelGARCON;
}
