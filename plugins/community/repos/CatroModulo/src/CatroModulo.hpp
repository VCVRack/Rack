#include "rack.hpp"
#include "dsp/digital.hpp"
#include "CM_helpers.hpp"

using namespace rack;

RACK_PLUGIN_DECLARE(CatroModulo);

#if defined(USE_VST2) && !defined(RACK_PLUGIN_SHARED)
#define plugin "CatroModulo"
#endif // USE_VST2


//// Forward-declare the Plugin, defined in Template.cpp
//extern Plugin *plugin;
//
//// Forward-declare each Model, defined in each module source file
//extern Model *modelCM1Module;
//extern Model *modelCM2Module;
//extern Model *modelCM3Module;
//extern Model *modelCM4Module;
//extern Model *modelCM5Module;
//extern Model *modelCM6Module;
//extern Model *modelCM7Module;
//extern Model *modelCM8Module;
//extern Model *modelCM9Module;
//extern Model *modelCM10Module;

//interface elements
struct CM_Knob_small_def : SVGKnob {
	CM_Knob_small_def() {
		minAngle = -1.0*M_PI;
		maxAngle = 1.0*M_PI;
		setSVG(SVG::load(assetPlugin(plugin, "res/CM-knob_small_def.svg")));
        shadow->opacity = 0;
        
	}
};

struct CM_Knob_small_def_half : CM_Knob_small_def {
	CM_Knob_small_def_half() {
		minAngle = -0.5*M_PI;
		maxAngle = 0.5*M_PI;        
	}
};

struct CM_Knob_small_red : SVGKnob {
	CM_Knob_small_red() {
		minAngle = -1.0*M_PI;
		maxAngle = 1.0*M_PI;
		setSVG(SVG::load(assetPlugin(plugin, "res/CM-knob_small_red.svg")));
        shadow->opacity = 0;
        
	}
};

struct CM_Knob_big_def : SVGKnob {
	CM_Knob_big_def() {
		minAngle = -1.0*M_PI;
		maxAngle = 1.0*M_PI;
		setSVG(SVG::load(assetPlugin(plugin, "res/CM-knob_big_def.svg")));
        shadow->opacity = 0;
	}
};

struct CM_Knob_big_attn : SVGKnob {
	CM_Knob_big_attn() {
		minAngle = -1.0*M_PI;
		maxAngle = 1.0*M_PI;
		setSVG(SVG::load(assetPlugin(plugin, "res/CM-knob_big_attn.svg")));
        shadow->opacity = 0;
	}
};

struct CM_Knob_big_offset : SVGKnob {
	CM_Knob_big_offset() {
		minAngle = -1.0*M_PI;
		maxAngle = 1.0*M_PI;
		setSVG(SVG::load(assetPlugin(plugin, "res/CM-knob_big_offset.svg")));
        shadow->opacity = 0;
	}
};

struct CM_Knob_big_def_tt : CM_Knob_big_def {
	CM_Knob_big_def_tt() {
		minAngle = -0.75*M_PI;
		maxAngle = 0.75*M_PI;
	}
};

struct CM_Knob_big_red : SVGKnob {
	CM_Knob_big_red() {
		minAngle = -1.0*M_PI;
		maxAngle = 1.0*M_PI;
		setSVG(SVG::load(assetPlugin(plugin, "res/CM-knob_big_red.svg")));
        shadow->opacity = 0;
	}
};

struct CM_Knob_huge_red : SVGKnob {
	CM_Knob_huge_red() {
		minAngle = -1.0*M_PI;
		maxAngle = 1.0*M_PI;
		setSVG(SVG::load(assetPlugin(plugin, "res/CM-knob_huge_red.svg")));
        shadow->opacity = 0;
	}
};
struct CM_Knob_huge_red_os : CM_Knob_huge_red {
	CM_Knob_huge_red_os() {
		minAngle = 0.0*M_PI;
		maxAngle = 2.0*M_PI;
	}
};

struct CM_Knob_bigeye : SVGKnob {
	CM_Knob_bigeye() {
		minAngle = -1.0*M_PI;
		maxAngle = 1.0*M_PI;
		setSVG(SVG::load(assetPlugin(plugin, "res/CM-knob_bigeye.svg")));
        shadow->opacity = 0;
	}
};

struct CM_Pot1_small : SVGScrew {
	CM_Pot1_small() {
		sw->setSVG(SVG::load(assetPlugin(plugin, "res/CM-pot1_small.svg")));
		box.size = sw->box.size;
	}
};

struct CM_Pot1_big : SVGScrew {
	CM_Pot1_big() {
		sw->setSVG(SVG::load(assetPlugin(plugin, "res/CM-pot1_big.svg")));
		box.size = sw->box.size;
        
	}
};

struct CM_Input_def : SVGPort {
	CM_Input_def() {
		setSVG(SVG::load(assetPlugin(plugin, "res/CM-input_def.svg")));
        shadow->opacity = 0;
	}
};

struct CM_I_def_tinybuttonL : SVGSwitch, MomentarySwitch {
	CM_I_def_tinybuttonL() {
		addFrame(SVG::load(assetPlugin(plugin, "res/CM-input_def_tinybuttonL.svg")));
		addFrame(SVG::load(assetPlugin(plugin, "res/CM-input_def_tinybuttonL_dn.svg")));
	}
};

struct CM_I_def_tinybuttonR : SVGSwitch, MomentarySwitch {
	CM_I_def_tinybuttonR() {
		addFrame(SVG::load(assetPlugin(plugin, "res/CM-input_def_tinybuttonR.svg")));
		addFrame(SVG::load(assetPlugin(plugin, "res/CM-input_def_tinybuttonR_dn.svg")));
	}
};

struct CM_Input_small : SVGPort {
	CM_Input_small() {
		setSVG(SVG::load(assetPlugin(plugin, "res/CM-input_small.svg")));
        shadow->opacity = 0;
	}
};

struct CM_I_small_tinybuttonL : SVGSwitch, MomentarySwitch {
	CM_I_small_tinybuttonL() {
		addFrame(SVG::load(assetPlugin(plugin, "res/CM-input_small_tinybuttonL.svg")));
		addFrame(SVG::load(assetPlugin(plugin, "res/CM-input_small_tinybuttonL_dn.svg")));
	}
};

struct CM_Input_bpm : SVGPort {
	CM_Input_bpm() {
		setSVG(SVG::load(assetPlugin(plugin, "res/CM-input_bpm.svg")));
        shadow->opacity = 0;
	}
};


struct CM_Output_def : SVGPort {
	CM_Output_def() {
		setSVG(SVG::load(assetPlugin(plugin, "res/CM-output_def.svg")));
        shadow->opacity = 0;
	}
};

struct CM_Output_def_dark : SVGPort {
	CM_Output_def_dark() {
		setSVG(SVG::load(assetPlugin(plugin, "res/CM-output_def)dark.svg")));
        shadow->opacity = 0;
	}
};

struct CM_Output_small : SVGPort {
	CM_Output_small() {
		setSVG(SVG::load(assetPlugin(plugin, "res/CM-output_small.svg")));
        shadow->opacity = 0;
	}
};

struct CM_Output_bpm : SVGPort {
	CM_Output_bpm() {
		setSVG(SVG::load(assetPlugin(plugin, "res/CM-output_bpm.svg")));
        shadow->opacity = 0;
	}
};

struct CM_Switch_small : SVGSwitch, ToggleSwitch {
	CM_Switch_small() {
		addFrame(SVG::load(assetPlugin(plugin, "res/CM-TS_small_0.svg")));
		addFrame(SVG::load(assetPlugin(plugin, "res/CM-TS_small_1.svg")));
	}
};

struct CM_TryMe_button : SVGSwitch, MomentarySwitch {
	CM_TryMe_button() {
		addFrame(SVG::load(assetPlugin(plugin, "res/CM-tryme_button.svg")));
		addFrame(SVG::load(assetPlugin(plugin, "res/CM-tryme_button_dn.svg")));
	}
};
	
struct CM_Recbutton : SVGSwitch, MomentarySwitch {
	CM_Recbutton() {
		addFrame(SVG::load(assetPlugin(plugin, "res/CM-recbutton.svg")));
		addFrame(SVG::load(assetPlugin(plugin, "res/CM-recbutton_dn.svg")));
	}
};

struct CM_Button_small_red : SVGSwitch, MomentarySwitch {
	CM_Button_small_red() {
		addFrame(SVG::load(assetPlugin(plugin, "res/CM-button_small_red.svg")));
		addFrame(SVG::load(assetPlugin(plugin, "res/CM-button_small_red_dn.svg")));
	}
};

struct CM_Slider_big_red : SVGSlider {
	CM_Slider_big_red() {
		minHandlePos = Vec(-4, 0);
		maxHandlePos = Vec(58, 0);
		setSVGs(SVG::load(assetPlugin(plugin, "res/CM-slider_big_red_bg.svg")), SVG::load(assetPlugin(plugin, "res/CM-slider_big_red.svg")));
	}
	void onDragMove(EventDragMove& e) override;
};

struct CM_Switch_small_3 : SVGSwitch, ToggleSwitch {
	CM_Switch_small_3() {
		addFrame(SVG::load(assetPlugin(plugin, "res/CM-TS_small_3_0.svg")));
		addFrame(SVG::load(assetPlugin(plugin, "res/CM-TS_small_3_1.svg")));
		addFrame(SVG::load(assetPlugin(plugin, "res/CM-TS_small_3_2.svg")));
	}
};



//mechanisms

//SELECT sequencer
struct CM_SelSeq {
	private:
	SchmittTrigger stepTrigger;
	SchmittTrigger resetTrigger;
	int patterns[16][16] = {};
	bool dostep;
	float recsel;
	int astep;
	
	public:
	bool patternized;

	CM_SelSeq(){
		dostep = true;
		recsel = 0.0f;
		astep = 0;
		patternized = false;
		patternize();
	}
	//sequencer built-in patterns
 	void patternize(){

		int pat0[16] =  {0,1,2,3,4,5,6,7,0,1,2,3,4,5,6,7};
		int pat1[16] =  {0,7,6,5,4,3,2,1,0,7,6,5,4,3,2,1};
		int pat2[16] =  {0,-1,1,-1,2,-1,3,-1,4,-1,5,-1,6,-1,7,-1};
		int pat3[16] =  {0,-1,7,-1,6,-1,5,-1,4,-1,3,-1,2,-1,1,-1};
 
		int pat4[16] =  {0,2,6,4,1,3,7,5,0,2,6,4,1,3,7,5};
		int pat5[16] =  {0,2,4,6,1,3,5,7,0,2,4,6,1,3,5,7};
		int pat6[16] =  {0,2,1,3,2,4,3,5,4,6,5,7,6,0,7,1};
		int pat7[16] =  {0,3,1,4,2,5,3,6,4,7,5,0,6,1,7,2};
 
		int pat8[16] =  {0,1,2,3,4,5,6,7,7,6,5,4,3,2,1,0};
		int pat9[16] =  {0,1,2,3,4,3,2,1,0,1,2,3,4,3,2,1};
		int pat10[16] = {0,1,2,3,2,1,0,1,2,3,2,1,0,1,2,3};
		int pat11[16] = {0,4,1,5,2,6,3,7,4,0,5,1,6,2,7,3};

		int pat12[16] = {0,1,2,3,1,2,3,4,2,3,4,5,3,4,5,6};
		int pat13[16] = {0,1,2,1,2,3,2,3,4,3,4,5,4,5,6,5};
		int pat14[16] = {7,5,6,4,5,3,4,2,1,3,2,4,3,5,4,6};
		int pat15[16] = {0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0};


		for (int i = 0; i < 16; i++){
		 patterns[0][i] = pat0[i];
		}
		for (int i = 0; i < 16; i++){
		 patterns[1][i] = pat1[i];
		}
		for (int i = 0; i < 16; i++){
		 patterns[2][i] = pat2[i];
		}
		for (int i = 0; i < 16; i++){
		 patterns[3][i] = pat3[i];
		}
		for (int i = 0; i < 16; i++){
		 patterns[4][i] = pat4[i];
		}
		for (int i = 0; i < 16; i++){
		 patterns[5][i] = pat5[i];
		}
		for (int i = 0; i < 16; i++){
		 patterns[6][i] = pat6[i];
		}
		for (int i = 0; i < 16; i++){
		 patterns[7][i] = pat7[i];
		}
		for (int i = 0; i < 16; i++){
		 patterns[8][i] = pat8[i];
		}
		for (int i = 0; i < 16; i++){
		 patterns[9][i] = pat9[i];
		}
		for (int i = 0; i < 16; i++){
		 patterns[10][i] = pat10[i];
		}
		for (int i = 0; i < 16; i++){
		 patterns[11][i] = pat11[i];
		}
		for (int i = 0; i < 16; i++){
		 patterns[12][i] = pat12[i];
		}
		for (int i = 0; i < 16; i++){
		 patterns[13][i] = pat13[i];
		}
		for (int i = 0; i < 16; i++){
		 patterns[14][i] = pat14[i];
		}
		for (int i = 0; i < 16; i++){
		 patterns[15][i] = pat15[i];
		}
		patternized = true;
	}

	void reset(float ireset){
		if (resetTrigger.process(ireset)) {
			astep = 0;
			dostep = true;
		}
	}

	void step(float istep, float len){
		if (stepTrigger.process(istep)) {
			if(astep < len && len <= 16){astep++;}else{astep = 0;}
			dostep = true;
		}
	}

	//Main sequencer function
	float sequence(int pat){
		if (dostep == true){
			if (pat == 15){
				recsel = cm_gauss(4.0, 4.0);
			}else{
				recsel = patterns[pat][astep];
			}
			dostep = false;
		}
		return recsel;
	}

};

//CV recorder order: scan, mix, output
struct CM_Recorder {
	private:
	float store[8][8] = {};
	float call[8] = {};
	float out[8] = {};
	float lastselect = -1.0;
	float lastscan = -1.0;
	SchmittTrigger randomTrigger;

	public:

	CM_Recorder(){
		randomize();
	}

	void reset(){
		lastselect = -1.0;
	}
	
	void randomize(){
		srand(time(NULL));
	}

	//Save
	float save(int i, int j){
		return store[i][j];
	}

	//Load
	void load(int i, int j, float val){
		store[i][j] = val;
	}

	//store recording
	void record(float *eyeval, int i){
		for (int j = 0; j < 8; j++) {
			store[i][j] = eyeval[j];
		}
		reset();
	}

	//randomize recordings
	void tryme(float dotry){
		if (randomTrigger.process(dotry)){
			for (int i = 0; i < 8; i++){
				for (int j = 0; j < 8; j++) {
					rand(); rand(); //call random twice to get more random
					store[i][j] = ((double) rand() / (RAND_MAX)); 
				}
			}
			reset();
		}
	}

	//selector scanning
	void scan(float select, float scanner){
		if (scanner == 0.0){
			if (select > 7.5){
				select = 0.0;
			}else{
			select = roundf(select);
			}
		}
		if (scanner != lastscan){
			reset();
		}

		if (select != lastselect){
			for (int i = 0; i < 8; i++) {
				if (select < (i + 1.0)){
					if (i == 7){
						for (int j = 0; j < 8; j++) {
							call[j] = (store[i][j] * (1.0f - (select - i)) + (store[0][j] * (select - i)));
						}
						i = 9;
					}else{
						for (int j = 0; j < 8; j++) {
							call[j] = (store[i][j] * (1.0f - (select - i)) + (store[i+1][j] * (select - i)));
						}
						i = 9;
					}
				}
			}	
			lastselect = select;
		}
		lastscan = scanner;
	}

	float callget(int eye){
		return call[eye];
	}

	//mix amount is between -1.0 and 1.0
	void mix(float *eyeval, float amount){
		if (amount >= 0.0f){
			for (int i = 0; i < 8; i++) {
				out[i] = (eyeval[i] * (1.0f - amount) + call[i] * amount);
			}
		}else{
			for (int i = 0; i < 8; i++) {
				out[i] = (eyeval[i] * call[i] * (0.0 - amount)) + (eyeval[i] * (1.0f + amount));
			}
		}
	}

	float output(int index){
		return out[index] * 10.0f;
	}
};

//try something else?
// struct CM_BpmStreamer {
// 	int mcount = 0;
// 	float bpm_cv = 0;
// 	float reset = 0;
// 	float demuxbuffer[4] = {};

// 	CM_BpmStreamer(){}

// 	void step(){
// 		mcount = (mcount < 3) ? mcount + 1 : 0;
// 	}

// 	float mux(){
// 		float muxed = 0.0;
// 		if (mcount == 0){
// 			muxed = -5.1;
// 		}
// 		if (mcount == 1){
// 			muxed = bpm_cv;
// 		}
// 		if (mcount == 2){
// 			muxed = reset;
// 		}
// 		if (mcount == 3){
// 			muxed = -10.1;
// 		}
// 		return muxed;
// 	}

// 	float mux(float signal){
// 		float muxed = 0.0;
// 		if (mcount == 0){
// 			muxed = -5.1;
// 		}
// 		if (mcount == 1){
// 			muxed = signal;
// 		}
// 		if (mcount == 2){
// 			muxed = reset;
// 		}
// 		if (mcount == 3){
// 			muxed = -10.1;
// 		}
// 		return muxed;
// 	}

// 	bool demux(float signal){
// 		demuxbuffer[3] = demuxbuffer[2];
// 		demuxbuffer[2] = demuxbuffer[1];
// 		demuxbuffer[1] = demuxbuffer[0];
// 		demuxbuffer[0] = signal;
// 		if (demuxbuffer[0] == -10.1 && demuxbuffer[3] == -5.1){
// 			bpm_cv = demuxbuffer[2];
// 			reset = demuxbuffer[1];
// 			return true;
// 		}else{
// 			return false;
// 		}
// 	}

// 	float test(){
// 		return (demuxbuffer[0] < -10.0 && demuxbuffer[0] > -11.0) * 10.0;
// 	}

// 	void setcv(float cv){
// 		bpm_cv = cv;
// 	}

// 	void setreset(float rst){
// 		reset = (rst >= 5.0) ? 10.0 : 0.0;
// 	}

// 	float getcv(){
// 		return bpm_cv;
// 	}

// 	float getreset(){
// 		return reset;
// 	}

// };

//BPM system
struct CM_BpmClock {
	private:

	float clk_bpm;
	float bpm_cv;
	float phase;
	float pw;
	float freq;
	SchmittTrigger resetTrigger;
	SchmittTrigger trackingTrigger[3];
	float bpm_out[3] = {};
	float clk_out[3] = {};

	public:

	CM_BpmClock(){
		clk_bpm = 0.0f;
		bpm_cv = 0.0f;
		phase = 0.0f;
		pw = 0.5f;
		freq = 1.0f;
	}

	//between 0 and 1000
	void setbpm(float bpm){
		bpm = max(bpm, 0.0f);
		clk_bpm = bpm;
		bpm_cv = bpmtocv(bpm);
		// freq = clk_bpm / 30.0; //double freq! -for halfstep
	}	
	void setcv(float cv){
		setbpm(cvtobpm(cv));
	}

	float addcv(float cv){
		setbpm(clk_bpm + cvtobpm(cv));
		return bpm_cv;
	}

	float getcv(){
		return bpm_cv;
	}

	float getbpm(){
		return clk_bpm;
	}

	void step(float dt){
		pulsegen();		
		freq = clk_bpm / 30.0; //double freq! -for halfstep
		float deltaPhase = fminf(freq * dt, 0.5f); //delta is halftime
		phase += deltaPhase;
		if (phase >= 1.0f){phase -= 1.0f;}
				
	}

  	void setReset(float reset) {
		if (resetTrigger.process(reset)) {
			phase = 0.0f;
			clk_out[0] = 1.0;
			clk_out[1] = 1.0;
			clk_out[2] = 1.0;
			trackingTrigger[0].reset();
			trackingTrigger[1].reset();
			trackingTrigger[2].reset();
		}
	}

	float track(int out){
		return clk_out[out];
	}

	float bpmtocv(float bpm){
		return bpm * 0.01;
	}

	float cvtobpm(float cv){
		return cv * 100.0;
	}

	private:

	float sqr(){
		float sqr = phase < pw ? 1.0f : -1.0f;
		return sqr;
	}

	void pulsegen(){
		if (trackingTrigger[0].process(sqr())){
			clk_out[0] = !(clk_out[0]);
		}
		if (trackingTrigger[1].process(clk_out[0])){
			clk_out[1] = !(clk_out[1]);
		}
		if (trackingTrigger[2].process(clk_out[1])){
			clk_out[2] = !(clk_out[2]);
		}
	}
};

//simple stepper (count start at 0)
struct CM_stepper {
	private:
	int step_active = 0;
	int step_max = 8;
	bool isreset = false;
	int cooldown = 0;

	public:

	CM_stepper(){}

	void reset(){
		isreset = true;
	}

	//advance step, return to 0 on reaching max;
	int step(int max){
		step_max = max;
		if (step_active < step_max && isreset == false){
			step_active++;
		}else{
			step_active = 0;
			isreset = false;
		}
		return step_active;
	}
};

//LCD display (from cf modules)
struct NumDisplayWidget : TransparentWidget {

  float *value;
  std::shared_ptr<Font> font;

	NumDisplayWidget() {
		font = Font::load(assetPlugin(plugin, "res/Segment7Standard.ttf"));
	};

	void draw(NVGcontext *vg) override {
		// Background
		NVGcolor backgroundColor = nvgRGB(0x25, 0x2f, 0x24);
		NVGcolor borderColor = nvgRGB(0x10, 0x10, 0x10);
		nvgBeginPath(vg);
		nvgRoundedRect(vg, 0.0, 0.0, box.size.x, box.size.y, 4.0);
		nvgFillColor(vg, backgroundColor);
		nvgFill(vg);
		nvgStrokeWidth(vg, 1.0);
		nvgStrokeColor(vg, borderColor);
		nvgStroke(vg);

		nvgFontSize(vg, 16);
		nvgFontFaceId(vg, font->handle);
		nvgTextLetterSpacing(vg, 2.2);

		std::string to_display = std::to_string(*value).substr(0,5);
		while(to_display.length()< 5 ) to_display = ' ' + to_display;

		Vec textPos = Vec(3.0f, 17.0f);

		NVGcolor textColor = nvgRGB(0xff, 0xf4, 0x00);
		nvgFillColor(vg, nvgTransRGBA(textColor, 16));
		nvgText(vg, textPos.x, textPos.y, "~~~~~", NULL);
		nvgText(vg, textPos.x, textPos.y, ".....", NULL);
		nvgText(vg, textPos.x, textPos.y, "\\\\\\\\\\", NULL);

		nvgFillColor(vg, textColor);
		nvgText(vg, textPos.x, textPos.y, to_display.c_str(), NULL);
	}
};

//TEXT LCD display
struct TxtDisplayWidget : TransparentWidget {

  std::string *txt;
  std::shared_ptr<Font> font;

	TxtDisplayWidget() {
		font = Font::load(assetPlugin(plugin, "res/Segment7Standard.ttf"));
	};

	void draw(NVGcontext *vg) override {
		// Background
		NVGcolor backgroundColor = nvgRGB(0x25, 0x2f, 0x24);
		NVGcolor borderColor = nvgRGB(0x10, 0x10, 0x10);
		nvgBeginPath(vg);
		nvgRoundedRect(vg, 0.0, 0.0, box.size.x, box.size.y, 4.0);
		nvgFillColor(vg, backgroundColor);
		nvgFill(vg);
		nvgStrokeWidth(vg, 1.0);
		nvgStrokeColor(vg, borderColor);
		nvgStroke(vg);

		nvgFontSize(vg, 16);
		nvgFontFaceId(vg, font->handle);
		nvgTextLetterSpacing(vg, 2.2);

		std::string to_display = *txt;
		while(to_display.length()< 3 ) to_display = ' ' + to_display;

		Vec textPos = Vec(3.0f, 17.0f);

		NVGcolor textColor = nvgRGB(0xff, 0xf4, 0x00);
		nvgFillColor(vg, nvgTransRGBA(textColor, 16));
		nvgText(vg, textPos.x, textPos.y, "~~~", NULL);
		nvgText(vg, textPos.x, textPos.y, "...", NULL);
		nvgText(vg, textPos.x, textPos.y, "\\\\\\", NULL);

		nvgFillColor(vg, textColor);
		nvgText(vg, textPos.x, textPos.y, to_display.c_str(), NULL);
	}
};

//recorder buttons for CM3
struct CM3_RecBall : TransparentWidget {

	float *recball_x;
	float *recball_y;

	CM3_RecBall() {};

	void draw(NVGcontext *vg) override {
		//position
		box.pos.x = *recball_x;
		box.pos.y = *recball_y;
		// circle
		NVGcolor yellow = nvgRGB(0xff, 0xf4, 0x00);
		nvgBeginPath(vg);
		nvgCircle(vg, 7.0, 7.0, 8.0);
		nvgFillColor(vg, yellow);
		nvgFill(vg);
	}
};

//bigeye indicators 
struct CM3_EyePatch : TransparentWidget {

	float *eyepatch_val;
	float dd;
	float rr;

	CM3_EyePatch(float x, float y, float d, float r) {
		box.pos.x = x;
		box.pos.y = y;
		dd = d;
		rr = r;
	};

	void draw(NVGcontext *vg) override {

		//position
		 float relx = -dd * -sin(*eyepatch_val * M_PI);
		 float rely = dd * -cos(*eyepatch_val * M_PI);
		
		// circle
		nvgBeginPath(vg);
		nvgCircle(vg, relx, rely, rr);
		nvgFillColor(vg, COLOR_WHITE);
		nvgFill(vg);

	}
};

//yellow led in CM9
struct CM9_LedIndicator : SVGWidget {

	float *posx;
	float *posy;

	CM9_LedIndicator() {
		setSVG(SVG::load(assetPlugin(plugin, "res/CM9_ledinc.svg")));
		wrap();
	};

	void draw(NVGcontext *vg) override {
		box.pos.x = *posx;
		box.pos.y = *posy;
		SVGWidget::draw(vg);
	}
};

//yellow big led indicator
struct BigLedIndicator : TransparentWidget {

  bool *lit;

	BigLedIndicator() {};

	void draw(NVGcontext *vg) override {
		// Background
		NVGcolor backgroundColor = nvgRGB(0x25, 0x2f, 0x24);
		NVGcolor borderColor = nvgRGB(0x10, 0x10, 0x10);
		nvgBeginPath(vg);
		nvgRoundedRect(vg, 0.0, 0.0, box.size.x, box.size.y, 5.0);
		nvgFillColor(vg, backgroundColor);
		nvgFill(vg);
		nvgStrokeWidth(vg, 1.5);
		nvgStrokeColor(vg, borderColor);
		nvgStroke(vg);
		if (*lit == true){
			nvgBeginPath(vg);
			nvgRoundedRect(vg, 4.0, 4.0, box.size.x - 8.0, box.size.y - 8.0, 4.0);
			nvgFillColor(vg, nvgRGB(0xff, 0xf4, 0x00));
			nvgFill(vg);;
		}	
	}
};
