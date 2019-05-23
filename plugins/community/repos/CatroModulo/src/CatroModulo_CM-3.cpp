#include "CatroModulo.hpp"

//Catro-Modulo CM3: PreSetSeq

struct CM3Module : Module {

	enum ParamIds {
		ENUMS(PARAM_REC, 8), 
		ENUMS(PARAM_EYE, 8), 
		PARAM_PATTERN,
		PARAM_MORPH,
		PARAM_LENGTH,
		PARAM_TRYME,
		PARAM_SCAN,
		PARAM_SELECT,
		PARAM_Q,
		PARAM_SEQ,
		PARAM_RESET,
		PARAM_STEP,
		NUM_PARAMS,
	};
	enum InputIds {
		ENUMS(INPUT_REC, 8), 
		ENUMS(INPUT_EYE, 8), 
		INPUT_PATTERN,
		INPUT_STEP,
		INPUT_MORPH,
		INPUT_RESET,
		INPUT_LENGTH,
		INPUT_SELECT,
		INPUT_BPM,
		NUM_INPUTS
	};
	enum OutputIds {
		ENUMS(OUTPUT_EYE, 8), 
		NUM_OUTPUTS
	};
	enum LightIds {
		NUM_LIGHTS
	};

	//initializations
	std::string display_pat = "";
	std::string display_len = "";
	std::string strings_pat[16] = {"SEQ", "REV", "S0Q", "R0V", "NZN", "CUC", "ZZ1", "ZZ2", "U7D", "U4D", "U3D", ">-<", "/\\/", ".-.", "\\/\\", "RND"};
	std::string strings_len[16] = {" 01", " 02", " 03", " 04", " 05", " 06", " 07", " 08", " 09", " 10", " 11", " 12", " 13", " 14", "15", "16"};
	SchmittTrigger recordTrigger[16];
	float iselect = 0.0f;	
	float recsel = 0.0f;
	float recball_x = 178.8;
	float recball_y = 89.5;
	float recball_xarray[8] = {178.8f , 212.4f , 242.7f , 212.4f , 178.8f , 145.3f , 115.0f , 145.3f};
	float recball_yarray[8] = {89.5f , 119.9f , 153.4f , 186.9f , 217.2f , 186.9f , 153.4f , 119.9f };
	float eyepatch_val[8] = {};	

	CM_SelSeq sequencer;
	CM_Recorder recorder;
	CM_BpmClock bpmclock;

	
	CM3Module() : Module(NUM_PARAMS, NUM_INPUTS, NUM_OUTPUTS, NUM_LIGHTS) {}
	
	void step() override;


	json_t *toJson() override {

		json_t *rootJ = json_object();

		json_t *recordsJ = json_array();
		for (int i = 0; i < 8; i++){
			for (int j = 0; j < 8; j++){
				json_array_append_new(recordsJ, json_real(recorder.save(i,j)));
			}
		}
		json_object_set_new(rootJ, "recorder", recordsJ);
	
		return rootJ;
	}


	void fromJson(json_t *rootJ) override {
		// running
		json_t *recorderJ = json_object_get(rootJ, "recorder");
		for (int i = 0; i < 8; i++){
			for (int j = 0; j < 8; j++){
				recorder.load(i, j, json_real_value(json_array_get(recorderJ, 8 * i + j)));
			}
		}

	}

	// For more advanced Module features, read Rack's engine.hpp header file
	// - toJson, fromJson: serialization of internal data
	// - onSampleRateChange: event triggered by a change of sample rate
	// - onReset, onRandomize, onCreate, onDelete: implements special behavior when user clicks these from the context menu
};

void CM3Module::step() {


	//mix params and inputs
	float morph = (inputs[INPUT_MORPH].active) ? inputs[INPUT_MORPH].value * 0.1f + params[PARAM_MORPH].value : params[PARAM_MORPH].value;
	float seq_active = 1.0 - params[PARAM_SEQ].value;
	float seq_reset = (inputs[INPUT_RESET].value || params[PARAM_RESET].value);
	float seq_pattern = clamp(roundf((inputs[INPUT_PATTERN].active) ? inputs[INPUT_PATTERN].value * 0.1f * params[PARAM_PATTERN].value : params[PARAM_PATTERN].value), 0.0, 15.0);
	float seq_len = clamp(roundf((inputs[INPUT_LENGTH].active) ? inputs[INPUT_LENGTH].value * 0.1f * params[PARAM_LENGTH].value : params[PARAM_LENGTH].value), 0.0, 15.0);
	float doscan = (params[PARAM_SCAN].value && params[PARAM_SEQ].value);

	//check for bpm cv
	float seq_step = 0;
	if (inputs[INPUT_BPM].active){
		bpmclock.setcv(inputs[INPUT_BPM].value);
	}
	
	bpmclock.setReset(inputs[INPUT_RESET].value || params[PARAM_RESET].value);
		
	if (inputs[INPUT_BPM].active){
		bpmclock.step(engineGetSampleTime());
		seq_step = bpmclock.track(1);
	}else{
		seq_step = (inputs[INPUT_STEP].value || params[PARAM_STEP].value);
	}

	//process tryme button
	recorder.tryme(params[PARAM_TRYME].value);


	//process eyes
	float eyeval[8] = {};
	for (int i = 0; i < 8; i++) {
		float in = 1.0f;
		float eye = params[i+PARAM_EYE].value;
		if (inputs[i+PARAM_EYE].active){
			in = inputs[i+PARAM_EYE].value * 0.1f;
		}
		eyeval[i] = clamp(in * eye, -1.0f, 1.0f);
	}

	//record when requested
		for (int i = 0; i < 8; i++) {
			if (recordTrigger[i].process((inputs[INPUT_REC+i].value || params[PARAM_REC+i].value))){
				recorder.record(eyeval, i);
			}
		}

	//process sequencer
	if (seq_active == 1.0){
		sequencer.reset(seq_reset);
		sequencer.step(seq_step, seq_len);
		if (sequencer.patternized == true) {
			iselect = sequencer.sequence(seq_pattern);
		}
	}else{
		iselect = clamp((inputs[INPUT_SELECT].active) ? inputs[INPUT_SELECT].value * 0.1f * params[PARAM_SELECT].value : params[PARAM_SELECT].value, 0.0, 7.99999f);
	}
	recorder.scan(iselect, doscan);
	recorder.mix(eyeval,morph);
	for (int i = 0; i < 8; i++) {
		if (iselect != -1.0){
			outputs[OUTPUT_EYE + i].value = recorder.output(i);
		}else{
			outputs[OUTPUT_EYE + i].value = 0.0f;
		}
		
	}

	//set eyepatches
	for (int i = 0; i < 8; i++) {
		eyepatch_val[i] = recorder.callget(i);
	}
	
	//set displays
	display_pat = (seq_active) ? strings_pat[int(seq_pattern)] : "OFF";
	display_len = strings_len[int(seq_len)];

	//recball
	if (iselect != -1.0){
		recball_x = recball_xarray[int(iselect)] + 9.0;
		recball_y = recball_yarray[int(iselect)] + 9.0;
	}
	
}

struct CM3ModuleWidget : ModuleWidget {

	CM3ModuleWidget(CM3Module *module) : ModuleWidget(module) {
		setPanel(SVG::load(assetPlugin(plugin, "res/CM-3.svg")));

		addChild(Widget::create<ScrewSilver>(Vec(10, 0)));
		addChild(Widget::create<ScrewSilver>(Vec(box.size.x - 20, 0)));
		addChild(Widget::create<ScrewSilver>(Vec(30, 365)));
		addChild(Widget::create<ScrewSilver>(Vec(box.size.x - 40, 365)));

		int y = 0; //initialize reusable counter

		//REC BUTTONS
		float recbuttons[16] = {178.8 , 89.5 ,
							    212.4 , 119.9 ,
							    242.7 , 153.4 ,
							    212.4 , 186.9 ,
							    178.8 , 217.2 ,
							    145.3 , 186.9 ,
							    115.0 , 153.4 ,
							    145.3 , 119.9 };
		y = 0;
		for(int i = 0; i < 16; i += 2){
			addParam(ParamWidget::create<CM_Recbutton>(Vec(recbuttons[i],recbuttons[i+1] - 0.5), module, CM3Module::PARAM_REC + y, 0.0f, 1.0f, 0.0f));
			y++;
		}

		//REC INPUTS
		float recin[16] = { 185.5 , 127.8 ,
							196.5 , 149.0 ,
							217.4 , 160.0 ,
							196.5 , 171.0 ,
							185.5 , 192.3 ,
							174.5 , 171.0 ,
							153.2 , 160.0 ,
							174.5 , 149.0 };
		y = 0;
		for(int i = 0; i < 16; i += 2){
			addInput(Port::create<CM_Input_small>(Vec(recin[i],recin[i+1] - 0.5), Port::INPUT, module, CM3Module::INPUT_REC + y));
			y++;
		}


		//BIGEYES
		float bigeyes[16] = {54.9 , 94.9 ,
							32.7 , 146.6 ,
							54.9 , 198.4 ,
							84.5 , 250.1 ,
							290.7 , 94.9 ,
							312.9 , 146.6 ,
							290.7 , 198.4 ,
							261.1 , 250.1};
		y = 0;

		for(int i = 0; i < 16; i += 2){
			
			addParam(ParamWidget::create<CM_Knob_bigeye>(Vec(bigeyes[i],bigeyes[i+1] - 0.5), module, CM3Module::PARAM_EYE + y, -1.0f, 1.0f, 0.0f));
			y++;
		}

		//EYE INPUTS
		float eyein[16] = {104.7 , 117.5 ,
							84.1 , 159.3 ,
							104.7 , 201.1 ,
							130.7 , 243.4 ,
							266.0 , 117.5 ,
							287.1 , 159.3 ,
							266.0 , 201.1 ,
							240.3 , 243.4};
		y = 0;
		for(int i = 0; i < 16; i += 2){
			addInput(Port::create<CM_Input_small>(Vec(eyein[i],eyein[i+1] - 0.5), Port::INPUT, module, CM3Module::INPUT_EYE + y));
			y++;
		}

		//EYE OUTPUTS
		float eyeout[16] = {30.2 , 97.1 ,
							6.4 , 158.8 ,
							30.2 , 220.5 ,
							63.6 , 281.4 ,
							340.0 , 97.1 ,
							363.5 , 158.8 ,
							340.0 , 220.5 ,
							304.5 , 281.4};
		y = 0;
		for(int i = 0; i < 16; i += 2){
			addOutput(Port::create<CM_Output_small>(Vec(eyeout[i],eyeout[i+1] - 0.5), Port::OUTPUT, module, CM3Module::OUTPUT_EYE + y));
			y++;
		}

		//OTHER ELEMENTS
		addParam(ParamWidget::create<CM_Knob_small_def_half>(Vec(33.4 , 34.7), module, CM3Module::PARAM_PATTERN, 0.0f, 15.0f, 0.0f));
		addParam(ParamWidget::create<CM_Slider_big_red>(Vec(156.5 , 17.9), module, CM3Module::PARAM_MORPH, -1.0f, 1.0f, 0.0f));
		addParam(ParamWidget::create<CM_Knob_small_def_half>(Vec(326.0 , 34.7), module, CM3Module::PARAM_LENGTH, 0.0f, 15.0f, 7.0f));
		addParam(ParamWidget::create<CM_TryMe_button>(Vec(17.0 , 322.1), module, CM3Module::PARAM_TRYME, 0.0f, 1.0f, 0.0f));
		addParam(ParamWidget::create<CM_Switch_small>(Vec(137.8 , 309.0), module, CM3Module::PARAM_SCAN, 0.0f, 1.0f, 0.0f));
		addParam(ParamWidget::create<CM_Knob_huge_red_os>(Vec(161.3 , 286.0), module, CM3Module::PARAM_SELECT, 0.0f, 7.99999f, 0.0f));
		//addParam(ParamWidget::create<CM_Knob_small_def>(Vec(232.2 , 304.5), module,PARAM_Q, 0.1f, 0.9f, 0.5f)); //maybe implement later?
		addParam(ParamWidget::create<CM_Switch_small>(Vec(366. , 309.0), module, CM3Module::PARAM_SEQ, 0.0f, 1.0f, 1.0f));
		addParam(ParamWidget::create<CM_I_def_tinybuttonR>(Vec(263.0 , 38.7), module, CM3Module::PARAM_RESET, 0.0f, 1.0f, 0.0f));
		addParam(ParamWidget::create<CM_I_def_tinybuttonL>(Vec(85.4 , 38.7), module, CM3Module::PARAM_STEP, 0.0f, 1.0f, 0.0f));



		addInput(Port::create<CM_Input_def>(Vec(15.7 , 60.1), Port::INPUT, module, CM3Module::INPUT_PATTERN));
		addInput(Port::create<CM_Input_def>(Vec(94.0 , 38.7), Port::INPUT, module, CM3Module::INPUT_STEP));
		addInput(Port::create<CM_Input_bpm>(Vec(127.5 , 38.7), Port::INPUT, module, CM3Module::INPUT_BPM));
		addInput(Port::create<CM_Input_def>(Vec(183.5 , 45.4), Port::INPUT, module, CM3Module::INPUT_MORPH));
		addInput(Port::create<CM_Input_def>(Vec(250.8 , 38.7), Port::INPUT, module, CM3Module::INPUT_RESET));
		addInput(Port::create<CM_Input_def>(Vec(352.3 , 61.4), Port::INPUT, module, CM3Module::INPUT_LENGTH));
		addInput(Port::create<CM_Input_def>(Vec(183.5 , 259.0), Port::INPUT, module, CM3Module::INPUT_SELECT));

		//LCD display pattern
		TxtDisplayWidget *dispat = new TxtDisplayWidget();
		dispat->box.pos = Vec(29.9, 11.0);
		dispat->box.size = Vec(38.0 , 20.4);
		dispat->txt = &module->display_pat;
		addChild(dispat);

		//LCD display length
		TxtDisplayWidget *dislen = new TxtDisplayWidget();
		dislen->box.pos = Vec(322.4 , 11.0);
		dislen->box.size = Vec(38.0 , 20.4);
		dislen->txt = &module->display_len;
		addChild(dislen);

		//selector indicator yellow
		CM3_RecBall *recball = new CM3_RecBall();
		recball->box.size = Vec(32.0, 32.0);
		recball->recball_x = &module->recball_x;
		recball->recball_y = &module->recball_y;
		addChild(recball);

		//eyepatches: indicate the actual output
		float dd = 20.5; //distance from origin
		float rr = 2.5; //radius of circle
		CM3_EyePatch *eyepatch[8] = {
			new CM3_EyePatch(77.4, 117.4 , dd, rr),
			new CM3_EyePatch(55.2, 169.1 , dd, rr),
			new CM3_EyePatch(77.4 , 220.9 , dd, rr),
			new CM3_EyePatch(107.0 , 272.6 , dd, rr),
			new CM3_EyePatch(313.2 , 117.4 , dd, rr),
			new CM3_EyePatch(335.4 , 169.1 , dd, rr),
			new CM3_EyePatch(313.2 , 220.9 , dd, rr),
			new CM3_EyePatch(283.6 , 272.6 , dd, rr)
		};

		for(int i = 0; i < 8; i ++){
				eyepatch[i]->eyepatch_val = &module->eyepatch_val[i];
				addChild(eyepatch[i]);
		}
	};
};


// Specify the Module and ModuleWidget subclass, human-readable
// author name for categorization per plugin, module slug (should never
// change), human-readable module name, and any number of tags
// (found in `include/tags.hpp`) separated by commas.
// Model *modelCM3Module = Model::create<CM3Module, CM3ModuleWidget>("CatroModulo", "CatroModulo_CM-3", "C/M3 : PreSetSeq", SEQUENCER_TAG);

RACK_PLUGIN_MODEL_INIT(CatroModulo, CM3Module) {
   Model *model = Model::create<CM3Module, CM3ModuleWidget>("CatroModulo", "CatroModulo_CM3", "C/M3 : PreSetSeq", SEQUENCER_TAG);
   return model;
}
