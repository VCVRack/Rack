struct OnOffSwitch : SVGSwitch, ToggleSwitch {
  OnOffSwitch() {
    addFrame(SVG::load(assetPlugin(plugin, "res/buttons/OffSwitch.svg")));
    addFrame(SVG::load(assetPlugin(plugin, "res/buttons/OnSwitch.svg")));
  }
};
struct LoopSwitch : SVGSwitch, ToggleSwitch {
  LoopSwitch() {
    addFrame(SVG::load(assetPlugin(plugin, "res/buttons/LoopOffButton.svg")));
    addFrame(SVG::load(assetPlugin(plugin, "res/buttons/LoopOnButton.svg")));
  }
};
struct CvGateSwitch : SVGSwitch, ToggleSwitch {
  CvGateSwitch() {
    addFrame(SVG::load(assetPlugin(plugin, "res/buttons/CvButton.svg")));
    addFrame(SVG::load(assetPlugin(plugin, "res/buttons/GateButton.svg")));
  }
};

struct AddButton : SVGSwitch, MomentarySwitch {
  AddButton() {
    addFrame(SVG::load(assetPlugin(plugin, "res/buttons/AddButton.svg")));
    addFrame(SVG::load(assetPlugin(plugin, "res/buttons/AddButtonDown.svg")));
  }
};
struct PlayButton : SVGSwitch, MomentarySwitch {
  PlayButton() {
    addFrame(SVG::load(assetPlugin(plugin, "res/buttons/PlayButton.svg")));
    addFrame(SVG::load(assetPlugin(plugin, "res/buttons/PlayButtonDown.svg")));
  }
};
struct DotButton : SVGSwitch, MomentarySwitch {
  DotButton() {
    addFrame(SVG::load(assetPlugin(plugin, "res/buttons/DotButton.svg")));
  }
};
struct DotDotButton : SVGSwitch, MomentarySwitch {
  DotDotButton() {
    addFrame(SVG::load(assetPlugin(plugin, "res/buttons/DotDotButton.svg")));
  }
};
struct StopButton : SVGSwitch, MomentarySwitch {
  StopButton() {
    addFrame(SVG::load(assetPlugin(plugin, "res/buttons/StopButton.svg")));
    addFrame(SVG::load(assetPlugin(plugin, "res/buttons/StopButtonDown.svg")));
  }
};
struct RecordButton : SVGSwitch, MomentarySwitch {
  RecordButton() {
    addFrame(SVG::load(assetPlugin(plugin, "res/buttons/RecordButton.svg")));
    addFrame(SVG::load(assetPlugin(plugin, "res/buttons/RecordButtonDown.svg")));
  }
};
struct UpButton : SVGSwitch, MomentarySwitch {
  UpButton() {
    addFrame(SVG::load(assetPlugin(plugin, "res/buttons/UpButton.svg")));
  }
};
struct DownButton : SVGSwitch, MomentarySwitch {
  DownButton() {
    addFrame(SVG::load(assetPlugin(plugin, "res/buttons/DownButton.svg")));
  }
};
struct LeftButton : SVGSwitch, MomentarySwitch {
  LeftButton() {
    addFrame(SVG::load(assetPlugin(plugin, "res/buttons/LeftButton.svg")));
    addFrame(SVG::load(assetPlugin(plugin, "res/buttons/LeftButtonDown.svg")));
  }
};
struct RightButton : SVGSwitch, MomentarySwitch {
  RightButton() {
    addFrame(SVG::load(assetPlugin(plugin, "res/buttons/RightButton.svg")));
    addFrame(SVG::load(assetPlugin(plugin, "res/buttons/RightButtonDown.svg")));
  }
};

struct RotaryEncoderKnob : SVGKnob{
  RotaryEncoderKnob(){
    minAngle = -99999.f;
    maxAngle = 99999.f;
    speed = 0.00001f;
    smooth = false;
  }
};
struct StartEncoder : RotaryEncoderKnob {
  StartEncoder() {
    // minAngle = -99999.f;
    // maxAngle = 99999.f;
    // speed = 0.00004f;
    setSVG(SVG::load(assetPlugin(plugin, "res/knobs/StartEncoder.svg")));
  }
};
struct StopEncoder : RotaryEncoderKnob {
  StopEncoder() {
    // minAngle = -99999.f;
    // maxAngle = 99999.f;
    // speed = 0.00004f;
    setSVG(SVG::load(assetPlugin(plugin, "res/knobs/StopEncoder.svg")));
  }
};
struct ArrowSnapKnob : SVGKnob {
  ArrowSnapKnob() {
    setSVG(SVG::load(assetPlugin(plugin, "res/knobs/ArrowKnob.svg")));
    snap = true;
    smooth = false;
    minAngle = -3.1414f;
    maxAngle = 3.1415f - (3.1415f * 0.25f);
    // shadow->opacity = 0.0;
  }
};
struct SmallKnob : SVGKnob {
  SmallKnob(){
    setSVG(SVG::load(assetPlugin(plugin, "res/knobs/SmallKnob.svg")));
    minAngle = -3.1414f + (3.1415f * 0.25f);
    maxAngle = 3.1415f - (3.1415f * 0.25f);
  }
};
struct MrChainkov : SVGKnob {
  MrChainkov(){
    setSVG(SVG::load(assetPlugin(plugin, "res/knobs/MrChainkov.svg")));
    minAngle = -3.1414f * 0.5f;
    maxAngle = 3.1415f * 0.5f;
  }
};
struct PiongKnob : SVGKnob {
  PiongKnob(){
    setSVG(SVG::load(assetPlugin(plugin, "res/knobs/PiongKnob.svg")));
    minAngle = -3.1414f + (3.1415f * 0.25f);
    maxAngle = 3.1415f - (3.1415f * 0.25f);
  }
};
struct PiongKnobSmall : SVGKnob {
  PiongKnobSmall(){
    setSVG(SVG::load(assetPlugin(plugin, "res/knobs/PiongKnobSmall.svg")));
    minAngle = -3.1414f + (3.1415f * 0.25f);
    maxAngle = 3.1415f - (3.1415f * 0.25f);
  }
};

struct TransparentSnapKnob : SVGKnob {
  TransparentSnapKnob() {
    setSVG(SVG::load(assetPlugin(plugin, "res/knobs/TransparentKnob.svg")));
    snap = true;
    smooth = false;
    shadow->opacity = 0.0;
  }
};
struct RoundSmallBlackSnapKnob : RoundSmallBlackKnob {
  RoundSmallBlackSnapKnob() {
    snap = true;
    smooth = false;
  }
};
struct SmallOutputPort : SVGPort {
  SmallOutputPort() {
    setSVG(SVG::load(assetPlugin(plugin, "res/ports/SmallDarkPort.svg")));    
    shadow->opacity = 0.0;
  }
};
struct SmallDarkPort : SVGPort {
  SmallDarkPort() {
    setSVG(SVG::load(assetPlugin(plugin, "res/ports/SmallDarkPort.svg")));    
    // shadow->opacity = 0.0;
  }
};
struct FlatDarkPort : SVGPort {
  FlatDarkPort() {
    setSVG(SVG::load(assetPlugin(plugin, "res/ports/FlatDarkPort.svg")));    
    // shadow->opacity = 0.0;
  }
};
struct DarkHole : SVGPort {
  DarkHole() {
    setSVG(SVG::load(assetPlugin(plugin, "res/ports/DarkHole.svg")));    
    shadow->opacity = 0.0;
  }
};
struct SmallHole : SVGPort {
  SmallHole() {
    setSVG(SVG::load(assetPlugin(plugin, "res/ports/SmallHole.svg")));    
    shadow->opacity = 0.0;
  }
};
struct SmallBrightPort : SVGPort {
  SmallBrightPort() {
    setSVG(SVG::load(assetPlugin(plugin, "res/ports/SmallBrightPort.svg")));    
    // shadow->opacity = 0.0;
  }
};
struct FlatInputPort : SVGPort {
  FlatInputPort() {
    setSVG(SVG::load(assetPlugin(plugin, "res/ports/FlatInputPort.svg")));    
  }
};
struct InputPort : SVGPort {
  InputPort() {
    setSVG(SVG::load(assetPlugin(plugin, "res/ports/InputPort.svg")));    
  }
};
struct InputGatePort : SVGPort {
  InputGatePort() {
    setSVG(SVG::load(assetPlugin(plugin, "res/ports/InputGatePort.svg")));    
    shadow->opacity = 0.0;
  }
};
struct VOctOutputPort : SVGPort {
  VOctOutputPort() {
    setSVG(SVG::load(assetPlugin(plugin, "res/ports/VOctOutputPort.svg")));    
    shadow->opacity = 0.0;
  }
};
struct VOctInputPort : SVGPort {
  VOctInputPort() {
    setSVG(SVG::load(assetPlugin(plugin, "res/ports/VOctInputPort.svg")));    
    shadow->opacity = 0.0;
  }
};
struct OutputGatePort : SVGPort {
  OutputGatePort() {
    setSVG(SVG::load(assetPlugin(plugin, "res/ports/OutputGatePort.svg")));    
    shadow->opacity = 0.0;
  }
};
struct OutputPort : SVGPort {
  OutputPort() {
    setSVG(SVG::load(assetPlugin(plugin, "res/ports/OutputPort.svg")));    
  }
};
struct PeaceScrewButton : SVGSwitch, MomentarySwitch {
  PeaceScrewButton() {
    addFrame(SVG::load(assetPlugin(plugin, "res/misc/PeaceScrew.svg")));
  }
};
struct PeaceScrew : SVGScrew {
  PeaceScrew() {
    sw->setSVG(SVG::load(assetPlugin(plugin, "res/misc/PeaceScrew.svg")));
    box.size = sw->box.size;
    // sw->rotate(randomUniform() * 360.f);
  }
};
struct TreeScrew : SVGScrew {
  TreeScrew() {
    sw->setSVG(SVG::load(assetPlugin(plugin, "res/misc/TreeScrew.svg")));
    box.size = sw->box.size;
    // rotate(randomUniform() * 360.f);
  }
};
struct OrangeLight : GrayModuleLightWidget {
  OrangeLight(){
    addBaseColor(nvgRGB(0xff, 0xaa, 0x44));
  }
};
struct WhiteLight : GrayModuleLightWidget {
  WhiteLight(){
    addBaseColor(nvgRGB(0xff, 0xff, 0xfa));
  }
};
struct AutoOrGate : SVGSwitch, ToggleSwitch {
  AutoOrGate() {
    addFrame(SVG::load(assetPlugin(plugin, "res/buttons/AG_Gate.svg")));
    addFrame(SVG::load(assetPlugin(plugin, "res/buttons/AG_Auto.svg")));
  }
};

class TriggerSwitch{
  public: 
  bool down;
  int state = 0;
  float wait = 0;
  int update(float v){
     if(v > 0.0f){
      if(!down){
        down = true;
        state = 2;
      }else
        state = 1;
     }else if(v <= 0.0f && down){
      down = false;
      state = -1;
     }else{
      state = 0;
     }
     return state;
  }
};

class RotaryEncoder{
  public: 
  float offset;
  float speed = 0.5f;
  float delta(float v){
     float r = v - offset;
     offset = v;
     return r * speed;
  }
  void init(float o){
    offset = o;
  }
};

// class Util{
// public:
//   bool blackkeys[12] = {false, true, false, true, false, false, true, false, true, false, true, false};
// };
// // const bool Util::blackkeys[12] = ;
