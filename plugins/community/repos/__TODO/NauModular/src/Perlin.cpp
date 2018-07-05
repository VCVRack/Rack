#include "NauModular.hpp"
#define DEBUG_PERLIN
#ifdef DEBUG_PERLIN
#include <iostream>
#include <limits.h>
#endif

#define FASTFLOOR(x) ( ((x)>0) ? ((int)x) : (((int)x)-1) )

struct Perlin : Module{
    enum ParamIds {
    	SPEED_PARAM,
    	SPEED_PCT_PARAM,
    	MULT_PARAM,
    	MULT_PCT_PARAM,
    	WGT0_PARAM,
        WGT1_PARAM,
        WGT2_PARAM,
        WGT3_PARAM,
        NUM_PARAMS
    };
    enum InputIds {
    	SPEED_INPUT,
	MULT_INPUT,
    	NUM_INPUTS
    };
    enum OutputIds {
    	NOISE_OUTPUT,
    	NOISE0_OUTPUT,
    	NOISE1_OUTPUT,
    	NOISE2_OUTPUT,
    	NOISE3_OUTPUT,
    	NUM_OUTPUTS
    };
    enum LightIds {
	NUM_LIGHTS
    };

    Perlin() : Module(NUM_PARAMS, NUM_INPUTS, NUM_OUTPUTS, NUM_LIGHTS){
        noise = new float[nOctaves];
    }
    ~Perlin(){
        delete [] noise;
    }

    void step() override;

float grad(int hash, float x);
    float getNoise(float x);
    void mixOctaves(float * nn);
    
    float getMixed(float & _val0, float & _val1, float & _mix);

    OutputIds octaveIndex2outputIndex(int octIdx){return OutputIds(octIdx+1);}
    ParamIds octaveIndex2paramIndex(int octIdx){return ParamIds(octIdx+4);}
    Output * getOctaveOutput(int octIdx);
    Param * getOctaveWeight(int octIdx);
    bool hasWire(InputIds _inIdx);
    
    const int nOctaves = 4;
    float curTime = 0.0;
    float minSpd = 1;
    float maxSpd = 500;
    float oldSpeedVal;
    float oldSpeedPctVal;
    float noiseOutMix;
    float * noise;

    
#ifdef debug_perlin
    int cursmp = 0;
#endif

    unsigned char perm[512] = {151,160,137,91,90,15,
    131,13,201,95,96,53,194,233,7,225,140,36,103,30,69,142,8,99,37,240,21,10,23,
    190, 6,148,247,120,234,75,0,26,197,62,94,252,219,203,117,35,11,32,57,177,33,
    88,237,149,56,87,174,20,125,136,171,168, 68,175,74,165,71,134,139,48,27,166,
    77,146,158,231,83,111,229,122,60,211,133,230,220,105,92,41,55,46,245,40,244,
    102,143,54, 65,25,63,161, 1,216,80,73,209,76,132,187,208, 89,18,169,200,196,
    135,130,116,188,159,86,164,100,109,198,173,186, 3,64,52,217,226,250,124,123,
    5,202,38,147,118,126,255,82,85,212,207,206,59,227,47,16,58,17,182,189,28,42,
    223,183,170,213,119,248,152, 2,44,154,163, 70,221,153,101,155,167, 43,172,9,
    129,22,39,253, 19,98,108,110,79,113,224,232,178,185, 112,104,218,246,97,228,
    251,34,242,193,238,210,144,12,191,179,162,241, 81,51,145,235,249,14,239,107,
    49,192,214, 31,181,199,106,157,184, 84,204,176,115,121,50,45,127, 4,150,254,
    138,236,205,93,222,114,67,29,24,72,243,141,128,195,78,66,215,61,156,180,
    151,160,137,91,90,15,
    131,13,201,95,96,53,194,233,7,225,140,36,103,30,69,142,8,99,37,240,21,10,23,
    190, 6,148,247,120,234,75,0,26,197,62,94,252,219,203,117,35,11,32,57,177,33,
    88,237,149,56,87,174,20,125,136,171,168, 68,175,74,165,71,134,139,48,27,166,
    77,146,158,231,83,111,229,122,60,211,133,230,220,105,92,41,55,46,245,40,244,
    102,143,54, 65,25,63,161, 1,216,80,73,209,76,132,187,208, 89,18,169,200,196,
    135,130,116,188,159,86,164,100,109,198,173,186, 3,64,52,217,226,250,124,123,
    5,202,38,147,118,126,255,82,85,212,207,206,59,227,47,16,58,17,182,189,28,42,
    223,183,170,213,119,248,152, 2,44,154,163, 70,221,153,101,155,167, 43,172,9,
    129,22,39,253, 19,98,108,110,79,113,224,232,178,185, 112,104,218,246,97,228,
    251,34,242,193,238,210,144,12,191,179,162,241, 81,51,145,235,249,14,239,107,
    49,192,214, 31,181,199,106,157,184, 84,204,176,115,121,50,45,127, 4,150,254,
    138,236,205,93,222,114,67,29,24,72,243,141,128,195,78,66,215,61,156,180 
    };
};

Output * Perlin::getOctaveOutput(int octIdx){
    return &outputs[octaveIndex2outputIndex(octIdx)];
}

Param * Perlin::getOctaveWeight(int octIdx){
    return &params[octaveIndex2paramIndex(octIdx)];
}

bool Perlin::hasWire(InputIds _inIdx){
    return (inputs[_inIdx].plugLights[0].getBrightness()>0 ||
            inputs[_inIdx].plugLights[1].getBrightness()>0);
}

float Perlin::grad(int hash, float x){
    int h = hash & 15;
    float grad  =1.0+(h&7);
    if(h&8)grad=-grad;
    return (grad*x);
}

float Perlin::getNoise(float x){
    int i0 = FASTFLOOR(x);
    int i1 = i0+1;
    float x0 = x-i0;
    float x1 = x0-1.0;
    float t1 = 1.0 - x1*x1;
    float n0;
    float n1;
    float t0 = 1.0 - x0*x0;
    t0 *= t0;
    n0 = t0*t0*grad(perm[i0 & 0xff], x0);
    t1 *= t1;
    n1 = t1*t1*grad(perm[i1 & 0xff], x1);
    return (0.25 * (n0+n1));
}

void Perlin::mixOctaves(float * nn){
    float totW = 0;
    noiseOutMix = 0;
    for(int i=0;i<nOctaves;i++){
        float nw = getOctaveWeight(i)->value;
        noiseOutMix += nn[i]*nw;
        totW += nw;
    }
    if(totW==0)totW=1.0;
    noiseOutMix /= totW;
    outputs[NOISE_OUTPUT].value = noiseOutMix;
}

float Perlin::getMixed(float & _val0, float & _val1, float & _mix){
    return ((_val0 * _mix)+(_val1 * (1.0-_mix)));
}

void Perlin::step(){
    float deltaTime = 1.0/engineGetSampleRate();
    curTime += deltaTime;
/*      
    auto now = std::chrono::system_clock::now();
    time_t t = std::chrono::system_clock::to_time_t(now);
    std::chrono::duration<double> s = now - std::chrono::system_clock::from_time_t(t);
    auto ms = s.count();
	auto tm = *std::localtime(&t);
  
    float timef = 60*tm.tm_min;
    timef += tm.tm_sec;
    timef += ms;

    curTime = timef;
*/
    float noiseSpd = params[SPEED_INPUT].value;
    if(inputs[SPEED_INPUT].active){
        float spdIn = inputs[SPEED_INPUT].value/5.0;
        noiseSpd = getMixed(spdIn, noiseSpd, params[SPEED_PCT_PARAM].value);
    }

    float noiseAmp = params[MULT_PARAM].value;
    if(inputs[MULT_INPUT].active){
        float ampIn = inputs[MULT_INPUT].value;
        noiseAmp =  getMixed(ampIn, noiseAmp, params[MULT_PCT_PARAM].value);
    }
    float octMult = 1.0;
    for(int i=0;i<nOctaves;i++){
        noise[i] = noiseAmp * getNoise(curTime * noiseSpd * octMult);
        getOctaveOutput(i)->value = noise[i];
        octMult *= 2;
    }

    mixOctaves(noise);

    //std::cout<<"cippa "<<curTime<<std::endl;

}

struct PerlinWidget : ModuleWidget{
	PerlinWidget(Perlin *module);
};

PerlinWidget::PerlinWidget(Perlin *module) : ModuleWidget(module){
    box.size = Vec(6 * RACK_GRID_WIDTH, RACK_GRID_HEIGHT);

    {
    	SVGPanel *panel = new SVGPanel();
    	panel->box.size = box.size;
    	panel->setBackground(SVG::load(assetPlugin(plugin, "res/Perlin.svg")));
    	addChild(panel);
    }

    

    addChild(Widget::create<ScrewSilver>(Vec(15, 0)));
    addChild(Widget::create<ScrewSilver>(Vec(box.size.x - 30, 0)));
    addChild(Widget::create<ScrewSilver>(Vec(15, 365)));
    addChild(Widget::create<ScrewSilver>(Vec(box.size.x - 30, 365)));
    
    float startSpd = (module->maxSpd-module->minSpd)/2 + module->minSpd;
    addParam(ParamWidget::create<Davies1900hBlackKnob>(Vec(10, 83), module, Perlin::SPEED_PARAM, module->minSpd, module->maxSpd, startSpd));
    addParam(ParamWidget::create<RoundSmallBlackKnob>(Vec(55, 100), module, Perlin::SPEED_PCT_PARAM, 0.0, 1.0, 0.5));

    addParam(ParamWidget::create<Davies1900hBlackKnob>(Vec(10, 150), module, Perlin::MULT_PARAM, 1.0, 10.0, 1.0));
    addParam(ParamWidget::create<RoundSmallBlackKnob>(Vec(55, 167), module, Perlin::MULT_PCT_PARAM, 0.0, 1.0, 0.5));
    addParam(ParamWidget::create<RoundSmallBlackKnob>(Vec(15, 205), module, Perlin::WGT0_PARAM, 0.0,1.0,0.25));
    addParam(ParamWidget::create<RoundSmallBlackKnob>(Vec(50, 205), module, Perlin::WGT1_PARAM, 0.0,1.0,0.25));
    addParam(ParamWidget::create<RoundSmallBlackKnob>(Vec(15, 235), module, Perlin::WGT2_PARAM, 0.0,1.0,0.25));
    addParam(ParamWidget::create<RoundSmallBlackKnob>(Vec(50, 235), module, Perlin::WGT3_PARAM, 0.0,1.0,0.25));

    addInput(Port::create<PJ301MPort>(Vec(55, 75), Port::INPUT, module, Perlin::SPEED_INPUT));
    addInput(Port::create<PJ301MPort>(Vec(55, 140), Port::INPUT, module, Perlin::MULT_INPUT));

    addOutput(Port::create<PJ301MPort>(Vec(17, 270), Port::OUTPUT, module, Perlin::NOISE0_OUTPUT));
    addOutput(Port::create<PJ301MPort>(Vec(45, 270), Port::OUTPUT, module, Perlin::NOISE1_OUTPUT));
    addOutput(Port::create<PJ301MPort>(Vec(17, 295), Port::OUTPUT, module, Perlin::NOISE2_OUTPUT));
    addOutput(Port::create<PJ301MPort>(Vec(45, 295), Port::OUTPUT, module, Perlin::NOISE3_OUTPUT));
    addOutput(Port::create<PJ301MPort>(Vec(33, 320), Port::OUTPUT, module, Perlin::NOISE_OUTPUT));
}

Model *modelPerlin = Model::create<Perlin, PerlinWidget>("NauModular", "Perlin", "Perlin", NOISE_TAG);
