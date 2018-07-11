#include "unless.hpp"
#include "widgets.hpp"
#include "dsp/digital.hpp"
#include <algorithm>

namespace rack_plugin_unless_modules {

#define RELEASED -1
#define UP 0
#define DOWN 1
#define PRESSED 2

struct Markov : Module {
  struct Edge {
    int note = 0;
    int count = 1;
    Edge(int n, int c){
      note = n;
      count = c;
    }
  };
  struct Node {
    int note = 0;
    int count = 1;
    int last = -1;
    std::vector<Edge> edges;
    Node(int n, int c){
      note = n;
      count = c;
    }
    int findEdge(int n){
      for(int i = 0; i<(int)edges.size(); i++){ 
        if(edges.at(i).note == n)
          return i;
      }
      return -1;
    }
    static bool sortByCount(const Edge &a, const Edge &b){
      return a.count > b.count;
    }
    void addEdge(int n){
      int i = findEdge(n);
      if(i < 0 || i >= (int)edges.size())
        edges.push_back(Edge(n, 1));
      else
        edges.at(i).count++;
      std::sort(edges.begin(), edges.end(), sortByCount);
    }
    // void reverseWeights(){
    // }
    int getNext(float randomness){
      float all = 0;
      float mult = randomness * -1.0f;
      if(randomness > 0)
        mult *= 0.9f;
      if(randomness < -0.9f && edges.size() > 0)
        return edges.at(0).note;
      for(auto& e : edges)
        all+= (float)e.count + ((float)e.count * mult);
      float r = randomUniform() * all;
      float acc = 0;
      for(int i = 0; i<(int)edges.size(); i++){
        acc += (float)edges.at(i).count + ((float)edges.at(i).count * mult);
        if(r <= acc)
          return edges.at(i).note;
      }

      return -1;
    }
  };

  struct MarkovChain{
    std::vector<Node> nodes;
    bool hasNodes = false;
    int current = -1;

    void clear(){
      nodes.clear();
      current = -1;
      hasNodes = false;
    }
    int findNode(int n){
      for(int i = 0; i<(int) nodes.size();i++){
        if(nodes.at(i).note == n)
          return i;
      }
      return -1;
    }
    void addNode(int n){
      hasNodes = true;
      nodes.push_back(Node(n, 1));
    }
    void setNote(int n){
      if(hasNodes){  
        int closest = 1000;
        for(int i = 0; i<(int) nodes.size();i++){
          int nt = nodes.at(i).note;
          if(nt == n){
            closest = nt;
            break;
          }else if(abs(nt - n) < abs(closest - n)){
            closest = nt;
          }
        }
        closest = findNode(closest);
        if(closest < (int)nodes.size() && closest >= 0)
          current = closest;
        // printf("%d\n", closest);
      }
    }
    void forget(){
      if(nodes.size() > 0){
        int n = nodes.at(current).note;
        nodes.erase(nodes.begin() + current);
        for (auto& node : nodes){
          if(node.edges.size() > 0){ 
            for(int i = (int)node.edges.size() - 1; i>= 0; i--)
              if(node.edges.at(i).note == n){
                node.count-= node.edges.at(i).count;
                node.edges.erase(node.edges.begin() + i);
              }
          }
        }
        if(nodes.size() == 0){
          hasNodes = false;
          current = -1;
        }else{
          setNote(current);
        }
      }
    }
    void addEdge(int n){
      int i = findNode(n);
      if(i == -1)
        addNode(n);
      if(current > -1)
        nodes.at(current).addEdge(n);
      current = i < 0 ? ((int) nodes.size()) - 1 : i;
    }
    int randomNode(){
      return clamp((int)floor(randomUniform() * (float) nodes.size()), 0, ((int) nodes.size()) - 1);
    }
    int step(float randomness){
      // printf("%f\n", randomness);
      if(current < 0 && hasNodes){
        current = 0;
        return current;
      }else if(current >= 0 && current < (int) nodes.size()){
        // DEAD END
        if(nodes.at(current).edges.size() == 0){
          if(randomness < -0.5f){
            // ORDER
            return current;
          }else{
            // RANDOM
            if(randomUniform() * 1.5f < randomness + 0.5f)
              return randomNode();
            else
              return current;
          }
        }else{
          int next = nodes.at(current).getNext(randomness);
          if(randomness > 0.5f){
            float r = (randomness - 0.5f) * 2.0f;
            if(randomUniform() < r){
              next = -1;
            }
          }
          current = next < 0 ? (randomness > -0.9f ? randomNode() : current) : findNode(next);
        }
      }
      return current;
    }
    int getCurrentNote(){
      if(current >= 0)
        return nodes.at(current).note;
      else
        return -1;
    }
  };
  enum ParamIds {
    LEARN_PARAM,
    FORGET_PARAM,
    RANDOMNESS_PARAM,
    NUM_PARAMS
  };
  enum InputIds {
    LEARN_INPUT,
    FORGET_INPUT,
    RANDOMNESS_INPUT,
    CV_INPUT,
    GATE_INPUT,
    TRIGGER_INPUT,
    NUM_INPUTS
  };
  enum OutputIds {
    CV_OUTPUT,
    GATE_OUTPUT,
    NUM_OUTPUTS
  };
  enum LightIds {
    LEARN_LIGHT,
    // DEADEND_LIGHT,
    NUM_LIGHTS
  };

  bool blackkeys[12] = {false, true, false, true, false, false, true, false, true, false, true, false};

  TriggerSwitch learn_trigger;
  TriggerSwitch forget_trigger;
  
  TriggerSwitch gate_trigger;
  TriggerSwitch trigger_trigger;

  bool learning = false;
  MarkovChain chain = MarkovChain();

  Markov() : Module(NUM_PARAMS, NUM_INPUTS, NUM_OUTPUTS, NUM_LIGHTS) {
    // onReset();
  }
  void onDelete() override{
    chain.clear();
  }
  void onRandomize() override{

  }
  void onReset() override{
    chain.clear();
  }
  int cvToMidi(float v){
    return round(v * 12.0f + 60.0f);
  }
  float midiToCV(int m){
    return ((float) m - 60) / 12.0f;
  }
  float cvToHertz(float cv){
    return 261.626f * powf(2.0f, cv);
  }
  void bypass(){
    outputs[CV_OUTPUT].value = inputs[CV_INPUT].value;
    outputs[GATE_OUTPUT].value = inputs[GATE_INPUT].value;
  }
  void updateTriggers(){
    learn_trigger.update(params[LEARN_PARAM].value + inputs[LEARN_INPUT].value);
    forget_trigger.update(params[FORGET_PARAM].value + inputs[FORGET_INPUT].value); 
    trigger_trigger.update(inputs[TRIGGER_INPUT].value);
    gate_trigger.update(inputs[GATE_INPUT].value);
  }

  void step() override {
    updateTriggers();
    
    int inputMidi = cvToMidi(inputs[CV_INPUT].value);
    
    if(forget_trigger.state == PRESSED)
      chain.forget();
    
    if(learn_trigger.state == PRESSED)
      learning = !learning;
    
    if(learning){
      if(gate_trigger.state > 0 && chain.getCurrentNote() != inputMidi){
        bypass();
        if(gate_trigger.state == PRESSED)
          outputs[GATE_OUTPUT].value = 0.0f;
        chain.addEdge(inputMidi);
      }
    }

    if(trigger_trigger.state == PRESSED){
      chain.step(
        inputs[RANDOMNESS_INPUT].active 
        ? (clamp(inputs[RANDOMNESS_INPUT].value, 0.0f, 10.0f) * 0.1f * 2.0f - 1.0f)
        : params[RANDOMNESS_PARAM].value 
      );
      outputs[GATE_OUTPUT].value = 0.0f;
    }else if(trigger_trigger.state == RELEASED || trigger_trigger.state == UP){
      outputs[GATE_OUTPUT].value = 0.0f;
    }else{
      outputs[GATE_OUTPUT].value = 10.0f;
    }

    if(gate_trigger.state == PRESSED && !learning)
      chain.setNote(cvToMidi(inputs[CV_INPUT].value));

    outputs[CV_OUTPUT].value = midiToCV(chain.getCurrentNote());
    
    lights[LEARN_LIGHT].value = learning ? 10.0f : 0.0f;
  }

  json_t *toJson() override {
    json_t *rootJ = json_object();
    json_t *nodesJ = json_array();
    int i = 0;
    for (auto& node : chain.nodes) {
      json_t *nodeJ = json_object();
      json_t *edges = json_array();
      int j = 0;
      for(auto& edge : node.edges){
        json_t *edgeJ = json_object();
        json_object_set_new(edgeJ, "note", json_integer(edge.note));
        json_object_set_new(edgeJ, "count", json_integer(edge.count));
        json_array_insert_new(edges, j, edgeJ);
        j++;
      }
      json_object_set_new(nodeJ, "edges", edges);
      json_object_set_new(nodeJ, "note", json_integer(node.note));
      json_object_set_new(nodeJ, "count", json_integer(node.count));
      json_array_insert_new(nodesJ, i, nodeJ);
      i++;
    }
    json_object_set_new(rootJ, "nodes", nodesJ);
    json_object_set_new(rootJ, "current", json_integer(chain.current));
    json_object_set_new(rootJ, "learning", json_boolean(learning));
    return rootJ;
  }

  void fromJson(json_t *rootJ) override {
    json_t *nodesJ = json_object_get(rootJ, "nodes");
    for (int i = 0; i>=0; i++){
      if(json_array_get(nodesJ, i) != NULL){
        json_t *node = json_array_get(nodesJ, i);
        json_t *edges = json_object_get(node, "edges");
        Node n = Node(json_integer_value(json_object_get(node, "note")), json_integer_value(json_object_get(node, "count")));
        for (int j = 0; j>=0; j++) {
          if(json_array_get(edges, j) != NULL){
            std::vector<Edge> es;
            json_t *edge = json_array_get(edges, j);
            n.edges.push_back(Edge(json_integer_value(json_object_get(edge, "note")), json_integer_value(json_object_get(edge, "count"))));
          }else{
            break;
          }
        }
        chain.nodes.push_back(n);
      }else{
        break;
      }
    }
    chain.hasNodes = (int)chain.nodes.size() > 0;
    chain.current = json_integer_value(json_object_get(rootJ, "current"));
    learning = json_boolean_value(json_object_get(rootJ, "learning"));
  }
};

struct MarkovDisplay : Widget {
  Markov *module;
  NVGcolor black = nvgRGBA(0x22, 0x22, 0x22, 0xee);
  NVGcolor white = nvgRGBA(0xf9, 0xfa, 0xea, 0xff);
  NVGcolor grey = nvgRGBA(0xee, 0xee, 0xee, 0x88);
  float hue = 0.0f;
  NVGcolor background = nvgRGB(142,140,136);
  std::shared_ptr<Font> font;
  
  MarkovDisplay(int x, int y) {
    box.pos = Vec(x, y);
    box.size = Vec(96,96);
    font = Font::load(assetPlugin(plugin, "res/font/Terminus.ttf"));
  }
  int cvToMidi(float v){
    return round((clamp(v + 5.0f, 0.0f, 10.58f) / 10.58f) * 127.0f);
  }

  void draw(NVGcontext *vg) override {
    // BACKGROUND
    nvgBeginPath(vg);
    nvgRect(vg, 0, 0, box.size.x, box.size.y);
    nvgFillColor(vg, background);
    nvgFill(vg);

    int w = 8;
    if(module->chain.hasNodes){
      //NODES
      for(auto& node : module->chain.nodes){
        int n = node.note;
        nvgBeginPath(vg);
        nvgRect(vg, (n % 12) * w, (n / 12) * w, w, w);
        nvgFillColor(vg, module->blackkeys[n % 12] ? black : white);
        nvgFill(vg);
      }

      int current = module->chain.current;
      if(current >= 0 && current < (int) module->chain.nodes.size()){
        int n = module->chain.nodes.at(current).note;
        int nx = (n % 12) * w + 4;
        int ny = (n / 12) * w + 4;
        int es = (int) module->chain.nodes.at(current).edges.size();
        Vec pos1 = Vec(nx, ny);
        for(int j = es - 1; j>=0; j--){
          float weight = 1.0f - ((float)j / (float)es);
          int next = module->chain.nodes.at(current).edges.at(j).note;
          // HIGHLIGHT
          nvgBeginPath(vg);
          nvgRect(vg, (next % 12) * w, (next / 12) * w, w, w);
          nvgFillColor(vg,nvgHSLA(hue + (1.0f - weight) * 0.3f, 0.5f, 0.5f, 50 + weight * 130) );
          nvgFill(vg);

          // EDGE
          // slightly modified version of the wire drawing code from Rack/src/app/WireWidget.cpp
          Vec pos2 = Vec((next % 12) * w + 4, (next / 12) * w + 4);
          float dist = pos1.minus(pos2).norm();
          Vec slump;
          slump.y = (1.0 - RACK_PLUGIN_UI_TOOLBAR->wireTensionSlider->value) * 0.4 * (150.0 + 1.0*dist);
          Vec pos3 = pos1.plus(pos2).div(2).plus(slump);
          nvgLineJoin(vg, NVG_ROUND);
          Vec pos4 = pos3.plus(slump.mult(0.08));
          nvgBeginPath(vg);
          nvgMoveTo(vg, pos1.x, pos1.y);
          nvgQuadTo(vg, pos4.x, pos4.y, pos2.x, pos2.y);
          nvgStrokeColor(vg, nvgHSLA(hue + (1.0f - weight) * 0.3f, 0.5f, 0.5f, weight * 255));
          nvgStrokeWidth(vg, 2);
          nvgStroke(vg);
        }
        //CURRENT
        nvgBeginPath(vg);
        nvgRect(vg, (n % 12) * w, (n / 12) * w, w, w);
        nvgFillColor(vg, nvgHSL(hue, 0.7f, 0.5f));
        nvgFill(vg);
      }

    }
  }
};


// struct MarkovSettingItem : MenuItem {
//   uint8_t *setting = NULL;
//   uint8_t offValue = 0;
//   uint8_t onValue = 1;
//   void onAction(EventAction &e) override {
//     // Toggle setting
//     *setting = (*setting == onValue) ? offValue : onValue;
//   }
//   void step() override {
//     rightText = (*setting == onValue) ? "✔" : "";
//     MenuItem::step();
//   }
// };

struct MarkovWidget : ModuleWidget {
  MarkovWidget(Markov *module) : ModuleWidget(module) {
    setPanel(SVG::load(assetPlugin(plugin, "res/panels/Markov.svg")));
    // SCREWS
    addChild(Widget::create<PeaceScrew>(Vec(RACK_GRID_WIDTH, RACK_GRID_HEIGHT - RACK_GRID_WIDTH)));
    addChild(Widget::create<PeaceScrew>(Vec(box.size.x - 2 * RACK_GRID_WIDTH, RACK_GRID_HEIGHT - RACK_GRID_WIDTH)));

    //HEAD
    int x = (int)(box.size.x * 0.5f);
    int y = 210;
    int spacex = 0;
    int spacey = 27;
    addParam(ParamWidget::create<MrChainkov>(Vec(x - 24, y), module, Markov::RANDOMNESS_PARAM, -1.0f, 1.0f, 0.0f));
    addInput(Port::create<InputPort>(Vec(x - 12, y - 30), Port::INPUT, module, Markov::RANDOMNESS_INPUT));
    // addChild(ModuleLightWidget::create<TinyLight<RedLight>>(Vec(x - 1.5f, y-8), module, Markov::DEADEND_LIGHT));
    
    // LEARN / FORGET
    x = 13;
    y = 18;
    spacex = (int) ((box.size.x - 16.0f) / 3.0f);
    spacey = 28;
    addChild(ModuleLightWidget::create<LargeLight<RedLight>>(Vec(x + 4.f, y + 4.f), module, Markov::LEARN_LIGHT));
    addParam(ParamWidget::create<RecordButton>(Vec(x, y), module, Markov::LEARN_PARAM, 0.0f, 1.0f, 0.0f));
    addParam(ParamWidget::create<StopButton>(Vec(x, y + spacey), module, Markov::FORGET_PARAM, 0.0f, 1.0f, 0.0f));
    x+=spacex * 2;
    addInput(Port::create<InputGatePort>(Vec(x, y), Port::INPUT, module, Markov::LEARN_INPUT));
    addInput(Port::create<InputGatePort>(Vec(x, y + spacey), Port::INPUT, module, Markov::FORGET_INPUT));

    // PORTS
    x = 13;
    y = 285;
    spacey = 42;
    addInput(Port::create<VOctInputPort>(Vec(x, y), Port::INPUT, module, Markov::CV_INPUT));
    addOutput(Port::create<VOctOutputPort>(Vec(x, y + spacey), Port::OUTPUT, module, Markov::CV_OUTPUT));
    addInput(Port::create<InputGatePort>(Vec(x + spacex + 1, y - 4), Port::INPUT, module, Markov::TRIGGER_INPUT));
    addInput(Port::create<InputGatePort>(Vec(x + 2 * spacex, y), Port::INPUT, module, Markov::GATE_INPUT));
    addOutput(Port::create<OutputGatePort>(Vec(x + 2 * spacex, y + spacey), Port::OUTPUT, module, Markov::GATE_OUTPUT));
  
    {
      MarkovDisplay *display = new MarkovDisplay(12,76);
      display->module = module;
      addChild(display);
    }
  }
  // void appendContextMenu(Menu *menu) override {
  //   Markov *markov = dynamic_cast<Markov*>(module);
  //   assert(markov);
  //   // menu->addChild(construct<MenuLabel>());
  //   // menu->addChild(construct<MenuLabel>(&MenuLabel::text, "alpha version !"));
  //   // menu->addChild(construct<MarkovSettingItem>(&MenuItem::text, "keep loop length", &MarkovSettingItem::setting, &markov->keepLoopLength));
  // }
};

} // namespace rack_plugin_unless_modules

using namespace rack_plugin_unless_modules;

RACK_PLUGIN_MODEL_INIT(unless_modules, Markov) {
   Model *modelMarkov = Model::create<Markov, MarkovWidget>("unless games", "markov", "Mr.Chainkov : markov chain sequencer", RANDOM_TAG, MIDI_TAG, RECORDING_TAG);
   return modelMarkov;
}

/*
  BUG

  TODO
  
  reverse weights option
  
  OPTIMIZE

  calculate all edge counts beforehand

*/
