
#include "Edge.hpp"
#include "dsp/resampler.hpp"
#include "dsp/filter.hpp"
#include "Edge_Component.hpp"
#include <iostream>
#include <fstream>
#include <string>

namespace rack_plugin_Edge {

using namespace std;


template <int OVERSAMPLE, int QUALITY>
struct VoltageControlledOscillator {

	float lastSyncValue = 0.0f;
	float phase = 0.0f;
	float freq;
	float pw = 0.6f;
	float pitch;
	//
	float width;
	float widthCv;
	float _dual;
	float al_window = 0.5f;
	float ar_window = 0.5f;
	float bl_window = 0.5f;
	float br_window = 0.5f;
	//
	bool syncEnabled = false;
	bool syncDirection = false;
	bool invert = false;
    bool tab_loaded = false;
	//FILE * temp_file_out = NULL;
    FILE * wave_f = NULL;
	short temp_buf[256]={0};
    float mid_phase = 0.0f;
	float buf_wavefront[256]={0};
	float buf_waverear[256]={0};
	float buf_final[257]={0};


    string plug_directory = assetPlugin(plugin, "res/waves/");
	float wave[64][256]={{0}};
    const string wavefiles[64]={"00.wav","01.wav","02.wav","03.wav","04.wav","05.wav","06.wav","07.wav","08.wav","09.wav","10.wav","11.wav","12.wav","13.wav","14.wav","15.wav","16.wav","17.wav","18.wav","19.wav","20.wav","21.wav","22.wav","23.wav","24.wav","25.wav","26.wav","27.wav","28.wav","29.wav","30.wav","31.wav","32.wav","33.wav","34.wav","35.wav","36.wav","37.wav","38.wav","39.wav","40.wav","41.wav","42.wav","43.wav","44.wav","45.wav","46.wav","47.wav","48.wav","49.wav","50.wav","51.wav","52.wav","53.wav","54.wav","55.wav","56.wav","57.wav","58.wav","59.wav","60.wav","61.wav","62.wav","63.wav"};
	Decimator<OVERSAMPLE, QUALITY> sinDecimator;

	RCFilter sqrFilter;

	// For analog detuning effect
	float pitchSlew = 0.0f;
	int pitchSlewIndex = 0;

	float sinBuffer[OVERSAMPLE] = {};



    void LoadWaves(){

        for(int j=0; j<64; j++){
            float chkcnt = 0;
            string file_name = plug_directory+wavefiles[j];
            const char *c = file_name.c_str();
            wave_f = fopen(c,"r");
            if(wave_f!=NULL){
                fseek(wave_f,44,SEEK_SET);
                fread(temp_buf,sizeof(temp_buf),256,wave_f);
                for(int i = 0; i<256 ; i++){
                    if(temp_buf[i]!=0.0f){
                        chkcnt++;
                    }
                    wave[j][i] = ((float)temp_buf[i])/pow(256,2);
                }
                if(chkcnt == 0){
                tab_loaded = false;
                j=64;
                }
                fclose(wave_f);
            }
            else{
                tab_loaded = false;
            }
            if(j==63){
                tab_loaded=true;
            }
        }
         //_fcloseall();

        }



// Il va falloir clamp PITCH !

	void setPitch(float pitchKnob, float pitchCv, float _lfo_param) {
		// Compute frequency
		pitch = pitchKnob;
		/*if (analog) {
			// Apply pitch slew
			const float pitchSlewAmount = 3.0f;
			pitch += pitchSlew * pitchSlewAmount;
		}
		else {
        */
			// Quantize coarse knob if digital mode
			pitch = roundf(pitch);
		//}
		pitch += pitchCv;
		// Note C4
		if(_lfo_param == 1){
          freq = 261.626f * powf(2.0f, pitch / 12.0f);
          freq = clamp(freq,1.0f,10000.0f);
		}
		else{
            pitch = fminf(pitch, 10.0f);
            freq = powf(2.0f, pitch);
		}

	}

	void setInvert(float _invert){
        if(_invert<0.5f){
            invert = false;
        }
        else{
            invert = true;
        }

	}
	////////////////////////////////////////////////////////////////////////////////////////////////////////
	void setWaves(float _wavefront,float _waverear){

	    _wavefront*=63;
	    _waverear*=63;
	    if(_wavefront>63.0f){_wavefront=63.0f;}
	    if(_waverear>63.0f){_waverear=63.0f;}
	    if(_wavefront<=0.0f){_wavefront=0.00f;}
	    if(_waverear<=0.0f){_waverear=0.00f;}
	    float coef1f = _wavefront-int(_wavefront);
	    float coef2f = 1 - coef1f;
        float coef1r = _waverear-int(_waverear);
	    float coef2r = 1 - coef1r;
        for(int i = 0; i<=255; i++){
            if(_wavefront<63){
                buf_wavefront[i] = coef2f*wave[int(_wavefront)][i]+coef1f*wave[int(_wavefront)+1][i];
            }
            else{
                buf_wavefront[i] = wave[int(_wavefront)][i];
            }
            if(_waverear<63){
                buf_waverear[i] = coef2r*wave[int(_waverear)][i]+coef1r*wave[int(_waverear)+1][i];
            }
            else{
                buf_waverear[i] = wave[int(_waverear)][i];
            }
        }
	}


	void setPulseWidth(float pulseWidth) {
		const float pwMin = 0.01f;
		pw = clamp(pulseWidth, pwMin, 1.0f - pwMin);
	}

	//
	void setWidth(float widthKnob,float widthCv, float dual) {
        width = widthKnob;
        width += (widthCv/10);
        if(width>1){width=1.0f;}
        if(width<0){width=0;}
        _dual = 1-dual;

        if(_dual < 0.5f){
            al_window = 0.5f - (width/2);
            ar_window = 0.5f + (width/2);
        }
        else{
            al_window = 0.25f - (width/4);
            ar_window = 0.25f + (width/4);
            bl_window = 0.75f - (width/4);
            br_window = 0.75f + (width/4);
        }
  	}


	void process(float deltaTime, float syncValue, float mode) {

		float deltaPhaseOver = clamp(freq * deltaTime, 1e-6, 0.5f)*(1.0f / OVERSAMPLE);

		// Detect sync
		int syncIndex = -1; // Index in the oversample loop where sync occurs [0, OVERSAMPLE)
		float syncCrossing = 0.0f; // Offset that sync occurs [0.0f, 1.0f)
		if (syncEnabled) {
			syncValue -= 0.01f;
			if (syncValue > 0.0f && lastSyncValue <= 0.0f) {
				float deltaSync = syncValue - lastSyncValue;
				syncCrossing = 1.0f - syncValue / deltaSync;
				syncCrossing *= OVERSAMPLE;
				syncIndex = (int)syncCrossing;
				syncCrossing -= syncIndex;
			}
			lastSyncValue = syncValue;
		}
		//sqrFilter.setCutoff(40.0f * deltaTime);
		//float i_phase = phase*255;

        //for(int i= 0; i<=255; i++){
        int i = int(phase*255);
            if(invert){
               if(_dual < 0.5f){
                    if( (phase > al_window && phase < ar_window) || (al_window == 0.0f) ) {
                        if(i==255){
                            buf_final[i] = (buf_waverear[0]+buf_waverear[255])/2;
                        }
                        else{
                            if(phase==al_window || phase==ar_window){
                                buf_final[i] = (buf_waverear[255-i]+buf_wavefront[i])/2;
                            }
                            else{
                                buf_final[i] = buf_waverear[255-i];
                            }
                        }
                    }
                    else{
                        if(i==255){
                            buf_final[i] = (buf_wavefront[0]+buf_wavefront[255])/2;
                        }
                        else{
                            buf_final[i] = buf_wavefront[i];
                        }
                    }
               }
               else{
                    if( (phase > al_window && phase < ar_window) || (phase > bl_window && phase < br_window) || ( al_window == 0.0f && ar_window == 0.5f)) {
                        if(i==255){
                            buf_final[i] = (buf_waverear[0] + buf_waverear[255])/2;
                        }
                        else{
                            if(phase==al_window || phase==ar_window){
                                buf_final[i] = (buf_waverear[255-i]+buf_wavefront[i])/2;
                            }
                            else{
                                buf_final[i] = buf_waverear[255-i];
                            }
                        }
                    }
                   else{
                        if(i==255){
                            buf_final[i] = (buf_wavefront[0]+buf_wavefront[255])/2;
                        }
                        else{
                            buf_final[i] = buf_wavefront[i];
                        }
                   }
               }
            }
            else{
                if(_dual < 0.5f){
                    if((phase > al_window && phase < ar_window) || (al_window == 0.0f && ar_window == 1.0f)) {
                        if(i==255){
                            buf_final[i] = (buf_waverear[0]+buf_waverear[255])/2;
                        }
                        else{
                            if(phase==al_window || phase==ar_window){
                                buf_final[i] = (buf_waverear[i]+buf_wavefront[i])/2;
                            }
                            else{
                                buf_final[i] = buf_waverear[i];
                            }
                        }
                    }
                    else{
                        if(i==255){
                            buf_final[i] = (buf_wavefront[0]+buf_wavefront[255])/2;
                        }
                        else{
                            buf_final[i] = buf_wavefront[i];
                        }
                    }
                }
                else{
                    if( (phase > al_window && phase < ar_window) || (phase > bl_window && phase < br_window) || ( al_window == 0.0f && ar_window == 1.0f)) {
                        if(i==255){
                            buf_final[i] = (buf_waverear[0]+buf_waverear[255])/2;
                        }
                        else{
                            if(phase==al_window || phase==ar_window){
                                buf_final[i] = (buf_waverear[i]+buf_wavefront[i])/2;
                            }
                            else{
                                buf_final[i] = buf_waverear[i];
                            }
                        }
                    }
                   else{
                        if(i==255){
                            buf_final[i] = (buf_wavefront[0]+buf_wavefront[255])/2;
                        }
                        else{
                            buf_final[i] = buf_wavefront[i];
                        }
                   }
                }
            }
        //}

        sqrFilter.setCutoff(22050.0f * deltaTime);
        mid_phase=phase+0.03f;
        while (mid_phase > 1.0f) {
            mid_phase -= 1.0f;
        }
        while (mid_phase < 0) {
            mid_phase += 1.0f;
        }


        for (int i = 0; i < OVERSAMPLE; i++) {
            if (syncIndex == i) {
					phase = 0.0f;
			}
			// Advance phase
            sinBuffer[i]=1.66f * interpolateLinear(buf_final, mid_phase*255.0f) ;
            sqrFilter.process(sinBuffer[i]);
            sinBuffer[i]=sqrFilter.lowpass();
            phase += deltaPhaseOver;
            mid_phase += deltaPhaseOver;
			while (phase > 1.0f) {
                phase -= 1.0f;
            }
            while (phase < 0) {
                phase += 1.0f;
            }



		}

	}

	float sin() {
		return sinDecimator.process(sinBuffer);
	}

	float light() {
		return sinf(2*M_PI * phase);
	}
};


struct WTFDoveVCO : Module {
	enum ParamIds {
		MODE_PARAM,
		INVERT_PARAM,
		LFO_NOISE_PARAM,
		FRONT_PARAM,
        WIDTH_PARAM,
		REAR_PARAM,
		CV_FRONT_PARAM,
        CV_WIDTH_PARAM,
		CV_REAR_PARAM,
		FREQ_PARAM,
		FINE_PARAM,
		FM_PARAM,
		NUM_PARAMS
	};
	enum InputIds {
	    FM_INPUT,
		PITCH_INPUT,
		FRONT_INPUT,
		WIDTH_INPUT,
		REAR_INPUT,
		NUM_INPUTS
	};
	enum OutputIds {
		OUTPUT,
		NUM_OUTPUTS
	};
	enum LightIds {
		//PHASE_POS_LIGHT,
	//	PHASE_NEG_LIGHT,
		NUM_LIGHTS
	};

	VoltageControlledOscillator<4, 16> oscillator;
	FILE * wave_f = NULL;
	float  BUFFER[256]={0};
	float l_FRONT_PARAM=1.0f;
	float l_WIDTH_PARAM;
	float l_REAR_PARAM;
	float l_CV_FRONT_PARAM;
	float l_CV_REAR_PARAM;
	float l_CV_WIDTH_PARAM;
	float l_INVERT_PARAM;
	float l_FRONT_INPUT;
	float l_REAR_INPUT;
	float l_WIDTH_INPUT;
	float l_MODE_PARAM;
	float l_FM_INPUT;

	WTFDoveVCO() : Module(NUM_PARAMS, NUM_INPUTS, NUM_OUTPUTS, NUM_LIGHTS) {
	srand(time(0));
	}
	void step() override;
};


void WTFDoveVCO::step() {



    if(oscillator.tab_loaded == false){
        oscillator.LoadWaves();
    }
	float pitchFine = 3.0f * quadraticBipolar(params[FINE_PARAM].value);
	float pitchCv = 12.0f * inputs[PITCH_INPUT].value;
	if ((inputs[FM_INPUT].active || inputs[FM_INPUT].value != l_FM_INPUT )&& params[LFO_NOISE_PARAM].value != 0 ) {
		pitchCv += quadraticBipolar(params[FM_PARAM].value) * 12.0f * inputs[FM_INPUT].value;
	}
	oscillator.setPitch(params[FREQ_PARAM].value, pitchFine + pitchCv,params[LFO_NOISE_PARAM].value);
	oscillator.setPulseWidth(0.5f);//oscillator.setPulseWidth(params[PW_PARAM].value + params[WIDTH_PARAM].value * inputs[PW_INPUT].value / 10.0f);
	oscillator.syncEnabled = !params[LFO_NOISE_PARAM].value;
    oscillator.setInvert(params[INVERT_PARAM].value);


    if( (params[WIDTH_PARAM].value != l_WIDTH_PARAM) || (inputs[WIDTH_INPUT].value != l_WIDTH_INPUT) || (params[CV_WIDTH_PARAM].value != l_WIDTH_PARAM) || (params[MODE_PARAM].value != l_MODE_PARAM) ){
        oscillator.setWidth( params[WIDTH_PARAM].value,(inputs[WIDTH_INPUT].value*params[CV_WIDTH_PARAM].value),params[MODE_PARAM].value) ;
    }

    if( ( params[FRONT_PARAM].value != l_FRONT_PARAM ) || (params[CV_FRONT_PARAM].value != l_CV_FRONT_PARAM ) || (inputs[FRONT_INPUT].value != l_FRONT_INPUT) || ( params[REAR_PARAM].value != l_REAR_PARAM ) || (params[CV_REAR_PARAM].value != l_CV_REAR_PARAM ) || (inputs[REAR_INPUT].value != l_REAR_INPUT) || oscillator.tab_loaded == false ){
        oscillator.setWaves( params[FRONT_PARAM].value+(params[CV_FRONT_PARAM].value*(inputs[FRONT_INPUT].value/10)),params[REAR_PARAM].value+(params[CV_REAR_PARAM].value*(inputs[REAR_INPUT].value/10)));
	}
	if(params[LFO_NOISE_PARAM].value == 0 ){
        oscillator.process(engineGetSampleTime(), inputs[FM_INPUT].value,1);
	}
	else{
        oscillator.process(engineGetSampleTime(),0.0f,0);
	}

	// Set output
	if (outputs[OUTPUT].active)
		outputs[OUTPUT].value = clamp ((5.0f * oscillator.sin()),-5.0f,5.0f);


    l_FRONT_PARAM = params[FRONT_PARAM].value;
	l_WIDTH_PARAM = params[WIDTH_PARAM].value;
	l_REAR_PARAM = params[REAR_PARAM].value;
	l_CV_FRONT_PARAM = params[CV_FRONT_PARAM].value;
	l_CV_REAR_PARAM = params[CV_REAR_PARAM].value;
	l_CV_WIDTH_PARAM = params[CV_WIDTH_PARAM].value;
	l_INVERT_PARAM = params[INVERT_PARAM].value;
	l_FRONT_INPUT = inputs[FRONT_INPUT].value;
	l_REAR_INPUT = inputs[REAR_INPUT].value;
	l_WIDTH_INPUT = inputs[WIDTH_INPUT].value;
	l_MODE_PARAM = params[MODE_PARAM].value;
	l_FM_INPUT = inputs[FM_INPUT].value;


}

struct OscDisplay : TransparentWidget {
	WTFDoveVCO *module;
	std::shared_ptr<Font> font;
	OscDisplay() {
		//font = Font::load(assetPlugin(plugin, "res/DejaVuSansMono.ttf"));
	}

void draw(NVGcontext *vg) override {



    nvgSave(vg);
	nvgBeginPath(vg);

    nvgRect(vg, 0,0, 64,56);
    nvgFillColor(vg, nvgRGBA(17,17,17,255));
    nvgFill(vg);
    nvgClosePath(vg);
    nvgRestore(vg);

    nvgSave(vg);




    nvgBeginPath(vg);
    nvgStrokeColor(vg, nvgRGBA(180,50,50,255));
    nvgStrokeWidth(vg, 1.5f);
    for(int i=0;i<=64;i++){
        int index = i*4;
        int x = i;
        float y;
        if(i==64)
            y = this->module->oscillator.buf_wavefront[255];
        else
            y = this->module->oscillator.buf_wavefront[index];
        y=y*54;
        if(i==0){
            nvgMoveTo(vg, 0, 28-y);
        }
        else{
            nvgLineTo(vg, x, 28-y);
        }
    }
    nvgStroke(vg);
    nvgClosePath(vg);

    nvgBeginPath(vg);
    nvgStrokeColor(vg, nvgRGBA(120,120,255,255));
    nvgStrokeWidth(vg, 1.5f);
    for(int i=0;i<=64;i++){
        int index = i*4;
        int x = i;
        float y;
        if(this->module->l_INVERT_PARAM != 0){
            if(i==64)
                y = this->module->oscillator.buf_waverear[255];
            else
                y = this->module->oscillator.buf_waverear[255-index];
        }
        else{
            if(i==64)
                y = this->module->oscillator.buf_waverear[255];
            else
                y = this->module->oscillator.buf_waverear[index];
        }
        y=y*54;
        if(i==0){
            nvgMoveTo(vg, 0, 28-y);
        }
        else{
            nvgLineTo(vg, x, 28-y);
        }
    }
    nvgStroke(vg);
    nvgClosePath(vg);

    nvgBeginPath(vg);
    nvgStrokeColor(vg, nvgRGBA(200,200,200,255));
    nvgStrokeWidth(vg, 1.2f);
    //nvgMoveTo(vg, 0, 28);
    int al_window = this->module->oscillator.al_window*256;
    int ar_window = this->module->oscillator.ar_window*256;
    int bl_window = this->module->oscillator.bl_window*256;
    int br_window = this->module->oscillator.br_window*256;

    for(int i=0;i<=64;i++){
        int x = i;
        float y;
        int index = i*4;
//INVERT
        if(this->module->oscillator.invert){
    //SIMPLE
               if(this->module->oscillator._dual < 0.5f){
                    if( (i > al_window/4 && i < ar_window/4) || ( al_window/4 == 0.0f && ar_window/4 == 64.0f)) {
                        if(i==64){
                            y = this->module->oscillator.buf_waverear[255];
                        }
                        else{
                            y = this->module->oscillator.buf_waverear[255-index];
                        }
                    }
                    else{
                         if(i==64)
                            y = this->module->oscillator.buf_wavefront[255];
                        else
                            y = this->module->oscillator.buf_wavefront[index];
                    }
               }
    //DUAL
               else{
                    if( (i > al_window/4 && i < ar_window/4) || (i > bl_window/4 && i < br_window/4) || ( al_window/4 == 0.0f && ar_window/4 == 32.0f)) {
                        if(i==64){
                            y = this->module->oscillator.buf_waverear[255];
                        }
                        else{
                            y = this->module->oscillator.buf_waverear[255-index];
                        }
                    }
                   else{
                        if(i==64)
                            y = this->module->oscillator.buf_wavefront[255];
                        else
                            y = this->module->oscillator.buf_wavefront[index];
                   }
               }
            }
//NORMAL
            else{
                if(this->module->oscillator._dual < 0.5f){
                    if( (i > al_window/4 && i < ar_window/4) || ( al_window/4 == 0.0f && ar_window/4 == 64.0f) ) {
                            if(i==64)
                                y = this->module->oscillator.buf_waverear[255];
                            else
                                y = this->module->oscillator.buf_waverear[index];
                    }
                    else{
                        if(i==64)
                                y = this->module->oscillator.buf_wavefront[255];
                        else
                                y = this->module->oscillator.buf_wavefront[index];
                    }
                }
                else{
                    if( (i >= al_window/4 && i < ar_window/4) || (i > bl_window/4 && i <= br_window/4) || ( al_window/4 == 0.0f && ar_window/4 == 32.0f)) {
                        if(i==64)
                            y = this->module->oscillator.buf_waverear[255];
                        else
                            y = this->module->oscillator.buf_waverear[index];
                    }
                   else{
                        if(i==64)
                            y = this->module->oscillator.buf_wavefront[255];
                        else
                            y = this->module->oscillator.buf_wavefront[index];
                   }
                }
            }
        y=y*54;
        if(i==0){
            nvgMoveTo(vg, 0, 28-y);
        }
        else{
            nvgLineTo(vg, x, 28-y);
        }
    }
nvgLineCap(vg, NVG_ROUND);
nvgLineJoin(vg, NVG_ROUND);
    nvgStroke(vg);
    nvgClosePath(vg);


    nvgRestore(vg);


}
};


struct WTFDoveVCOWidget : ModuleWidget {
	WTFDoveVCOWidget(WTFDoveVCO *module);
};

WTFDoveVCOWidget::WTFDoveVCOWidget(WTFDoveVCO *module) : ModuleWidget(module) {
	setPanel(SVG::load(assetPlugin(plugin, "res/WTFDoveVCO.svg")));
	addChild(Widget::create<ScrewSilver>(Vec(15, 0)));
	addChild(Widget::create<ScrewSilver>(Vec(box.size.x-30, 0)));
	addChild(Widget::create<ScrewSilver>(Vec(15, 365)));
	addChild(Widget::create<ScrewSilver>(Vec(box.size.x-30, 365)));

        OscDisplay *display = new OscDisplay();
		display->module = module;
		display->box.pos = Vec(43.0f, 32.0f);
		display->box.size = Vec(110.0f, 68.0f);
		addChild(display);


	addParam(ParamWidget::create<CKSS>(Vec(15, 48), module, WTFDoveVCO::MODE_PARAM, 0.0f, 1.0f, 1.0f));
	addParam(ParamWidget::create<CKSS>(Vec(122, 48), module, WTFDoveVCO::INVERT_PARAM, 0.0f, 1.0f, 1.0f));
    addParam(ParamWidget::create<CKSS>(Vec(68.6, 330), module, WTFDoveVCO::LFO_NOISE_PARAM, 0.0f, 1.0f, 1.0f));


    addParam(ParamWidget::create<EdgeRedKnob>(Vec(14.8, 211.8), module, WTFDoveVCO::FRONT_PARAM, 0.0f, 1.0f, 0.0f));
	addParam(ParamWidget::create<RoundLargeBlackKnob>(Vec(56.5, 199.5), module, WTFDoveVCO::WIDTH_PARAM, 0.0f, 1.0f, 0.0f));
	addParam(ParamWidget::create<EdgeBlueKnob>(Vec(108, 211.8), module, WTFDoveVCO::REAR_PARAM, 0.0f, 1.0f, 0.0f));
	addParam(ParamWidget::create<RoundSmallBlackKnob>(Vec(17.7, 255), module, WTFDoveVCO::CV_FRONT_PARAM, 0.0f, 1.0f, 0.0f));
	addParam(ParamWidget::create<RoundSmallBlackKnob>(Vec(63.5, 248.5), module, WTFDoveVCO::CV_WIDTH_PARAM, 0.0f, 1.0f, 0.0f));
	addParam(ParamWidget::create<RoundSmallBlackKnob>(Vec(110.6, 254.8), module, WTFDoveVCO::CV_REAR_PARAM, 0.0f, 1.0f, 0.0f));
    addParam(ParamWidget::create<RoundBlackKnob>(Vec(37.5, 101), module, WTFDoveVCO::FREQ_PARAM, -54.0f, 54.0f, 0.0f));
    addParam(ParamWidget::create<RoundBlackKnob>(Vec(84.5, 101), module, WTFDoveVCO::FINE_PARAM, -1.0f, 1.0f, 0.0f));
	addParam(ParamWidget::create<RoundSmallBlackKnob>(Vec(87.5, 151.5), module, WTFDoveVCO::FM_PARAM, 0.0f, 1.0f, 0.0f));


	addInput(Port::create<PJ301MPort>(Vec(39.5, 151.5), Port::INPUT, module, WTFDoveVCO::FM_INPUT));
	addOutput(Port::create<PJ301MPort>(Vec(110.5, 328), Port::OUTPUT, module, WTFDoveVCO::OUTPUT));
	addInput(Port::create<PJ301MPort>(Vec(17.5, 328), Port::INPUT, module, WTFDoveVCO::PITCH_INPUT));
    addInput(Port::create<PJ301MPort>(Vec(17.5, 300), Port::INPUT, module, WTFDoveVCO::FRONT_INPUT));
	addInput(Port::create<PJ301MPort>(Vec(63, 300), Port::INPUT, module, WTFDoveVCO::WIDTH_INPUT));
	addInput(Port::create<PJ301MPort>(Vec(110.5, 300), Port::INPUT, module, WTFDoveVCO::REAR_INPUT));


}

} // namespace rack_plugin_Edge

using namespace rack_plugin_Edge;

RACK_PLUGIN_MODEL_INIT(Edge, WTFDoveVCO) {
   Model *modelWTFDoveVCO = Model::create<WTFDoveVCO, WTFDoveVCOWidget>("Edge", "WTFDoveVCO", "WTFDoveVCO", OSCILLATOR_TAG);
   return modelWTFDoveVCO;
}
