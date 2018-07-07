#include <string.h>
#include <complex.h>
#include <math.h>
#include <float.h>
#include "LOGinstruments.hpp"
#include "kiss_fft.h"
#include "dsp/digital.hpp"

namespace rack_plugin_LOGinstruments {

/*
 * FFT SCOPE, BASED ON SCOPE
 * Todo: precise f0 estimate
 */

#define BUFFER_SIZE 2048
#define FFT_POINTS BUFFER_SIZE
#define FFT_POINTS_NYQ FFT_POINTS/2+1
#define DIR_FFT 0
#define INV_FFT 1
#define FOFFS_RANGE 1.0
#define ZOOM_RANGE 8.0

float cabsf_LG(kiss_fft_cpx v) {
	return sqrtf((float)(v.r*v.r + v.i*v.i));
}

void HannWindow(float *w, int size) {
	if (size <= 0) return;

	if (w == NULL) {
		w = (float*)malloc(size * sizeof(float));
	}

	for (int i = 0; i < size; i++) {
	    w[i] = 0.5 * (1 - cos(2*M_PI*i / (size-1))); // maybe double is better?
	}
}

struct Speck : Module {
	enum ParamIds {
		SCALE_1_PARAM,
		POS_1_PARAM,
		SCALE_2_PARAM,
		POS_2_PARAM,
		ZOOM_PARAM, // was ZOOM
		LINLOG_PARAM, // was MODE
		FOFFS_PARAM, // was TRIG
		ONOFF_PARAM,
		NUM_PARAMS
	};
	enum InputIds {
		INPUT_1,
		INPUT_2,
		NUM_INPUTS
	};
	enum OutputIds {
		OUTPUT_1,
		OUTPUT_2,
		NUM_OUTPUTS
	};
	enum LightsIds {
		LIGHTS_0_LIN,
		LIGHTS_1_LOG,
		LIGHTS_2_ON,
		NUM_LIGHTS,
	};

	float buffer1[BUFFER_SIZE] = {};
	float buffer2[BUFFER_SIZE] = {};
	float FFT1[FFT_POINTS_NYQ] = {}; // poi cambiare il numero di punti in maniera arbitraria
	float FFT2[FFT_POINTS_NYQ] = {};
	int bufferIndex = 0;
	float frameIndex = 0;

	SchmittTrigger linLogTrig;
	SchmittTrigger onOffTrig;
	bool forceOff = false;
	bool linLog = false; // lin = 0, log = 1
	bool onOff = false;
	kiss_fft_cfg cfg_for_FFT, cfg_for_IFFT;
	float HannW[BUFFER_SIZE];

	Speck() : Module(NUM_PARAMS, NUM_INPUTS, NUM_OUTPUTS, NUM_LIGHTS) {
		cfg_for_FFT = kiss_fft_alloc( FFT_POINTS, DIR_FFT, 0, 0 );
		cfg_for_IFFT = kiss_fft_alloc( FFT_POINTS, INV_FFT, 0, 0 );
		//HannW = NULL;
		HannWindow(&HannW[0], BUFFER_SIZE);
	}
	~Speck();
	void step() override;

	json_t *toJson() override {
		json_t *rootJ = json_object();
		json_object_set_new(rootJ, "linLog", json_integer((int) linLog));
		return rootJ;
	}

	void fromJson(json_t *rootJ) override {
		json_t *sumJ = json_object_get(rootJ, "linLog");
		if (sumJ)
			linLog = json_integer_value(sumJ);
	}

	void reset() override {
		linLog = false;
		onOff = false;
	}
};

Speck::~Speck() {
	free(cfg_for_FFT);
	free(cfg_for_IFFT);
}

void Speck::step() {
	int n;
	kiss_fft_cpx cBufIn[FFT_POINTS], cBufOut[FFT_POINTS];
	// Modes
    if (onOffTrig.process(params[ONOFF_PARAM].value)) {
    	forceOff = !forceOff;
    }

    if (inputs[INPUT_1].active || inputs[INPUT_2].active)  {
    	if (onOff == false && forceOff == false) {
    		onOff = true;
    	} else if (onOff == true && forceOff == true) {
    		onOff = false;
    	}
    } else {
    	onOff = false;
    	forceOff = false;
    }
    lights[LIGHTS_2_ON].value = onOff;

    if (linLogTrig.process(params[LINLOG_PARAM].value)) {
            linLog = !linLog;
    }
    lights[LIGHTS_0_LIN].value = linLog ? 0.0 : 1.0;
    lights[LIGHTS_1_LOG].value = linLog ? 1.0 : 0.0;

    // copy in to out
    if (outputs[OUTPUT_1].active) {
    	outputs[OUTPUT_1].value = (inputs[INPUT_1].value);
    }
    if (outputs[OUTPUT_2].active) {
    	outputs[OUTPUT_2].value = (inputs[INPUT_2].value);
    }


	// Compute time
	if (onOff) {
		//float deltaTime = powf(2.0, -14.0); // this could be the NFFT in the future (if rounded to nearest 2^N)
		//int frameCount = (int)ceilf(deltaTime * engineGetSampleRate());
		int frameCount = 1;

		// Add frame to buffer
		if (bufferIndex < BUFFER_SIZE) {
			if (++frameIndex > frameCount) {
				frameIndex = 0;
				buffer1[bufferIndex] = (inputs[INPUT_1].value);
				buffer2[bufferIndex] = (inputs[INPUT_2].value);
				bufferIndex++;
			}
		} else {
			// TIME TO COMPUTE FFT
			for ( n = 0; n < FFT_POINTS; n++ ) {
				cBufIn[n].r = HannW[n] * buffer1[n];
				cBufIn[n].i = 0.0; // forse devo copiare anche qui?
			}
			kiss_fft(cfg_for_FFT, cBufIn, cBufOut);
			for ( n = 0; n < FFT_POINTS_NYQ; n++ ) {
				FFT1[n] = logf(cabsf_LG(cBufOut[n]));
			}

			for ( n = 0; n < FFT_POINTS; n++ ) {
				cBufIn[n].r = HannW[n] * buffer2[n];
				cBufIn[n].i = 0.0; // forse devo copiare anche qui?
			}
			kiss_fft(cfg_for_FFT, cBufIn, cBufOut);
			for ( n = 0; n < FFT_POINTS_NYQ; n++ ) {
				FFT2[n] = logf(cabsf_LG(cBufOut[n]));
			}
			bufferIndex = 0; frameIndex = 0; // reset all. remove for future overlaps
		}
		/*
		// Reset buffer
		if (bufferIndex >= BUFFER_SIZE) {
			bufferIndex = 0; frameIndex = 0; return;
		}
		*/
	}
}


struct SpeckDisplay : TransparentWidget {
	Speck *module;
	int frame = 0;
	std::shared_ptr<Font> font;

	struct Stats {
		float f0, peakx, peaky;
		void calculate(float *values) {
			f0 = 0.0;
			peakx = 0.0;
			peaky = 0.0;
			for (int i = 0; i < FFT_POINTS_NYQ; i++) {
				float v = values[i];
				if (v > peaky) {
					peaky = v;
					peakx = i;
				}
			}
			// f0 heuristic: descend from peakx and look for salient derivative change
#ifdef FO_HEUR
			for (int i = 1; i < peakx+10; i++) {
				float * diff = new float(peakx+10);
				diff[i-1] = values[i] - values[i-1];
			}
			int new_f0;
			for (int i = peakx; i > 1; i--) {
				if (values[i] < 0.0-MIN_DELTA) {
					int i_neg = findLocalMinBackw(&values[i], i);
					int i_pos = findLocalMaxBackw(&values[i], i);
					if (i_neg - i_pos < PK_DIST) {
						new_f0 = findZero(&values[i_neg], i_neg-i_pos);
					}
					i = i_pos; // go on but skip the data processed here
				}
			}
#endif

			peakx = engineGetSampleRate()/2.0 * ((float)(peakx) / (float)(FFT_POINTS_NYQ));
			f0 = peakx; // todo calculate the real f0
		}
	};
	Stats stats1, stats2;

	SpeckDisplay() {
		font = Font::load(assetPlugin(plugin, "res/DejaVuSansMono.ttf"));
	}

	#define LOG_LOWER_FREQ 10.0 // lowest freq we are going to show in log mode
	float drawWaveform(NVGcontext *vg, float *values, float gain, float offset, float fzoom, float foffs, bool linLog) {
		int xpos;
		float nyq = engineGetSampleRate() / 2.0;
		float logMax = log10(nyq);
		float semilogx[FFT_POINTS_NYQ];
		float vgrid[100];
		float negOffs;
		int maxj = 0;
		Vec p;
		nvgSave(vg);
		Rect b = Rect(Vec(0, 15), box.size.minus(Vec(0, 15*2)));
		nvgScissor(vg, b.pos.x, b.pos.y, b.size.x, b.size.y);
		nvgBeginPath(vg);
		// Draw maximum display left to right
		int lwp = 0; // lowest point to show
		int spacing = (nyq/FFT_POINTS_NYQ);
		for (lwp = 0; lwp < FFT_POINTS_NYQ; lwp++) {
			if (lwp*spacing > LOG_LOWER_FREQ) break;
		}

		// create the semilogx axis
		if (linLog) {
			vgrid[0] = log10(LOG_LOWER_FREQ);
			vgrid[0] = (vgrid[0] * b.size.x / logMax);
			int j = 1;
			// create lin grid values
			for (int f = 100; f < 1000; f+=100) {
				vgrid[j++] = f;
			}
			for (int f = 1000; f < nyq; f+=1000) {
				vgrid[j++] = f;
			}
			maxj = j;
			for (int i = 0; i < maxj; i++) {
				vgrid[i] = log10((float)(vgrid[i]));
				vgrid[i] = (vgrid[i] * ((float)(b.size.x) + vgrid[0]) / logMax);
			}

			semilogx[lwp] = log10((float)(lwp) * nyq / (float)FFT_POINTS_NYQ );
			semilogx[lwp] = (semilogx[lwp] * b.size.x / logMax); // apply the range of the box
			for (int i = lwp+1; i < FFT_POINTS_NYQ; i++) {
				semilogx[i] = log10((float)(i) * nyq / (float)FFT_POINTS_NYQ );
				semilogx[i] = (b.size.x + semilogx[lwp] + 60) * semilogx[i] / logMax ; // apply the range of the box
			}

			float residual = semilogx[FFT_POINTS_NYQ-1] - (semilogx[FFT_POINTS_NYQ-1]/fzoom); // excluded from plot
			negOffs = - (0.8*foffs / FOFFS_RANGE) * residual;
/*
			for (int i = 0; i < FFT_POINTS_NYQ; i++) {
				semilogx[i] = negOffs + semilogx[i]; // apply the range of the box TODO togliere?
			}
			for (int j = 0; j < maxj; j++) {
				vgrid[j] += negOffs;
			}
*/
			for (int i = lwp; i < FFT_POINTS_NYQ; i++) {
				float value = values[i] * gain + offset;

				p = Vec(b.pos.x + fzoom*(((semilogx[i])-semilogx[lwp]) + negOffs), b.pos.y + b.size.y * (1-value)/2);

				if (i <= lwp)
					nvgMoveTo(vg, p.x, p.y);
				else
					nvgLineTo(vg, p.x, p.y);
			}

		} else {
			int zoomPoints = floor((float)(FFT_POINTS_NYQ) / (fzoom < 1.0 ? 1.0 : fzoom));
			int fstart = floor(foffs * ((float)(FFT_POINTS_NYQ) - (float)(zoomPoints)));

			for (int i = 0; i < zoomPoints; i++) {
				float value = values[i+fstart] * gain + offset;
				xpos = i;
				p = Vec(b.pos.x + xpos * b.size.x / (zoomPoints/*FFT_POINTS_NYQ*/-1), b.pos.y + b.size.y * (1 - value) / 2);

				if (i == 0)
					nvgMoveTo(vg, p.x, p.y);
				else
					nvgLineTo(vg, p.x, p.y);
			}
		}
		//printf("xpos %d, bsize %f, zoomPts %d, bpos %f, x %f\n", xpos, b.size.x, zoomPoints, b.pos.x, p.x);
		nvgLineCap(vg, NVG_ROUND);
		nvgMiterLimit(vg, 2.0);
		nvgStrokeWidth(vg, 1.75);
		nvgGlobalCompositeOperation(vg, NVG_LIGHTER);
		nvgStroke(vg);
		nvgResetScissor(vg);
		nvgRestore(vg);

		if (linLog) {

			// UP TO 1k
			for (int j = 0; j < maxj; j++) {

				Vec p = Vec(b.pos.x + fzoom*(vgrid[j] - vgrid[0]+ negOffs), box.size.y);
				nvgStrokeColor(vg, nvgRGBA(0xff, 0xff, 0xff, 0x10));
				{
					nvgBeginPath(vg);
					nvgMoveTo(vg, p.x, p.y);
					nvgLineTo(vg, p.x, 0);
					nvgClosePath(vg);
				}
				nvgStroke(vg);
			}
		}

		return negOffs;
	}

#define VERT_GRID_DIST 20
#define HORZ_GRID_DIST 20
	void drawGrid(NVGcontext *vg, float fzoom, float foffs, bool linLog, float negOffs) {
		Rect b = Rect(Vec(0, 15), box.size.minus(Vec(0, 15*2)));
		nvgScissor(vg, b.pos.x, b.pos.y, b.size.x, b.size.y);
		float nyq = engineGetSampleRate() / 2.0;
		float range = nyq / (fzoom < 1.0 ? 1.0 : fzoom);
		float fstart = foffs * (nyq - range);
		int first = ceil(fstart / 1000) * 1000;
		float diff = first - fstart;

		// VERT LINES
		if (linLog == 0) {
			for (int f = first; f < first+range; f+=1000) {
				float v = ((f-first+diff) / range) * box.size.x;
				Vec p = Vec(v, box.size.y);
				nvgStrokeColor(vg, nvgRGBA(0xff, 0xff, 0xff, 0x10));
				{
					nvgBeginPath(vg);
					nvgMoveTo(vg, p.x, p.y);
					nvgLineTo(vg, p.x, 0);
					nvgClosePath(vg);
				}
				nvgStroke(vg);
			}
		} else { ; } // is done in the drawWaveform for convenience

		// HORZ LINES
		for (int h = 0; h < box.size.y; h+= HORZ_GRID_DIST) {
			Vec p = Vec(box.size.x, h);
			nvgStrokeColor(vg, nvgRGBA(0xff, 0xff, 0xff, 0x10));
			{
				nvgBeginPath(vg);
				nvgMoveTo(vg, p.x, p.y);
				nvgLineTo(vg, 0, p.y);
				nvgClosePath(vg);
			}
			nvgStroke(vg);
		}
	}

/*	void drawTrig(NVGcontext *vg, float value, float gain, float offset) {
		Rect b = Rect(Vec(0, 15), box.size.minus(Vec(0, 15*2)));
		nvgScissor(vg, b.pos.x, b.pos.y, b.size.x, b.size.y);

		value = value * gain + offset;
		Vec p = Vec(box.size.x, b.pos.y + b.size.y * (1 - value) / 2);

		// Draw line
		nvgStrokeColor(vg, nvgRGBA(0xff, 0xff, 0xff, 0x10));
		{
			nvgBeginPath(vg);
			nvgMoveTo(vg, p.x - 13, p.y);
			nvgLineTo(vg, 0, p.y);
			nvgClosePath(vg);
		}
		nvgStroke(vg);

		// Draw indicator
		nvgFillColor(vg, nvgRGBA(0xff, 0xff, 0xff, 0x60));
		{
			nvgBeginPath(vg);
			nvgMoveTo(vg, p.x - 2, p.y - 4);
			nvgLineTo(vg, p.x - 9, p.y - 4);
			nvgLineTo(vg, p.x - 13, p.y);
			nvgLineTo(vg, p.x - 9, p.y + 4);
			nvgLineTo(vg, p.x - 2, p.y + 4);
			nvgClosePath(vg);
		}
		nvgFill(vg);

		nvgFontSize(vg, 8);
		nvgFontFaceId(vg, font->handle);
		nvgFillColor(vg, nvgRGBA(0x1e, 0x28, 0x2b, 0xff));
		nvgText(vg, p.x - 8, p.y + 3, "T", NULL);
		nvgResetScissor(vg);
	}*/

	void drawStats(NVGcontext *vg, Vec pos, const char *title, Stats *stats) {
		nvgFontSize(vg, 10);
		nvgFontFaceId(vg, font->handle);
		nvgTextLetterSpacing(vg, -2);

		nvgFillColor(vg, nvgRGBA(0xff, 0xff, 0xff, 0xff));
		nvgText(vg, pos.x + 5, pos.y + 10, title, NULL);

		nvgFillColor(vg, nvgRGBA(0xff, 0xff, 0xff, 0x80));
		char text[128];
		snprintf(text, sizeof(text), /*"F0 %5.2f  */"PeakX %5.1f  PeakY % 5.1f", /*stats->f0, */stats->peakx, stats->peaky);
		//printf("%s\n", text);
		nvgText(vg, pos.x + 17, pos.y + 10, text, NULL);
	}

	void draw(NVGcontext *vg) override {
		float gain1 = powf(2.0, roundf(module->params[Speck::SCALE_1_PARAM].value)) / 12.0;
		float gain2 = powf(2.0, roundf(module->params[Speck::SCALE_2_PARAM].value)) / 12.0;
		float pos1 = module->params[Speck::POS_1_PARAM].value;
		float pos2 = module->params[Speck::POS_2_PARAM].value;
		float zoom = module->params[Speck::ZOOM_PARAM].value;
		float freqOffs = module->params[Speck::FOFFS_PARAM].value;
		float negOffs;

		// Draw waveforms
		// Y
		if (module->inputs[Speck::INPUT_2].active) {
			nvgStrokeColor(vg, nvgRGBA(0x0E, 0x99, 0x00, 0xA0));
			//drawWaveform(vg, module->buffer2, gain2, pos2);
			negOffs = drawWaveform(vg, module->FFT2, gain2, pos2, zoom, freqOffs, module->linLog);
		}

		// X
		if (module->inputs[Speck::INPUT_1].active) {
			nvgStrokeColor(vg, nvgRGBA(0xF4, 0x51, 0x00, 0xA0));
			//drawWaveform(vg, module->buffer1, gain1, pos1);
			negOffs = drawWaveform(vg, module->FFT1, gain1, pos1, zoom, freqOffs, module->linLog);
		}
		//drawTrig(vg, module->params[Speck::TRIG_PARAM], gain1, pos1);

		// Calculate and draw stats
		if (++frame >= 4) {
			frame = 0;
			stats1.calculate(module->FFT1);
			stats2.calculate(module->FFT2);
		}
		drawStats(vg, Vec(0, 0), "IN1", &stats1);
		drawStats(vg, Vec(0, box.size.y - 15), "IN2", &stats2);
		drawGrid(vg, zoom, freqOffs, module->linLog, negOffs);
	}
};


struct SpeckWidget : ModuleWidget {
	SpeckWidget(Speck *module);
};

SpeckWidget::SpeckWidget(Speck *module) : ModuleWidget(module) {
	box.size = Vec(15*20, 380);

	{
		SVGPanel *panel = new SVGPanel();
		panel->box.size = box.size;
		panel->setBackground(SVG::load(assetPlugin(plugin, "res/Speck_nofonts2.svg")));
		addChild(panel);
	}
/*
	addChild(Widget::create<ScrewSilver>(Vec(15, 0)));
	addChild(Widget::create<ScrewSilver>(Vec(box.size.x-30, 0)));
	addChild(Widget::create<ScrewSilver>(Vec(15, 365)));
	addChild(Widget::create<ScrewSilver>(Vec(box.size.x-30, 365)));
*/
	{
		SpeckDisplay *display = new SpeckDisplay();
		display->module = module;
		display->box.pos = Vec(0, 44);
		display->box.size = Vec(box.size.x, 165);
		addChild(display);
	}

	addParam(ParamWidget::create<RoundBlackSnapKnob>(Vec(118, 244), module, Speck::SCALE_1_PARAM, -10.0, 20.0, -1.0));
	addParam(ParamWidget::create<RoundSmallBlackKnob>(Vec(118, 297), module, Speck::POS_1_PARAM, -1.0, 1.0, 0.0));
	addParam(ParamWidget::create<RoundBlackSnapKnob>(Vec(167, 244), module, Speck::SCALE_2_PARAM, -10.0, 20.0, -1.0));
	addParam(ParamWidget::create<RoundSmallBlackKnob>(Vec(167, 297), module, Speck::POS_2_PARAM, -1.0, 1.0, 0.0));
	addParam(ParamWidget::create<RoundSmallBlackKnob>(Vec(213, 244), module, Speck::ZOOM_PARAM, 1.0, ZOOM_RANGE, 1.0));
	addParam(ParamWidget::create<CKD6>(Vec(258, 244), module, Speck::LINLOG_PARAM, 0.0, 1.0, 0.0));
	addParam(ParamWidget::create<RoundSmallBlackKnob>(Vec(213, 297), module, Speck::FOFFS_PARAM, 0.0, FOFFS_RANGE, 0.0));
	addParam(ParamWidget::create<CKD6>(Vec(239, 12), module, Speck::ONOFF_PARAM, 0.0, 1.0, 0.0));

	addInput(Port::create<PJ301MPort>(Vec(12, 240), Port::INPUT, module, Speck::INPUT_1));
	addInput(Port::create<PJ301MPort>(Vec(59, 240), Port::INPUT, module, Speck::INPUT_2));

	addOutput(Port::create<PJ3410Port>(Vec(9, 306), Port::OUTPUT, module, Speck::OUTPUT_1));
	addOutput(Port::create<PJ3410Port>(Vec(56, 306), Port::OUTPUT, module, Speck::OUTPUT_2));

	addChild(ModuleLightWidget::create<TinyLight<GreenLight>>(Vec(286, 230), module, Speck::LIGHTS_0_LIN));
	addChild(ModuleLightWidget::create<TinyLight<GreenLight>>(Vec(286, 280), module, Speck::LIGHTS_1_LOG));
	addChild(ModuleLightWidget::create<TinyLight<GreenLight>>(Vec(265, 8), module, Speck::LIGHTS_2_ON));

}

} // namespace rack_plugin_LOGinstruments

using namespace rack_plugin_LOGinstruments;

RACK_PLUGIN_MODEL_INIT(LOGinstruments, Speck) {
   Model *modelSpeck = Model::create<Speck, SpeckWidget>("LOGinstruments", "Speck", "Spectrum Analyzer", VISUAL_TAG, UTILITY_TAG);
   return modelSpeck;
}
