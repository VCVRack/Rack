//============================================================================================================
//!
//! \file Scope-G1.cpp
//!
//! \brief Scope-G1 is a...
//!
//============================================================================================================


#include <string.h>
#include "dsp/digital.hpp"
#include "Gratrix.hpp"

namespace rack_plugin_Gratrix {

#define BUFFER_SIZE 512

struct Scope : Module {
	enum ParamIds {
		X_SCALE_PARAM,
		X_POS_PARAM,
		TIME_PARAM,
		TRIG_PARAM,
		EXTERNAL_PARAM,
		DISP_PARAM,
		NUM_PARAMS
	};
	enum InputIds {
		X_INPUT,     // N
		TRIG_INPUT,  // N
		NUM_INPUTS
	};
	enum OutputIds {
		NUM_OUTPUTS
	};
	enum LightIds {
		NUM_LIGHTS
	};

	struct Voice {
		bool active = false;
		float bufferX[BUFFER_SIZE] = {};
		int bufferIndex = 0;
		float frameIndex = 0;
		SchmittTrigger resetTrigger;
		void step(bool external, int frameCount, const Param &trig_param, const Input &x_input, const Input &trig_input);
	};

	bool external = false;
	Voice voice[GTX__N+1];

	Scope() : Module(NUM_PARAMS, GTX__N * NUM_INPUTS, NUM_OUTPUTS, NUM_LIGHTS) {}

	static constexpr std::size_t imap(std::size_t port, std::size_t bank)
	{
		return port + bank * NUM_INPUTS;
	}

	void step() override;
};


void Scope::step() {
	// Modes
	external = params[EXTERNAL_PARAM].value <= 0.0f;

	// Compute time
	float deltaTime = powf(2.0f, params[TIME_PARAM].value);
	int frameCount = (int)ceilf(deltaTime * engineGetSampleRate());

	Input x_sum, trig_sum;
	int count = 0;

	for (int k=0; k<GTX__N; ++k)
	{
		voice[k].step(external, frameCount, params[TRIG_PARAM], inputs[imap(X_INPUT, k)], inputs[imap(TRIG_INPUT, k)]);

		if (inputs[imap(X_INPUT, k)].active)
		{
			x_sum.active = true;
			x_sum.value += inputs[imap(X_INPUT, k)].value;
			count++;
		}

		if (inputs[imap(TRIG_INPUT, k)].active) // GTX TODO - may need better logic here
		{
			trig_sum.active = true;
			trig_sum.value += inputs[imap(TRIG_INPUT, k)].value;
		}
	}

	if (count > 0)
	{
		x_sum.value /= static_cast<float>(count);
	}

	voice[GTX__N].step(external, frameCount, params[TRIG_PARAM], x_sum, trig_sum);
}

void Scope::Voice::step(bool external, int frameCount, const Param &trig_param, const Input &x_input, const Input &trig_input) {
	// Copy active state
	active = x_input.active;

	// Add frame to buffer
	if (bufferIndex < BUFFER_SIZE) {
		if (++frameIndex > frameCount) {
			frameIndex = 0;
			bufferX[bufferIndex] = x_input.value;
			bufferIndex++;
		}
	}

	// Are we waiting on the next trigger?
	if (bufferIndex >= BUFFER_SIZE) {
		// Trigger immediately if external but nothing plugged in
		if (external && !trig_input.active) {
			bufferIndex = 0;
			frameIndex = 0;
			return;
		}

		// Reset the Schmitt trigger so we don't trigger immediately if the input is high
		if (frameIndex == 0) {
			resetTrigger.reset();
		}
		frameIndex++;

		// Must go below 0.1fV to trigger
		float gate = external ? trig_input.value : x_input.value;

		// Reset if triggered
		float holdTime = 0.1f;
		if (resetTrigger.process(rescale(gate, trig_param.value - 0.1f, trig_param.value, 0.f, 1.f)) || (frameIndex >= engineGetSampleRate() * holdTime)) {
			bufferIndex = 0; frameIndex = 0; return;
		}

		// Reset if we've waited too long
		if (frameIndex >= engineGetSampleRate() * holdTime) {
			bufferIndex = 0; frameIndex = 0; return;
		}
	}
}


struct Display : TransparentWidget {
	Scope *module;
	int frame = 0;
	std::shared_ptr<Font> font;

	struct Stats {
		float vrms, vpp, vmin, vmax;
		void calculate(float *values) {
			vrms = 0.0f;
			vmax = -INFINITY;
			vmin = INFINITY;
			for (int i = 0; i < BUFFER_SIZE; i++) {
				float v = values[i];
				vrms += v*v;
				vmax = fmaxf(vmax, v);
				vmin = fminf(vmin, v);
			}
			vrms = sqrtf(vrms / BUFFER_SIZE);
			vpp = vmax - vmin;
		}
	};
	Stats statsX[GTX__N + 1];

	Display() {
		font = Font::load(assetPlugin(plugin, "res/fonts/Sudo.ttf"));
	}

	void drawWaveform(NVGcontext *vg, float *valuesX, const Rect &b) {
		if (!valuesX)
			return;
		nvgSave(vg);
		nvgScissor(vg, b.pos.x, b.pos.y, b.size.x, b.size.y);
		nvgBeginPath(vg);
		// Draw maximum display left to right
		for (int i = 0; i < BUFFER_SIZE; i++) {
			float x = (float)i / (BUFFER_SIZE - 1);
			float y = valuesX[i] / 2.0f + 0.5f;
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
		nvgStrokeWidth(vg, 1.5);
		nvgGlobalCompositeOperation(vg, NVG_LIGHTER);
		nvgStroke(vg);
		nvgResetScissor(vg);
		nvgRestore(vg);
	}

	void drawTrig(NVGcontext *vg, float value, const Rect &b) {
		nvgScissor(vg, b.pos.x, b.pos.y, b.size.x, b.size.y);
		value = value / 2.0f + 0.5f;
		Vec p = Vec(b.pos.x + b.size.x, b.pos.y + b.size.y * (1.0f - value));

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

		nvgFontSize(vg, 9);
		nvgFontFaceId(vg, font->handle);
		nvgFillColor(vg, nvgRGBA(0x1e, 0x28, 0x2b, 0xff));
		nvgText(vg, p.x - 8, p.y + 3, "T", NULL);
		nvgResetScissor(vg);
	}

	void drawStats(NVGcontext *vg, Vec pos, const char *title, const Stats &stats) {
		nvgFontSize(vg, 13);
		nvgFontFaceId(vg, font->handle);
		nvgTextLetterSpacing(vg, -2);

		nvgFillColor(vg, nvgRGBA(0xff, 0xff, 0xff, 0x80));
		char text[128];
		snprintf(text, sizeof(text), "%s. %4.1f [%+5.1f %+5.1f]", title, stats.vpp, stats.vmin, stats.vmax);
		nvgText(vg, pos.x + 6, pos.y + 11, text, NULL);
	}

	void draw(NVGcontext *vg) override {
		float gainX = powf(2.0, roundf(module->params[Scope::X_SCALE_PARAM].value));
		float offsetX = module->params[Scope::X_POS_PARAM].value;
		int   disp = static_cast<int>(module->params[Scope::DISP_PARAM].value + 0.5f);

		int k0, k1, kM;

		     if (disp == 0       ) { k0 =      0; k1 = GTX__N;   kM = 0;   } // six up
		else if (disp == GTX__N+1) { k0 = GTX__N; k1 = GTX__N+1; kM = 100; } // sum
		else                       { k0 = disp-1; k1 =   disp  ; kM = 100; } // individual

		static const char *stats_lab[GTX__N+1] = {"1", "2", "3", "4", "5", "6", "SUM"};

		for (int k=k0; k<k1; ++k)
		{
			Rect a = Rect(Vec(0, 15), box.size.minus(Vec(0, 15*2)));
			Rect b = a;

			// GTX TODO - imporve
			if (kM < GTX__N)
			{
				b.size.x /= (GTX__N/2);
				b.size.y /= (GTX__N/3);
				b.pos.x  += (k%3) * b.size.x;
				b.pos.y  += (k/3) * b.size.y;
			}

			float valuesX[BUFFER_SIZE];
			for (int i = 0; i < BUFFER_SIZE; i++) {
				valuesX[i] = (module->voice[k].bufferX[i] + offsetX) * gainX / 10.0;
			}

			// Draw waveforms
			if (module->voice[k].active) {
				if (k&1) nvgStrokeColor(vg, nvgRGBA(0xe1, 0x02, 0x78, 0xc0));
				else     nvgStrokeColor(vg, nvgRGBA(0x28, 0xb0, 0xf3, 0xc0));
				drawWaveform(vg, valuesX, b);
			}

			float valueTrig = (module->params[Scope::TRIG_PARAM].value + offsetX) * gainX / 10.0;
			drawTrig(vg, valueTrig, b);

			// Calculate and draw stats
			if (frame == 0) {
				statsX[k].calculate(module->voice[k].bufferX);
			}

			Vec stats_pos = b.pos;
			if (k >= 3 && k < GTX__N)
			{
				stats_pos.y += b.size.y;
			}
			else
			{
				stats_pos.y -= 15;
			}
			drawStats(vg, stats_pos, stats_lab[k], statsX[k]);
		}

		if (++frame >= 4)
		{
			frame = 0;
		}
	}
};


//============================================================================================================
//! \brief The widget.

struct GtxWidget_Scope_G1 : ModuleWidget
{
	GtxWidget_Scope_G1(Scope *module) : ModuleWidget(module)
	{
		GTX__WIDGET();
		box.size = Vec(24*15, 380);

		auto screen_pos  = Vec(0, 35);
		auto screen_size = Vec(box.size.x, 220);

		#if GTX__SAVE_SVG
		{
			PanelGen pg(assetPlugin(plugin, "build/res/Scope-G1.svg"), box.size, "SCOPE-G1");

			pg.rect(screen_pos, screen_size, "fill:#222222;stroke:none");

			pg.bus_in(0, 2, "IN");
			pg.bus_in(2, 2, "EXT TRIG");

			pg.nob_sml_raw(gx(1-0.22), gy(2-0.24), "SCALE");
			pg.nob_sml_raw(gx(1-0.22), gy(2+0.22), "POS");
			pg.nob_sml_raw(gx(1+0.22), gy(2-0.24), "TIME");
			pg.nob_sml_raw(gx(1+0.22), gy(2+0.22), "TRIG");
			pg.tog_raw    (gx(3-0.22), gy(2-0.24), "INT", "EXT");
			pg.nob_sml_raw(gx(3+0.22), gy(2-0.24), "DISP");

			for (std::size_t i=0; i<=12; i++)
			{
				float x = screen_pos.x + screen_size.x * i / 12.0f;
				pg.line(Vec(x, screen_pos.y + 15), Vec(x, screen_pos.y + screen_size.y - 15), "fill:none;stroke:#666666;stroke-width:1");
			}

			for (std::size_t i=0; i<=8; i++)
			{
				float y = screen_pos.y + 15 + (screen_size.y - 30) * i /  8.0f;
				pg.line(Vec(screen_pos.x, y), Vec(screen_pos.x + screen_size.x, y), "fill:none;stroke:#666666;stroke-width:1");
			}
		}
		#endif

		setPanel(SVG::load(assetPlugin(plugin, "res/Scope-G1.svg")));

		{
			Display *display  = new Display();
			display->module   = module;
			display->box.pos  = screen_pos;
			display->box.size = screen_size;
			addChild(display);
		}

		addParam(createParamGTX<KnobSnapSml>(Vec(gx(1-0.22), gy(2-0.24)), module, Scope::X_SCALE_PARAM, -2.0f,      8.0f,   0.0f));
		addParam(createParamGTX<KnobFreeSml>(Vec(gx(1-0.22), gy(2+0.22)), module, Scope::X_POS_PARAM,  -10.0f,     10.0f,   0.0f));
		addParam(createParamGTX<KnobFreeSml>(Vec(gx(1+0.22), gy(2-0.24)), module, Scope::TIME_PARAM,    -6.0f,    -16.0f, -14.0f));
		addParam(createParamGTX<KnobFreeSml>(Vec(gx(1+0.22), gy(2+0.22)), module, Scope::TRIG_PARAM,   -10.0f,     10.0f,   0.0f));
		addParam(ParamWidget::create<CKSS>  (tog(gx(3-0.22), gy(2-0.24)), module, Scope::EXTERNAL_PARAM, 0.0f,      1.0f,   1.0f));
		addParam(createParamGTX<KnobSnapSml>(Vec(gx(3+0.22), gy(2-0.24)), module, Scope::DISP_PARAM,     0.0f,  GTX__N+1,   0.0f));

		for (std::size_t i=0; i<GTX__N; ++i)
		{
			addInput(createInputGTX<PortInMed>(Vec(px(0, i), py(2, i)), module, Scope::imap(Scope::X_INPUT,    i)));
			addInput(createInputGTX<PortInMed>(Vec(px(2, i), py(2, i)), module, Scope::imap(Scope::TRIG_INPUT, i)));
		}
	}
};

} // namespace rack_plugin_Gratrix

using namespace rack_plugin_Gratrix;

RACK_PLUGIN_MODEL_INIT(Gratrix, Scope_G1) {
   Model *model = Model::create<Scope, GtxWidget_Scope_G1>("Gratrix", "Scope-G1", "Scope-G1", VISUAL_TAG);
   return model;
}
