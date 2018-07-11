#include "unless.hpp"
#include "widgets.hpp"
#include "dsp/digital.hpp"
#include <math.h> 

namespace rack_plugin_unless_modules {

struct Piong : Module {
  enum ParamIds {
    P1_PARAM,
    P2_PARAM,
    S1_PARAM,
    S2_PARAM,
    SPEED_PARAM,
    AUTOBALL_PARAM,
    NUM_PARAMS
  };
  enum InputIds {
    P1_INPUT,
    P2_INPUT,
    S1_INPUT,
    S2_INPUT,
    ANGLE_INPUT,
    Y_INPUT,
    BALL_INPUT,
    CLK_INPUT,
    SPEED_INPUT,
    NUM_INPUTS
  };
  enum OutputIds {
    HIT1_OUTPUT,
    HIT2_OUTPUT,
    LEFT_OUTPUT,
    RIGHT_OUTPUT,
    WALLT_OUTPUT,
    WALLB_OUTPUT,
    X_OUTPUT,
    Y_OUTPUT,
    CENTER_OUTPUT,
    VCCENTER_OUTPUT,
    VCHIT1_OUTPUT,
    VCHIT2_OUTPUT,
    VCLEFT_OUTPUT,
    VCRIGHT_OUTPUT,
    VCWALLT_OUTPUT,
    VCWALLB_OUTPUT,
    NUM_OUTPUTS
  };
  enum LightIds {
    NUM_LIGHTS
  };

  SchmittTrigger gateTrigger;

  Vec dir = Vec(0.0f, 0.0f);

  Vec pos = Vec(0.5f,0.5f);
  float p1 = 0.0f;
  float p2 = 0.0f;
  float s1 = 0.0f;
  float s2 = 0.0f;
  float p1last = 0.0f;
  float p2last = 0.0f;
  float speed = 0.0f;
  float PI = 3.141592f;
  int score1 = 0;
  int score2 = 0;
  float aspectRatio = 180.f / 135.0f;
  bool crossedCenter = false;
  bool served = false;
  int lastHit = 0;

  bool ball = false;
  bool updated = false;
  float ballWidth = 0.02f;
  float playerWidth = 0.04f;

  Piong() : Module(NUM_PARAMS, NUM_INPUTS, NUM_OUTPUTS, NUM_LIGHTS) {
    onReset();
  }
  // void onReset() override {
  //   pos.x = 0.5f;
  //   pos.y = 0.5f;
  //   ball = false;

  // }
  int vcToDir(float v){
    return (int) floor(v * 0.1f * 8);
  }
  void normalize(Vec v){
    float mag = sqrt(v.x * v.x + v.y * v.y);
    v.x /= mag;
    v.y /= mag;
  }
  void spawnBall(){
    pos.x = 0.5f;
    pos.y = inputs[Y_INPUT].active
      ? inputs[Y_INPUT].value * 0.1f
      : randomUniform();
    float angle = inputs[ANGLE_INPUT].active
      ? inputs[ANGLE_INPUT].value * 0.1f * 2.0f * PI
      : (2 * PI * (0.25f + (randomUniform() > 0.5f ?  0.30f : 0.0f)) + PI * randomUniform() * 0.20f) + (randomUniform() > 0.5f ? PI : 0.0f);

    dir.x = sin(angle);
    dir.y = cos(angle);

    ball = true;
    crossedCenter = false;
    served = false;
  }
  void ballOut(int side, int vcside){
    outputs[side].value = 10.0f;
    ball = false;
    outputs[vcside].value = 10.0f * clamp(pos.y, 0.0f, 1.0f);
    if(params[AUTOBALL_PARAM].value == 1) 
      spawnBall();
  }
  void hitPlayer(int vcout, int hitout, float p, float s, float v, float x){
    outputs[vcout].value = 10.0f * ((pos.y - p) / s);
    outputs[hitout].value = 10.0f;
    pos.x = x; 
    dir.x *= -1.0f;
    if(v != 0.0f){
      dir.y *= (v > 0.0f) == (dir.y > 0.0f) ? 1.0f : -1.0f;
      dir.y += v * 1000.0f;
    }
    served = true;
    crossedCenter = false;
  }
  void hitWall(int wall, int vcwall, float y){
    outputs[vcwall].value = 10.0f * pos.x;
    outputs[wall].value = 10.0f;
    pos.y = y; 
    dir.y *= -1.0f;
  }
  void step() override {
    if(gateTrigger.process(inputs[BALL_INPUT].value / 2.f) || (!ball && params[AUTOBALL_PARAM].value == 1))
      spawnBall();

    speed = inputs[SPEED_INPUT].active ? inputs[SPEED_INPUT].value * 0.1f : 0.01f;

    s1 =  0.04f + 0.96f * (inputs[S1_INPUT].active ? (inputs[S1_INPUT].value / 10.f) : params[S1_PARAM].value);
    s2 =  0.04f + 0.96f * (inputs[S2_INPUT].active ? (inputs[S2_INPUT].value / 10.f) : params[S2_PARAM].value);

    p1 = (1.0f - s1) * (inputs[P1_INPUT].active ? inputs[P1_INPUT].value / 10.f : params[P1_PARAM].value);
    p2 = (1.0f - s2) * (inputs[P2_INPUT].active ? inputs[P2_INPUT].value / 10.f : params[P2_PARAM].value);

    float vel1 = p1 - p1last;
    float vel2 = p2 - p2last;

    p1last = p1;
    p2last = p2;


    bool update = false;
    if(inputs[CLK_INPUT].value > 0.0f){
      if(!updated){
        update = true;
        updated = true;
      }
    }else
      updated = false;

    outputs[HIT1_OUTPUT].value = 0.0f;
    outputs[HIT2_OUTPUT].value = 0.0f;
    outputs[WALLT_OUTPUT].value = 0.0f;
    outputs[WALLB_OUTPUT].value = 0.0f;
    outputs[CENTER_OUTPUT].value = 0.0f;
    outputs[LEFT_OUTPUT].value = 0.0f;
    outputs[RIGHT_OUTPUT].value = 0.0f;
    
    if(update && ball){
      pos.x += dir.x * speed;
      pos.y += dir.y * speed * aspectRatio;

      // players, sides
      if(dir.x < 0.0f){

        if(pos.x - ballWidth * 0.5 < playerWidth){ 
          if(pos.y > p1 && p1 + s1 > pos.y)
            hitPlayer(VCHIT1_OUTPUT, HIT1_OUTPUT, p1, s1, vel1, playerWidth + (ballWidth * 0.5));
          else if(pos.x <= 0.0f)
            ballOut(LEFT_OUTPUT, VCLEFT_OUTPUT);
        }
      }
      else{
        if(pos.x + ballWidth * 0.5f >= 1.0f - playerWidth){ 
          if(pos.y > p2 && p2 + s2 > pos.y)
            hitPlayer(VCHIT2_OUTPUT, HIT2_OUTPUT, p2, s2, vel2, 1.0f - playerWidth - (ballWidth * 0.5));
          else if(pos.x >= 1.0f)
            ballOut(RIGHT_OUTPUT, VCRIGHT_OUTPUT);
        }
      }
      // top & bottom walls
      if(dir.y < 0.0f){
        if(pos.y - ballWidth * 0.5f < 0.0f)
          hitWall(WALLT_OUTPUT, VCWALLT_OUTPUT, ballWidth * 0.5f);
      }
      else{
        if(pos.y + ballWidth * 0.5f > 1.0f)
          hitWall(WALLB_OUTPUT, VCWALLB_OUTPUT, 1.0f - ballWidth * 0.5f);
      }

      // center
      if(served && !crossedCenter){
        crossedCenter = (dir.x > 0.0f && pos.x > 0.5f) || (dir.x < 0.0f && pos.x < 0.5f);
        if(crossedCenter){
          outputs[CENTER_OUTPUT].value = 10.0f;
          outputs[VCCENTER_OUTPUT].value = pos.y * 10.0f;
          served = false;
        }
      }

      outputs[X_OUTPUT].value = pos.x * 10.0f;
      outputs[Y_OUTPUT].value = pos.y * 10.0f;    
    }

  }
};

struct PiongDisplay : SVGWidget {
  Piong *module;
  NVGcolor foreground = nvgRGB(0xe6, 0xd9, 0xcc);
  NVGcolor background = nvgRGB(0x36, 0x0a, 0x0a);
  // std::shared_ptr<Font> font;

  PiongDisplay(int x, int y) {
    setSVG(SVG::load(assetPlugin(plugin, "res/PiongDisplay.svg")));
    box.pos = Vec(x, y);
    box.size = Vec(180,135);
  }

  void draw(NVGcontext *vg) override {

    // nvgBeginPath(vg);
    // nvgRoundedRect(vg, 0, 0, 180, 135, 4.0);
    // nvgFillColor(vg, background);
    // nvgFill(vg);

    // BALL
    if(module->ball){
      nvgBeginPath(vg);
      nvgRoundedRect(vg, module->pos.x * 180 - module->ballWidth * 180.0f * 0.5f, module->pos.y * 135 - module->ballWidth * 180.0f * 0.5f, module->ballWidth * 180.0f, module->ballWidth * 180.0f, 0.0);
      nvgFillColor(vg,foreground);
      nvgFill(vg);
    }

    //PLAYERS
    nvgBeginPath(vg);
    nvgRoundedRect(vg, 0, module->p1 * 135, module->playerWidth * 180.0f, module->s1 * 135, 0.0);
    nvgFillColor(vg,foreground);
    nvgFill(vg);

    nvgBeginPath(vg);
    nvgRoundedRect(vg, 174, module->p2 * 135, module->playerWidth * 180.0f, module->s2 * 135, 0.0);
    nvgFillColor(vg,foreground);
    nvgFill(vg);
  }
};


struct PiongWidget : ModuleWidget {
  PiongWidget(Piong *module) : ModuleWidget(module) {
    setPanel(SVG::load(assetPlugin(plugin, "res/panels/Piong.svg")));
    
    addParam(ParamWidget::create<AutoOrGate>(Vec(170, 18), module, Piong::AUTOBALL_PARAM, 0, 1, 1));
    
    int x = 18;
    int y = 58;
    int space = 30;
    y = 23;
    addInput(Port::create<SmallBrightPort>(Vec(62, y), Port::INPUT, module, Piong::CLK_INPUT));
    addInput(Port::create<SmallBrightPort>(Vec(124, y), Port::INPUT, module, Piong::SPEED_INPUT));
    y += 15;
    addInput(Port::create<SmallBrightPort>(Vec(35, y), Port::INPUT, module, Piong::S1_INPUT));
    addInput(Port::create<SmallBrightPort>(Vec(149, y), Port::INPUT, module, Piong::S2_INPUT));

    y = 56;
    addInput(Port::create<SmallBrightPort>(Vec(10, y), Port::INPUT, module, Piong::P1_INPUT));
    addInput(Port::create<SmallBrightPort>(Vec(63,y), Port::INPUT, module, Piong::ANGLE_INPUT));
    addInput(Port::create<SmallBrightPort>(Vec(93,y), Port::INPUT, module, Piong::BALL_INPUT));
    addInput(Port::create<SmallBrightPort>(Vec(123,y), Port::INPUT, module, Piong::Y_INPUT));
    addInput(Port::create<SmallBrightPort>(Vec(175, y), Port::INPUT, module, Piong::P2_INPUT));

    y = 228;
    addParam(ParamWidget::create<PiongKnob>(Vec(26, y), module, Piong::P1_PARAM, 1.0f, 0.0f, 0.5f));
    addParam(ParamWidget::create<PiongKnobSmall>(Vec(71, y), module, Piong::S1_PARAM, 0.0f, 1.0f, 0.15f));
    addParam(ParamWidget::create<PiongKnobSmall>(Vec(120, y), module, Piong::S2_PARAM, 0.0f, 1.0f, 0.15f));
    addParam(ParamWidget::create<PiongKnob>(Vec(155, y), module, Piong::P2_PARAM, 1.0f, 0.0f, 0.5f));
    

    x = 40;
    y = 271;
    addOutput(Port::create<SmallDarkPort>(Vec(x,y), Port::OUTPUT, module, Piong::X_OUTPUT));
    x += 105;
    addOutput(Port::create<SmallDarkPort>(Vec(x,y), Port::OUTPUT, module, Piong::Y_OUTPUT));

    addOutput(Port::create<DarkHole>(Vec(80, y + 4), Port::OUTPUT, module, Piong::CENTER_OUTPUT));
    addOutput(Port::create<SmallDarkPort>(Vec(110, y), Port::OUTPUT, module, Piong::VCCENTER_OUTPUT));
    
    y = 338;
    x = 23;
    addOutput(Port::create<DarkHole>(Vec(x, y), Port::OUTPUT, module, Piong::HIT1_OUTPUT));
    x += space;
    addOutput(Port::create<DarkHole>(Vec(x, y), Port::OUTPUT, module, Piong::LEFT_OUTPUT));
    x += space;
    addOutput(Port::create<DarkHole>(Vec(x, y), Port::OUTPUT, module, Piong::WALLT_OUTPUT));
    x += space;
    addOutput(Port::create<DarkHole>(Vec(x, y), Port::OUTPUT, module, Piong::WALLB_OUTPUT));
    x += space;
    addOutput(Port::create<DarkHole>(Vec(x, y), Port::OUTPUT, module, Piong::RIGHT_OUTPUT));
    x += space;
    addOutput(Port::create<DarkHole>(Vec(x, y), Port::OUTPUT, module, Piong::HIT2_OUTPUT));

    y = 304;
    x = 18;
    addOutput(Port::create<SmallDarkPort>(Vec(x, y), Port::OUTPUT, module, Piong::VCHIT1_OUTPUT));
    x += space;
    addOutput(Port::create<SmallDarkPort>(Vec(x, y), Port::OUTPUT, module, Piong::VCLEFT_OUTPUT));
    x += space;
    addOutput(Port::create<SmallDarkPort>(Vec(x, y), Port::OUTPUT, module, Piong::VCWALLT_OUTPUT));
    x += space;
    addOutput(Port::create<SmallDarkPort>(Vec(x, y), Port::OUTPUT, module, Piong::VCWALLB_OUTPUT));
    x += space;
    addOutput(Port::create<SmallDarkPort>(Vec(x, y), Port::OUTPUT, module, Piong::VCRIGHT_OUTPUT));
    x += space;
    addOutput(Port::create<SmallDarkPort>(Vec(x, y), Port::OUTPUT, module, Piong::VCHIT2_OUTPUT));

    {
      PiongDisplay *display = new PiongDisplay(15,90);
      display->module = module;
      addChild(display);
    }

    addChild(Widget::create<ScrewSilver>(Vec(30, 0)));
    addChild(Widget::create<ScrewSilver>(Vec(box.size.x-45, 0)));
    addChild(Widget::create<ScrewSilver>(Vec(30, 365)));
    addChild(Widget::create<ScrewSilver>(Vec(box.size.x-45, 365)));
  }
};

} // namespace rack_plugin_unless_modules

using namespace rack_plugin_unless_modules;

RACK_PLUGIN_MODEL_INIT(unless_modules, Piong) {
   Model *modelPiong = Model::create<Piong, PiongWidget>("unless games", "piong", "piong", SEQUENCER_TAG, RANDOM_TAG, CLOCK_MODULATOR_TAG);
   return modelPiong;
}
