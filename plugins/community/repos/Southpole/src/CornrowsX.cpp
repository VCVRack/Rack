//
// CornrowsX Mutable Instruments Braids 
// copied from VCV Audible Instruments
//
//


#include <string.h>

#include "Southpole.hpp"
#include "CornrowsSettings.h"

#include "dsp/samplerate.hpp"
#include "dsp/ringbuffer.hpp"
#include "braids/macro_oscillator.h"
#include "braids/vco_jitter_source.h"
#include "braids/signature_waveshaper.h"

#include "braids/envelope.h"
#include "braids/quantizer.h"
#include "braids/quantizer_scales.h"

namespace rack_plugin_Southpole {

struct CornrowsX : Module {
	enum ParamIds {
		FINE_PARAM,
		COARSE_PARAM,
		FM_PARAM,
		TIMBRE_PARAM,
		MODULATION_PARAM,
		COLOR_PARAM,
		SHAPE_PARAM,

		PITCH_RANGE_PARAM,
		PITCH_OCTAVE_PARAM,
		TRIG_DELAY_PARAM,
		ATT_PARAM,
		DEC_PARAM,
		AD_TIMBRE_PARAM,
		AD_MODULATION_PARAM,
		AD_COLOR_PARAM,
		RATE_PARAM,
		BITS_PARAM,
		SCALE_PARAM,
		ROOT_PARAM,

		NUM_PARAMS
	};
	enum InputIds {
		TRIG_INPUT,
		PITCH_INPUT,
		FM_INPUT,
		TIMBRE_INPUT,
		COLOR_INPUT,
		NUM_INPUTS
	};
	enum OutputIds {
		OUT_OUTPUT,
		NUM_OUTPUTS
	};

	braids::MacroOscillator osc;
	braids::SettingsData settings;
	braids::VcoJitterSource jitter_source;
	braids::SignatureWaveshaper ws;
	braids::Envelope envelope;
	braids::Quantizer quantizer;

	uint8_t current_scale = 0xff;

	bool trigger_detected_flag;
	bool trigger_flag;
	uint16_t trigger_delay;

	uint16_t gain_lp;
    int16_t previous_pitch = 0;

	SampleRateConverter<1> src;
	DoubleRingBuffer<Frame<1>, 256> outputBuffer;
	bool lastTrig = false;
	bool lowCpu = false;
	bool paques = false;
	
	const uint16_t bit_reduction_masks[7] = {
		0xc000,
		0xe000,
		0xf000,
		0xf800,
		0xff00,
		0xfff0,
		0xffff };

	const uint16_t decimation_factors[7] = { 24, 12, 6, 4, 3, 2, 1 };

	// only for display
	braids::SettingsData last_settings;
	braids::Setting last_setting_changed;
	uint32_t disp_timeout = 0;

	CornrowsX();
	void step() override;
	void setShape(int shape);

	json_t *toJson() override {
		json_t *rootJ = json_object();
		json_t *settingsJ = json_array();
		uint8_t *settingsArray = &settings.shape;
		for (int i = 0; i < 20; i++) {
			json_t *settingJ = json_integer(settingsArray[i]);
			json_array_insert_new(settingsJ, i, settingJ);
		}
		json_object_set_new(rootJ, "settings", settingsJ);

		json_t *lowCpuJ = json_boolean(lowCpu);
		json_object_set_new(rootJ, "lowCpu", lowCpuJ);

		return rootJ;
	}

	void fromJson(json_t *rootJ) override {
		json_t *settingsJ = json_object_get(rootJ, "settings");
		if (settingsJ) {
			uint8_t *settingsArray = &settings.shape;
			for (int i = 0; i < 20; i++) {
				json_t *settingJ = json_array_get(settingsJ, i);
				if (settingJ)
					settingsArray[i] = json_integer_value(settingJ);
			}
		}

		json_t *lowCpuJ = json_object_get(rootJ, "lowCpu");
		if (lowCpuJ) {
			lowCpu = json_boolean_value(lowCpuJ);
		}
	}
};


CornrowsX::CornrowsX() : Module(NUM_PARAMS, NUM_INPUTS, NUM_OUTPUTS) {
	memset(&osc, 0, sizeof(osc));
	osc.Init();
	memset(&quantizer, 0, sizeof(quantizer));
	quantizer.Init();
	memset(&envelope, 0, sizeof(envelope));
	envelope.Init();

	memset(&jitter_source, 0, sizeof(jitter_source));
	jitter_source.Init();
	memset(&ws, 0, sizeof(ws));
	ws.Init(0x0000);
	memset(&settings, 0, sizeof(settings));

}

void CornrowsX::step() {

	settings.quantizer_scale = params[SCALE_PARAM].value * 48.; //sizeof(quantization_values);
	settings.quantizer_root  = params[ROOT_PARAM].value * 11.;
	settings.pitch_range = params[PITCH_RANGE_PARAM].value*4.;
	settings.pitch_octave = params[PITCH_OCTAVE_PARAM].value*4.;
	settings.trig_delay  = params[TRIG_DELAY_PARAM].value*6.;
	settings.sample_rate = params[RATE_PARAM].value*6.;
	settings.resolution  = params[BITS_PARAM].value*6.;
	settings.ad_attack 	= params[ATT_PARAM].value*15.;
	settings.ad_decay  	= params[DEC_PARAM].value*15.;
	settings.ad_timbre 	= params[AD_TIMBRE_PARAM].value*15.;
	settings.ad_fm	   	= params[AD_MODULATION_PARAM].value*15.;
	settings.ad_color  	= params[AD_COLOR_PARAM].value*15.;

	// Display - return to SHAPE after 2s
	if (last_setting_changed != braids::SETTING_OSCILLATOR_SHAPE) {
		disp_timeout++;
	}
	if (disp_timeout > 1.0*engineGetSampleRate()) {
		last_setting_changed = braids::SETTING_OSCILLATOR_SHAPE;
		disp_timeout=0;
	}
	uint8_t *last_settingsArray = &last_settings.shape;
	uint8_t *settingsArray = &settings.shape;
	for (int i = 0; i < 20; i++) {
		if (settingsArray[i] != last_settingsArray[i]) {
			last_settingsArray[i] = settingsArray[i];
			last_setting_changed = static_cast<braids::Setting>(i);
			disp_timeout=0;	
		}
	}	

	// Trigger
	bool trig = inputs[TRIG_INPUT].value >= 1.0;
	if (!lastTrig && trig) {
		trigger_detected_flag = trig;
	}
	lastTrig = trig;

	if ( trigger_detected_flag ) {		
      trigger_delay = settings.trig_delay
          ? (1 << settings.trig_delay) : 0;
      ++trigger_delay;
      trigger_detected_flag = false;
    }
    if (trigger_delay) {
      --trigger_delay;
      if (trigger_delay == 0) {
        trigger_flag = true;
      }
	}

	// Quantizer
    if (current_scale != settings.quantizer_scale) {
      current_scale = settings.quantizer_scale;
      quantizer.Configure(braids::scales[current_scale]);
    }

	// Render frames
	if (outputBuffer.empty()) {

		envelope.Update( settings.ad_attack*8, settings.ad_decay*8 );
	  	uint32_t ad_value = envelope.Render();

		float fm = params[FM_PARAM].value * inputs[FM_INPUT].value;

		// Set shape
		if (paques) {
			osc.set_shape(braids::MACRO_OSC_SHAPE_QUESTION_MARK);	
		} else {
			int shape = roundf(params[SHAPE_PARAM].value * braids::MACRO_OSC_SHAPE_LAST_ACCESSIBLE_FROM_META);
			if (settings.meta_modulation) {
				shape += roundf(fm / 10.0 * braids::MACRO_OSC_SHAPE_LAST_ACCESSIBLE_FROM_META);
			}
			settings.shape = clamp(shape, 0, braids::MACRO_OSC_SHAPE_LAST_ACCESSIBLE_FROM_META);

			// Setup oscillator from settings
			osc.set_shape((braids::MacroOscillatorShape) settings.shape);
		}

		// Set timbre/modulation
		float timbre = params[TIMBRE_PARAM].value + params[MODULATION_PARAM].value * inputs[TIMBRE_INPUT].value / 5.0;
		float modulation = params[COLOR_PARAM].value + inputs[COLOR_INPUT].value / 5.0;

	    timbre += ad_value/65535. * settings.ad_timbre / 16.;
	    modulation += ad_value/65535. * settings.ad_color / 16.;

		int16_t param1 = rescale(clamp(timbre, 0.0, 1.0), 0.0, 1.0, 0, INT16_MAX);
		int16_t param2 = rescale(clamp(modulation, 0.0, 1.0), 0.0, 1.0, 0, INT16_MAX);
		osc.set_parameters(param1, param2);

		// Set pitch
		float pitchV = inputs[PITCH_INPUT].value + params[COARSE_PARAM].value + params[FINE_PARAM].value / 12.0;
		if (!settings.meta_modulation)
			pitchV += fm;
		if (lowCpu)
			pitchV += log2f(96000.0 / engineGetSampleRate());
		int32_t pitch = (pitchV * 12.0 + 60) * 128;

		// pitch_range
		if (settings.pitch_range == braids::PITCH_RANGE_EXTERNAL ||
			settings.pitch_range == braids::PITCH_RANGE_LFO) {
			// no change - calibration not implemented
		} else if (settings.pitch_range == braids::PITCH_RANGE_FREE) {
			pitch = pitch - 1638;
		} else if (settings.pitch_range == braids::PITCH_RANGE_440) {
			pitch = 69 << 7;
		} else { // PITCH_RANGE_EXTENDED
			pitch -= 60 << 7;
			pitch = (pitch - 1638) * 9 >> 1;
			pitch += 60 << 7;
		}

		pitch = quantizer.Process( pitch, (60 + settings.quantizer_root) << 7);

		// Check if the pitch has changed to cause an auto-retrigger
		int32_t pitch_delta = pitch - previous_pitch;
		if (settings.auto_trig &&
			(pitch_delta >= 0x40 || -pitch_delta >= 0x40)) {
			trigger_detected_flag = true;
		}
		previous_pitch = pitch;

		pitch += jitter_source.Render(settings.vco_drift);
		pitch += ad_value * settings.ad_fm >> 7;

		pitch = clamp(int(pitch), 0, 16383);

  		if (settings.vco_flatten) {
    		pitch = braids::Interpolate88(braids::lut_vco_detune, pitch << 2);
  		}

	    // pitch_transposition() 
		int32_t t = settings.pitch_range == braids::PITCH_RANGE_LFO ? -(36 << 7) : 0;
    	t += (static_cast<int32_t>(settings.pitch_octave) - 2) * 12 * 128;
		osc.set_pitch(pitch + t);
		//osc.set_pitch(pitch);

		if ( trigger_flag ) {		
			osc.Strike();
	    	envelope.Trigger(braids::ENV_SEGMENT_ATTACK);
			trigger_flag = false;
		}

		// TODO: add a sync input buffer (must be sample rate converted)
		uint8_t sync_buffer[24] = {};

		int16_t render_buffer[24];
		osc.Render(sync_buffer, render_buffer, 24);

		// Signature waveshaping, decimation (not yet supported), and bit reduction (not yet supported)
		int16_t sample = 0;
  		size_t decimation_factor = decimation_factors[settings.sample_rate];
  		uint16_t bit_mask = bit_reduction_masks[settings.resolution];
		int32_t gain = settings.ad_vca>0 ? ad_value : 65535;
		uint16_t signature = settings.signature * settings.signature * 4095;
		for (size_t i = 0; i < 24; i++) {
			//const int16_t bit_mask = 0xffff;
		    if ((i % decimation_factor) == 0) {
				sample = render_buffer[i] & bit_mask;
			}
		    sample = sample * gain_lp >> 16;
    		gain_lp += (gain - gain_lp) >> 4;
			int16_t warped = ws.Transform(sample);
			render_buffer[i] = stmlib::Mix(sample, warped, signature);
		}

		if (lowCpu) {
			for (int i = 0; i < 24; i++) {
				Frame<1> f;
				f.samples[0] = render_buffer[i] / 32768.0;
				outputBuffer.push(f);
			}
		}
		else {
			// Sample rate convert
			Frame<1> in[24];
			for (int i = 0; i < 24; i++) {
				in[i].samples[0] = render_buffer[i] / 32768.0;
			}
			src.setRates(96000, engineGetSampleRate());

			int inLen = 24;
			int outLen = outputBuffer.capacity();
			src.process(in, &inLen, outputBuffer.endData(), &outLen);
			outputBuffer.endIncr(outLen);
		}
	}

	// Output
	if (!outputBuffer.empty()) {
		Frame<1> f = outputBuffer.shift();
		outputs[OUT_OUTPUT].value = 5.0 * f.samples[0];
	}
}

struct CornrowsXDisplay : TransparentWidget {

	CornrowsX *module;
	std::shared_ptr<Font> font;

	CornrowsXDisplay() {
		font = Font::load(assetPlugin(plugin, "res/hdad-segment14-1.002/Segment14.ttf"));
	}

	void draw(NVGcontext *vg) override {
		int shape=0;
		const char *text="";

		// Background
		NVGcolor backgroundColor = nvgRGB(0x30, 0x10, 0x10);
		NVGcolor borderColor = nvgRGB(0xd0, 0xd0, 0xd0);
		nvgBeginPath(vg);
		nvgRoundedRect(vg, 0.0, 0.0, box.size.x, box.size.y, 5.0);
		nvgFillColor(vg, backgroundColor);
		nvgFill(vg);
		nvgStrokeWidth(vg, 1.5);
		nvgStrokeColor(vg, borderColor);
		nvgStroke(vg);

		nvgFontSize(vg, 20.);
		nvgFontFaceId(vg, font->handle);
		nvgTextLetterSpacing(vg, 2.);

		Vec textPos = Vec(5, 28);
		NVGcolor textColor = nvgRGB(0xff, 0x00, 0x00);
		nvgFillColor(vg, nvgTransRGBA(textColor, 16));
		nvgText(vg, textPos.x, textPos.y, "~~~~", NULL);
		nvgFillColor(vg, textColor);
		//blink
		if ( module->disp_timeout & 0x1000 ) return;
		if (module->last_setting_changed == braids::SETTING_OSCILLATOR_SHAPE) {
			shape = module->settings.shape;
			if (module->paques) {
			  text = "  49"; 		
			} else {
			  text = algo_values[shape];
			}
		}
		if (module->last_setting_changed == braids::SETTING_META_MODULATION) {
		    shape = module->settings.ad_timbre;
		 	text = "META";
		}
		if (module->last_setting_changed == braids::SETTING_RESOLUTION) {
		    shape = module->settings.resolution;
		 	text = bits_values[shape];
		}
		if (module->last_setting_changed == braids::SETTING_SAMPLE_RATE) {
		    shape = module->settings.sample_rate;
		 	text = rates_values[shape];
		}
		if (module->last_setting_changed == braids::SETTING_TRIG_SOURCE) {
		    shape = module->settings.ad_timbre;
		 	text = "AUTO";
		}
		if (module->last_setting_changed == braids::SETTING_TRIG_DELAY) {
		    shape = module->settings.trig_delay;
		 	text = trig_delay_values[shape];
		}
		if (module->last_setting_changed == braids::SETTING_AD_ATTACK) {
		    shape = module->settings.ad_attack;
		 	text = zero_to_fifteen_values[shape];
		}
		if (module->last_setting_changed == braids::SETTING_AD_DECAY) {
		    shape = module->settings.ad_decay;
		 	text = zero_to_fifteen_values[shape];
		}
		if (module->last_setting_changed == braids::SETTING_AD_FM) {
		    shape = module->settings.ad_fm;
		 	text = zero_to_fifteen_values[shape];
		}
		if (module->last_setting_changed == braids::SETTING_AD_TIMBRE) {
		    shape = module->settings.ad_color;
		 	text = zero_to_fifteen_values[shape];
		}
		if (module->last_setting_changed == braids::SETTING_AD_COLOR) {
		    shape = module->settings.ad_color;
		 	text = zero_to_fifteen_values[shape];
		}
		if (module->last_setting_changed == braids::SETTING_AD_VCA) {
		    shape = module->settings.ad_color;
		 	text = "\\VCA";
		}
		if (module->last_setting_changed == braids::SETTING_PITCH_RANGE) {
		    shape = module->settings.pitch_range;
		 	text = pitch_range_values[shape];
		}
		if (module->last_setting_changed == braids::SETTING_PITCH_OCTAVE) {
		    shape = module->settings.pitch_octave;
		 	text = octave_values[shape];
		}
		if (module->last_setting_changed == braids::SETTING_QUANTIZER_SCALE) {
		    shape = module->settings.quantizer_scale;
		 	text = quantization_values[shape];
		}
		if (module->last_setting_changed == braids::SETTING_QUANTIZER_ROOT) {
		    shape = module->settings.quantizer_root;
		 	text = note_values[shape];
		}
		if (module->last_setting_changed == braids::SETTING_VCO_FLATTEN) {
		    shape = module->settings.quantizer_scale;
		 	text = "FLAT";
		}
		if (module->last_setting_changed == braids::SETTING_VCO_DRIFT) {
		    shape = module->settings.quantizer_scale;
		 	text = "DRFT";
		}
		if (module->last_setting_changed == braids::SETTING_SIGNATURE) {
		    shape = module->settings.quantizer_scale;
		 	text = "SIGN";
		}
		nvgText(vg, textPos.x, textPos.y, text, NULL);
		//nvgText(vg, textPos.x, textPos.y, algo_values[shape], NULL);
	}
};

struct CornrowsXWidget : ModuleWidget {

	Menu *createContextMenu() override;

	CornrowsXWidget(CornrowsX *module)  : ModuleWidget(module) {

		box.size = Vec( 8 * RACK_GRID_WIDTH, RACK_GRID_HEIGHT );

		{
			SVGPanel *panel = new SVGPanel();
			panel->setBackground(SVG::load(assetPlugin(plugin, "res/Cornrows.svg")));
			panel->box.size = box.size;
			addChild(panel);	
		}

		{
			CornrowsXDisplay *display = new CornrowsXDisplay();
			display->box.pos = Vec(8, 32);
			display->box.size = Vec(80., 34.);
			display->module = module;
			addChild(display);
		}

		const float x1 = 4.;
		const float xh = 30.;
		const float x2 = x1+xh;
		const float x3 = x1+2*xh;
		const float x4 = x1+3*xh;
//		const float x5 = x1+4*xh;
//		const float x6 = x1+5*xh;

		const float y1 = 115;	
		const float yh = 36.;

		addParam(ParamWidget::create<sp_Encoder>(Vec(x3+4, 78), module, CornrowsX::SHAPE_PARAM, 0.0, 1.0, 0.0));

		addInput(Port::create<sp_Port>(Vec(x1, y1-1*yh), Port::INPUT, module, CornrowsX::TRIG_INPUT));
		addParam(ParamWidget::create<sp_Trimpot>(Vec(x2, y1-1*yh), module, CornrowsX::TRIG_DELAY_PARAM,  0.0, 1.0, 0.0));

		addParam(ParamWidget::create<sp_SmallBlackKnob>(Vec(x1, y1+0*yh), module, CornrowsX::ATT_PARAM,   0.0, 1.0, 0.0));
		addParam(ParamWidget::create<sp_SmallBlackKnob>(Vec(x2, y1+0*yh), module, CornrowsX::DEC_PARAM,   0.0, 1.0, 0.5));

		addInput(Port::create<sp_Port>(Vec(x1, y1+yh), Port::INPUT, module, CornrowsX::PITCH_INPUT));
		addParam(ParamWidget::create<sp_SmallBlackKnob>(Vec(x2, y1+yh), module, CornrowsX::FINE_PARAM, -1.0, 1.0, 0.0));
		addParam(ParamWidget::create<sp_SmallBlackKnob>(Vec(x3, y1+yh), module, CornrowsX::COARSE_PARAM, -2.0, 2.0, 0.0));
		addParam(ParamWidget::create<sp_Trimpot>(Vec(x4, y1+yh), module, CornrowsX::PITCH_OCTAVE_PARAM,  0.0, 1.0, 0.5));

		addParam(ParamWidget::create<sp_SmallBlackKnob>(Vec(x1, y1+2*yh), module, CornrowsX::ROOT_PARAM,  0.0, 1.0, 0.0));
		addParam(ParamWidget::create<sp_SmallBlackKnob>(Vec(x2, y1+2*yh), module, CornrowsX::SCALE_PARAM, 0.0, 1.0, 0.0));
		addParam(ParamWidget::create<sp_Trimpot>(Vec(x4, y1+2*yh), module, CornrowsX::PITCH_RANGE_PARAM,  0.0, 1.0, 0.));
		
		addInput(Port::create<sp_Port>(Vec(x1, y1+3*yh), Port::INPUT, module, CornrowsX::FM_INPUT));
		addParam(ParamWidget::create<sp_SmallBlackKnob>(Vec(x3, y1+3*yh), module, CornrowsX::FM_PARAM, -1.0, 1.0, 0.0));
		addParam(ParamWidget::create<sp_Trimpot>(Vec(x4, y1+3*yh), module, CornrowsX::AD_MODULATION_PARAM, 0.0, 1.0, 0.0));

		addInput(Port::create<sp_Port>(Vec(x1, y1+4*yh), Port::INPUT, module, CornrowsX::TIMBRE_INPUT));
		addParam(ParamWidget::create<sp_Trimpot>(Vec(x2, y1+4*yh), module, CornrowsX::MODULATION_PARAM, -1.0, 1.0, 0.0));
		addParam(ParamWidget::create<sp_SmallBlackKnob>(Vec(x3, y1+4*yh), module, CornrowsX::TIMBRE_PARAM, 0.0, 1.0, 0.5));
		addParam(ParamWidget::create<sp_Trimpot>(Vec(x4, y1+4*yh), module, CornrowsX::AD_TIMBRE_PARAM,	   0.0, 1.0, 0.0));

		addInput(Port::create<sp_Port>(Vec(x1, y1+5*yh), Port::INPUT, module, CornrowsX::COLOR_INPUT));
		addParam(ParamWidget::create<sp_SmallBlackKnob>(Vec(x3, y1+5*yh), module, CornrowsX::COLOR_PARAM, 0.0, 1.0, 0.5));
		addParam(ParamWidget::create<sp_Trimpot>(Vec(x4, y1+5*yh), module, CornrowsX::AD_COLOR_PARAM, 	   0.0, 1.0, 0.0));

		addParam(ParamWidget::create<sp_SmallBlackKnob>(Vec(x1, y1+5.75*yh), module, CornrowsX::BITS_PARAM,  0.0, 1.0, 1.0));
		addParam(ParamWidget::create<sp_SmallBlackKnob>(Vec(x2, y1+5.75*yh), module, CornrowsX::RATE_PARAM,  0.0, 1.0, 1.0));
		addOutput(Port::create<sp_Port>(Vec(x4, y1+5.75*yh), Port::OUTPUT, module, CornrowsX::OUT_OUTPUT));

	}
};

struct CornrowsXSettingItem : MenuItem {
	uint8_t *setting = NULL;
	uint8_t offValue = 0;
	uint8_t onValue = 1;
	void onAction(EventAction &e) override {
		// Toggle setting
		*setting = (*setting == onValue) ? offValue : onValue;
	}
	void step() override {
		rightText = (*setting == onValue) ? "✔" : "";
		MenuItem::step();
	}
};

struct CornrowsXLowCpuItem : MenuItem {
	CornrowsX *braids;
	void onAction(EventAction &e) override {
		braids->lowCpu = !braids->lowCpu;
	}
	void step() override {
		rightText = (braids->lowCpu) ? "✔" : "";
		MenuItem::step();
	}
};

struct CornrowsXPaquesItem : MenuItem {
	CornrowsX *braids;
	void onAction(EventAction &e) override {
		braids->paques = !braids->paques;
	}
	void step() override {
		rightText = (braids->paques) ? "✔" : "";
		MenuItem::step();
	}
};

Menu *CornrowsXWidget::createContextMenu() {
	Menu *menu = ModuleWidget::createContextMenu();

	CornrowsX *braids = dynamic_cast<CornrowsX*>(module);
	assert(braids);

	menu->addChild(construct<MenuLabel>());
	menu->addChild(construct<MenuLabel>(&MenuLabel::text, "Options"));
	menu->addChild(construct<CornrowsXSettingItem>(&MenuItem::text, "META", &CornrowsXSettingItem::setting, &braids->settings.meta_modulation));
	menu->addChild(construct<CornrowsXSettingItem>(&MenuItem::text, "AUTO", &CornrowsXSettingItem::setting, &braids->settings.auto_trig));
	menu->addChild(construct<CornrowsXSettingItem>(&MenuItem::text, "|\\VCA", &CornrowsXSettingItem::setting, &braids->settings.ad_vca));
	menu->addChild(construct<CornrowsXSettingItem>(&MenuItem::text, "FLAT", &CornrowsXSettingItem::setting, &braids->settings.vco_flatten, &CornrowsXSettingItem::onValue, 4));
	menu->addChild(construct<CornrowsXSettingItem>(&MenuItem::text, "DRFT", &CornrowsXSettingItem::setting, &braids->settings.vco_drift, &CornrowsXSettingItem::onValue, 4));
	menu->addChild(construct<CornrowsXSettingItem>(&MenuItem::text, "SIGN", &CornrowsXSettingItem::setting, &braids->settings.signature, &CornrowsXSettingItem::onValue, 4));
	menu->addChild(construct<CornrowsXLowCpuItem>(&MenuItem::text, "Low CPU", &CornrowsXLowCpuItem::braids, braids));
	menu->addChild(construct<CornrowsXPaquesItem>(&MenuItem::text, "Paques",  &CornrowsXPaquesItem::braids, braids));

	return menu;
}

} // namespace rack_plugin_Southpole

using namespace rack_plugin_Southpole;

RACK_PLUGIN_MODEL_INIT(Southpole, CornrowsX) {
   Model *modelCornrowsX = Model::create<CornrowsX,CornrowsXWidget>("Southpole", "CornrowsX", 	"CornrowsX - macro osc", OSCILLATOR_TAG, WAVESHAPER_TAG);
   return modelCornrowsX;
}
