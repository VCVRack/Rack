#include "dsp/digital.hpp"
#include "moDllz.hpp"
/*
 * TwinGlider
 */

namespace rack_plugin_moDllz {

struct TwinGlider : Module {
    enum ParamIds {
        RISE_PARAM,
        FALL_PARAM=2,
        LINK_PARAM=4,
        RISEMODE_PARAM=6,
        FALLMODE_PARAM=8,
        TRIG_PARAM=10,
        SMPNGLIDE_PARAM=12,
        NUM_PARAMS=14
    };
    enum InputIds {
        RISE_INPUT,
        FALL_INPUT=2,
        GATE_INPUT=4,
        CLOCK_INPUT=6,
        IN_INPUT=8,
        NUM_INPUTS=10
    };
    enum OutputIds {
        TRIGRISE_OUTPUT,
        TRIG_OUTPUT=2,
        TRIGFALL_OUTPUT=4,
        GATERISE_OUTPUT=6,
        GATEFALL_OUTPUT=8,
        OUT_OUTPUT=10,
        NUM_OUTPUTS=12
    };
    enum LightIds {
        RISING_LIGHT,
        FALLING_LIGHT=2,
        NUM_LIGHTS=4
    };
 
    struct gliderObj{
        float out = 0.0f;
        float in = 0.0f;
   //     float jump = 0.0f;
        int risemode = 0.0f;
        int fallmode = 0.0f;
        float riseval = 0.0f;
        float fallval = 0.0f;
        float prevriseval = 0.0f;
        float prevfallval = 0.0f;
        float riseramp = 0.0f;
        float fallramp = 0.0f;
        bool rising = false;
        bool falling = false;
        bool newgate = false;
        bool pulse = false;
        bool newin = false;
        bool trigR = false;
        bool trigF = false;
        int clocksafe = 0;
        //   int frameTime = 0;
        PulseGenerator risePulse;
        PulseGenerator fallPulse;
        
    };
    
    gliderObj glider[2];
    
    const float threshold = 0.01f;
    
  //  int testVal = 0;
    
    TwinGlider() : Module(NUM_PARAMS, NUM_INPUTS, NUM_OUTPUTS, NUM_LIGHTS) {}
//    ~TwinGlider() {
//    };
    
    void step() override;
    void onReset() override {
        for (int ix = 0; ix < 2 ; ix++){
       // gliding[ix] = false;
        outputs[OUT_OUTPUT + ix].value = inputs[IN_INPUT + ix].value;
        lights[RISING_LIGHT + ix].value = 0;
        lights[FALLING_LIGHT + ix].value = 0;
        outputs[GATERISE_OUTPUT + ix].value = 0;
        outputs[GATEFALL_OUTPUT + ix].value = 0;
        }
    }
};

///////////////////////////////////////////
///////////////STEP //////////////////
/////////////////////////////////////////////

void TwinGlider::step() {
    for (int ix = 0; ix < 2; ix++){
        if (inputs[IN_INPUT + ix].active) {
            if (std::abs(glider[ix].in - inputs[IN_INPUT + ix].value) > threshold){
                glider[ix].newin = true;
            }
        if (params[SMPNGLIDE_PARAM + ix].value > 0.5f){
            bool allowin = false;
            if (inputs[CLOCK_INPUT + ix].active){
             // External clock
                if ((glider[ix].clocksafe > 8) && (inputs[CLOCK_INPUT + ix].value > 2.5f)){
                    allowin = true;
                    glider[ix].in = inputs[IN_INPUT + ix].value;
                    glider[ix].clocksafe = 0;
                }else if ((glider[ix].clocksafe <10) && (inputs[CLOCK_INPUT + ix].value < 0.01f)) glider[ix].clocksafe ++;
            }  else allowin = ((!glider[ix].rising) && (!glider[ix].falling));
            if (allowin) if (glider[ix].newin) glider[ix].in = inputs[IN_INPUT + ix].value;
        }
        else if (glider[ix].newin) glider[ix].in = inputs[IN_INPUT + ix].value;
     //Check for legato from Gate
    bool glideMe = false;
    if (inputs[GATE_INPUT + ix].active){
        if (inputs[GATE_INPUT + ix ].value < 0.5f) {
            glider[ix].newgate = true;
            glider[ix].out = glider[ix].in;
        }else if (glider[ix].newgate) {
                glider[ix].out = glider[ix].in ;
                glider[ix].newgate = false;
        }else glideMe= true;/// GLIDE !!!!!
    }else glideMe = true;//// GLIDE !!!!

            if (glideMe) {
                //////////////// GLIDE FUNCTION ////////////>>>>>>>>>>>>>>>>>>>>>

                    glider[ix].risemode = static_cast<int> (params[RISEMODE_PARAM + ix].value);
                    glider[ix].fallmode = static_cast<int> (params[FALLMODE_PARAM + ix].value);
                    if (glider[ix].in  > glider[ix].out){
                        if (inputs[RISE_INPUT + ix].active)
                            glider[ix].riseval = inputs[RISE_INPUT + ix].value / 10.0f * params[RISE_PARAM + ix].value;
                        else glider[ix].riseval = params[RISE_PARAM + ix].value;

                        if (glider[ix].riseval > 0.0f) {
                            switch (glider[ix].risemode) {
                                case 0: /// Hi Rate
                                    glider[ix].riseramp = 1.0f/(1.0f + glider[ix].riseval * 0.005f * engineGetSampleRate());
                                    break;
                                case 1: /// Rate
                                    glider[ix].riseramp = 1.0f/(1.0f + glider[ix].riseval * 2.0f * engineGetSampleRate());
                                    break;
                                case 2: /// Time
                                    if ((glider[ix].newin)||(glider[ix].riseval != glider[ix].prevriseval)){
                                        glider[ix].prevriseval = glider[ix].riseval;
                                        glider[ix].newin = false;
                                        glider[ix].riseramp =  (glider[ix].in - glider[ix].out) * engineGetSampleTime() /(glider[ix].riseval * glider[ix].riseval * 10.0f);
                                        if  (glider[ix].riseramp< 1e-6)  glider[ix].riseramp = 1e-6;
                                    }
                                    break;
                                default:
                                    break;
                            }
                            glider[ix].out += glider[ix].riseramp;
                            glider[ix].rising = true;
                            glider[ix].falling = false;
                            if (glider[ix].out >= glider[ix].in) {///////REACH RISE
                                glider[ix].out = glider[ix].in;
                                glider[ix].rising = false;
                                glider[ix].trigR = true;
                            }
                        } else {
                            glider[ix].rising = false;
                            glider[ix].out = glider[ix].in;
                            glider[ix].trigR = true;
                        }

                    } else  if (glider[ix].in  < glider[ix].out){
                        if (params[LINK_PARAM + ix].value > 0.5f) {
                            glider[ix].fallmode  = glider[ix].risemode;
                            glider[ix].fallval  = glider[ix].riseval;
                        }else{
                            if (inputs[FALL_INPUT + ix].active)
                                glider[ix].fallval = inputs[FALL_INPUT + ix].value / 10.0f * params[FALL_PARAM + ix].value;
                            else glider[ix].fallval = params[FALL_PARAM + ix].value;
                        }
                        if (glider[ix].fallval > 0.0f) {
                            switch (glider[ix].fallmode) {
                                case 0:
                                    glider[ix].fallramp = 1.0f/(1.0f + glider[ix].fallval * 0.005f * engineGetSampleRate());
                                    break;
                                case 1:
                                    glider[ix].fallramp = 1.0f/(1.0f + glider[ix].fallval * 2.0f * engineGetSampleRate());
                                    break;
                                case 2:
                                    if ((glider[ix].newin) || (glider[ix].fallval != glider[ix].prevfallval)){
                                        glider[ix].prevfallval = glider[ix].fallval;
                                        glider[ix].newin = false;
                                        glider[ix].fallramp =  (glider[ix].out - glider[ix].in) * engineGetSampleTime() /(glider[ix].fallval * glider[ix].fallval * 10.0f);
                                        if  (glider[ix].fallramp < 1e-6) glider[ix].fallramp= 1e-6;
                                    }
                                    break;
                                default:
                                    break;
                            }
                            glider[ix].out -= glider[ix].fallramp;
                            glider[ix].falling = true;
                            glider[ix].rising = false;
                            if (glider[ix].out <= glider[ix].in){////////REACH FALL
                                glider[ix].out = glider[ix].in;
                                glider[ix].falling = false;
                                glider[ix].trigF = true;
                            }
                        }else{
                            glider[ix].falling = false;
                            glider[ix].out = glider[ix].in;
                            glider[ix].trigF = true;
                        }
                    } else {
                        glider[ix].rising = false;
                        glider[ix].falling = false;
                        glider[ix].out = glider[ix].in;
                    }
            }else{
                glider[ix].rising = false;
                glider[ix].falling = false;
                glider[ix].out = glider[ix].in;
            }

    ///testVal = static_cast<int> (gliding[0] * 10^7);


    lights[RISING_LIGHT + ix].value = glider[ix].rising? 1.0 : 0.0f;
    lights[FALLING_LIGHT + ix].value = glider[ix].falling? 1.0 : 0.0f;
    outputs[GATERISE_OUTPUT + ix].value = glider[ix].rising? 10.0 : 0.0f;
    outputs[GATEFALL_OUTPUT + ix].value = glider[ix].falling? 10.0 : 0.0f;

    outputs[OUT_OUTPUT + ix].value = glider[ix].out;

//            //// do triggers and reset flags
           if (outputs[TRIGRISE_OUTPUT + ix].active||outputs[TRIG_OUTPUT + ix].active||outputs[TRIGFALL_OUTPUT + ix].active) {
               if (glider[ix].trigR)  {
                   glider[ix].risePulse.trigger(1e-3f);
                   glider[ix].trigR = false;
                }
               if (glider[ix].trigF)  {
                   glider[ix].fallPulse.trigger(1e-3f);
                   glider[ix].trigF = false;
               }
//
            bool pulseR = glider[ix].risePulse.process(1.0f /  engineGetSampleRate());
            outputs[TRIGRISE_OUTPUT + ix].value = pulseR ? 10.0 : 0.0;

            bool pulseF = glider[ix].fallPulse.process(1.0f /  engineGetSampleRate());
            outputs[TRIGFALL_OUTPUT + ix].value = pulseF ? 10.0 : 0.0;

            outputs[TRIG_OUTPUT + ix].value = (pulseR || pulseF) ? 10.0 : 0.0;
//
            }
           ///else {
//               trigR[ix] = false;
//               trigF[ix] = false;
//           }


     /// else from if input ACTIVE....
 }else{
     //disconnected in...reset Output if connected...
    // outputs[OUT_OUTPUT + ix].value = 0;
     outputs[GATERISE_OUTPUT + ix].value = 0.0f;
     outputs[GATEFALL_OUTPUT + ix].value = 0.0f;
     lights[RISING_LIGHT + ix].value = 0.0f;
     lights[FALLING_LIGHT + ix].value = 0.0f;
     glider[ix].out = 0.0f;
     glider[ix].in = 0.0f;
 //    gliding[ix] = false;
     glider[ix].newgate = false;
 //    allowin[ix] = true;
 }/// Closing if input  ACTIVE

} //for loop ix
  
}//closing STEP

struct TwinGliderWidget : ModuleWidget {
    
    TwinGliderWidget(TwinGlider *module): ModuleWidget(module){
        setPanel(SVG::load(assetPlugin(plugin, "res/TwinGlider.svg")));

    //Screws
    addChild(Widget::create<ScrewBlack>(Vec(0, 0)));
    addChild(Widget::create<ScrewBlack>(Vec(box.size.x - 15, 0)));
    addChild(Widget::create<ScrewBlack>(Vec(0, 365)));
    addChild(Widget::create<ScrewBlack>(Vec(box.size.x - 15, 365)));
    
    float Ystep = 30.0f ;
    float Ypos = 0.0f;
    
    for (int i=0; i<2; i++){
        
        Ypos = 25.0f + static_cast<float>(i* 173);  //2nd panel offset

   // Gliding Leds
    addChild(ModuleLightWidget::create<TinyLight<RedLight>>(Vec(6.75, Ypos+1), module, TwinGlider::RISING_LIGHT + i));
    addChild(ModuleLightWidget::create<TinyLight<RedLight>>(Vec(154.75, Ypos+1), module, TwinGlider::FALLING_LIGHT + i));
    /// Glide Knobs
    addParam(ParamWidget::create<moDllzKnobM>(Vec(19, Ypos-4), module, TwinGlider::RISE_PARAM + i, 0.0, 1.0, 0.0));
    addParam(ParamWidget::create<moDllzKnobM>(Vec(102, Ypos-4), module, TwinGlider::FALL_PARAM + i, 0.0, 1.0, 0.0));

    Ypos += Ystep; //55

    // LINK SWITCH//CKSS
    addParam(ParamWidget::create<moDllzSwitchLedH>(Vec(73, Ypos-12), module, TwinGlider::LINK_PARAM + i, 0.0, 1.0, 1.0));
    /// Glides CVs
    addInput(Port::create<moDllzPort>(Vec(23, Ypos+5.5), Port::INPUT, module, TwinGlider::RISE_INPUT + i));
    addInput(Port::create<moDllzPort>(Vec(117.5, Ypos+5.5), Port::INPUT, module, TwinGlider::FALL_INPUT + i));

    Ypos += Ystep; //85

    /// Mode switches
    addParam(ParamWidget::create<moDllzSwitchT>(Vec(55, Ypos-7), module, TwinGlider::RISEMODE_PARAM + i, 0.0, 2.0, 2.0));
    addParam(ParamWidget::create<moDllzSwitchT>(Vec(100, Ypos-7), module, TwinGlider::FALLMODE_PARAM + i, 0.0, 2.0, 2.0));

    /// GATES OUT
    addOutput(Port::create<moDllzPort>(Vec(10.5, Ypos+14), Port::OUTPUT, module, TwinGlider::GATERISE_OUTPUT + i));
    addOutput(Port::create<moDllzPort>(Vec(130.5, Ypos+14), Port::OUTPUT, module, TwinGlider::GATEFALL_OUTPUT + i));

    Ypos += Ystep; //115

    /// TRIGGERS OUT
    addOutput(Port::create<moDllzPort>(Vec(43, Ypos-4.5), Port::OUTPUT, module, TwinGlider::TRIGRISE_OUTPUT + i));
    addOutput(Port::create<moDllzPort>(Vec(71, Ypos-4.5), Port::OUTPUT, module, TwinGlider::TRIG_OUTPUT + i));
    addOutput(Port::create<moDllzPort>(Vec(98, Ypos-4.5), Port::OUTPUT, module, TwinGlider::TRIGFALL_OUTPUT + i));

    Ypos += Ystep; //145

    // GATE IN
    addInput(Port::create<moDllzPort>(Vec(44, Ypos+7), Port::INPUT, module, TwinGlider::GATE_INPUT + i));
    // CLOCK IN
    addInput(Port::create<moDllzPort>(Vec(75, Ypos+7), Port::INPUT, module, TwinGlider::CLOCK_INPUT + i));
    // Sample&Glide SWITCH
    addParam(ParamWidget::create<moDllzSwitchLed>(Vec(108, Ypos+19), module, TwinGlider::SMPNGLIDE_PARAM + i, 0.0, 1.0, 0.0));
    // IN OUT
    addInput(Port::create<moDllzPort>(Vec(13.5, Ypos+6.5), Port::INPUT, module, TwinGlider::IN_INPUT + i));
    addOutput(Port::create<moDllzPort>(Vec(128.5, Ypos+6.5), Port::OUTPUT, module, TwinGlider::OUT_OUTPUT + i));


    }
//    {
//        testDisplay *mDisplay = new testDisplay();
//        mDisplay->box.pos = Vec(0.0f, 360.0f);
//        mDisplay->box.size = {165.0f, 20.0f};
//        mDisplay->valP = &(module->testVal);
//        addChild(mDisplay);
//    }
    }
};

// Specify the Module and ModuleWidget subclass, human-readable
// manufacturer name for categorization, module slug (should never
// change), human-readable module name, and any number of tags
// (found in `include/tags.hpp`) separated by commas.

} // namespace rack_plugin_moDllz

using namespace rack_plugin_moDllz;

RACK_PLUGIN_MODEL_INIT(moDllz, TwinGlider) {
   Model *modelTwinGlider = Model::create<TwinGlider, TwinGliderWidget>("moDllz", "TwinGlider", "TwinGlider Dual Portamento", SLEW_LIMITER_TAG,  DUAL_TAG, ENVELOPE_FOLLOWER_TAG);
   return modelTwinGlider;
}
