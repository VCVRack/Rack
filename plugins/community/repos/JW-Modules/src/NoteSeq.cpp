#include <string.h>
#include <algorithm>
#include "JWModules.hpp"
#include "dsp/digital.hpp"

namespace rack_plugin_JW_Modules {

#define ROWS 32
#define COLS 32
#define CELLS 1024
#define POLY 16
#define HW 11.75 //cell height and width

struct NoteSeq : Module,QuantizeUtils {
	enum ParamIds {
		STEP_BTN_PARAM,
		LENGTH_KNOB_PARAM,
		PLAY_MODE_KNOB_PARAM,
		RESET_BTN_PARAM,
		CLEAR_BTN_PARAM,
		RND_MODE_KNOB_PARAM,
		RND_TRIG_BTN_PARAM,
		RND_AMT_KNOB_PARAM,
		ROT_RIGHT_BTN_PARAM,
		ROT_LEFT_BTN_PARAM,
		FLIP_HORIZ_BTN_PARAM,
		FLIP_VERT_BTN_PARAM,
		SHIFT_UP_BTN_PARAM,
		SHIFT_DOWN_BTN_PARAM,
		LIFE_ON_SWITCH_PARAM,
		LIFE_SPEED_KNOB_PARAM,
		SCALE_KNOB_PARAM,
		NOTE_KNOB_PARAM,
		OCTAVE_KNOB_PARAM,
		LOW_HIGH_SWITCH_PARAM,
		HIGHEST_NOTE_PARAM,
		LOWEST_NOTE_PARAM,
		INCLUDE_INACTIVE_PARAM,
		NUM_PARAMS
	};
	enum InputIds {
		CLOCK_INPUT,
		RESET_INPUT,
		CLEAR_INPUT,
		RND_TRIG_INPUT,
		RND_AMT_INPUT,
		ROT_RIGHT_INPUT,
		ROT_LEFT_INPUT,
		FLIP_HORIZ_INPUT,
		FLIP_VERT_INPUT,
		SHIFT_UP_INPUT,
		SHIFT_DOWN_INPUT,
		HIGHEST_NOTE_INPUT,
		LOWEST_NOTE_INPUT,
		NUM_INPUTS
	};
	enum OutputIds {
		VOCT_MAIN_OUTPUT,
		GATE_MAIN_OUTPUT = VOCT_MAIN_OUTPUT + POLY,
		MIN_VOCT_OUTPUT = GATE_MAIN_OUTPUT + POLY,
		MIN_GATE_OUTPUT,
		MID_VOCT_OUTPUT,
		MID_GATE_OUTPUT,
		MAX_VOCT_OUTPUT,
		MAX_GATE_OUTPUT,
		RND_VOCT_OUTPUT,
		RND_GATE_OUTPUT,
		NUM_OUTPUTS
	};
	enum LightIds {
		GATES_LIGHT,
		NUM_LIGHTS = GATES_LIGHT + POLY,
	};
	enum PlayMode {
		PM_FWD_LOOP,
		PM_BWD_LOOP,
		PM_FWD_BWD_LOOP,
		PM_BWD_FWD_LOOP,
		PM_RANDOM_POS,
		NUM_PLAY_MODES
	};
	enum RndMode {
		RND_BASIC,
		RND_EUCLID,
		RND_SIN_WAVE,
		RND_LIFE_GLIDERS,
		NUM_RND_MODES
	};
	enum ShiftDirection {
		DIR_UP,
		DIR_DOWN
	};
	enum RotateDirection {
		DIR_LEFT,
		DIR_RIGHT
	};
	enum FlipDirection {
		DIR_HORIZ,
		DIR_VERT
	};

	float displayWidth = 0, displayHeight = 0;
	float rate = 1.0 / engineGetSampleRate();	
	float lifeRate = 0.5 * engineGetSampleRate();	
	long lifeCounter = 0;
	int seqPos = 0;
	float rndFloat0to1AtClockStep = randomUniform();
	bool goingForward = true;
	bool *cells = new bool[CELLS];
	SchmittTrigger clockTrig, resetTrig, clearTrig;
	SchmittTrigger rndTrig, shiftUpTrig, shiftDownTrig;
	SchmittTrigger rotateRightTrig, rotateLeftTrig, flipHorizTrig, flipVertTrig;
	PulseGenerator gatePulse;

	NoteSeq() : Module(NUM_PARAMS, NUM_INPUTS, NUM_OUTPUTS, NUM_LIGHTS) {
		reset();
	}

	~NoteSeq() {
		delete [] cells;
	}

	void randomize() override {
		randomizeCells();
	}

	void reset() override {
		resetSeq();
		clearCells();
	}

	void onSampleRateChange() override {
		rate = 1.0 / engineGetSampleRate();
	}

	json_t *toJson() override {
		json_t *rootJ = json_object();
		
		json_t *cellsJ = json_array();
		for (int i = 0; i < CELLS; i++) {
			json_t *cellJ = json_integer((int) cells[i]);
			json_array_append_new(cellsJ, cellJ);
		}
		json_object_set_new(rootJ, "cells", cellsJ);
		
		return rootJ;
	}

	void fromJson(json_t *rootJ) override {
		json_t *cellsJ = json_object_get(rootJ, "cells");
		if (cellsJ) {
			for (int i = 0; i < CELLS; i++) {
				json_t *cellJ = json_array_get(cellsJ, i);
				if (cellJ)
					cells[i] = json_integer_value(cellJ);
			}
		}
	}

	//TODO maybe add length input
	//TODO maybe add start pos knob and input

	void step() override {
		if(params[LIFE_ON_SWITCH_PARAM].value){
			if(lifeCounter % int(params[LIFE_SPEED_KNOB_PARAM].value) == 0){ 
				stepLife();
				lifeCounter = 1;
			}
		}

		if (clearTrig.process(params[CLEAR_BTN_PARAM].value + inputs[CLEAR_INPUT].value)) { clearCells(); }
		if (rndTrig.process(params[RND_TRIG_BTN_PARAM].value + inputs[RND_TRIG_INPUT].value)) { randomizeCells(); }

		if (rotateRightTrig.process(params[ROT_RIGHT_BTN_PARAM].value + inputs[ROT_RIGHT_INPUT].value)) { rotateCells(DIR_RIGHT); }
		if (rotateLeftTrig.process(params[ROT_LEFT_BTN_PARAM].value + inputs[ROT_LEFT_INPUT].value)) { rotateCells(DIR_LEFT); }

		if (flipHorizTrig.process(params[FLIP_HORIZ_BTN_PARAM].value + inputs[FLIP_HORIZ_INPUT].value)) { flipCells(DIR_HORIZ); }
		if (flipVertTrig.process(params[FLIP_VERT_BTN_PARAM].value + inputs[FLIP_VERT_INPUT].value)) { flipCells(DIR_VERT); }

		if (shiftUpTrig.process(params[SHIFT_UP_BTN_PARAM].value + inputs[SHIFT_UP_INPUT].value)) { shiftCells(DIR_UP); }
		if (shiftDownTrig.process(params[SHIFT_DOWN_BTN_PARAM].value + inputs[SHIFT_DOWN_INPUT].value)) { shiftCells(DIR_DOWN); }

		if (resetTrig.process(params[RESET_BTN_PARAM].value + inputs[RESET_INPUT].value)) {
			resetSeq();
		}

		if (clockTrig.process(inputs[CLOCK_INPUT].value + params[STEP_BTN_PARAM].value)) {
			clockStep();
		}

		bool pulse = gatePulse.process(1.0 / engineGetSampleRate());
		////////////////////////////////////////////// POLY OUTPUTS //////////////////////////////////////////////

		std::vector<int> polyYVals = getYValsFromBottomAtSeqPos(params[INCLUDE_INACTIVE_PARAM].value, pulse);
		for(int i=0;i<POLY;i++){ //param # starts from bottom
			bool hasVal = i < int(polyYVals.size());
			bool cellActive = hasVal && cells[iFromXY(seqPos, ROWS - polyYVals[i] - 1)];
			if(cellActive){ 
				outputs[VOCT_MAIN_OUTPUT + i].value = closestVoltageForRow(polyYVals[i]);
			}
			outputs[GATE_MAIN_OUTPUT + i].value = pulse && cellActive ? 10.0 : 0.0;
			lights[GATES_LIGHT + i].value = cellActive ? 1.0 : 0.0;
		}

		////////////////////////////////////////////// MONO OUTPUTS //////////////////////////////////////////////
		std::vector<int> monoYVals = getYValsFromBottomAtSeqPos(false, pulse);
		bool atLeastOne = int(monoYVals.size()) > 0;
		if(outputs[MIN_VOCT_OUTPUT].active && atLeastOne){
			std::vector<int>::iterator pos = std::min_element(monoYVals.begin(), monoYVals.end());
			outputs[MIN_VOCT_OUTPUT].value = closestVoltageForRow(*pos);
		}
		if(outputs[MIN_GATE_OUTPUT].active){
			outputs[MIN_GATE_OUTPUT].value = (pulse && atLeastOne) ? 10.0 : 0.0;
		}

		if(outputs[MID_VOCT_OUTPUT].active && atLeastOne){
			std::vector<int>::iterator minPos = std::min_element(monoYVals.begin(), monoYVals.end());
			std::vector<int>::iterator maxPos = std::max_element(monoYVals.begin(), monoYVals.end());
			outputs[MID_VOCT_OUTPUT].value = closestVoltageForRow((*minPos + *maxPos) * 0.5);
		}
		if(outputs[MID_GATE_OUTPUT].active){
			outputs[MID_GATE_OUTPUT].value = (pulse && atLeastOne) ? 10.0 : 0.0;
		}

		if(outputs[MAX_VOCT_OUTPUT].active && atLeastOne){
			std::vector<int>::iterator pos = std::max_element(monoYVals.begin(), monoYVals.end());
			outputs[MAX_VOCT_OUTPUT].value = closestVoltageForRow(*pos);
		}
		if(outputs[MAX_GATE_OUTPUT].active){
			outputs[MAX_GATE_OUTPUT].value = (pulse && atLeastOne) ? 10.0 : 0.0;
		}

		if(outputs[RND_VOCT_OUTPUT].active && atLeastOne){
			outputs[RND_VOCT_OUTPUT].value = closestVoltageForRow(monoYVals[int(rndFloat0to1AtClockStep * monoYVals.size())]);
		}
		if(outputs[RND_GATE_OUTPUT].active){
			outputs[RND_GATE_OUTPUT].value = (pulse && atLeastOne) ? 10.0 : 0.0;
		}

	}

	std::vector<int> getYValsFromBottomAtSeqPos(bool includeInactive, bool pulse){
		std::vector<int> v;
		for(int i=CELLS-1;i>=0;i--){
			int x = xFromI(i);
			if(x == seqPos && (cells[i] || includeInactive)){
				int y = yFromI(i);
				int yFromBottom = ROWS - 1 - y;
				if(yFromBottom <= getFinalHighestNote1to32()-1 && 
				   yFromBottom >= getFinalLowestNote1to32()-1){
					v.push_back(yFromBottom);
				}
			}
		}
		// if(pulse){
		// 	printf("getActiveYValsAtSeqPos ");
		// 	for(int n : v) { printf("%i ", n); }
		// 	printf("\n\n");
		// }
		return v;
	}

	int getFinalHighestNote1to32(){
		int inputOffset = inputs[HIGHEST_NOTE_INPUT].active ? clampijw(int(rescalefjw(inputs[HIGHEST_NOTE_INPUT].value, -5, 5, -16, 16)), 1, 32) : 0;
		return params[HIGHEST_NOTE_PARAM].value + inputOffset;
	}

	int getFinalLowestNote1to32(){
		int inputOffset = inputs[LOWEST_NOTE_INPUT].active ? clampijw(int(rescalefjw(inputs[LOWEST_NOTE_INPUT].value, -5, 5, -16, 16)), 1, 32) : 0;
		return params[LOWEST_NOTE_PARAM].value + inputOffset;
	}

	float closestVoltageForRow(int cellYFromBottom){
		int octave = params[OCTAVE_KNOB_PARAM].value;
		int rootNote = params[NOTE_KNOB_PARAM].value;
		int scale = params[SCALE_KNOB_PARAM].value;
		return closestVoltageInScale(octave + (cellYFromBottom * 0.0833), rootNote, scale);
	}

	void clockStep(){
		gatePulse.trigger(1e-3);
		lifeCounter++;
		rndFloat0to1AtClockStep = randomUniform();

		//iterate seq pos
		int curPlayMode = int(params[PLAY_MODE_KNOB_PARAM].value);
		int seqLen = int(params[NoteSeq::LENGTH_KNOB_PARAM].value);
		if(curPlayMode == PM_FWD_LOOP){
			seqPos = (seqPos + 1) % seqLen;
			goingForward = true;
		} else if(curPlayMode == PM_BWD_LOOP){
			seqPos = seqPos > 0 ? seqPos - 1 : seqLen - 1;
			goingForward = false;
		} else if(curPlayMode == PM_FWD_BWD_LOOP || curPlayMode == PM_BWD_FWD_LOOP){
			if(goingForward){
				if(seqPos < seqLen - 1){
					seqPos++;
				} else {
					seqPos--;
					goingForward = false;
				}
			} else {
				if(seqPos > 0){
					seqPos--;
				} else {
					seqPos++;
					goingForward = true;
				}
			}
		} else if(curPlayMode == PM_RANDOM_POS){
			seqPos = int(randomUniform() * seqLen);
		}
	}

	void resetSeq(){
		int curPlayMode = int(params[PLAY_MODE_KNOB_PARAM].value);
		if(curPlayMode == PM_FWD_LOOP || curPlayMode == PM_FWD_BWD_LOOP || curPlayMode == PM_RANDOM_POS){
			seqPos = 0;
		} else if(curPlayMode == PM_BWD_LOOP || curPlayMode == PM_BWD_FWD_LOOP){
			seqPos = int(params[NoteSeq::LENGTH_KNOB_PARAM].value) - 1;
		}
	}

	void rotateCells(RotateDirection dir){
		bool *newCells = new bool[CELLS];
		for(int x=0; x < COLS; x++){
			for(int y=0; y < ROWS; y++){
				switch(dir){
					case DIR_RIGHT:
						newCells[iFromXY(x, y)] = cells[iFromXY(y, COLS - x - 1)];
						break;
					case DIR_LEFT:
						newCells[iFromXY(x, y)] = cells[iFromXY(COLS - y - 1, x)];
						break;
				}

			}
		}
		cells = newCells;
	}

	void flipCells(FlipDirection dir){
		bool *newCells = new bool[CELLS];
		for(int x=0; x < COLS; x++){
			for(int y=0; y < ROWS; y++){
				switch(dir){
					case DIR_HORIZ:
						newCells[iFromXY(x, y)] = cells[iFromXY(COLS - 1 - x, y)];
						break;
					case DIR_VERT:
						newCells[iFromXY(x, y)] = cells[iFromXY(x, ROWS - 1 - y)];
						break;
				}

			}
		}
		cells = newCells;
	}

	void shiftCells(ShiftDirection dir){
		bool *newCells = new bool[CELLS];
		for(int x=0; x < COLS; x++){
			for(int y=0; y < ROWS; y++){
				switch(dir){
					case DIR_UP:
						newCells[iFromXY(x, y)] = cells[iFromXY(x, y==ROWS-1 ? 0 : y + 1)];
						break;
					case DIR_DOWN:
						newCells[iFromXY(x, y)] = cells[iFromXY(x, y==0 ? ROWS-1 : y - 1)];
						break;
				}

			}
		}
		cells = newCells;
	}

	void clearCells() {
		for(int i=0;i<CELLS;i++){
			cells[i] = false;
		}
	}

	void randomizeCells() {
		clearCells();
		float rndAmt = params[RND_AMT_KNOB_PARAM].value + inputs[RND_AMT_INPUT].value*0.1;
		switch(int(params[RND_MODE_KNOB_PARAM].value)){
			case RND_BASIC:{
				for(int i=0;i<CELLS;i++){
					setCellOn(xFromI(i), yFromI(i), randomUniform() < rndAmt);
				}
				break;
			}
			case RND_EUCLID:{
				for(int y=0; y < ROWS; y++){
					if(randomUniform() < rndAmt){
						int div = int(randomUniform() * COLS * 0.5) + 1;
						for(int x=0; x < COLS; x++){
							setCellOn(x, y, x % div == 0);
						}
					}
				}
				break;
			}
			case RND_SIN_WAVE:{
				int sinCount = int(rndAmt * 3) + 1;
				for(int i=0;i<sinCount;i++){
					float angle = 0;
					float angleInc = randomUniform();
					float offset = ROWS * 0.5;
					for(int x=0;x<COLS;x+=1){
						int y = int(offset + (sinf(angle)*(offset)));
						setCellOn(x, y, true);
						angle+=angleInc;
					}
				}
				break;
			}
			case RND_LIFE_GLIDERS:{
				int gliderCount = int(rndAmt * 20);
				int size = 3;
				for(int i=0;i<gliderCount;i++){
					int x = size + int(randomUniform() * (COLS-size*2));
					int y = size + int(randomUniform() * (ROWS-size*2));
					if(randomUniform() < 0.5){
						//down
						if(randomUniform() < 0.5){
							//right
							setCellOn(x, y, true);
							setCellOn(x+1, y+1, true);
							setCellOn(x+1, y+2, true);
							setCellOn(x, y+2, true);
							setCellOn(x-1, y+2, true);
						} else {
							//left
							setCellOn(x, y, true);
							setCellOn(x-1, y+1, true);
							setCellOn(x+1, y+2, true);
							setCellOn(x, y+2, true);
							setCellOn(x-1, y+2, true);
						}
					} else {
						//up
						if(randomUniform() < 0.5){
							//right
							setCellOn(x, y, true);
							setCellOn(x+1, y-1, true);
							setCellOn(x+1, y-2, true);
							setCellOn(x, y-2, true);
							setCellOn(x-1, y-2, true);
						} else {
							//left
							setCellOn(x, y, true);
							setCellOn(x-1, y-1, true);
							setCellOn(x+1, y-2, true);
							setCellOn(x, y-2, true);
							setCellOn(x-1, y-2, true);
						}
					}
				}
				break;
			}
		}
	}

	void stepLife(){
		bool *newCells = new bool[CELLS];
		for(int x=0; x < COLS; x++){
			for(int y=0; y < ROWS; y++){
				int cellIdx = iFromXY(x, y);
				int neighbors = getNeighborCount(x,y);
				newCells[cellIdx] = cells[cellIdx];
				if(neighbors<2 || neighbors>3) {
					newCells[cellIdx] = false;
				} else if (neighbors==3 && !cells[cellIdx]) {
					newCells[cellIdx] = true;
				}
			}
		}
		cells = newCells;
	}
	
	int getNeighborCount(int x, int y){
		int result = 0;
		for(int i=-1; i<=1; i++){
			for(int j=-1; j<=1; j++){
				int newX = x + i;
				int newY = y + j;
				if(newX==x && newY==y)continue;
				if(newX>=0 && newY>=0 && newX<COLS && newY<ROWS && 
					cells[iFromXY(newX, newY)]) result++;
			}
		}
		return result;
	}
  
	void setCellOnByDisplayPos(float displayX, float displayY, bool on){
		setCellOn(int(displayX / HW), int(displayY / HW), on);
	}

	void setCellOn(int cellX, int cellY, bool on){
		if(cellX >= 0 && cellX < COLS && 
		   cellY >=0 && cellY < ROWS){
			cells[iFromXY(cellX, cellY)] = on;
		}
	}

	bool isCellOnByDisplayPos(float displayX, float displayY){
		return isCellOn(int(displayX / HW), int(displayY / HW));
	}

	bool isCellOn(int cellX, int cellY){
		return cells[iFromXY(cellX, cellY)];
	}

	int iFromXY(int cellX, int cellY){
		return cellX + cellY * ROWS;
	}

	int xFromI(int cellI){
		return cellI % COLS;
	}

	int yFromI(int cellI){
		return cellI / COLS;
	}
};

struct NoteSeqDisplay : Widget {
	NoteSeq *module;
	bool currentlyTurningOn = false;
	float initX = 0;
	float initY = 0;
	float dragX = 0;
	float dragY = 0;
	NoteSeqDisplay(){}

	void onMouseDown(EventMouseDown &e) override { 
		if (e.button == 0) {
			e.consumed = true;
			e.target = this;
			initX = e.pos.x;
			initY = e.pos.y;
			currentlyTurningOn = !module->isCellOnByDisplayPos(e.pos.x, e.pos.y);
			module->setCellOnByDisplayPos(e.pos.x, e.pos.y, currentlyTurningOn);
		}
	}
	
	void onMouseMove(EventMouseMove &e) override {}
	void onMouseUp(EventMouseUp &e) override {}

	void onDragStart(EventDragStart &e) override {
		dragX = RACK_PLUGIN_UI_RACKWIDGET->lastMousePos.x;
		dragY = RACK_PLUGIN_UI_RACKWIDGET->lastMousePos.y;
	}

	void onDragEnd(EventDragEnd &e) override {}

	void onDragMove(EventDragMove &e) override {
		float newDragX = RACK_PLUGIN_UI_RACKWIDGET->lastMousePos.x;
		float newDragY = RACK_PLUGIN_UI_RACKWIDGET->lastMousePos.y;
		module->setCellOnByDisplayPos(initX+(newDragX-dragX), initY+(newDragY-dragY), currentlyTurningOn);
	}

	void draw(NVGcontext *vg) override {
		//background
		nvgFillColor(vg, nvgRGB(20, 30, 33));
		nvgBeginPath(vg);
		nvgRect(vg, 0, 0, box.size.x, box.size.y);
		nvgFill(vg);

		//grid
		nvgStrokeColor(vg, nvgRGB(60, 70, 73));
		for(int i=1;i<COLS;i++){
			nvgStrokeWidth(vg, (i % 4 == 0) ? 2 : 1);
			nvgBeginPath(vg);
			nvgMoveTo(vg, i * HW, 0);
			nvgLineTo(vg, i * HW, box.size.y);
			nvgStroke(vg);
		}
		for(int i=1;i<ROWS;i++){
			nvgStrokeWidth(vg, (i % 4 == 0) ? 2 : 1);
			nvgBeginPath(vg);
			nvgMoveTo(vg, 0, i * HW);
			nvgLineTo(vg, box.size.x, i * HW);
			nvgStroke(vg);
		}

		//cells
		nvgFillColor(vg, nvgRGB(25, 150, 252)); //blue
		for(int i=0;i<CELLS;i++){
			if(module->cells[i]){
				int y = i / ROWS;
				int x = i % COLS;
				nvgBeginPath(vg);
				nvgRect(vg, x * HW, y * HW, HW, HW);
				nvgFill(vg);
			}
		}

		nvgStrokeWidth(vg, 2);

		//highest note line
		float rowHighLimitY = (32-module->getFinalHighestNote1to32()) * HW;
		nvgStrokeColor(vg, nvgRGB(255, 151, 9));//orange
		nvgBeginPath(vg);
		nvgMoveTo(vg, 0, rowHighLimitY);
		nvgLineTo(vg, box.size.x, rowHighLimitY);
		nvgStroke(vg);

		//lowest note line
		float rowLowLimitY = (33-module->getFinalLowestNote1to32()) * HW;
		nvgStrokeColor(vg, nvgRGB(255, 243, 9));//yellow
		nvgBeginPath(vg);
		nvgMoveTo(vg, 0, rowLowLimitY);
		nvgLineTo(vg, box.size.x, rowLowLimitY);
		nvgStroke(vg);

		//seq length line
		float colLimitX = module->params[NoteSeq::LENGTH_KNOB_PARAM].value * HW;
		nvgStrokeColor(vg, nvgRGB(144, 26, 252));//purple
		nvgBeginPath(vg);
		nvgMoveTo(vg, colLimitX, 0);
		nvgLineTo(vg, colLimitX, box.size.y);
		nvgStroke(vg);

		//seq pos
		nvgStrokeColor(vg, nvgRGB(255, 255, 255));
		nvgBeginPath(vg);
		nvgRect(vg, module->seqPos * HW, 0, HW, box.size.y);
		nvgStroke(vg);
	}
};

struct PlayModeKnob : JwSmallSnapKnob {
	PlayModeKnob(){}
	std::string formatCurrentValue() override {
		switch(int(value)){
			case NoteSeq::PM_FWD_LOOP:return "→";
			case NoteSeq::PM_BWD_LOOP:return "←";
			case NoteSeq::PM_FWD_BWD_LOOP:return "→←";
			case NoteSeq::PM_BWD_FWD_LOOP:return "←→";
			case NoteSeq::PM_RANDOM_POS:return "*";
		}
		return "";
	}
};

struct RndModeKnob : JwSmallSnapKnob {
	RndModeKnob(){}
	std::string formatCurrentValue() override {
		switch(int(value)){
			case NoteSeq::RND_BASIC:return "Basic";
			case NoteSeq::RND_EUCLID:return "Euclid";
			case NoteSeq::RND_SIN_WAVE:return "Sine";
			case NoteSeq::RND_LIFE_GLIDERS:return "Gliders";
		}
		return "";
	}
};

struct NoteSeqWidget : ModuleWidget { 
	NoteSeqWidget(NoteSeq *module); 
};

NoteSeqWidget::NoteSeqWidget(NoteSeq *module) : ModuleWidget(module) {
	box.size = Vec(RACK_GRID_WIDTH*48, RACK_GRID_HEIGHT);

	SVGPanel *panel = new SVGPanel();
	panel->box.size = box.size;
	panel->setBackground(SVG::load(assetPlugin(plugin, "res/NoteSeq.svg")));
	addChild(panel);

	NoteSeqDisplay *display = new NoteSeqDisplay();
	display->module = module;
	display->box.pos = Vec(172, 2);
	display->box.size = Vec(376, RACK_GRID_HEIGHT - 4);
	addChild(display);
	module->displayWidth = display->box.size.x;
	module->displayHeight = display->box.size.y;

	addChild(Widget::create<Screw_J>(Vec(16, 1)));
	addChild(Widget::create<Screw_J>(Vec(16, 365)));
	addChild(Widget::create<Screw_W>(Vec(box.size.x-29, 1)));
	addChild(Widget::create<Screw_W>(Vec(box.size.x-29, 365)));

	///////////////////////////////////////////////////// LEFT SIDE /////////////////////////////////////////////////////

	//row 1
	addInput(Port::create<TinyPJ301MPort>(Vec(25, 40), Port::INPUT, module, NoteSeq::CLOCK_INPUT));
	addParam(ParamWidget::create<SmallButton>(Vec(58, 35), module, NoteSeq::STEP_BTN_PARAM, 0.0, 1.0, 0.0));
	addParam(ParamWidget::create<JwSmallSnapKnob>(Vec(92, 35), module, NoteSeq::LENGTH_KNOB_PARAM, 1.0, 32.0, 32.0));
	
	PlayModeKnob *playModeKnob = dynamic_cast<PlayModeKnob*>(ParamWidget::create<PlayModeKnob>(Vec(126, 35), module, NoteSeq::PLAY_MODE_KNOB_PARAM, 0.0, NoteSeq::NUM_PLAY_MODES - 1, 0.0));
	CenteredLabel* const playModeLabel = new CenteredLabel;
	playModeLabel->box.pos = Vec(69.5, 35);
	playModeLabel->text = "";
	playModeKnob->connectLabel(playModeLabel);
	addChild(playModeLabel);
	addParam(playModeKnob);

	//row 2
	addInput(Port::create<TinyPJ301MPort>(Vec(10, 92), Port::INPUT, module, NoteSeq::RESET_INPUT));
	addParam(ParamWidget::create<SmallButton>(Vec(30, 87), module, NoteSeq::RESET_BTN_PARAM, 0.0, 1.0, 0.0));
	addInput(Port::create<TinyPJ301MPort>(Vec(60, 92), Port::INPUT, module, NoteSeq::CLEAR_INPUT));
	addParam(ParamWidget::create<SmallButton>(Vec(80, 87), module, NoteSeq::CLEAR_BTN_PARAM, 0.0, 1.0, 0.0));

	RndModeKnob *rndModeKnob = dynamic_cast<RndModeKnob*>(ParamWidget::create<RndModeKnob>(Vec(120, 87), module, NoteSeq::RND_MODE_KNOB_PARAM, 0.0, NoteSeq::NUM_RND_MODES - 1, 0.0));
	CenteredLabel* const rndModeLabel = new CenteredLabel;
	rndModeLabel->box.pos = Vec(67, 61);
	rndModeLabel->text = "";
	rndModeKnob->connectLabel(rndModeLabel);
	addChild(rndModeLabel);
	addParam(rndModeKnob);

	//row 3
	addInput(Port::create<TinyPJ301MPort>(Vec(60, 150), Port::INPUT, module, NoteSeq::RND_TRIG_INPUT));
	addParam(ParamWidget::create<SmallButton>(Vec(80, 145), module, NoteSeq::RND_TRIG_BTN_PARAM, 0.0, 1.0, 0.0));
	addInput(Port::create<TinyPJ301MPort>(Vec(118, 150), Port::INPUT, module, NoteSeq::RND_AMT_INPUT));
	addParam(ParamWidget::create<SmallWhiteKnob>(Vec(138, 145), module, NoteSeq::RND_AMT_KNOB_PARAM, 0.0, 1.0, 0.1));

	addInput(Port::create<TinyPJ301MPort>(Vec(60, 201), Port::INPUT, module, NoteSeq::SHIFT_UP_INPUT));
	addParam(ParamWidget::create<SmallButton>(Vec(80, 196), module, NoteSeq::SHIFT_UP_BTN_PARAM, 0.0, 1.0, 0.0));
	addInput(Port::create<TinyPJ301MPort>(Vec(118, 201), Port::INPUT, module, NoteSeq::SHIFT_DOWN_INPUT));
	addParam(ParamWidget::create<SmallButton>(Vec(138, 196), module, NoteSeq::SHIFT_DOWN_BTN_PARAM, 0.0, 1.0, 0.0));

	addInput(Port::create<TinyPJ301MPort>(Vec(60, 252), Port::INPUT, module, NoteSeq::ROT_RIGHT_INPUT));
	addParam(ParamWidget::create<SmallButton>(Vec(80, 247), module, NoteSeq::ROT_RIGHT_BTN_PARAM, 0.0, 1.0, 0.0));
	addInput(Port::create<TinyPJ301MPort>(Vec(118, 252), Port::INPUT, module, NoteSeq::ROT_LEFT_INPUT));
	addParam(ParamWidget::create<SmallButton>(Vec(138, 247), module, NoteSeq::ROT_LEFT_BTN_PARAM, 0.0, 1.0, 0.0));

	addInput(Port::create<TinyPJ301MPort>(Vec(60, 304), Port::INPUT, module, NoteSeq::FLIP_HORIZ_INPUT));
	addParam(ParamWidget::create<SmallButton>(Vec(80, 299), module, NoteSeq::FLIP_HORIZ_BTN_PARAM, 0.0, 1.0, 0.0));
	addInput(Port::create<TinyPJ301MPort>(Vec(118, 304), Port::INPUT, module, NoteSeq::FLIP_VERT_INPUT));
	addParam(ParamWidget::create<SmallButton>(Vec(138, 299), module, NoteSeq::FLIP_VERT_BTN_PARAM, 0.0, 1.0, 0.0));

	addParam(ParamWidget::create<JwHorizontalSwitch>(Vec(68, 345), module, NoteSeq::LIFE_ON_SWITCH_PARAM, 0.0, 1.0, 0.0));
	addParam(ParamWidget::create<JwSmallSnapKnob>(Vec(125, 345), module, NoteSeq::LIFE_SPEED_KNOB_PARAM, 16.0, 1.0, 4.0));

	///////////////////////////////////////////////////// RIGHT SIDE /////////////////////////////////////////////////////

	float outputRowTop = 35.0;
	float outputRowDist = 21.0;
	for(int i=0;i<POLY;i++){
		int paramIdx = POLY - i - 1;
		addOutput(Port::create<TinyPJ301MPort>(Vec(559.081, outputRowTop + i * outputRowDist), Port::OUTPUT, module, NoteSeq::VOCT_MAIN_OUTPUT + paramIdx)); //param # from bottom up
		addChild(ModuleLightWidget::create<SmallLight<MyBlueValueLight>>(Vec(580, (outputRowTop+3) + i * outputRowDist), module, NoteSeq::GATES_LIGHT + paramIdx));
		addOutput(Port::create<TinyPJ301MPort>(Vec(591.858, outputRowTop + i * outputRowDist), Port::OUTPUT, module, NoteSeq::GATE_MAIN_OUTPUT + paramIdx)); //param # from bottom up
	}

	addOutput(Port::create<TinyPJ301MPort>(Vec(656.303, outputRowTop), Port::OUTPUT, module, NoteSeq::MIN_VOCT_OUTPUT));
	addOutput(Port::create<TinyPJ301MPort>(Vec(689.081, outputRowTop), Port::OUTPUT, module, NoteSeq::MIN_GATE_OUTPUT));
	addOutput(Port::create<TinyPJ301MPort>(Vec(656.303, outputRowTop + outputRowDist), Port::OUTPUT, module, NoteSeq::MID_VOCT_OUTPUT));
	addOutput(Port::create<TinyPJ301MPort>(Vec(689.081, outputRowTop + outputRowDist), Port::OUTPUT, module, NoteSeq::MID_GATE_OUTPUT));
	addOutput(Port::create<TinyPJ301MPort>(Vec(656.303, outputRowTop + 2*outputRowDist), Port::OUTPUT, module, NoteSeq::MAX_VOCT_OUTPUT));
	addOutput(Port::create<TinyPJ301MPort>(Vec(689.081, outputRowTop + 2*outputRowDist), Port::OUTPUT, module, NoteSeq::MAX_GATE_OUTPUT));
	addOutput(Port::create<TinyPJ301MPort>(Vec(656.303, outputRowTop + 3*outputRowDist), Port::OUTPUT, module, NoteSeq::RND_VOCT_OUTPUT));
	addOutput(Port::create<TinyPJ301MPort>(Vec(689.081, outputRowTop + 3*outputRowDist), Port::OUTPUT, module, NoteSeq::RND_GATE_OUTPUT));

	addInput(Port::create<TinyPJ301MPort>(Vec(643, 152), Port::INPUT, module, NoteSeq::HIGHEST_NOTE_INPUT));
	addParam(ParamWidget::create<JwSmallSnapKnob>(Vec(663, 147), module, NoteSeq::HIGHEST_NOTE_PARAM, 1.0, 32.0, 32.0));
	addInput(Port::create<TinyPJ301MPort>(Vec(643, 195), Port::INPUT, module, NoteSeq::LOWEST_NOTE_INPUT));
	addParam(ParamWidget::create<JwSmallSnapKnob>(Vec(663, 190), module, NoteSeq::LOWEST_NOTE_PARAM, 1.0, 32.0, 1.0));

	addParam(ParamWidget::create<JwHorizontalSwitch>(Vec(654, 236), module, NoteSeq::INCLUDE_INACTIVE_PARAM, 0.0, 1.0, 0.0));
	addParam(ParamWidget::create<JwSmallSnapKnob>(Vec(652, 276), module, NoteSeq::OCTAVE_KNOB_PARAM, -5.0, 7.0, 2.0));

	///// NOTE AND SCALE CONTROLS /////
	float pitchParamYVal = 320;
	float labelY = 178;

	NoteKnob *noteKnob = dynamic_cast<NoteKnob*>(ParamWidget::create<NoteKnob>(Vec(626, pitchParamYVal), module, NoteSeq::NOTE_KNOB_PARAM, 0.0, QuantizeUtils::NUM_NOTES-1, QuantizeUtils::NOTE_C));
	CenteredLabel* const noteLabel = new CenteredLabel;
	noteLabel->box.pos = Vec(319, labelY);
	noteLabel->text = "";
	noteKnob->connectLabel(noteLabel);
	addChild(noteLabel);
	addParam(noteKnob);

	ScaleKnob *scaleKnob = dynamic_cast<ScaleKnob*>(ParamWidget::create<ScaleKnob>(Vec(677, pitchParamYVal), module, NoteSeq::SCALE_KNOB_PARAM, 0.0, QuantizeUtils::NUM_SCALES-1, QuantizeUtils::MINOR));
	CenteredLabel* const scaleLabel = new CenteredLabel;
	scaleLabel->box.pos = Vec(345, labelY);
	scaleLabel->text = "";
	scaleKnob->connectLabel(scaleLabel);
	addChild(scaleLabel);
	addParam(scaleKnob);
}

} // namespace rack_plugin_JW_Modules

using namespace rack_plugin_JW_Modules;

RACK_PLUGIN_MODEL_INIT(JW_Modules, NoteSeq) {
   Model *modelNoteSeq = Model::create<NoteSeq, NoteSeqWidget>("JW-Modules", "NoteSeq", "NoteSeq", SEQUENCER_TAG);
   return modelNoteSeq;
}
