#include "Bidoo.hpp"
#include "dsp/digital.hpp"
#include "BidooComponents.hpp"
#include "osdialog.h"
#include "dep/audiofile/AudioFile.h"
#include <vector>
#include "cmath"
#include <iomanip> // setprecision
#include <sstream> // stringstream
#include <algorithm>
#include "window.hpp"
#include "Gist.h"

using namespace std;

namespace rack_plugin_Bidoo {

struct CANARD : Module {
	enum ParamIds {
		RECORD_PARAM,
		SAMPLE_START_PARAM,
		LOOP_LENGTH_PARAM,
		READ_MODE_PARAM,
		SPEED_PARAM,
		FADE_PARAM,
		MODE_PARAM,
		SLICE_PARAM,
		CLEAR_PARAM,
		THRESHOLD_PARAM,
		NUM_PARAMS
	};
	enum InputIds {
		INL_INPUT,
		INR_INPUT,
		TRIG_INPUT,
		GATE_INPUT,
		SAMPLE_START_INPUT,
		LOOP_LENGTH_INPUT,
		READ_MODE_INPUT,
		SPEED_INPUT,
		RECORD_INPUT,
		FADE_INPUT,
		SLICE_INPUT,
		CLEAR_INPUT,
		NUM_INPUTS
	};
	enum OutputIds {
		OUTL_OUTPUT,
		OUTR_OUTPUT,
		EOC_OUTPUT,
		NUM_OUTPUTS
	};
	enum LightIds {
		REC_LIGHT,
		NUM_LIGHTS
	};

	bool play = false;
	bool record = false;
	AudioFile<float> playBuffer, recordBuffer;
	float samplePos = 0.0f, sampleStart = 0.0f, loopLength = 0.0f, fadeLenght = 0.0f, fadeCoeff = 1.0f, speedFactor = 1.0f;
	size_t prevPlayedSlice = 0;
	size_t playedSlice = 0;
	bool changedSlice = false;
	int readMode = 0; // 0 formward, 1 backward, 2 repeat
	float speed;
	std::vector<int> slices;
	int selected = -1;
	bool deleteFlag = false;
	int addSliceMarker = -1;
	bool addSliceMarkerFlag = false;
	int deleteSliceMarker = -1;
	bool deleteSliceMarkerFlag = false;
	size_t index = 0;
	float prevGateState = 0.0f;
	float prevTrigState = 0.0f;
	string lastPath;
	string waveFileName;
	string waveExtension;
	bool loading = false;
	SchmittTrigger trigTrigger;
	SchmittTrigger recordTrigger;
	SchmittTrigger clearTrigger;
	PulseGenerator eocPulse;
	std::mutex mylock;
	bool newStop = false;

	CANARD() : Module(NUM_PARAMS, NUM_INPUTS, NUM_OUTPUTS, NUM_LIGHTS) {
		recordBuffer.setBitDepth(16);
		recordBuffer.setSampleRate(engineGetSampleRate());
		recordBuffer.setNumChannels(2);
		recordBuffer.samples[0].resize(0);
		recordBuffer.samples[1].resize(0);
		playBuffer.setBitDepth(16);
		playBuffer.setSampleRate(engineGetSampleRate());
		playBuffer.setNumChannels(2);
		playBuffer.samples[0].resize(0);
		playBuffer.samples[1].resize(0);
	}

	void step() override;
	void calcLoop();
	void initPos();
	void loadSample(std::string path);
	// persistence

	json_t *toJson() override {
		json_t *rootJ = json_object();
		// lastPath
		json_object_set_new(rootJ, "lastPath", json_string(lastPath.c_str()));

		json_t *slicesJ = json_array();
		for (size_t i = 0; i<slices.size() ; i++) {
			json_t *sliceJ = json_integer(slices[i]);
			json_array_append_new(slicesJ, sliceJ);
		}
		json_object_set_new(rootJ, "slices", slicesJ);

		return rootJ;
	}

	void fromJson(json_t *rootJ) override {
		json_t *lastPathJ = json_object_get(rootJ, "lastPath");
		if (lastPathJ) {
			lastPath = json_string_value(lastPathJ);
			waveFileName = stringFilename(lastPath);
			waveExtension = stringExtension(lastPath);
			loadSample(lastPath);
			if (playBuffer.getNumSamplesPerChannel()>0) {
				json_t *slicesJ = json_object_get(rootJ, "slices");
				if (slicesJ) {
					size_t i;
					json_t *sliceJ;
					json_array_foreach(slicesJ, i, sliceJ) {
							if (i != 0)
								slices.push_back(json_integer_value(sliceJ));
					}
				}
			}
		}
	}
};

void CANARD::loadSample(std::string path) {
	loading = true;
	playBuffer.setNumChannels(1);
	if (playBuffer.load(path.c_str())) {
		lastPath = path;
		waveFileName = stringFilename(path);
		waveExtension = stringExtension(path);
		slices.clear();
		slices.push_back(0);
		if (playBuffer.getNumChannels() == 1) {
			playBuffer.setNumChannels(2);
			playBuffer.samples[1] = playBuffer.samples[0];
		}
	}
	loading = false;
}

void CANARD::calcLoop() {
	prevPlayedSlice = index;
	index = 0;
	int sliceStart = 0;;
	int sliceEnd = playBuffer.getNumSamplesPerChannel() > 0 ? playBuffer.getNumSamplesPerChannel() - 1 : 0;
	if ((params[MODE_PARAM].value == 1) && (slices.size()>0))
	{
		index = round(clamp(params[SLICE_PARAM].value + inputs[SLICE_INPUT].value, 0.0f,10.0f)*(slices.size()-1)/10);
		sliceStart = slices[index];
		sliceEnd = (index < (slices.size() - 1)) ? (slices[index+1] - 1) : (playBuffer.getNumSamplesPerChannel() - 1);
	}

	if (playBuffer.getNumSamplesPerChannel() > 0) {
		sampleStart = rescale(clamp(inputs[SAMPLE_START_INPUT].value + params[SAMPLE_START_PARAM].value, 0.0f, 10.0f), 0.0f, 10.0f, sliceStart, sliceEnd);
		loopLength = clamp(rescale(clamp(inputs[LOOP_LENGTH_INPUT].value + params[LOOP_LENGTH_PARAM].value, 0.0f, 10.0f), 0.0f, 10.0f, 0.0f, sliceEnd - sliceStart + 1),1.0f,sliceEnd-sampleStart+1);
		fadeLenght = rescale(clamp(inputs[FADE_INPUT].value + params[FADE_PARAM].value, 0.0f, 10.0f), 0.0f, 10.0f,0.0f, floor(loopLength/2));
	}
	else {
		loopLength = 0;
		sampleStart = 0;
		fadeLenght = 0;
	}
	playedSlice = index;
}

void CANARD::initPos() {
	if ((inputs[SPEED_INPUT].value + params[SPEED_PARAM].value)>=0)
	{
		samplePos = sampleStart;
	}
	else
	{
		samplePos = sampleStart + loopLength;
	}
	speedFactor = 1.0f;
}

void CANARD::step() {
	if (!loading) {
		if (clearTrigger.process(inputs[CLEAR_INPUT].value + params[CLEAR_PARAM].value))
		{
			mylock.lock();
			playBuffer.samples[0].clear();
			playBuffer.samples[1].clear();
			slices.clear();
			mylock.unlock();
			lastPath = "";
			waveFileName = "";
			waveExtension = "";
		}

		if ((selected>=0) && (deleteFlag)) {
			int nbSample=0;
			if ((size_t)selected<(slices.size()-1)) {
				nbSample = slices[selected + 1] - slices[selected] - 1;
				mylock.lock();
				playBuffer.samples[0].erase(playBuffer.samples[0].begin() + slices[selected], playBuffer.samples[0].begin() + slices[selected + 1]-1);
				playBuffer.samples[1].erase(playBuffer.samples[1].begin() + slices[selected], playBuffer.samples[1].begin() + slices[selected + 1]-1);
				mylock.unlock();
			}
			else {
				nbSample = playBuffer.getNumSamplesPerChannel() - slices[selected];
				mylock.lock();
				playBuffer.samples[0].erase(playBuffer.samples[0].begin() + slices[selected], playBuffer.samples[0].end());
				playBuffer.samples[1].erase(playBuffer.samples[1].begin() + slices[selected], playBuffer.samples[1].end());
				mylock.unlock();
			}
			slices.erase(slices.begin()+selected);
			for (size_t i = selected; i < slices.size(); i++)
			{
				slices[i] = slices[i]-nbSample;
			}
			selected = -1;
			deleteFlag = false;
			calcLoop();
		}

		if ((addSliceMarker>=0) && (addSliceMarkerFlag)) {
			if (std::find(slices.begin(), slices.end(), addSliceMarker) != slices.end()) {
				addSliceMarker = -1;
				addSliceMarkerFlag = false;
			}
			else {
				auto it = std::upper_bound(slices.begin(), slices.end(), addSliceMarker);
				mylock.lock();
				slices.insert(it, addSliceMarker);
				mylock.unlock();
				addSliceMarker = -1;
				addSliceMarkerFlag = false;
				calcLoop();
			}
		}

		if ((deleteSliceMarker>=0) && (deleteSliceMarkerFlag)) {
			if (std::find(slices.begin(), slices.end(), deleteSliceMarker) != slices.end()) {
				mylock.lock();
				slices.erase(std::find(slices.begin(), slices.end(), deleteSliceMarker));
				mylock.unlock();
				deleteSliceMarker = -1;
				deleteSliceMarkerFlag = false;
				calcLoop();
			}
		}

		if (recordTrigger.process(inputs[RECORD_INPUT].value + params[RECORD_PARAM].value))
		{
			lights[REC_LIGHT].value = 10.0f;
			if(record) {
				if (floor(params[MODE_PARAM].value) == 0) {
					mylock.lock();
					slices.clear();
					slices.push_back(0);
					playBuffer.setAudioBuffer(recordBuffer.samples);
					mylock.unlock();
					lastPath = "";
					waveFileName = "";
					waveExtension = "";
				}
				else {
					mylock.lock();
					slices.push_back(playBuffer.getNumSamplesPerChannel() > 0 ? (playBuffer.getNumSamplesPerChannel()-1) : 0);
					playBuffer.samples[0].insert(playBuffer.samples[0].end(), recordBuffer.samples[0].begin(), recordBuffer.samples[0].end());
					playBuffer.samples[1].insert(playBuffer.samples[1].end(), recordBuffer.samples[1].begin(), recordBuffer.samples[1].end());
					mylock.unlock();
				}
				mylock.lock();
				recordBuffer.samples[0].resize(0);
				recordBuffer.samples[1].resize(0);
				mylock.unlock();
				lights[REC_LIGHT].value = 0.0f;
			}
			record = !record;
		}

		if (record) {
			mylock.lock();
			recordBuffer.samples[0].push_back(inputs[INL_INPUT].value/10);
			recordBuffer.samples[1].push_back(inputs[INR_INPUT].value/10);
			mylock.unlock();
		}

		int trigMode = inputs[TRIG_INPUT].active ? 1 : (inputs[GATE_INPUT].active ? 2 : 0);
		int readMode = round(clamp(inputs[READ_MODE_INPUT].value + params[READ_MODE_PARAM].value,0.0f,2.0f));
		speed = inputs[SPEED_INPUT].value + params[SPEED_PARAM].value;
		calcLoop();

		if (trigMode == 1) {
			if (trigTrigger.process(inputs[TRIG_INPUT].value) && (prevTrigState == 0.0f))
			{
				initPos();
				play = true;
			}
			else {
				if ((readMode == 0) && (speed>=0) && (samplePos == (sampleStart+loopLength))) {
					play = false;
					if (newStop) {
						eocPulse.trigger(10 / engineGetSampleRate());
						newStop = false;
					}
				}
				else if ((readMode == 0) && (speed<0) && (samplePos == sampleStart)) {
					play = false;
					if (newStop) {
						eocPulse.trigger(10 / engineGetSampleRate());
						newStop = false;
					}
				}
				else if ((readMode == 1) && (speed>=0) && (samplePos == (sampleStart+loopLength))) {
					initPos();
				}
				else if ((readMode == 1) && (speed<0) && (samplePos == sampleStart)) {
					initPos();
				}
				else if ((readMode == 2) && ((samplePos == (sampleStart)) || (samplePos == (sampleStart+loopLength)))) {
					speedFactor = -1 * speedFactor;
					samplePos = samplePos + speedFactor * speed;
				}
				else {
					samplePos = samplePos + speedFactor * speed;
				}
			}
			samplePos = clamp(samplePos,sampleStart,sampleStart+loopLength);
		}
		else if (trigMode == 2)
		{
			if (inputs[GATE_INPUT].value>0)
			{
				if (prevGateState == 0.0f) {
					initPos();
					play = true;
				}
				else {
					if ((readMode == 0) && (speed>=0) && (samplePos == (sampleStart+loopLength))) {
						play = false;
						if (newStop) {
							eocPulse.trigger(10 / engineGetSampleRate());
							newStop = false;
						}
					}
					else if ((readMode == 0) && (speed<0) && (samplePos == sampleStart)) {
						play = false;
						if (newStop) {
							eocPulse.trigger(10 / engineGetSampleRate());
							newStop = false;
						}
					}
					else if ((readMode == 1) && (speed>=0) && (samplePos == (sampleStart+loopLength))) {
						initPos();
					}
					else if ((readMode == 1) && (speed<0) && (samplePos == sampleStart)) {
						initPos();
					}
					else if ((readMode == 2) && ((samplePos == (sampleStart)) || (samplePos == (sampleStart+loopLength)))) {
						speedFactor = -1 * speedFactor;
						samplePos = samplePos + speedFactor * speed;
					}
					else {
						samplePos = samplePos + speedFactor * speed;
					}
				}
				samplePos = clamp(samplePos,sampleStart,sampleStart+loopLength);
			}
			else {
				play = false;
			}
		}
		prevGateState = inputs[GATE_INPUT].value;
		prevTrigState = inputs[TRIG_INPUT].value;

		if (play) {
			newStop = true;
			if (samplePos<playBuffer.getNumSamplesPerChannel()) {
				if (fadeLenght>1000) {
					if ((samplePos-sampleStart)<fadeLenght)
						fadeCoeff = rescale(samplePos-sampleStart,0.0f,fadeLenght,0.0f,1.0f);
					else if ((sampleStart+loopLength-samplePos)<fadeLenght)
						fadeCoeff = rescale(sampleStart+loopLength-samplePos,fadeLenght,0.0f,1.0f,0.0f);
					else
						fadeCoeff = 1.0f;
				}
				else
					fadeCoeff = 1.0f;

				outputs[OUTL_OUTPUT].value = playBuffer.samples[0][floor(samplePos)]*fadeCoeff*10;
				outputs[OUTR_OUTPUT].value = playBuffer.samples[1][floor(samplePos)]*fadeCoeff*10;
			}
		}
		else {
			outputs[OUTL_OUTPUT].value = 0.0f;
			outputs[OUTR_OUTPUT].value = 0.0f;
		}
	}
	else {
		outputs[OUTL_OUTPUT].value = 0.0f;
		outputs[OUTR_OUTPUT].value = 0.0f;
	}
	outputs[EOC_OUTPUT].value = eocPulse.process(1 / engineGetSampleRate()) ? 10.0f : 0.0f;
}

struct CANARDWidget : ModuleWidget {
	CANARDWidget(CANARD *module);
	Menu *createContextMenu() override;
};

struct CANARDDisplay : OpaqueWidget {
	CANARD *module;
	shared_ptr<Font> font;
	const float width = 175.0f;
	const float height = 50.0f;
	float zoomWidth = 175.0f;
	float zoomLeftAnchor = 0.0f;
	int refIdx = 0;
	float refX = 0.0f;

	CANARDDisplay() {
		font = Font::load(assetPlugin(plugin, "res/DejaVuSansMono.ttf"));
	}

	void onMouseDown(EventMouseDown &e) override {
		if (module->slices.size()>0) {
			refX = e.pos.x;
			refIdx = ((e.pos.x - zoomLeftAnchor)/zoomWidth)*(float)module->playBuffer.getNumSamplesPerChannel();
			module->addSliceMarker = refIdx;
			auto lower = std::lower_bound(module->slices.begin(), module->slices.end(), refIdx);
			module->selected = distance(module->slices.begin(),lower-1);
			module->deleteSliceMarker = *(lower-1);
		}
		if (e.button == 0)
			OpaqueWidget::onMouseDown(e);
		else {
			CANARDWidget *cANARdWidget = dynamic_cast<CANARDWidget*>(parent);
			cANARdWidget->createContextMenu();
		}
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
		std::vector<float> vL(module->playBuffer.samples[0]);
		std::vector<float> vR(module->playBuffer.samples[1]);
		std::vector<int> s(module->slices);
		module->mylock.unlock();
		size_t nbSample = vL.size();

		// Draw play line
		if ((module->play) && (!module->loading)) {
			nvgStrokeColor(vg, LIGHTBLUE_BIDOO);
			{
				nvgBeginPath(vg);
				nvgStrokeWidth(vg, 2);
				if (module->playBuffer.getNumSamplesPerChannel()>0) {
					nvgMoveTo(vg, module->samplePos * zoomWidth / nbSample + zoomLeftAnchor, 0);
					nvgLineTo(vg, module->samplePos * zoomWidth / nbSample + zoomLeftAnchor, 2*height+10);
				}
				else {
					nvgMoveTo(vg, 0, 0);
					nvgLineTo(vg, 0, 2*height+10);
				}
				nvgClosePath(vg);
			}
			nvgStroke(vg);
		}

		// Draw ref line
		nvgStrokeColor(vg, nvgRGBA(0xff, 0xff, 0xff, 0x30));
		nvgStrokeWidth(vg, 1);
		{
			nvgBeginPath(vg);
			nvgMoveTo(vg, 0, height/2);
			nvgLineTo(vg, width, height/2);
			nvgClosePath(vg);
		}
		nvgStroke(vg);

		nvgStrokeColor(vg, nvgRGBA(0xff, 0xff, 0xff, 0x30));
		nvgStrokeWidth(vg, 1);
		{
			nvgBeginPath(vg);
			nvgMoveTo(vg, 0, 3*height/2+10);
			nvgLineTo(vg, width, 3*height/2+10);
			nvgClosePath(vg);
		}
		nvgStroke(vg);

		if ((!module->loading) && (vL.size()>0)) {
			// Draw loop
			nvgFillColor(vg, nvgRGBA(255, 255, 255, 60));
			nvgStrokeWidth(vg, 1);
			{
				nvgBeginPath(vg);
				nvgMoveTo(vg, (module->sampleStart + module->fadeLenght) * zoomWidth / nbSample + zoomLeftAnchor, 0);
				nvgLineTo(vg, module->sampleStart * zoomWidth / vL.size() + zoomLeftAnchor, 2*height+10);
				nvgLineTo(vg, (module->sampleStart + module->loopLength) * zoomWidth / nbSample + zoomLeftAnchor, 2*height+10);
				nvgLineTo(vg, (module->sampleStart + module->loopLength - module->fadeLenght) * zoomWidth / nbSample + zoomLeftAnchor, 0);
				nvgLineTo(vg, (module->sampleStart + module->fadeLenght) * zoomWidth / nbSample + zoomLeftAnchor, 0);
				nvgClosePath(vg);
			}
			nvgFill(vg);

			//draw selected
			if ((module->selected >= 0) && ((size_t)module->selected < s.size())) {
				nvgStrokeColor(vg, RED_BIDOO);
				{
					nvgScissor(vg, 0, 0, width, 2*height+10);
					nvgBeginPath(vg);
					nvgStrokeWidth(vg, 4);
					nvgMoveTo(vg, (s[module->selected] * zoomWidth / nbSample) + zoomLeftAnchor , 2*height+9);
					if ((size_t)module->selected < (s.size()-1))
						nvgLineTo(vg, (s[module->selected+1] * zoomWidth / nbSample) + zoomLeftAnchor , 2*height+9);
					else
						nvgLineTo(vg, zoomWidth + zoomLeftAnchor, 2*height+9);
					nvgClosePath(vg);
				}
				nvgStroke(vg);
				nvgResetScissor(vg);
			}

			// Draw waveform
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

			if (floor(module->params[CANARD::MODE_PARAM].value) == 1) {
				nvgScissor(vg, 0, 0, width, 2*height+10);
				for (size_t i = 0; i < s.size(); i++) {
					if (s[i] != module->deleteSliceMarker) {
						nvgStrokeColor(vg, YELLOW_BIDOO);
					}
					else {
						nvgStrokeColor(vg, RED_BIDOO);
					}
					nvgStrokeWidth(vg, 1);
					{
						nvgBeginPath(vg);
						nvgMoveTo(vg, s[i] * zoomWidth / nbSample + zoomLeftAnchor , 0);
						nvgLineTo(vg, s[i] * zoomWidth / nbSample + zoomLeftAnchor , 2*height+10);
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


CANARDWidget::CANARDWidget(CANARD *module) : ModuleWidget(module) {
		setPanel(SVG::load(assetPlugin(plugin, "res/CANARD.svg")));

		addChild(Widget::create<ScrewSilver>(Vec(RACK_GRID_WIDTH, 0)));
		addChild(Widget::create<ScrewSilver>(Vec(box.size.x - 2 * RACK_GRID_WIDTH, 0)));
		addChild(Widget::create<ScrewSilver>(Vec(RACK_GRID_WIDTH, RACK_GRID_HEIGHT - RACK_GRID_WIDTH)));
		addChild(Widget::create<ScrewSilver>(Vec(box.size.x - 2 * RACK_GRID_WIDTH, RACK_GRID_HEIGHT - RACK_GRID_WIDTH)));

		{
			CANARDDisplay *display = new CANARDDisplay();
			display->module = module;
			display->box.pos = Vec(10, 35);
			display->box.size = Vec(175, 110);
			addChild(display);
		}

		static const float portX0[5] = {16, 53, 90, 126, 161};

		addChild(ModuleLightWidget::create<SmallLight<RedLight>>(Vec(portX0[0]-10, 167), module, CANARD::REC_LIGHT));
		addParam(ParamWidget::create<BlueCKD6>(Vec(portX0[0]-6, 170), module, CANARD::RECORD_PARAM, 0.0f, 1.0f, 0.0f));

		addParam(ParamWidget::create<BidooBlueKnob>(Vec(portX0[2]-7, 170), module, CANARD::SAMPLE_START_PARAM, 0.0f, 10.0f, 0.0f));
		addParam(ParamWidget::create<BidooBlueKnob>(Vec(portX0[3]-7, 170), module, CANARD::LOOP_LENGTH_PARAM, 0.0f, 10.0f, 10.0f));
		addParam(ParamWidget::create<BidooBlueKnob>(Vec(portX0[4]-7, 170), module, CANARD::READ_MODE_PARAM, 0.0f, 2.0f, 0.0f));

		addInput(Port::create<PJ301MPort>(Vec(portX0[0]-4, 202), Port::INPUT, module, CANARD::RECORD_INPUT));
		addInput(Port::create<PJ301MPort>(Vec(portX0[1]-4, 202), Port::INPUT, module, CANARD::TRIG_INPUT));
		addInput(Port::create<PJ301MPort>(Vec(portX0[1]-4, 172), Port::INPUT, module, CANARD::GATE_INPUT));
		addInput(Port::create<PJ301MPort>(Vec(portX0[2]-4, 202), Port::INPUT, module, CANARD::SAMPLE_START_INPUT));
		addInput(Port::create<PJ301MPort>(Vec(portX0[3]-4, 202), Port::INPUT, module, CANARD::LOOP_LENGTH_INPUT));
		addInput(Port::create<PJ301MPort>(Vec(portX0[4]-4, 202), Port::INPUT, module, CANARD::READ_MODE_INPUT));

		addParam(ParamWidget::create<BidooBlueKnob>(Vec(portX0[0]-7, 245), module, CANARD::SPEED_PARAM, -4.0f, 4.0f, 1.0f));
		addParam(ParamWidget::create<BidooBlueKnob>(Vec(portX0[1]-7, 245), module, CANARD::FADE_PARAM, 0.0f, 10.0f, 0.0f));
		addParam(ParamWidget::create<BidooBlueKnob>(Vec(portX0[2]-7, 245), module, CANARD::SLICE_PARAM, 0.0f, 10.0f, 0.0f));
		addParam(ParamWidget::create<BlueCKD6>(Vec(portX0[3]-6, 245), module, CANARD::CLEAR_PARAM, 0.0f, 1.0f, 0.0f));
		addOutput(Port::create<PJ301MPort>(Vec(portX0[4]-4, 247), Port::OUTPUT, module, CANARD::EOC_OUTPUT));

		addInput(Port::create<PJ301MPort>(Vec(portX0[0]-4, 277), Port::INPUT, module, CANARD::SPEED_INPUT));
		addInput(Port::create<PJ301MPort>(Vec(portX0[1]-4, 277), Port::INPUT, module, CANARD::FADE_INPUT));
		addInput(Port::create<PJ301MPort>(Vec(portX0[2]-4, 277), Port::INPUT, module, CANARD::SLICE_INPUT));
		addInput(Port::create<PJ301MPort>(Vec(portX0[3]-4, 277), Port::INPUT, module, CANARD::CLEAR_INPUT));
		addParam(ParamWidget::create<BidooBlueTrimpot>(Vec(portX0[4]-1, 280), module, CANARD::THRESHOLD_PARAM, 0.0001f, 0.05f, 0.05f));

		addParam(ParamWidget::create<CKSS>(Vec(90, 325), module, CANARD::MODE_PARAM, 0.0f, 1.0f, 0.0f));

		addInput(Port::create<TinyPJ301MPort>(Vec(19, 331), Port::INPUT, module, CANARD::INL_INPUT));
		addInput(Port::create<TinyPJ301MPort>(Vec(19+24, 331), Port::INPUT, module, CANARD::INR_INPUT));
		addOutput(Port::create<TinyPJ301MPort>(Vec(138, 331), Port::OUTPUT, module, CANARD::OUTL_OUTPUT));
		addOutput(Port::create<TinyPJ301MPort>(Vec(138+24, 331), Port::OUTPUT, module, CANARD::OUTR_OUTPUT));
}

struct CANARDDeleteSlice : MenuItem {
	CANARDWidget *canardWidget;
	CANARD *canardModule;
	void onAction(EventAction &e) override {
		canardModule->deleteFlag = true;
	}
};

struct CANARDDeleteSliceMarker : MenuItem {
	CANARDWidget *canardWidget;
	CANARD *canardModule;
	void onAction(EventAction &e) override {
		canardModule->deleteSliceMarkerFlag = true;
	}
};

struct CANARDAddSliceMarker : MenuItem {
	CANARDWidget *canardWidget;
	CANARD *canardModule;
	void onAction(EventAction &e) override {
		canardModule->addSliceMarkerFlag = true;
	}
};

struct CANARDTransientDetect : MenuItem {
	CANARDWidget *canardWidget;
	CANARD *canardModule;
	void onAction(EventAction &e) override {
		canardModule->slices.clear();
		canardModule->slices.push_back(0);
		int i = 0;
		int size = 256;
		Gist<float> gist = Gist<float>(size,engineGetSampleRate());
		vector<float>::const_iterator first;
		vector<float>::const_iterator last;
		while (i+size<canardModule->playBuffer.getNumSamplesPerChannel()) {
			first = canardModule->playBuffer.samples[0].begin() + i;
			last = canardModule->playBuffer.samples[0].begin() + i + size;
			vector<float> newVec(first, last);
			gist.processAudioFrame(newVec);
			if (((gist.energyDifference()/size)>canardModule->params[CANARD::THRESHOLD_PARAM].value)
			&& ((gist.complexSpectralDifference()/size)>canardModule->params[CANARD::THRESHOLD_PARAM].value)
			&& ((gist.zeroCrossingRate()/size)>canardModule->params[CANARD::THRESHOLD_PARAM].value)) {
				canardModule->slices.push_back(i);
			}
			i+=size;
		}
	}
};

struct CANARDLoadSample : MenuItem {
	CANARDWidget *canardWidget;
	CANARD *canardModule;
	void onAction(EventAction &e) override {
		std::string dir = canardModule->lastPath.empty() ? assetLocal("") : stringDirectory(canardModule->lastPath);
		char *path = osdialog_file(OSDIALOG_OPEN, dir.c_str(), NULL, NULL);
		if (path) {
			canardModule->loadSample(path);
			free(path);
		}
	}
};

struct CANARDSaveSample : MenuItem {
	CANARDWidget *canardWidget;
	CANARD *canardModule;
	void onAction(EventAction &e) override {
		std::string dir = canardModule->lastPath.empty() ? assetLocal("") : stringDirectory(canardModule->lastPath);
		char *path = osdialog_file(OSDIALOG_SAVE, dir.c_str(), (canardModule->waveFileName).c_str(), NULL);
		if (path) {
			canardModule->lastPath = path;
			canardModule->waveFileName = stringDirectory(path);
			canardModule->waveExtension = stringExtension(path);
			canardModule->playBuffer.setSampleRate(engineGetSampleRate());
			canardModule->playBuffer.save(path);
			free(path);
		}
	}
};

Menu *CANARDWidget::createContextMenu() {
	CANARDWidget *canardWidget = dynamic_cast<CANARDWidget*>(this);
	assert(canardWidget);

	CANARD *canardModule = dynamic_cast<CANARD*>(module);
	assert(canardModule);

	Menu *menu = ModuleWidget::createContextMenu();

	MenuLabel *spacerLabel;

	if ((canardModule->selected>=0) || (canardModule->playBuffer.getNumSamplesPerChannel()>=0)) {
		spacerLabel = new MenuLabel();
		menu->addChild(spacerLabel);
	}

	if (canardModule->selected>=0) {
		CANARDDeleteSlice *deleteItem = new CANARDDeleteSlice();
		deleteItem->text = "Delete slice";
		deleteItem->canardWidget = this;
		deleteItem->canardModule = canardModule;
		menu->addChild(deleteItem);
	}

	if (canardModule->playBuffer.getNumSamplesPerChannel()>=0) {
		CANARDAddSliceMarker *addSliceItem = new CANARDAddSliceMarker();
		addSliceItem->text = "Add slice marker";
		addSliceItem->canardWidget = this;
		addSliceItem->canardModule = canardModule;
		menu->addChild(addSliceItem);

		CANARDDeleteSliceMarker *deleteSliceItem = new CANARDDeleteSliceMarker();
		deleteSliceItem->text = "Delete slice marker";
		deleteSliceItem->canardWidget = this;
		deleteSliceItem->canardModule = canardModule;
		menu->addChild(deleteSliceItem);

		CANARDTransientDetect *trnsientItem = new CANARDTransientDetect();
		trnsientItem->text = "Search transients";
		trnsientItem->canardWidget = this;
		trnsientItem->canardModule = canardModule;
		menu->addChild(trnsientItem);
	}

	spacerLabel = new MenuLabel();
	menu->addChild(spacerLabel);

	CANARDLoadSample *loadItem = new CANARDLoadSample();
	loadItem->text = "Load sample";
	loadItem->canardWidget = this;
	loadItem->canardModule = canardModule;
	menu->addChild(loadItem);

	CANARDSaveSample *saveItem = new CANARDSaveSample();
	saveItem->text = "Save sample";
	saveItem->canardWidget = this;
	saveItem->canardModule = canardModule;
	menu->addChild(saveItem);

	return menu;
}

} // namespace rack_plugin_Bidoo

using namespace rack_plugin_Bidoo;

RACK_PLUGIN_MODEL_INIT(Bidoo, CANARD) {
   Model *modelCANARD = Model::create<CANARD, CANARDWidget>("Bidoo","cANARd", "cANARd sampler", SAMPLER_TAG);
   return modelCANARD;
}
