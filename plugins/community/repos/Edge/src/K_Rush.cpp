#include "Edge.hpp"

#include "dsp/resampler.hpp"
#include "dsp/filter.hpp"
#include <iostream>
#include <fstream>
#include <string>



using namespace std;

namespace rack_plugin_Edge {

inline float clip(float x) {
	return tanhf(x);
}

/**Diode -> in  -1V / +1V **/
struct Diode{
    float offset,limit1,limit2,phase_in, phase_out = 0.0f;
    float weight_accum = 0;
    Upsampler<16,16> Upsample;
    Decimator<16,16> Decimate;
    float Ov_Buffer[16] = {0};

    RCFilter filter1;

    string plug_directory = assetPlugin(plugin, "res/waves2/");
	float wave[64][256]={{0}};
    const string wavefiles[64]={"00.wav","01.wav","02.wav","03.wav","04.wav","05.wav","06.wav","07.wav","08.wav","09.wav","10.wav","11.wav","12.wav","13.wav","14.wav","15.wav","16.wav","17.wav","18.wav","19.wav","20.wav","21.wav","22.wav","23.wav","24.wav","25.wav","26.wav","27.wav","28.wav","29.wav","30.wav","31.wav","32.wav","33.wav","34.wav","35.wav","36.wav","37.wav","38.wav","39.wav","40.wav","41.wav","42.wav","43.wav","44.wav","45.wav","46.wav","47.wav","48.wav","49.wav","50.wav","51.wav","52.wav","53.wav","54.wav","55.wav","56.wav","57.wav","58.wav","59.wav","60.wav","61.wav","62.wav","63.wav"};
    FILE * wave_f = NULL;
	short temp_buf[256]={0};
    bool tab_loaded = false;


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
                    wave[j][i] = ((float)temp_buf[i]/pow(256,2)+0.5);

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



    float proc_f_d1(float in, float gain, int type,float feedback){

        if(tab_loaded == false){
            LoadWaves();
        }


       // in *= gain;
        //float weight = 0.0f;


        //weight = abs(d_memory[0] - d_memory[1]);




        //phase_out = phase_in;
       // weight_accum += weight* phase_in;
/*
        if(weight_accum > 1.0f){
            weight_accum = d_memory[0]-weight*-1;

        }
        if(weight_accum < -1.0f){
            weight_accum = d_memory[0]+weight*-1;
        }


*/

        //weight_accum = clamp(weight_accum,-1.0f,1.0f);




        //weight = clamp( weight, -0.1, 0.1);

       //  float out = weight_accum;

       // float out = d_memory[1] + (weight*phase);

/*
        float phase,weight = 0.0f;
        d_memory[1] = d_memory[0];
        d_memory[0] = in;

        if(d_memory[0] > d_memory[1]){
            phase = 1.0f;
            weight = d_memory[0] - d_memory[1];
        }
        else{
                if(d_memory[0] == d_memory[1]){
                    phase = 0.0f;
                    weight = 0.0f;
                }
                else{
                    phase = -1.0f;
                    weight = d_memory[1] - d_memory[0];
                }
        }
*/
/*
        if(weight > 0.008)
            phase = -phase;
*/
        //float temp=in;

        /**-> 0 à +1 **/

        //if(in<0)
          //  in=in*-1;
        //in = -1*pow(in*gain,2)+1;


       /**WAVEFOLD**/
        //if(in >= 1.0f)
        //    in = 1+(1-in);
        //if(in <= -1.0f)
        //    in = -1-(in+1);


        //in = sgn(temp)*in;

        //float out = (d_memory[1]+weight)*phase;


        //out=tanh(gain*out);

        Upsample.process(in,Ov_Buffer);
        filter1.setCutoff((44100 *engineGetSampleTime()/16));
        float index= 0.0f;

        for(int i = 0 ; i< 16 ; i++){

            if(Ov_Buffer[i]>=0)
                phase_in=1.0f;
            if(Ov_Buffer[i]<0)
                phase_in= -1.0f;
            Ov_Buffer[i] = abs(Ov_Buffer[i]*gain);
            index =  clamp(Ov_Buffer[i]-1.0f, 0.0f, 20.0f)*gain;

            while(index>255){
                index-=255;
            }
            while(index<0){
                index+=255;
            }
            Ov_Buffer[i] =  Ov_Buffer[i]-( gain*interpolateLinear(wave[type],index));

            Ov_Buffer[i] = Ov_Buffer[i]*phase_in;

            filter1.process(Ov_Buffer[i]);
            Ov_Buffer[i]=filter1.lowpass();
        }
        in=Decimate.process(Ov_Buffer);
        float out = clamp(in,-1.0f,1.0f);
        return out ;


    }


};



struct K_Rush : Module {
	enum ParamIds {
		FEEDBACK_PARAM,
		GAIN_PARAM,
		BLEND_PARAM,
		CV_GAIN_PARAM,
		CV_FEEDBACK_PARAM,
		NUM_PARAMS
	};
	enum InputIds {
		CV_GAIN_INPUT,
		IN_INPUT,
		CV_FEEDBACK_INPUT,
		FEEDBACK_INPUT,
		NUM_INPUTS
	};
	enum OutputIds {
		OUT_OUTPUT,
		FEEDBACK_OUTPUT,
		NUM_OUTPUTS
	};
	enum LightIds {
		NUM_LIGHTS
	};


    float feed_back = 0.0f;


	/*****************

    const float m_thermalVoltage = 0.025864f;
	const float m_saturationCurrent = 10e-17f;
	float m_resistor = 15000.f;
	float m_loadResistor = 7500.f;
	float m_loadResistor2 = m_loadResistor * 2.f;
	// Derived values
	float m_alpha = m_loadResistor2 / m_resistor;
	float m_beta = (m_resistor + m_loadResistor2) / (m_thermalVoltage * m_resistor);
	float m_delta = (m_loadResistor * m_saturationCurrent) / m_thermalVoltage;

	*******************/

    Diode d_pos,d_neg;

	K_Rush() : Module(NUM_PARAMS, NUM_INPUTS, NUM_OUTPUTS, NUM_LIGHTS) {
	}



	void step() override;

	// For more advanced Module features, read Rack's engine.hpp header file
	// - toJson, fromJson: serialization of internal data
	// - onSampleRateChange: event triggered by a change of sample rate
	// - onReset, onRandomize, onCreate, onDelete: implements special behavior when user clicks these from the context menu
};





void K_Rush::step() {


    float gain = params[GAIN_PARAM].value;


    feed_back = params[FEEDBACK_PARAM].value + (params[CV_FEEDBACK_PARAM].value*inputs[CV_FEEDBACK_INPUT].value);
    feed_back = clamp(feed_back,0.0f,1.0f);
    float in = (inputs[IN_INPUT].value/5.0f)-((inputs[FEEDBACK_INPUT].value/5.0f)/clamp(16/gain,1.0f,16.0f) *feed_back);


    if(inputs[CV_GAIN_INPUT].active)
        gain += (inputs[CV_GAIN_INPUT].value*params[CV_GAIN_PARAM].value);

    gain = clamp(gain,0.0f,16.0f);


    /*****WAvefolder chelou (1 seule rebond en haut / infini en bas

    if(in >= 1.0f)
        in = 1+(1-in);
    if(in <= -1.0f)
        in = -1-(in+1);
    ***/

    /***HARDCLIPPER
    if(in >= 1.0f)
        in+=(1-in);
    if(in <= -1.0f)
        in-=(1+in);
    *****/

    //Upsample.process(in,Ov_Buffer);

    /**** SQUAREWAVE Le signal;
    in = sgn(in)*(in+(1-in));
    ****/

    //d_pos.offset=-2.0f;
    /***   Offset et HardClip    ***/

    //in = in+d_pos.offset+d_neg.offset;




/***Fonction Diode+ Diode- */

    //in=gain*in;


   // in = d_pos.proc_f_d1(in ,gain,1);


    /*

    filter1.setCutoff(40 / (engineGetSampleTime()/16));
    for(int i = 0 ; i< 16 ; i++){
        Ov_Buffer[i] = d_pos.proc_f_d1(Ov_Buffer[i] ,gain,1);
        filter1.process(Ov_Buffer[i]);
        Ov_Buffer[i]=filter1.lowpass();
    }


    // DECIMATION !
     in=Decimate.process(Ov_Buffer);


    //in = Decimate.process(Ov_Buffer);
    //in = clamp(in,-5.0f,5.0f);
*/
    int type_diode = int(params[BLEND_PARAM].value *16);
    in = d_pos.proc_f_d1(in ,gain,type_diode,params[FEEDBACK_PARAM].value);
    outputs[OUT_OUTPUT].value = in*5;
    outputs[FEEDBACK_OUTPUT].value = in*5;
/*
    //float out = in;

    //outputs[OUT_OUTPUT].value = out;


// DIODE

    float result = 0.0f;
    float feedback = params[FEEDBACK_PARAM].value;


    //float in =inputs[IN_INPUT].value/5.0f;
    buff[0]=in;
    for(int i = 905 ; i >= 0 ; i--){
        buff[i+1]=buff[i];
    }

    if(inputs[FEEDBACK_INPUT].active){
        in += feedback*inputs[FEEDBACK_INPUT].value/5;
    }
    else{
        in += feedback*buff[280]/2;
    }
    in*=gain;
    in = tanh(in);
    const float theta = sgn(in);

    Upsample.process(in,Ov_Buffer);

    for (int i = 0; i < 2; i++) {
        Ov_Buffer[i] =  m_alpha*Ov_Buffer[i] - (theta * m_thermalVoltage * LambertW(m_delta * exp(theta * m_beta * Ov_Buffer[i])));
    }
    result = Decimate.process(Ov_Buffer);
    result = m_alpha*in - (theta * m_thermalVoltage * LambertW(m_delta * exp(theta * m_beta * in))) ;
    outputs[OUT_OUTPUT].value = tanh(result)*20;
    outputs[FEEDBACK_OUTPUT].value = tanh(result)*20;

    last_step = tanh(result);

*///////////////////////////////////////


}


struct K_RushWidget : ModuleWidget {
	K_RushWidget(K_Rush *module) : ModuleWidget(module) {
		setPanel(SVG::load(assetPlugin(plugin, "res/K_Rush.svg")));

        addParam(ParamWidget::create<RoundLargeBlackKnob>(Vec(56.4, 83.2), module, K_Rush::BLEND_PARAM, 0.0f, 1.0f, 0.0f));
		addParam(ParamWidget::create<RoundLargeBlackKnob>(Vec(12.2, 158.7), module, K_Rush::GAIN_PARAM, 0.0f, 16.0f, 1.0f));
		addParam(ParamWidget::create<RoundLargeBlackKnob>(Vec(100, 256.7), module, K_Rush::FEEDBACK_PARAM, 0.0f, 1.0f, 0.0f));

		addParam(ParamWidget::create<RoundSmallBlackKnob>(Vec(106.9, 165.8), module, K_Rush::CV_GAIN_PARAM, -1.0f, 1.0f, 0.0f));
		addParam(ParamWidget::create<RoundSmallBlackKnob>(Vec(19.3, 263.8), module, K_Rush::CV_FEEDBACK_PARAM, 0.0f, 0.3f, 0.0f));

		addInput(Port::create<PJ301MPort>(Vec(62.3, 205), Port::INPUT, module, K_Rush::CV_GAIN_INPUT));
        addInput(Port::create<PJ301MPort>(Vec(62.3, 302.6), Port::INPUT, module, K_Rush::CV_FEEDBACK_INPUT));

        addInput(Port::create<PJ301MPort>(Vec(9.3, 345), Port::INPUT, module, K_Rush::FEEDBACK_INPUT));
		addInput(Port::create<PJ301MPort>(Vec(62.3, 345), Port::INPUT, module, K_Rush::IN_INPUT));
		addOutput(Port::create<PJ301MPort>(Vec(115.3, 345), Port::OUTPUT, module, K_Rush::OUT_OUTPUT));


	}
};

} // namespace rack_plugin_Edge

using namespace rack_plugin_Edge;

RACK_PLUGIN_MODEL_INIT(Edge, K_Rush) {
   Model *modelK_Rush = Model::create<K_Rush, K_RushWidget>("Edge", "K_Rush", "K_Rush", OSCILLATOR_TAG);
   return modelK_Rush;
}
