#include "modular80.hpp"

#include <stdio.h>
#include <cfloat>
#include <thread>
#include <algorithm>
#include <sys/stat.h>

#include "dsp/digital.hpp"
#include "dsp/vumeter.hpp"
#include "dsp/samplerate.hpp"
#include "dsp/ringbuffer.hpp"

#include "osdialog.h"

namespace rack_plugin_modular80 {

#define DR_WAV_IMPLEMENTATION
#include "dep/dr_libs/dr_wav.h"


#define MAX_BANK_SIZE 2147483648l // 2GB max per bank, 32GB total (16 banks)
#define MAX_NUM_BANKS 16
#define MAX_DIR_DEPTH 1


class FileScanner {

public:

FileScanner() :
  scanDepth(0),
  bankCount(0),
  bankSize(0)
  {}
~FileScanner() {};

void reset() {
	bankCount = 0;
	bankSize = 0;
	scanDepth = 0;
	banks.clear();
}

static bool isSupportedAudioFormat(std::string& path) {
	const std::string tmpF = stringLowercase(path);
	return (stringEndsWith(tmpF, ".wav") || stringEndsWith(tmpF, ".raw"));
}

void scan(std::string& root, const bool sort = false, const bool filter = true) {

	std::vector<std::string> files;
	std::vector<std::string> entries;

	entries = systemListEntries(root);

	if (sort) {
		std::sort(entries.begin(), entries.end());
	}

	for (std::string &entry : entries) {
		if (systemIsDirectory(entry)) {
			if (stringStartsWith(entry, "SPOTL") ||
			    stringStartsWith(entry, "TRASH") ||
				stringStartsWith(entry, "__MACOSX")) {
				continue;
			}

			if (bankCount > MAX_NUM_BANKS) {
				warn("Max number of banks reached. Ignoring subdirectories.");
				return;
			}

			if (scanDepth++ > MAX_DIR_DEPTH) {
				warn("Directory has too many subdirectories: %s", entry.c_str());
				continue;
			};

			scan(entry, sort, filter);

		} else {
			struct stat statbuf;
			if (stat(entry.c_str(), &statbuf)) {
				warn("Failed to get file stats: %s", entry.c_str());
				continue;
			}
			bankSize += (intmax_t)statbuf.st_size;
			if (bankSize > MAX_BANK_SIZE) {
				warn("Bank size limit reached. Ignoring file: %s", entry.c_str());
				continue;
			} else {
				files.push_back(entry);
			}
		}
	}

	if (filter) {
		for (std::vector<std::string>::iterator it = files.begin(); it != files.end(); /* */) {
			if (!isSupportedAudioFormat(*it)) it = files.erase(it);
			else ++it;
		}
	}

	if (!files.empty()) {
		bankSize = 0;
		bankCount++;
		banks.push_back(files);
	}
	scanDepth--;
}

int scanDepth;
int bankCount;
intmax_t bankSize;
std::vector< std::vector<std::string> > banks;

};


// Base class
class AudioObject {

public:

AudioObject() :
  filePath(),
  currentPos(0),
  channels(0),
  sampleRate(0),
  bytesPerSample(2),
  totalSamples(0),
  samples(nullptr),
  peak(0.0f) {};

virtual ~AudioObject() {};

virtual bool load(const std::string &path) = 0;

std::string filePath;
unsigned long currentPos;
unsigned int channels;
unsigned int sampleRate;
unsigned int bytesPerSample;
drwav_uint64 totalSamples;
float *samples;
float peak;

};


class WavAudioObject : public AudioObject {

public:

WavAudioObject() : AudioObject() {
	bytesPerSample = 4;
};
~WavAudioObject() {
	if (samples) {
		drwav_free(samples);
	}
};

bool load(const std::string &path) override {
	filePath = path;
	samples = drwav_open_file_and_read_f32(
		filePath.c_str(), &channels, &sampleRate, &totalSamples
	);

	if (samples) {
		for (size_t i = 0; i < totalSamples; ++i) {
			if (samples[i] > peak) peak = samples[i];
		}
	}

	return (samples != NULL);
}

};


class RawAudioObject : public AudioObject {

public:

RawAudioObject() : AudioObject() {
	channels = 1;
	sampleRate = 44100;
	bytesPerSample = 2;
};
~RawAudioObject() {
	if (samples) {
		free(samples);
	}
}

bool load(const std::string &path) override {
	filePath = path;

	FILE *wav = fopen(filePath.c_str(), "rb");

	if (wav) {
		fseek(wav, 0, SEEK_END);
		const long fsize = ftell(wav);
		rewind(wav);

		int16_t *rawSamples = (int16_t*)malloc(sizeof(int16_t) * fsize/bytesPerSample);
		if (rawSamples) {
			const long samplesRead = fread(rawSamples, (size_t)sizeof(int16_t), fsize/bytesPerSample, wav);
			fclose(wav);
			if (samplesRead != fsize/(int)bytesPerSample) { warn("Failed to read entire file"); }
			totalSamples = samplesRead;

			samples = (float*)malloc(sizeof(float) * totalSamples);
			for (size_t i = 0; i < totalSamples; ++i) {
				samples[i] = static_cast<float>(rawSamples[i]);
				if (samples[i] > peak) peak = samples[i];
			}
		} else {
			fatal("Failed to allocate memory");
		}

		free(rawSamples);

	} else {
		fatal("Failed to load file: %s", filePath.c_str());
	}

    return (samples != NULL);
}

};


class AudioPlayer {

public:
AudioPlayer() :
  startPos(0)
{};
~AudioPlayer() {};

void load(std::shared_ptr<AudioObject> object) {
	audio = std::move(object);
}

void skipTo(unsigned long pos) {
	if (audio) {
		audio->currentPos = pos;
	}
}

float play(unsigned int channel) {
	float sample(0.0f);

	if (audio) {
		if (channel < audio->channels) {
			if  (audio->currentPos < (audio->totalSamples/audio->channels) ||
			   ((audio->currentPos + channel) < audio->totalSamples)) {
				sample = audio->samples[channel + audio->currentPos];
			}
		}
	}

	return sample;
}

void advance(bool repeat = false) {
	if (audio) {
		const unsigned long nextPos = audio->currentPos + audio->channels;
		const unsigned long maxPos = audio->totalSamples/audio->channels;
		if (nextPos >= maxPos) {
			if (repeat) {
				audio->currentPos = startPos;
			} else {
				audio->currentPos = maxPos;
			}
		} else {
			audio->currentPos = nextPos;
		}
	}
}

void resetTo(unsigned long pos) {
	if (audio) {
		startPos = pos;
		audio->currentPos = startPos;
	}
}

bool ready() {
	if (audio) {
		return audio->totalSamples > 0;
	} else {
		return false;
	}
}

void reset() {
	if (audio) {
		audio.reset();
	}
}

std::shared_ptr<AudioObject> object() {
	return audio;
}

private:

std::shared_ptr<AudioObject> audio;
long startPos;

};


struct RadioMusic : Module {
	enum ParamIds {
		CHANNEL_PARAM,
		START_PARAM,
		RESET_PARAM,
		NUM_PARAMS
	};
	enum InputIds {
		STATION_INPUT,
		START_INPUT,
		RESET_INPUT,
		NUM_INPUTS
	};
	enum OutputIds {
		OUT_OUTPUT,
		NUM_OUTPUTS
	};
	enum LightIds {
		RESET_LIGHT,
		LED_0_LIGHT,
		LED_1_LIGHT,
		LED_2_LIGHT,
		LED_3_LIGHT,
		NUM_LIGHTS
	};

	RadioMusic() : Module(NUM_PARAMS, NUM_INPUTS, NUM_OUTPUTS, NUM_LIGHTS)
	{
		currentPlayer = &audioPlayer1;
		previousPlayer = &audioPlayer2;

		init();
	}

	void step() override;
	void reset() override;
	void init();

	void threadedScan();
	void scanAudioFiles();
	void threadedLoad();
	void loadAudioFiles();
	void resetCurrentPlayer(float start);

	std::string rootDir;
	bool loadFiles;
	bool scanFiles;

	bool selectBank;

	// Settings
	bool loopingEnabled;
	bool enableCrossfade;
	bool sortFiles;
	bool allowAllFiles;

	json_t *toJson() override {
		json_t *rootJ = json_object();

		// Option: Loop Samples
		json_t *loopingJ = json_boolean(loopingEnabled);
		json_object_set_new(rootJ, "loopingEnabled", loopingJ);

		// Option: Enable Crossfade
		json_t *crossfadeJ = json_boolean(enableCrossfade);
		json_object_set_new(rootJ, "enableCrossfade", crossfadeJ);

		// Option: Sort Files
		json_t *sortJ = json_boolean(sortFiles);
		json_object_set_new(rootJ, "sortFiles", sortJ);

		// Option: Allow All Files
		json_t *filesJ = json_boolean(allowAllFiles);
		json_object_set_new(rootJ, "allowAllFiles", filesJ);

		// Internal state: rootDir
		json_t *rootDirJ = json_string(rootDir.c_str());
		json_object_set_new(rootJ, "rootDir", rootDirJ);

		// Internal state: currentBank
		json_t *bankJ = json_integer(currentBank);
		json_object_set_new(rootJ, "currentBank", bankJ);

		return rootJ;
	}

	void fromJson(json_t *rootJ) override {
		// Option: Loop Samples
		json_t *loopingJ = json_object_get(rootJ, "loopingEnabled");
		if (loopingJ) loopingEnabled = json_boolean_value(loopingJ);

		// Option: Enable Crossfade
		json_t *crossfadeJ = json_object_get(rootJ, "enableCrossfade");
		if (crossfadeJ) enableCrossfade = json_boolean_value(crossfadeJ);

		// Option: Sort Files
		json_t *sortJ = json_object_get(rootJ, "sortFiles");
		if (sortJ) sortFiles = json_boolean_value(sortJ);

		// Option: Allow All Files
		json_t *filesJ = json_object_get(rootJ, "allowAllFiles");
		if (filesJ) allowAllFiles = json_boolean_value(filesJ);

		// Internal state: rootDir
		json_t *rootDirJ = json_object_get(rootJ, "rootDir");
		if (rootDirJ) rootDir = json_string_value(rootDirJ);

		// Internal state: currentBank
		json_t *bankJ = json_object_get(rootJ, "currentBank");
		if (bankJ) currentBank = json_integer_value(bankJ);

		scanFiles = true;
	}

private:

	AudioPlayer audioPlayer1;
	AudioPlayer audioPlayer2;

	AudioPlayer *currentPlayer;
	AudioPlayer *previousPlayer;

	std::vector<std::shared_ptr<AudioObject>> objects;

	SchmittTrigger rstButtonTrigger;
	SchmittTrigger rstInputTrigger;

	int prevIndex;
	unsigned long tick;
	unsigned long elapsedMs;

	bool crossfade;
	bool fadeout;
	float fadeOutGain;
	float xfadeGain1;
	float xfadeGain2;

	bool flashResetLed;
	unsigned long ledTimerMs;

	VUMeter vumeter;

	SampleRateConverter<1> outputSrc;
	DoubleRingBuffer<Frame<1>, 256> outputBuffer;

	bool ready;
	int currentBank;

	FileScanner scanner;

	static const int BLOCK_SIZE = 16;
   float block[BLOCK_SIZE];
};

void RadioMusic::reset() {
	init();
}

void RadioMusic::init() {
	prevIndex = -1;
	tick = 0;
	elapsedMs = 0;
	crossfade = false;
	fadeout = false;
	flashResetLed = false;
	ledTimerMs = 0;
	rootDir = "";
	loadFiles = false;
	scanFiles = false;
	selectBank = 0;
	ready = false;
	fadeOutGain = 1.0f;
	xfadeGain1 = 0.0f;
	xfadeGain2 = 1.0f;

	// Settings
	loopingEnabled = true;
	enableCrossfade = true;
	sortFiles = false;
	allowAllFiles = false;

	// Persistent
	rootDir = "";
	currentBank = 0;

	// Internal state
	scanner.banks.clear();

	if (currentPlayer->object()) {
		currentPlayer->reset();
	}
	if (previousPlayer->object()) {
		previousPlayer->reset();
	}

	for (size_t i = 0; i < NUM_LIGHTS; i++) {
		lights[RESET_LIGHT + i].value = 0.0f;
	}
}

void RadioMusic::threadedScan() {
	if (rootDir.empty()) {
		warn("No root directory defined. Scan failed.");
		return;
	}

	scanner.reset();
	scanner.scan(rootDir, sortFiles, !allowAllFiles);
	if (scanner.banks.size() == 0) {
		return;
	}

	loadFiles = true;
}

void RadioMusic::scanAudioFiles() {
	std::thread worker(&RadioMusic::threadedScan, this);
	worker.detach();
}

void RadioMusic::threadedLoad() {
	if (scanner.banks.empty()) {
		warn("No banks available. Failed to load audio files.");
		return;
	}

	objects.clear();

	currentBank = clamp(currentBank, 0, (int)scanner.banks.size()-1);

	const std::vector<std::string> files = scanner.banks[currentBank];
	for (unsigned int i = 0; i < files.size(); ++i) {
		std::shared_ptr<AudioObject> object;

		// Quickly determine file type
		drwav wav;
		if (drwav_init_file(&wav, files[i].c_str())) {
			object = std::make_shared<WavAudioObject>();
		} else {
			object = std::make_shared<RawAudioObject>();
		}
		drwav_uninit(&wav);

		// Actually load files
		if (object->load(files[i])) {
			objects.push_back(std::move(object));
		} else {
			warn("Failed to load object %d %s", i, files[i].c_str());
		}
	}

	prevIndex = -1; // Force channel change detection upon loading files
	elapsedMs = 0; // Reset station to beginning

	ready = true;
}

void RadioMusic::loadAudioFiles() {
	std::thread worker(&RadioMusic::threadedLoad, this);
	worker.detach();
}

void RadioMusic::resetCurrentPlayer(float start) {
	const unsigned int channels = currentPlayer->object()->channels;
	unsigned long pos = static_cast<int>(start * (currentPlayer->object()->totalSamples / channels));
	if (pos >= channels) { pos -= channels; }
	pos = pos % (currentPlayer->object()->totalSamples / channels);
	currentPlayer->resetTo(pos);
}

void RadioMusic::step() {

	if (rootDir.empty()) {
		// No files loaded yet. Idle.
		return;
	}

	if (scanFiles) {
		scanAudioFiles();

		scanFiles = false;
	}

	if (loadFiles) {
		// Disable channel switching and resetting while loading files.
		ready = false;

		loadAudioFiles();

		loadFiles = false;
	}

	// Bank selection mode
	if (selectBank) {
		// Bank is selected via Reset button
		if (rstButtonTrigger.process(params[RESET_PARAM].value)) {
			currentBank++;
			currentBank %= scanner.banks.size();
		}

		// Show bank selection in LED bar
		lights[LED_0_LIGHT].value = (1 && (currentBank & 1));
		lights[LED_1_LIGHT].value = (1 && (currentBank & 2));
		lights[LED_2_LIGHT].value = (1 && (currentBank & 4));
		lights[LED_3_LIGHT].value = (1 && (currentBank & 8));
		lights[RESET_LIGHT].value = 1.0f;
	}

	// Keep track of milliseconds of elapsed time
	if (tick++ % (static_cast<int>(engineGetSampleRate())/1000) == 0) {
		elapsedMs++;
		ledTimerMs++;
	}

	// Start knob & input
	const float start = clamp(params[START_PARAM].value + inputs[START_INPUT].value/5.0f, 0.0f, 1.0f);

	if (ready && (rstButtonTrigger.process(params[RESET_PARAM].value) ||
		(inputs[RESET_INPUT].active && rstInputTrigger.process(inputs[RESET_INPUT].value)))) {

		fadeOutGain = 1.0f;

		if (enableCrossfade) {
			fadeout = true;
		} else {
			resetCurrentPlayer(start);
		}

		flashResetLed = true;
	}

	// Channel knob & input
	const float channel = clamp(params[CHANNEL_PARAM].value + inputs[STATION_INPUT].value/5.0f, 0.0f, 1.0f);
	const int index = \
		clamp(static_cast<int>(rescale(channel, 0.0f, 1.0f, 0.0f, static_cast<float>(objects.size()))),
			0, objects.size() - 1);


	// Channel switch detection
	if (ready && index != prevIndex) {
		AudioPlayer *tmp;
		tmp = previousPlayer;
		previousPlayer = currentPlayer;
		currentPlayer = tmp;

		if (index < (int)objects.size()) {
			currentPlayer->load(objects[index]);

			unsigned long pos = objects[index]->currentPos + \
				(currentPlayer->object()->channels * elapsedMs * objects[index]->sampleRate) / 1000;
			pos = pos % (objects[index]->totalSamples / objects[index]->channels);

			currentPlayer->skipTo(pos);

			elapsedMs = 0;
		}

		xfadeGain1 = 0.0f;
		xfadeGain2 = 1.0f;

		crossfade = enableCrossfade;

		// Different number of channels while crossfading leads to audible artifacts.
		if (previousPlayer->object()) {
			if (currentPlayer->object()->channels != previousPlayer->object()->channels) {
				crossfade = false;
			}
		}

		flashResetLed = true;
	}

	prevIndex = index;

	// Reset LED
	if (flashResetLed) {
		static int initTimer(true);
		static int timerStart(0);

		if (initTimer) {
			timerStart = ledTimerMs;
			initTimer = false;
		}

		lights[RESET_LIGHT].value = 1.0f;

		if ((ledTimerMs - timerStart) > 50) { // 50ms flash time
			initTimer = true;
			ledTimerMs = 0;
			flashResetLed = false;
		}
	}

	if (!flashResetLed && !selectBank) {
		lights[RESET_LIGHT].value = 0.0f;
	}

	// Audio processing
	if (outputBuffer.empty()) {
		// Nothing to play if no audio objects are loaded into players.
		if (!currentPlayer->object() && !previousPlayer->object()) {
			return;
		}

		for (int i = 0; i < BLOCK_SIZE; i++) {
			float output(0.0f);

			// Crossfade?
			if (crossfade) {

				xfadeGain1 = rack::crossfade(xfadeGain1, 1.0f, 0.005); // 0.005 = ~25ms
				xfadeGain2 = rack::crossfade(xfadeGain2, 0.0f, 0.005); // 0.005 = ~25ms

				for (size_t channel = 0; channel < currentPlayer->object()->channels; channel++) {
					const float currSample = currentPlayer->play(channel);
					const float prevSample = previousPlayer->play(channel);
					const float out = currSample * xfadeGain1 + prevSample * xfadeGain2;

					output += 5.0f * out / currentPlayer->object()->peak;
				}
				output /= currentPlayer->object()->channels;

				currentPlayer->advance(loopingEnabled);
				previousPlayer->advance(loopingEnabled);

				if (isNear(xfadeGain1+0.005, 1.0f) || isNear(xfadeGain2, 0.0f)) {
					crossfade = false;
				}

			}
			// Fade out (before resetting)?
			else if (fadeout)
			{

				fadeOutGain = rack::crossfade(fadeOutGain, 0.0f, 0.05); // 0.05 = ~5ms

				for (size_t channel = 0; channel < currentPlayer->object()->channels; channel++) {
					const float sample = currentPlayer->play(channel);
					const float out = sample * fadeOutGain;

					output += 5.0f * out / currentPlayer->object()->peak;
				}
				output /= currentPlayer->object()->channels;

				currentPlayer->advance(loopingEnabled);

				if (isNear(fadeOutGain, 0.0f)) {

					resetCurrentPlayer(start);

					fadeout = false;
				}
			}
			else // No fading
			{
				for (size_t channel = 0; channel < currentPlayer->object()->channels; channel++) {
					const float out = currentPlayer->play(channel);

					output += 5.0f * out / currentPlayer->object()->peak;
				}
				output /= currentPlayer->object()->channels;

				currentPlayer->advance(loopingEnabled);
			}

			block[i] = output;
		}

		// Sample rate conversion to match Rack engine sample rate.
		outputSrc.setRates(currentPlayer->object()->sampleRate, engineGetSampleRate());
		int inLen = BLOCK_SIZE;
		int outLen = outputBuffer.capacity();

		Frame<1> frame[BLOCK_SIZE];

		for (int i = 0; i < BLOCK_SIZE; i++) {
			frame[i].samples[0] = block[i];
		}

		outputSrc.process(frame, &inLen, outputBuffer.endData(), &outLen);
		outputBuffer.endIncr(outLen);
	}

	// Output processing & metering
	if (!outputBuffer.empty()) {
		Frame<1> frame = outputBuffer.shift();
		outputs[OUT_OUTPUT].value = frame.samples[0];

		// Disable VU Meter in Bank Selection mode.
		if (!selectBank) {
			for (int i = 0; i < 4; i++){
				vumeter.setValue(frame.samples[0]/5.0f);
				lights[LED_3_LIGHT - i].setBrightnessSmooth(vumeter.getBrightness(i));
			}
		}
	}
}

struct RadioMusicWidget : ModuleWidget {
	RadioMusicWidget(RadioMusic *module);
	Menu *createContextMenu() override;
};

RadioMusicWidget::RadioMusicWidget(RadioMusic *module) : ModuleWidget(module) {
	setPanel(SVG::load(assetPlugin(plugin, "res/Radio.svg")));

	addChild(Widget::create<ScrewSilver>(Vec(14, 0)));

	addChild(ModuleLightWidget::create<MediumLight<RedLight>>(Vec(6, 33), module, RadioMusic::LED_0_LIGHT));
	addChild(ModuleLightWidget::create<MediumLight<RedLight>>(Vec(19, 33), module, RadioMusic::LED_1_LIGHT));
	addChild(ModuleLightWidget::create<MediumLight<RedLight>>(Vec(32, 33), module, RadioMusic::LED_2_LIGHT));
	addChild(ModuleLightWidget::create<MediumLight<RedLight>>(Vec(45, 33), module, RadioMusic::LED_3_LIGHT));

	addParam(ParamWidget::create<Davies1900hBlackKnob>(Vec(12, 49), module, RadioMusic::CHANNEL_PARAM, 0.0f, 1.0f, 0.0f));
	addParam(ParamWidget::create<Davies1900hBlackKnob>(Vec(12, 131), module, RadioMusic::START_PARAM, 0.0f, 1.0f, 0.0f));

	addChild(ModuleLightWidget::create<MediumLight<RedLight>>(Vec(44, 188), module, RadioMusic::RESET_LIGHT));

	addParam(ParamWidget::create<PB61303>(Vec(25, 202), module, RadioMusic::RESET_PARAM, 0, 1, 0));

	addInput(Port::create<PJ301MPort>(Vec(3, 274), Port::INPUT, module, RadioMusic::STATION_INPUT));
	addInput(Port::create<PJ301MPort>(Vec(32, 274), Port::INPUT, module, RadioMusic::START_INPUT));

	addInput(Port::create<PJ301MPort>(Vec(3, 318), Port::INPUT, module, RadioMusic::RESET_INPUT));
	addOutput(Port::create<PJ301MPort>(Vec(32, 318), Port::OUTPUT, module, RadioMusic::OUT_OUTPUT));

	addChild(Widget::create<ScrewSilver>(Vec(14, 365)));
}

struct RadioMusicDirDialogItem : MenuItem {
	RadioMusic *rm;
	void onAction(EventAction &e) override {

		const std::string dir = \
			rm->rootDir.empty() ? assetLocal("") : rm->rootDir;
		char *path = osdialog_file(OSDIALOG_OPEN_DIR, dir.c_str(), NULL, NULL);
		if (path) {
			rm->rootDir = std::string(path);
			rm->scanFiles = true;
			free(path);
		}
	}
};

struct RadioMusicSelectBankItem : MenuItem {
	RadioMusic *rm;
	void onAction(EventAction &e) override {
		rm->selectBank = !rm->selectBank;

		if (rm->selectBank == false) {
			rm->loadFiles = true;
		}
	}
	void step() override {
		text = (rm->selectBank != true) ? "Enter Bank Select Mode" : "Exit Bank Select Mode";
		rightText = (rm->selectBank == true) ? "✔" : "";
	}
};

struct RadioMusicLoopingEnabledItem : MenuItem {
	RadioMusic *rm;
	void onAction(EventAction &e) override {
		rm->loopingEnabled = !rm->loopingEnabled;
	}
	void step() override {
		rightText = (rm->loopingEnabled == true) ? "✔" : "";
	}
};

struct RadioMusicCrossfadeItem : MenuItem {
	RadioMusic *rm;
	void onAction(EventAction &e) override {
		rm->enableCrossfade = !rm->enableCrossfade;
	}
	void step() override {
		rightText = (rm->enableCrossfade == true) ? "✔" : "";
	}
};

struct RadioMusicFileSortItem : MenuItem {
	RadioMusic *rm;
	void onAction(EventAction &e) override {
		rm->sortFiles = !rm->sortFiles;
	}
	void step() override {
		rightText = (rm->sortFiles == true) ? "✔" : "";
	}
};

struct RadioMusicFilesAllowedItem : MenuItem {
	RadioMusic *rm;
	void onAction(EventAction &e) override {
		rm->allowAllFiles = !rm->allowAllFiles;
	}
	void step() override {
		rightText = (rm->allowAllFiles == true) ? "✔" : "";
	}
};

Menu *RadioMusicWidget::createContextMenu() {
	Menu *menu = ModuleWidget::createContextMenu();

	MenuLabel *spacerLabel = new MenuLabel();
	menu->addChild(spacerLabel);

	RadioMusic *rm = dynamic_cast<RadioMusic*>(module);
	assert(rm);

	RadioMusicDirDialogItem *rootDirItem = new RadioMusicDirDialogItem();
	rootDirItem->text = "Set Root Directory";
	rootDirItem->rm = rm;
	menu->addChild(rootDirItem);

	RadioMusicSelectBankItem *selectBankItem = new RadioMusicSelectBankItem();
	selectBankItem->text = "";
	selectBankItem->rm = rm;
	menu->addChild(selectBankItem);

	MenuLabel *spacerLabel2 = new MenuLabel();
	menu->addChild(spacerLabel2);

	RadioMusicLoopingEnabledItem *loopingEnabledItem = new RadioMusicLoopingEnabledItem();
	loopingEnabledItem->text = "Enable Looping";
	loopingEnabledItem->rm = rm;
	menu->addChild(loopingEnabledItem);

	RadioMusicCrossfadeItem *crossfadeItem = new RadioMusicCrossfadeItem();
	crossfadeItem->text = "Enable Crossfade";
	crossfadeItem->rm = rm;
	menu->addChild(crossfadeItem);

	RadioMusicFileSortItem *fileSortItem = new RadioMusicFileSortItem();
	fileSortItem->text = "Sort Files";
	fileSortItem->rm = rm;
	menu->addChild(fileSortItem);

	RadioMusicFilesAllowedItem *filesAllowedItem = new RadioMusicFilesAllowedItem();
	filesAllowedItem->text = "Allow All Files";
	filesAllowedItem->rm = rm;
	menu->addChild(filesAllowedItem);

	return menu;
}

} // namespace rack_plugin_modular80

using namespace rack_plugin_modular80;

RACK_PLUGIN_MODEL_INIT(modular80, RadioMusic) {
   Model *modelRadioMusic = Model::create<RadioMusic, RadioMusicWidget>("modular80", "Radio Music", "Radio Music", SAMPLER_TAG);
   return modelRadioMusic;
}
