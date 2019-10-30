#include "plugin.hpp"
#include <audio.hpp>
#include <app.hpp>
#include <mutex>
#include <chrono>
#include <thread>
#include <mutex>
#include <condition_variable>


namespace rack {
namespace core {


template <int NUM_AUDIO_INPUTS, int NUM_AUDIO_OUTPUTS>
struct AudioInterface : Module, audio::Port {
	enum ParamIds {
		NUM_PARAMS
	};
	enum InputIds {
		ENUMS(AUDIO_INPUTS, NUM_AUDIO_INPUTS),
		NUM_INPUTS
	};
	enum OutputIds {
		ENUMS(AUDIO_OUTPUTS, NUM_AUDIO_OUTPUTS),
		NUM_OUTPUTS
	};
	enum LightIds {
		ENUMS(INPUT_LIGHTS, NUM_AUDIO_INPUTS / 2),
		ENUMS(OUTPUT_LIGHTS, NUM_AUDIO_OUTPUTS / 2),
		NUM_LIGHTS
	};

	dsp::DoubleRingBuffer<dsp::Frame<NUM_AUDIO_INPUTS>, 32768> inputBuffer;
	dsp::DoubleRingBuffer<dsp::Frame<NUM_AUDIO_OUTPUTS>, 32768> outputBuffer;

	dsp::SampleRateConverter<NUM_AUDIO_INPUTS> inputSrc;
	dsp::SampleRateConverter<NUM_AUDIO_OUTPUTS> outputSrc;

	AudioInterface() {
		config(NUM_PARAMS, NUM_INPUTS, NUM_OUTPUTS, NUM_LIGHTS);
		for (int i = 0; i < NUM_AUDIO_INPUTS; i++)
			configInput(AUDIO_INPUTS + i, string::f("To device %d", i + 1));
		for (int i = 0; i < NUM_AUDIO_OUTPUTS; i++)
			configOutput(AUDIO_OUTPUTS + i, string::f("From device %d", i + 1));
		maxChannels = std::max(NUM_AUDIO_INPUTS, NUM_AUDIO_OUTPUTS);
		inputSrc.setQuality(6);
		outputSrc.setQuality(6);
	}

	~AudioInterface() {
		// Close stream here before destructing AudioInterfacePort, so the mutexes are still valid when waiting to close.
		setDeviceId(-1, 0);
	}

	void onSampleRateChange(const SampleRateChangeEvent& e) override {
		inputBuffer.clear();
		outputBuffer.clear();
	}

	void process(const ProcessArgs& args) override {
		// Get inputs
		if (!inputBuffer.full()) {
			dsp::Frame<NUM_AUDIO_INPUTS> inputFrame;
			for (int i = 0; i < NUM_AUDIO_INPUTS; i++) {
				inputFrame.samples[i] = inputs[AUDIO_INPUTS + i].getVoltage() / 10.f;
			}
			inputBuffer.push(inputFrame);
		}

		// Set outputs
		if (!outputBuffer.empty()) {
			dsp::Frame<NUM_AUDIO_OUTPUTS> outputFrame = outputBuffer.shift();
			for (int i = 0; i < NUM_AUDIO_OUTPUTS; i++) {
				outputs[AUDIO_OUTPUTS + i].setVoltage(10.f * outputFrame.samples[i]);
			}
		}

		// Turn on light if at least one port is enabled in the nearby pair
		for (int i = 0; i < NUM_AUDIO_INPUTS / 2; i++) {
			lights[INPUT_LIGHTS + i].setBrightness(numOutputs >= 2 * i + 1);
		}
		for (int i = 0; i < NUM_AUDIO_OUTPUTS / 2; i++) {
			lights[OUTPUT_LIGHTS + i].setBrightness(numInputs >= 2 * i + 1);
		}
	}

	json_t* dataToJson() override {
		json_t* rootJ = json_object();
		json_object_set_new(rootJ, "audio", audio::Port::toJson());
		return rootJ;
	}

	void dataFromJson(json_t* rootJ) override {
		json_t* audioJ = json_object_get(rootJ, "audio");
		if (audioJ)
			audio::Port::fromJson(audioJ);
	}

	void onReset() override {
		setDeviceId(-1, 0);
	}

	// audio::Port

	void processStream(const float* input, float* output, int frames) override {
		// Claim primary module if there is none
		if (!APP->engine->getPrimaryModule()) {
			APP->engine->setPrimaryModule(this);
		}

		// Clear output in case the audio driver uses this buffer in another thread before this method returns. (Not sure if any do this in practice.)
		std::memset(output, 0, sizeof(float) * numOutputs * frames);

		// Initialize sample rate converters
		int engineSampleRate = (int) APP->engine->getSampleRate();
		outputSrc.setRates((int) sampleRate, engineSampleRate);
		outputSrc.setChannels(numInputs);
		inputSrc.setRates(engineSampleRate, (int) sampleRate);
		inputSrc.setChannels(numOutputs);

		int engineFrames = 0;

		if (numInputs > 0) {
			// audio input -> module output
			dsp::Frame<NUM_AUDIO_OUTPUTS> inputAudioBuffer[frames];
			std::memset(inputAudioBuffer, 0, sizeof(inputAudioBuffer));
			for (int i = 0; i < frames; i++) {
				for (int j = 0; j < std::min(numInputs, NUM_AUDIO_OUTPUTS); j++) {
					inputAudioBuffer[i].samples[j] = input[i * numInputs + j];
				}
			}
			int inputAudioFrames = frames;
			int outputFrames = outputBuffer.capacity();
			outputSrc.process(inputAudioBuffer, &inputAudioFrames, outputBuffer.endData(), &outputFrames);
			outputBuffer.endIncr(outputFrames);
			engineFrames = outputBuffer.size();
		}
		else {
			// Upper bound on number of frames so that `outputAudioFrames >= frames` at the end of this method.
			double ratio = (double) engineSampleRate / sampleRate;
			engineFrames = std::ceil(frames * ratio - inputBuffer.size());
			engineFrames = std::max(engineFrames, 0);
		}

		// Step engine to consume the entire output buffer
		if (APP->engine->getPrimaryModule() == this && engineFrames > 0) {
			APP->engine->step(engineFrames);
		}

		if (numOutputs > 0) {
			// module input -> audio output
			dsp::Frame<NUM_AUDIO_OUTPUTS> outputAudioBuffer[frames];
			int inputFrames = inputBuffer.size();
			int outputAudioFrames = frames;
			inputSrc.process(inputBuffer.startData(), &inputFrames, outputAudioBuffer, &outputAudioFrames);
			inputBuffer.startIncr(inputFrames);
			for (int i = 0; i < outputAudioFrames; i++) {
				for (int j = 0; j < std::min(numOutputs, NUM_AUDIO_INPUTS); j++) {
					output[i * numOutputs + j] = outputAudioBuffer[i].samples[j];
				}
			}
		}

		// If we're left with too many output samples, flush the buffer.
		// if (inputBuffer.size() >= 2.f * ) {
		// 	inputBuffer.clear();
		// }

		// DEBUG("%p %d: frames %d\toutputBuffer %d inputBuffer %d engineFrames %d\t",
		// 	this, APP->engine->getPrimaryModule() == this, frames,
		// 	outputBuffer.size(), inputBuffer.size(), engineFrames);
	}

	void onOpenStream() override {
		inputBuffer.clear();
		outputBuffer.clear();
	}

	void onCloseStream() override {
		inputBuffer.clear();
		outputBuffer.clear();
	}

	void onChannelsChange() override {
	}
};


template <typename TAudioInterface>
struct PrimaryModuleItem : MenuItem {
	TAudioInterface* module;

	void onAction(const event::Action& e) override {
		APP->engine->setPrimaryModule(module);
	}
};


template <int NUM_AUDIO_INPUTS, int NUM_AUDIO_OUTPUTS>
struct AudioInterfaceWidget : ModuleWidget {
	typedef AudioInterface<NUM_AUDIO_INPUTS, NUM_AUDIO_OUTPUTS> TAudioInterface;

	AudioInterfaceWidget(TAudioInterface* module) {
		setModule(module);

		if (NUM_AUDIO_INPUTS == 8 && NUM_AUDIO_OUTPUTS == 8) {
			setPanel(APP->window->loadSvg(asset::system("res/Core/AudioInterface.svg")));

			addChild(createWidget<ScrewSilver>(Vec(RACK_GRID_WIDTH, 0)));
			addChild(createWidget<ScrewSilver>(Vec(box.size.x - 2 * RACK_GRID_WIDTH, 0)));
			addChild(createWidget<ScrewSilver>(Vec(RACK_GRID_WIDTH, RACK_GRID_HEIGHT - RACK_GRID_WIDTH)));
			addChild(createWidget<ScrewSilver>(Vec(box.size.x - 2 * RACK_GRID_WIDTH, RACK_GRID_HEIGHT - RACK_GRID_WIDTH)));

			addInput(createInput<PJ301MPort>(mm2px(Vec(3.7069211, 55.530807)), module, TAudioInterface::AUDIO_INPUTS + 0));
			addInput(createInput<PJ301MPort>(mm2px(Vec(15.307249, 55.530807)), module, TAudioInterface::AUDIO_INPUTS + 1));
			addInput(createInput<PJ301MPort>(mm2px(Vec(26.906193, 55.530807)), module, TAudioInterface::AUDIO_INPUTS + 2));
			addInput(createInput<PJ301MPort>(mm2px(Vec(38.506519, 55.530807)), module, TAudioInterface::AUDIO_INPUTS + 3));
			addInput(createInput<PJ301MPort>(mm2px(Vec(3.7069209, 70.144905)), module, TAudioInterface::AUDIO_INPUTS + 4));
			addInput(createInput<PJ301MPort>(mm2px(Vec(15.307249, 70.144905)), module, TAudioInterface::AUDIO_INPUTS + 5));
			addInput(createInput<PJ301MPort>(mm2px(Vec(26.906193, 70.144905)), module, TAudioInterface::AUDIO_INPUTS + 6));
			addInput(createInput<PJ301MPort>(mm2px(Vec(38.506519, 70.144905)), module, TAudioInterface::AUDIO_INPUTS + 7));

			addOutput(createOutput<PJ301MPort>(mm2px(Vec(3.7069209, 92.143906)), module, TAudioInterface::AUDIO_OUTPUTS + 0));
			addOutput(createOutput<PJ301MPort>(mm2px(Vec(15.307249, 92.143906)), module, TAudioInterface::AUDIO_OUTPUTS + 1));
			addOutput(createOutput<PJ301MPort>(mm2px(Vec(26.906193, 92.143906)), module, TAudioInterface::AUDIO_OUTPUTS + 2));
			addOutput(createOutput<PJ301MPort>(mm2px(Vec(38.506519, 92.143906)), module, TAudioInterface::AUDIO_OUTPUTS + 3));
			addOutput(createOutput<PJ301MPort>(mm2px(Vec(3.7069209, 108.1443)), module, TAudioInterface::AUDIO_OUTPUTS + 4));
			addOutput(createOutput<PJ301MPort>(mm2px(Vec(15.307249, 108.1443)), module, TAudioInterface::AUDIO_OUTPUTS + 5));
			addOutput(createOutput<PJ301MPort>(mm2px(Vec(26.906193, 108.1443)), module, TAudioInterface::AUDIO_OUTPUTS + 6));
			addOutput(createOutput<PJ301MPort>(mm2px(Vec(38.506523, 108.1443)), module, TAudioInterface::AUDIO_OUTPUTS + 7));

			addChild(createLight<SmallLight<GreenLight>>(mm2px(Vec(12.524985, 54.577202)), module, TAudioInterface::INPUT_LIGHTS + 0));
			addChild(createLight<SmallLight<GreenLight>>(mm2px(Vec(35.725647, 54.577202)), module, TAudioInterface::INPUT_LIGHTS + 1));
			addChild(createLight<SmallLight<GreenLight>>(mm2px(Vec(12.524985, 69.158226)), module, TAudioInterface::INPUT_LIGHTS + 2));
			addChild(createLight<SmallLight<GreenLight>>(mm2px(Vec(35.725647, 69.158226)), module, TAudioInterface::INPUT_LIGHTS + 3));
			addChild(createLight<SmallLight<GreenLight>>(mm2px(Vec(12.524985, 91.147583)), module, TAudioInterface::OUTPUT_LIGHTS + 0));
			addChild(createLight<SmallLight<GreenLight>>(mm2px(Vec(35.725647, 91.147583)), module, TAudioInterface::OUTPUT_LIGHTS + 1));
			addChild(createLight<SmallLight<GreenLight>>(mm2px(Vec(12.524985, 107.17003)), module, TAudioInterface::OUTPUT_LIGHTS + 2));
			addChild(createLight<SmallLight<GreenLight>>(mm2px(Vec(35.725647, 107.17003)), module, TAudioInterface::OUTPUT_LIGHTS + 3));

			AudioWidget* audioWidget = createWidget<AudioWidget>(mm2px(Vec(3.2122073, 14.837339)));
			audioWidget->box.size = mm2px(Vec(44, 28));
			audioWidget->setAudioPort(module);
			addChild(audioWidget);
		}
		else if (NUM_AUDIO_INPUTS == 16 && NUM_AUDIO_OUTPUTS == 16) {
			setPanel(APP->window->loadSvg(asset::system("res/Core/AudioInterface16.svg")));

			addChild(createWidget<ScrewSilver>(Vec(RACK_GRID_WIDTH, 0)));
			addChild(createWidget<ScrewSilver>(Vec(box.size.x - 2 * RACK_GRID_WIDTH, 0)));
			addChild(createWidget<ScrewSilver>(Vec(RACK_GRID_WIDTH, RACK_GRID_HEIGHT - RACK_GRID_WIDTH)));
			addChild(createWidget<ScrewSilver>(Vec(box.size.x - 2 * RACK_GRID_WIDTH, RACK_GRID_HEIGHT - RACK_GRID_WIDTH)));

			addInput(createInputCentered<PJ301MPort>(mm2px(Vec(7.661, 59.638)), module, TAudioInterface::AUDIO_INPUTS + 0));
			addInput(createInputCentered<PJ301MPort>(mm2px(Vec(19.26, 59.638)), module, TAudioInterface::AUDIO_INPUTS + 1));
			addInput(createInputCentered<PJ301MPort>(mm2px(Vec(30.86, 59.638)), module, TAudioInterface::AUDIO_INPUTS + 2));
			addInput(createInputCentered<PJ301MPort>(mm2px(Vec(42.461, 59.638)), module, TAudioInterface::AUDIO_INPUTS + 3));
			addInput(createInputCentered<PJ301MPort>(mm2px(Vec(54.06, 59.638)), module, TAudioInterface::AUDIO_INPUTS + 4));
			addInput(createInputCentered<PJ301MPort>(mm2px(Vec(65.661, 59.638)), module, TAudioInterface::AUDIO_INPUTS + 5));
			addInput(createInputCentered<PJ301MPort>(mm2px(Vec(77.26, 59.638)), module, TAudioInterface::AUDIO_INPUTS + 6));
			addInput(createInputCentered<PJ301MPort>(mm2px(Vec(88.86, 59.638)), module, TAudioInterface::AUDIO_INPUTS + 7));
			addInput(createInputCentered<PJ301MPort>(mm2px(Vec(7.661, 74.251)), module, TAudioInterface::AUDIO_INPUTS + 8));
			addInput(createInputCentered<PJ301MPort>(mm2px(Vec(19.26, 74.251)), module, TAudioInterface::AUDIO_INPUTS + 9));
			addInput(createInputCentered<PJ301MPort>(mm2px(Vec(30.86, 74.251)), module, TAudioInterface::AUDIO_INPUTS + 10));
			addInput(createInputCentered<PJ301MPort>(mm2px(Vec(42.461, 74.251)), module, TAudioInterface::AUDIO_INPUTS + 11));
			addInput(createInputCentered<PJ301MPort>(mm2px(Vec(54.06, 74.251)), module, TAudioInterface::AUDIO_INPUTS + 12));
			addInput(createInputCentered<PJ301MPort>(mm2px(Vec(65.661, 74.251)), module, TAudioInterface::AUDIO_INPUTS + 13));
			addInput(createInputCentered<PJ301MPort>(mm2px(Vec(77.26, 74.251)), module, TAudioInterface::AUDIO_INPUTS + 14));
			addInput(createInputCentered<PJ301MPort>(mm2px(Vec(88.86, 74.251)), module, TAudioInterface::AUDIO_INPUTS + 15));

			addOutput(createOutputCentered<PJ301MPort>(mm2px(Vec(7.661, 96.251)), module, TAudioInterface::AUDIO_OUTPUTS + 0));
			addOutput(createOutputCentered<PJ301MPort>(mm2px(Vec(19.26, 96.251)), module, TAudioInterface::AUDIO_OUTPUTS + 1));
			addOutput(createOutputCentered<PJ301MPort>(mm2px(Vec(30.86, 96.251)), module, TAudioInterface::AUDIO_OUTPUTS + 2));
			addOutput(createOutputCentered<PJ301MPort>(mm2px(Vec(42.461, 96.251)), module, TAudioInterface::AUDIO_OUTPUTS + 3));
			addOutput(createOutputCentered<PJ301MPort>(mm2px(Vec(54.06, 96.251)), module, TAudioInterface::AUDIO_OUTPUTS + 4));
			addOutput(createOutputCentered<PJ301MPort>(mm2px(Vec(65.661, 96.251)), module, TAudioInterface::AUDIO_OUTPUTS + 5));
			addOutput(createOutputCentered<PJ301MPort>(mm2px(Vec(77.26, 96.251)), module, TAudioInterface::AUDIO_OUTPUTS + 6));
			addOutput(createOutputCentered<PJ301MPort>(mm2px(Vec(88.86, 96.251)), module, TAudioInterface::AUDIO_OUTPUTS + 7));
			addOutput(createOutputCentered<PJ301MPort>(mm2px(Vec(7.661, 112.252)), module, TAudioInterface::AUDIO_OUTPUTS + 8));
			addOutput(createOutputCentered<PJ301MPort>(mm2px(Vec(19.26, 112.252)), module, TAudioInterface::AUDIO_OUTPUTS + 9));
			addOutput(createOutputCentered<PJ301MPort>(mm2px(Vec(30.86, 112.252)), module, TAudioInterface::AUDIO_OUTPUTS + 10));
			addOutput(createOutputCentered<PJ301MPort>(mm2px(Vec(42.461, 112.252)), module, TAudioInterface::AUDIO_OUTPUTS + 11));
			addOutput(createOutputCentered<PJ301MPort>(mm2px(Vec(54.06, 112.252)), module, TAudioInterface::AUDIO_OUTPUTS + 12));
			addOutput(createOutputCentered<PJ301MPort>(mm2px(Vec(65.661, 112.252)), module, TAudioInterface::AUDIO_OUTPUTS + 13));
			addOutput(createOutputCentered<PJ301MPort>(mm2px(Vec(77.26, 112.252)), module, TAudioInterface::AUDIO_OUTPUTS + 14));
			addOutput(createOutputCentered<PJ301MPort>(mm2px(Vec(88.86, 112.252)), module, TAudioInterface::AUDIO_OUTPUTS + 15));

			addChild(createLightCentered<SmallLight<GreenLight>>(mm2px(Vec(13.46, 55.667)), module, TAudioInterface::INPUT_LIGHTS + 0));
			addChild(createLightCentered<SmallLight<GreenLight>>(mm2px(Vec(36.661, 55.667)), module, TAudioInterface::INPUT_LIGHTS + 1));
			addChild(createLightCentered<SmallLight<GreenLight>>(mm2px(Vec(59.861, 55.667)), module, TAudioInterface::INPUT_LIGHTS + 2));
			addChild(createLightCentered<SmallLight<GreenLight>>(mm2px(Vec(83.061, 55.667)), module, TAudioInterface::INPUT_LIGHTS + 3));
			addChild(createLightCentered<SmallLight<GreenLight>>(mm2px(Vec(13.46, 70.248)), module, TAudioInterface::INPUT_LIGHTS + 4));
			addChild(createLightCentered<SmallLight<GreenLight>>(mm2px(Vec(36.661, 70.248)), module, TAudioInterface::INPUT_LIGHTS + 5));
			addChild(createLightCentered<SmallLight<GreenLight>>(mm2px(Vec(59.861, 70.248)), module, TAudioInterface::INPUT_LIGHTS + 6));
			addChild(createLightCentered<SmallLight<GreenLight>>(mm2px(Vec(83.061, 70.248)), module, TAudioInterface::INPUT_LIGHTS + 7));
			addChild(createLightCentered<SmallLight<GreenLight>>(mm2px(Vec(13.46, 92.238)), module, TAudioInterface::OUTPUT_LIGHTS + 0));
			addChild(createLightCentered<SmallLight<GreenLight>>(mm2px(Vec(36.661, 92.238)), module, TAudioInterface::OUTPUT_LIGHTS + 1));
			addChild(createLightCentered<SmallLight<GreenLight>>(mm2px(Vec(59.861, 92.238)), module, TAudioInterface::OUTPUT_LIGHTS + 2));
			addChild(createLightCentered<SmallLight<GreenLight>>(mm2px(Vec(83.061, 92.238)), module, TAudioInterface::OUTPUT_LIGHTS + 3));
			addChild(createLightCentered<SmallLight<GreenLight>>(mm2px(Vec(13.46, 108.259)), module, TAudioInterface::OUTPUT_LIGHTS + 4));
			addChild(createLightCentered<SmallLight<GreenLight>>(mm2px(Vec(36.661, 108.259)), module, TAudioInterface::OUTPUT_LIGHTS + 5));
			addChild(createLightCentered<SmallLight<GreenLight>>(mm2px(Vec(59.861, 108.259)), module, TAudioInterface::OUTPUT_LIGHTS + 6));
			addChild(createLightCentered<SmallLight<GreenLight>>(mm2px(Vec(83.061, 108.259)), module, TAudioInterface::OUTPUT_LIGHTS + 7));

			AudioWidget* audioWidget = createWidget<AudioWidget>(mm2px(Vec(2.57, 14.839)));
			audioWidget->box.size = mm2px(Vec(91.382, 28.0));
			audioWidget->setAudioPort(module);
			addChild(audioWidget);
		}
	}

	void appendContextMenu(Menu* menu) override {
		TAudioInterface* module = dynamic_cast<TAudioInterface*>(this->module);

		menu->addChild(new MenuEntry);

		PrimaryModuleItem<TAudioInterface>* primaryModuleItem = new PrimaryModuleItem<TAudioInterface>;
		primaryModuleItem->text = "Primary audio module";
		primaryModuleItem->rightText = CHECKMARK(APP->engine->getPrimaryModule() == module);
		primaryModuleItem->module = module;
		menu->addChild(primaryModuleItem);
	}
};


Model* modelAudioInterface = createModel<AudioInterface<8, 8>, AudioInterfaceWidget<8, 8>>("AudioInterface");
Model* modelAudioInterface16 = createModel<AudioInterface<16, 16>, AudioInterfaceWidget<16, 16>>("AudioInterface16");


} // namespace core
} // namespace rack
