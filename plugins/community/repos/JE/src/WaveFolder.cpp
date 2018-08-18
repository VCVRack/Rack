#include "JE.hpp"
#include "../common/constants.hpp"
#include "../utils/meta.hpp"
#include "../ext/LambertW/LambertW.h"

// STL
#include <cmath>
#include <iostream>

/*
http://smc2017.aalto.fi/media/materials/proceedings/SMC17_p336.pdf
*/

namespace rack_plugin_JE {

struct WaveFolder : rack::Module
{
	enum ParamIds
	{
		INPUT_GAIN_PARAM = 0,
		DC_OFFSET_PARAM,

		OUTPUT_GAIN_PARAM,

		RESISTOR_PARAM,
		LOAD_RESISTOR_PARAM,

		NUM_PARAMS
	};

	enum InputIds
	{
		INPUT_INPUT = 0,
		INPUT_GAIN_INPUT,
		DC_OFFSET_INPUT,

		OUTPUT_GAIN_INPUT,

		NUM_INPUTS
	};

	enum OutputIds
	{
		OUTPUT_OUTPUT = 0,

		NUM_OUTPUTS
	};

	enum LightIds
	{
		NUM_LIGHTS = 0
	};

	WaveFolder()
	: Module(NUM_PARAMS, NUM_INPUTS, NUM_OUTPUTS, NUM_LIGHTS)
	{}

	inline float getParameterValue(const ParamIds id) const
	{
		return params[id].value;
	}

	inline float getAudioInputValue(const InputIds id) const
	{
		return inputs[id].value;
	}

	inline float getControlInputValue(const InputIds id) const
	{
		return inputs[id].value;
	}

	inline void setOutputValue(OutputIds id, float v)
	{
		outputs[id].value = v;
	}

	inline bool needToStep()
	{
		if (!outputs[OUTPUT_OUTPUT].active)
			return false;

		if (!inputs[INPUT_INPUT].active)
		{
			outputs[OUTPUT_OUTPUT].value = 0.f;
			return false;
		}

		return true;
	}

	inline void updateResistors()
	{
		if (meta::updateIfDifferent(m_resistor, getParameterValue(RESISTOR_PARAM)))
		{
			m_alpha = m_loadResistor2 / m_resistor;
			m_beta = (m_resistor + m_loadResistor2) / (m_thermalVoltage * m_resistor);
		}

		if (meta::updateIfDifferent(m_loadResistor, getParameterValue(LOAD_RESISTOR_PARAM)))
		{
			m_loadResistor2 = m_loadResistor * 2.f;
			m_alpha = m_loadResistor2 / m_resistor;
			m_beta = (m_resistor + m_loadResistor2) / (m_thermalVoltage * m_resistor);
			m_delta = (m_loadResistor * m_saturationCurrent) / m_thermalVoltage;
		}
	}

	inline float getGainedOffsetedInputValue()
	{
		return (getAudioInputValue(INPUT_INPUT) * meta::clamp(getParameterValue(INPUT_GAIN_PARAM) + getControlInputValue(INPUT_GAIN_INPUT) / g_audioPeakVoltage, 0.f, 1.f)) +
			   (getParameterValue(DC_OFFSET_PARAM) + getControlInputValue(DC_OFFSET_INPUT)) / 2.f;
	}

	inline float waveFolder(float in)
	{
		const float theta = (in >= 0.f) ? 1.f : -1.f;
		return theta * m_thermalVoltage * utl::LambertW<0>(m_delta * meta::exp(theta * m_beta * in)) - m_alpha * in;
	}

	inline void step() override
	{
		if (!needToStep())
			return;

		updateResistors();

		setOutputValue(OUTPUT_OUTPUT, std::tanh(waveFolder(getGainedOffsetedInputValue())) *
			meta::clamp(getParameterValue(OUTPUT_GAIN_PARAM) + getControlInputValue(OUTPUT_GAIN_INPUT), 0.f, 10.f));
	}

private:
	const float m_thermalVoltage = 0.026f;
	const float m_saturationCurrent = 10e-17f;
	float m_resistor = 15000.f;
	float m_loadResistor = 7500.f;
	float m_loadResistor2 = m_loadResistor * 2.f;
	// Derived values
	float m_alpha = m_loadResistor2 / m_resistor;
	float m_beta = (m_resistor + m_loadResistor2) / (m_thermalVoltage * m_resistor);
	float m_delta = (m_loadResistor * m_saturationCurrent) / m_thermalVoltage;
};

struct WaveFolderWidget : rack::ModuleWidget
{
	WaveFolderWidget(WaveFolder *module);
};

WaveFolderWidget::WaveFolderWidget(WaveFolder *module) : ModuleWidget(module)
{
	box.size = rack::Vec(15*7, 380);

	{
		rack::SVGPanel *panel = new rack::SVGPanel();
		panel->box.size = box.size;
		panel->setBackground(rack::SVG::load(assetPlugin(plugin, "res/CleanWaveFolder.svg")));
		addChild(panel);
	}

	addChild(rack::createScrew<rack::ScrewBlack>(rack::Vec(15, 0)));
	addChild(rack::createScrew<rack::ScrewBlack>(rack::Vec(box.size.x-30, 0)));
	addChild(rack::createScrew<rack::ScrewBlack>(rack::Vec(15, 365)));
	addChild(rack::createScrew<rack::ScrewBlack>(rack::Vec(box.size.x-30, 365)));

	float yOffset = 67.f;

	float portY = 63.f;
	float knobY = 57.f;
	addInput(rack::createInput<rack::PJ301MPort>(rack::Vec(9, portY), module, WaveFolder::INPUT_GAIN_INPUT));
	addParam(rack::createParam<rack::RoundBlackKnob>(rack::Vec(54, knobY), module, WaveFolder::INPUT_GAIN_PARAM, 0.0f, 1.0f, 0.1f));

	portY += yOffset;
	knobY += yOffset;
	addInput(rack::createInput<rack::PJ301MPort>(rack::Vec(9, portY), module, WaveFolder::DC_OFFSET_INPUT));
	addParam(rack::createParam<rack::RoundBlackKnob>(rack::Vec(54, knobY), module, WaveFolder::DC_OFFSET_PARAM, -5.0f, 5.0f, 0.0f));

	portY += yOffset;
	knobY += yOffset;
	addInput(rack::createInput<rack::PJ301MPort>(rack::Vec(9, portY), module, WaveFolder::OUTPUT_GAIN_INPUT));
	addParam(rack::createParam<rack::RoundBlackKnob>(rack::Vec(54, knobY), module, WaveFolder::OUTPUT_GAIN_PARAM, 0.0f, 10.0f, 1.0f));

	portY += yOffset;
	knobY += yOffset;
	addInput(rack::createInput<rack::PJ301MPort>(rack::Vec(18, portY), module, WaveFolder::INPUT_INPUT));
	addOutput(rack::createOutput<rack::PJ301MPort>(rack::Vec(box.size.x-43, portY), module, WaveFolder::OUTPUT_OUTPUT));

	portY += yOffset;
	knobY += yOffset;
	const float y = knobY - 6.f;
	float xOffset = 52.f;
	float x = 9.f;
	addParam(rack::createParam<rack::RoundSmallBlackKnob>(rack::Vec(x, y), module, WaveFolder::RESISTOR_PARAM, 10000.f, 100000.f, 15000.f));
	x += xOffset;
	addParam(rack::createParam<rack::RoundSmallBlackKnob>(rack::Vec(x, y), module, WaveFolder::LOAD_RESISTOR_PARAM, 1000.f, 10000.f, 7500.f));
}

} // namespace rack_plugin_JE

using namespace rack_plugin_JE;

RACK_PLUGIN_MODEL_INIT(JE, SimpleWaveFolder) {
   return Model::create<WaveFolder, WaveFolderWidget>(
		"JE", "SimpleWaveFolder", "Simple Wave Folder",
		rack::EFFECT_TAG, rack::WAVESHAPER_TAG
                                                 );
}
