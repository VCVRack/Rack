// Lots of this is poached from Strum's Mental Chord and  Bogaudio's Retone!

#include "dsp/digital.hpp"
#include <iostream>
#include <cmath>
#include <sstream>
#include <iomanip>
#include "RJModules.hpp"

namespace rack_plugin_RJModules {

// Displays
struct StringDisplayWidget : TransparentWidget {

  std::string *value;
  std::shared_ptr<Font> font;

  StringDisplayWidget() {
    font = Font::load(assetPlugin(plugin, "res/Pokemon.ttf"));
  };

  void draw(NVGcontext *vg) override
  {
    // Background
    NVGcolor backgroundColor = nvgRGB(0xC0, 0xC0, 0xC0);
    nvgBeginPath(vg);
    nvgRoundedRect(vg, 0.0, 0.0, box.size.x, box.size.y, 4.0);
    nvgFillColor(vg, backgroundColor);
    nvgFill(vg);

    // text
    nvgFontSize(vg, 24);
    nvgFontFaceId(vg, font->handle);
    nvgTextLetterSpacing(vg, 2.5);

    std::stringstream to_display;
    to_display << std::setw(3) << *value;

    Vec textPos = Vec(16.0f, 33.0f);
    NVGcolor textColor = nvgRGB(0x00, 0x00, 0x00);
    nvgFillColor(vg, textColor);
    nvgText(vg, textPos.x, textPos.y, to_display.str().c_str(), NULL);
  }
};

struct LargeSnapKnob : RoundHugeBlackKnob
{
    LargeSnapKnob()
    {
        minAngle = -0.83 * M_PI;
        maxAngle = 0.83 * M_PI;
        snap = true;
    }
};

// Main
struct Chord : Module {
    enum ParamIds {
        CHORD_PARAM,
        SHAPE_PARAM,
        NUM_PARAMS
    };
    enum InputIds {
        CHORD_CV_INPUT,
        SHAPE_CV_INPUT,
        NUM_INPUTS
    };
    enum OutputIds {
        ROOT_OUTPUT,
        THREE_OUTPUT,
        FIVE_OUTPUT,
        SEVEN_OUTPUT,
        NINE_OUTPUT,
        NUM_OUTPUTS
    };
    enum LightIds {
        NUM_LIGHTS
    };

    std::string chord_name = "Hello!";

    // Pitchies
    float referenceFrequency = 261.626; // C4; frequency at which Rack 1v/octave CVs are zero.
    float referenceSemitone = 60.0; // C4; value of C4 in semitones is arbitrary here, so have it match midi note numbers when rounded to integer.
    float twelfthRootTwo = 1.0594630943592953;
    float logTwelfthRootTwo = logf(1.0594630943592953);
    int referencePitch = 0;
    int referenceOctave = 4;

    float frequencyToSemitone(float frequency) {
        return logf(frequency / referenceFrequency) / logTwelfthRootTwo + referenceSemitone;
    }

    float semitoneToFrequency(float semitone) {
        return powf(twelfthRootTwo, semitone - referenceSemitone) * referenceFrequency;
    }

    float frequencyToCV(float frequency) {
        return log2f(frequency / referenceFrequency);
    }

    float cvToFrequency(float cv) {
        return powf(2.0, cv) * referenceFrequency;
    }

    float cvToSemitone(float cv) {
        return frequencyToSemitone(cvToFrequency(cv));
    }

    float semitoneToCV(float semitone) {
        return frequencyToCV(semitoneToFrequency(semitone));
    }

    Chord() : Module(NUM_PARAMS, NUM_INPUTS, NUM_OUTPUTS, NUM_LIGHTS) {}
    void step() override;
};

void Chord::step() {

    float offset_raw = (params[CHORD_PARAM].value) * 12 - 6 + (inputs[CHORD_CV_INPUT].value) / 1.5;
    float pitch_offset = round(offset_raw) / 12;
    float root = 1.0*1 + pitch_offset;

    float _input_pitch = params[CHORD_PARAM].value * clamp(inputs[CHORD_CV_INPUT].normalize(10.0f) / 10.0f, 0.0f, 1.0f);;
    float _pitch = (int) _input_pitch % (int) 12;
    float _octave = int(_input_pitch / 12);

    float _shape = params[SHAPE_PARAM].value * clamp(inputs[SHAPE_CV_INPUT].normalize(10.0f) / 10.0f, 0.0f, 1.0f);; 
    float _three_interval;
    float _five_interval;
    float _seven_interval;
    char* shape = NULL;
    // via https://en.wikibooks.org/wiki/Music_Theory/Chords
    switch ((int) _shape) {
        case 0: {
            // Maj
            shape = "Maj";
            _three_interval = 4;
            _five_interval = 7;
            _seven_interval = 11;
            break;
        }
        case 1: {
            // Min
            shape = "Min";
            _three_interval = 3;
            _five_interval = 7;
            _seven_interval = 10;
            break;
        }
        case 2: {
            // Dim
            shape = "Dim";
            _three_interval = 3;
            _five_interval = 6;
            _seven_interval = 10;
            break;
        }
        case 3: {
            shape = "Aug";
            _three_interval = 4;
            _five_interval = 8;
            _seven_interval = 12;
            break;
        }
    }

    float _root_frequency = semitoneToFrequency(referenceSemitone + 12 * (_octave - referenceOctave) + (_pitch - referencePitch));
    float _root_cv = frequencyToCV(_root_frequency);

    float _third_frequency = semitoneToFrequency(referenceSemitone + 12 * (_octave - referenceOctave) + (_pitch + _three_interval - referencePitch));
    float _third_cv = frequencyToCV(_third_frequency);

    float _fifth_frequency = semitoneToFrequency(referenceSemitone + 12 * (_octave - referenceOctave) + (_pitch + _five_interval - referencePitch));
    float _fifth_cv = frequencyToCV(_fifth_frequency);

    float _seventh_frequency = semitoneToFrequency(referenceSemitone + 12 * (_octave - referenceOctave) + (_pitch + _seven_interval - referencePitch));
    float _seventh_cv = frequencyToCV(_seventh_frequency);

    outputs[ROOT_OUTPUT].value = _root_cv;
    outputs[THREE_OUTPUT].value = _third_cv;
    outputs[FIVE_OUTPUT].value = _fifth_cv;
    outputs[SEVEN_OUTPUT].value = _seventh_cv;

    char* pitch = NULL;
    char* sharpFlat = NULL;
    switch ((int) _pitch) {
        case 0: {
            pitch = "C";
            break;
        }
        case 1: {
            pitch = "C#";
            sharpFlat = "#";
            break;
        }
        case 2: {
            pitch = "D";
            break;
        }
        case 3: {
            pitch = "D#";
            sharpFlat = "#";
            break;
        }
        case 4: {
            pitch = "E";
            break;
        }
        case 5: {
            pitch = "F";
            break;
        }
        case 6: {
            pitch = "F#";
            sharpFlat = "#";
            break;
        }
        case 7: {
            pitch = "G";
            break;
        }
        case 8: {
            pitch = "G#";
            sharpFlat = "#";
            break;
        }
        case 9: {
            pitch = "A";
            break;
        }
        case 10: {
            pitch = "A#";
            sharpFlat = "#";
            break;
        }
        case 11: {
            pitch = "B";
            break;
        }
    }

    chord_name = std::string(pitch) + std::to_string((int)_octave) + std::string(shape);

}

struct ChordWidget: ModuleWidget {
    ChordWidget(Chord *module);
};

ChordWidget::ChordWidget(Chord *module) : ModuleWidget(module) {
    box.size = Vec(15*10, 380);

    {
        SVGPanel *panel = new SVGPanel();
        panel->box.size = box.size;
        panel->setBackground(SVG::load(assetPlugin(plugin, "res/Chord.svg")));
        addChild(panel);
    }

    addChild(Widget::create<ScrewSilver>(Vec(15, 0)));
    addChild(Widget::create<ScrewSilver>(Vec(box.size.x-30, 0)));
    addChild(Widget::create<ScrewSilver>(Vec(15, 365)));
    addChild(Widget::create<ScrewSilver>(Vec(box.size.x-30, 365)));

    addParam(ParamWidget::create<LargeSnapKnob>(Vec(47, 143), module, Chord::CHORD_PARAM, 0.0, 59.0, 24.0));
    addParam(ParamWidget::create<LargeSnapKnob>(Vec(47, 228), module, Chord::SHAPE_PARAM, 0.0, 3.0, 0.0));

    addInput(Port::create<PJ301MPort>(Vec(22, 190), Port::INPUT, module, Chord::CHORD_CV_INPUT));
    addInput(Port::create<PJ301MPort>(Vec(22, 270), Port::INPUT, module, Chord::SHAPE_CV_INPUT));

    addOutput(Port::create<PJ301MPort>(Vec(16, 319), Port::OUTPUT, module, Chord::ROOT_OUTPUT));
    addOutput(Port::create<PJ301MPort>(Vec(48, 319), Port::OUTPUT, module, Chord::THREE_OUTPUT));
    addOutput(Port::create<PJ301MPort>(Vec(81, 319), Port::OUTPUT, module, Chord::FIVE_OUTPUT));
    addOutput(Port::create<PJ301MPort>(Vec(114, 319), Port::OUTPUT, module, Chord::SEVEN_OUTPUT));

    StringDisplayWidget *display = new StringDisplayWidget();
    display->box.pos = Vec(28, 70);
    display->box.size = Vec(100, 40);
    display->value = &module->chord_name;
    addChild(display);

}


} // namespace rack_plugin_RJModules

using namespace rack_plugin_RJModules;

RACK_PLUGIN_MODEL_INIT(RJModules, Chord) {
   Model *modelChord = Model::create<Chord, ChordWidget>("RJModules", "Chord", "[GEN] Chord", LFO_TAG);
   return modelChord;
}

