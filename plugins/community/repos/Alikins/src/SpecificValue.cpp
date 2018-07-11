#include "global_pre.hpp"
#include <stdio.h>
#include <string>
#include <vector>
#include <unordered_map>

#include "alikins.hpp"
#include "ui.hpp"
#include "global_ui.hpp"

namespace rack_plugin_Alikins {

struct SpecificValue : Module
{
    enum ParamIds
    {
        VALUE1_PARAM,
        OCTAVE_PARAM,
        NUM_PARAMS
    };
    enum InputIds
    {
    	VALUE1_INPUT,
        NUM_INPUTS
    };
    enum OutputIds
    {
        VALUE1_OUTPUT,
        NUM_OUTPUTS
    };

    enum LightIds
    {
        NUM_LIGHTS
    };

    SpecificValue() : Module(NUM_PARAMS, NUM_INPUTS, NUM_OUTPUTS, NUM_LIGHTS) {}

    float A440_octave = 4.0f;

    void step() override;

    // TODO: toJson/fromJson for saving values

    float volt_value;
    float hz_value;
    float period_value;
    float cents_value;

};

struct NoteInfo {
    std::string name;
    float offset_cents;
};

// TODO: mv to header
float A440_VOLTAGE = 4.75f;
int A440_MIDI_NUMBER = 69;

// TODO: support other enharmonic names Db G♭ A♯?
std::vector<std::string> note_name_vec = {"C", "C#", "D", "D#", "E", "F", "F#", "G", "G#", "A", "A#", "B"};

std::unordered_map<std::string, float> gen_note_name_map() {
    float volt = -10.0f;
    std::string fs = note_name_vec[4];
    std::unordered_map<std::string, float> note_name_map;

    // FIXME: add a map of note name (including enharmonic) to voltage offset from C
    //        then just iterate over it for each octave
    for (int i = -10; i <= 10; i++)
    {
        for (int j = 0; j < 12; j++)
        {
            // debug("oct=%d note=%s volt=%f ", i, note_name_vec[j].c_str(), volt);
            note_name_map[stringf("%s%d", note_name_vec[j].c_str(), i)] = volt;
            volt += (1.0f / 12.0f);
        }
    }
    return note_name_map;
}

std::unordered_map<std::string, float> note_name_to_volts_map = gen_note_name_map();

// FIXME: can/should be inline
// FIXME: likely should be a NoteInfo type/struct/object
// These are assuming A440 == A4 == 4.75v
float freq_to_cv(float freq, float a440_octave) {
    float volts = log2f(freq / 440.0f * powf(2.0f, A440_VOLTAGE)) - a440_octave;
    // debug("freq_to_vc freq=%f a440_octave=%f volts=%f A440_voltage=%f", freq, a440_octave, volts, A440_VOLTAGE);
    return volts;
}

float cv_to_freq(float volts, float a440_octave) {
    float freq = 440.0f / powf(2.0f, A440_VOLTAGE) * powf(2.0f, volts + a440_octave);
    // debug("cv_to_freq freq=%f a440_octave=%f volts=%f A440_voltage=%f", freq, a440_octave, volts, A440_VOLTAGE);
    return freq;
}

// can return negative
float volts_of_nearest_note(float volts) {
    float res =  roundf( (volts * 12.0f) )  / 12.0f;
    return res;
}

int volts_to_note(float volts) {
    int res = abs(static_cast<int>( roundf( (volts * 12.0f) ) ) ) % 12;
    // debug("volts_to_note volts=%f res=%d", volts, res);
    return res;
}

int volts_to_octave(float volts, float a440_octave) {
    // debug("a440_octave=%f", a440_octave);
    int octave = floor(volts + a440_octave);
    // debug("volts_to_octaves volts=%f, a440_octave=%f, octave=%d", volts, a440_octave, octave);
    return octave;
}

float volts_to_note_cents(float volts, float a440_octave) {
    float nearest_note = volts_of_nearest_note(volts);
    float cent_volt = 1.0f / 12.0f / 100.0f;

    float offset_cents = (volts-nearest_note)/cent_volt;
    // debug("volts: %f volts_of_nearest: %f volts-volts_nearest: %f offset_cents %f",
    //     volts, nearest_note, volts-nearest_note, offset_cents);

    return offset_cents;
}

int volts_to_midi(float volts, float a440_octave) {
    int midi_note = floor(volts * 12.0f + a440_octave) + 21;
    return midi_note;
}

void SpecificValue::step()
{
    A440_octave = params[OCTAVE_PARAM].value;

    if (inputs[VALUE1_INPUT].active) {
        params[VALUE1_PARAM].value = inputs[VALUE1_INPUT].value;
    }
    volt_value = params[VALUE1_PARAM].value;
    outputs[VALUE1_OUTPUT].value = volt_value;
}

struct FloatField : TextField
{
    float value;
    SpecificValue *module;

    FloatField(SpecificValue *_module);

    void onAction(EventAction &e) override;
    void onChange(EventChange &e) override;

    float textToVolts(std::string field_text);
    std::string voltsToText(float param_volts);
};

FloatField::FloatField(SpecificValue *_module)
{
    module = _module;
    value = module->params[SpecificValue::VALUE1_PARAM].value;
    text = voltsToText(value);
}

// TODO: this is really data stuff, so could be in type/struct/class for the data (volt, freq/hz, period/seconds, note_name)
//       and instanced and provided to a generic ValueField widget that has-a data type converter thingy
float FloatField::textToVolts(std::string field_text) {
    return atof(field_text.c_str());
}

std::string FloatField::voltsToText(float param_volts){
    return stringf("%0.3f", param_volts);
}

void FloatField::onChange(EventChange &e) {
    //debug("FloatField onChange  text=%s param=%f", text.c_str(), module->params[SpecificValue::VALUE1_PARAM].value);

     if (this != RACK_PLUGIN_UI_FOCUSED_WIDGET) {
        std::string new_text = voltsToText(module->params[SpecificValue::VALUE1_PARAM].value);
        setText(new_text);
     }
}

void FloatField::onAction(EventAction &e)
{
    //debug("FloatField onAction text=%s", text.c_str());

    //update text first?
    TextField::onAction(e);

    float volts = textToVolts(text);

    //debug("FloatField setting volts=%f text=%s", volts, text.c_str());
    module->params[SpecificValue::VALUE1_PARAM].value = volts;

    //debug("FloatField onAction2 text=%s volts=%f module->volt_values=%f",
    //      text.c_str(), volts, module->volt_value);

}


struct HZFloatField : TextField
{
    float value;
    SpecificValue *module;

    HZFloatField(SpecificValue *_module);
    void onChange(EventChange &e) override;
    void onAction(EventAction &e) override;

    float textToVolts(std::string field_text);
    std::string voltsToText(float param_volts);
};

HZFloatField::HZFloatField(SpecificValue *_module)
{
    module = _module;
}

float HZFloatField::textToVolts(std::string field_text) {
    float freq = strtof(text.c_str(), NULL);
    return freq_to_cv(freq, module->A440_octave);
}

std::string HZFloatField::voltsToText(float param_volts){
    float freq = cv_to_freq(param_volts, module->A440_octave);
    std::string new_text = stringf("%0.*f", freq < 100 ? 4 : 3, freq);
    return new_text;
}


void HZFloatField::onChange(EventChange &e) {
    //debug("HZFloatField onChange  text=%s param=%f", text.c_str(), module->params[SpecificValue::VALUE1_PARAM].value);

     //TextField::onChange(e);

     if (this != RACK_PLUGIN_UI_FOCUSED_WIDGET)
     {
         std::string new_text = voltsToText(module->params[SpecificValue::VALUE1_PARAM].value);
         setText(new_text);
     }
}

void HZFloatField::onAction(EventAction &e)
{
    //debug("HZFloatField onAction text=%s", text.c_str());

    //update text first?
    TextField::onAction(e);

    float volts = textToVolts(text);

    //debug("HZ FloatField onAction about to set VALUE*_PARAM to volts: %f", volts);
    module->params[SpecificValue::VALUE1_PARAM].value = volts;
}


struct SecondsFloatField : TextField {
    float value;
    SpecificValue *module;

    SecondsFloatField(SpecificValue *_module);
    void onAction(EventAction &e) override;
    void onChange(EventChange &e) override;

    float textToVolts(std::string field_text);
    std::string voltsToText(float param_volts);
};

SecondsFloatField::SecondsFloatField(SpecificValue *_module)
{
    module = _module;
}


float SecondsFloatField::textToVolts(std::string field_text) {
    float period = strtof(text.c_str(), NULL);
    float freq = 1.0f / period;
    return freq_to_cv(freq, module->A440_octave);
}

std::string SecondsFloatField::voltsToText(float param_volts){
    float period = 1.0f / cv_to_freq(param_volts, module->A440_octave);
    std::string new_text = stringf("%0.*f", period < 100 ? 4 : 3, period);
    return new_text;
}

void SecondsFloatField::onChange(EventChange &e) {
    //debug("SecondsFloatField onChange  text=%s param=%f", text.c_str(), module->params[SpecificValue::VALUE1_PARAM].value);

     //TextField::onChange(e);

     if (this != RACK_PLUGIN_UI_FOCUSED_WIDGET)
     {
         std::string new_text = voltsToText(module->params[SpecificValue::VALUE1_PARAM].value);
         setText(new_text);
     }
}


void SecondsFloatField::onAction(EventAction &e) {
    //debug("SecondsFloatField onAction text=%s", text.c_str());

    //update text first?
    TextField::onAction(e);

    float volts = textToVolts(text);

    //debug("SecondsFloatField onAction about to set VALUE*_PARAM to volts: %f", volts);
    module->params[SpecificValue::VALUE1_PARAM].value = volts;
}

struct CentsField : TextField {
    float value;
    SpecificValue *module;

    CentsField(SpecificValue *_module);
    void onChange(EventChange &e) override;
    void onAction(EventAction &e) override;

};

CentsField::CentsField(SpecificValue *_module) {
    module = _module;
}

void CentsField::onChange(EventChange &e) {
    // debug("CentsField onChange");
    float cents = volts_to_note_cents(module->params[SpecificValue::VALUE1_PARAM].value,
                                     module->params[SpecificValue::OCTAVE_PARAM].value);

    // debug("CentsField onChange cents: %f", cents);
    if (this != RACK_PLUGIN_UI_FOCUSED_WIDGET || fabs(cents) >= 0.50f)
    {
        float cents = volts_to_note_cents(module->params[SpecificValue::VALUE1_PARAM].value,
                                          module->params[SpecificValue::OCTAVE_PARAM].value);
        std::string new_text = stringf("% 0.2f", cents);
        setText(new_text);
    }
}


void CentsField::onAction(EventAction &e) {

    TextField::onAction(e);
    float cents = strtof(text.c_str(), NULL);

    // figure what to tweak the current volts
    float cent_volt = 1.0f / 12.0f / 100.0f;
    float delta_volt = cents * cent_volt;
    float nearest_note_voltage = volts_of_nearest_note(module->params[SpecificValue::VALUE1_PARAM].value);
    //debug("volts: %f nearest_volts: %f", module->params[SpecificValue::VALUE1_PARAM].value, nearest_note_voltage);
    //debug("delta_volt: %+f nearest_note_voltage+delta_volt: %f", delta_volt, nearest_note_voltage,
    //    nearest_note_voltage + delta_volt);
    module->params[SpecificValue::VALUE1_PARAM].value = nearest_note_voltage + delta_volt;

}



struct NoteNameField : TextField {
    float value;
    SpecificValue *module;

    NoteNameField(SpecificValue *_module);
    void onChange(EventChange &e) override;
    void onAction(EventAction &e) override;
};

NoteNameField::NoteNameField(SpecificValue *_module)
{
    module = _module;
}

void NoteNameField::onChange(EventChange &e) {
    //debug("NoteNameField onChange  text=%s param=%f", text.c_str(), module->params[SpecificValue::VALUE1_PARAM].value);

     //TextField::onChange(e);

     if (this != RACK_PLUGIN_UI_FOCUSED_WIDGET)
     {
        float cv_volts = module->params[SpecificValue::VALUE1_PARAM].value;
        int octave = volts_to_octave(cv_volts, module->params[SpecificValue::OCTAVE_PARAM].value);
        int note_number = volts_to_note(cv_volts);
        // float semi_cents = volts_to_note_and_cents(cv_volts, module->params[SpecificValue::OCTAVE_PARAM].value);
        // note_info = volts_to_note_info(cv_volts, module->params[SpecificValue::OCTAVE_PARAM].value);
        // TODO: modf for oct/fract part, need to get +/- cents from chromatic notes
        std::string new_text = stringf("%s%d", note_name_vec[note_number].c_str(), octave);
        // debug("foo %f bar %f", )
        setText(new_text);
     }

}


void NoteNameField::onAction(EventAction &e) {
    //debug("NoteNameField onAction");
    TextField::onAction(e);
    // FIXME: Haven't tested but seems like this does a lot.
    // FIXME: I suspect just a array of structs with name/freq in it and a linear search makes more sense
    //        but lets c++ stuff
    auto search = note_name_to_volts_map.find(text);
    if(search != note_name_to_volts_map.end()) {
        /*
        debug("note_name_to_volts_map[%s] = %f (%f) %f", text.c_str(),
             note_name_to_volts_map[text],
             (note_name_to_volts_map[text] - module->A440_octave),
             module->A440_octave );
             */
        module->params[SpecificValue::VALUE1_PARAM].value = note_name_to_volts_map[text] - module->A440_octave;
        return;
    }
    else {
        // TODO: change the text color to indicate bogus name?
        debug("%s was  NOT A VALID note name", text.c_str());
        return;
    }
}

struct SmallPurpleTrimpot : Trimpot {
    SmallPurpleTrimpot();
};

SmallPurpleTrimpot::SmallPurpleTrimpot() : Trimpot() {
    setSVG(SVG::load(assetPlugin(plugin, "res/SmallPurpleTrimpot.svg")));
    shadow->blurRadius = 0.0;
    shadow->opacity = 0.10;
    shadow->box.pos = Vec(0.0, box.size.y * 0.1);
}

struct PurpleTrimpot : Trimpot {
	Module *module;
    bool initialized = false;
    PurpleTrimpot();
    void step() override;
    void reset() override;
    void randomize() override;
};

PurpleTrimpot::PurpleTrimpot() : Trimpot() {
    setSVG(SVG::load(assetPlugin(plugin, "res/PurpleTrimpot.svg")));
    shadow->blurRadius = 0.0;
    shadow->opacity = 0.10;
    shadow->box.pos = Vec(0.0, box.size.y * 0.05);
}

// FIXME: if we are getting moving inputs and we are hovering
//        over the trimpot, we kind of jitter arround.
// maybe run this via an onChange()?
void PurpleTrimpot::step() {
	//debug("paramId=%d this->initialized: %d initialized: %d this->value: %f value: %f param.value: %f",
     // paramId,  this->initialized, initialized, this->value, value, module->params[paramId].value);

    if (this->value != module->params[paramId].value) {
       if (this != RACK_PLUGIN_UI_HOVERED_WIDGET && this->initialized) {
			// this->value = module->params[paramId].value;
			setValue(module->params[paramId].value);
		} else {
			module->params[paramId].value = this->value;
            this->initialized |= true;
		}
		EventChange e;
		onChange(e);
	}

	Trimpot::step();
}

void PurpleTrimpot::reset() {
    this->initialized = false;
    Trimpot::reset();
    }

void PurpleTrimpot::randomize() {
    reset();
    setValue(rescale(randomUniform(), 0.0f, 1.0f, minValue, maxValue));
}

struct SpecificValueWidget : ModuleWidget
{
    SpecificValueWidget(SpecificValue *module);

    void step() override;
    void onChange(EventChange &e) override;

    float prev_volts = 0.0f;
    float prev_octave = 4.0f;
    float prev_input = 0.0f;

    FloatField *volts_field;
    HZFloatField *hz_field;
    SecondsFloatField *period_field;
    NoteNameField *note_name_field;
    CentsField *cents_field;
};


SpecificValueWidget::SpecificValueWidget(SpecificValue *module) : ModuleWidget(module)
{
    setPanel(SVG::load(assetPlugin(plugin, "res/SpecificValue.svg")));

    // TODO: widget with these children?
    float y_baseline = 45.0f;

    Vec volt_field_size = Vec(70.0f, 22.0f);
    Vec hz_field_size = Vec(70.0, 22.0f);
    Vec seconds_field_size = Vec(70.0, 22.0f);

    float x_pos = 10.0f;
    // debug("adding field %d", i);

    y_baseline = 45.0f;

    volts_field = new FloatField(module);
    volts_field->box.pos = Vec(x_pos, y_baseline);
    volts_field->box.size = volt_field_size;
    volts_field->value = module->params[SpecificValue::VALUE1_PARAM].value;
    addChild(volts_field);

    y_baseline = 90.0f;

    float h_pos = x_pos;
    hz_field = new HZFloatField(module);
    hz_field->box.pos = Vec(x_pos, y_baseline);
    hz_field->box.size = hz_field_size;
    hz_field->value = module->hz_value;
    addChild(hz_field);

    y_baseline = 135.0f;

    period_field = new SecondsFloatField(module);
    period_field->box.pos = Vec(h_pos, y_baseline);
    period_field->box.size = seconds_field_size;
    period_field->value = module->period_value;

    addChild(period_field);

    y_baseline = 180.0f;

    note_name_field = new NoteNameField(module);
    note_name_field->box.pos = Vec(x_pos, y_baseline);
    note_name_field->box.size = Vec(70.0f, 22.0f);
    note_name_field->value = module->volt_value;
    addChild(note_name_field);

    y_baseline += note_name_field->box.size.y;
    y_baseline += 5.0f;
    // y_baseline += 20.0f;

    cents_field = new CentsField(module);
    cents_field->box.pos = Vec(x_pos, y_baseline);
    cents_field->box.size = Vec(55.0f, 22.0f);
    cents_field->value = module->cents_value;
    addChild(cents_field);

    // y_baseline += period_field->box.size.y;
    y_baseline += 20.0f;

    float middle = box.size.x / 2.0f;
    float in_port_x = 15.0f;

    y_baseline += 24.0f + 12.0f;

    Port *value_in_port = Port::create<PJ301MPort>(
        Vec(in_port_x, y_baseline),
        Port::INPUT,
        module,
        SpecificValue::VALUE1_INPUT);
    //value_in_port->box.pos = Vec(middle - value_in_port->box.size.x / 2, y_baseline);
    value_in_port->box.pos = Vec(2.0f, y_baseline);

    inputs.push_back(value_in_port);
    addChild(value_in_port);

    // octave trimpot
    SmallPurpleTrimpot *octaveTrimpot = ParamWidget::create<SmallPurpleTrimpot>(
        Vec(middle, y_baseline + 2.5f),
        module,
        SpecificValue::OCTAVE_PARAM,
        0.0f, 8.0f, 4.0f);

    params.push_back(octaveTrimpot);
    octaveTrimpot->box.pos = Vec(middle - octaveTrimpot->box.size.x / 2, y_baseline + 2.5f);
    octaveTrimpot->snap = true;
    addChild(octaveTrimpot);

    float out_port_x = middle + 24.0f;

    Port *value_out_port = Port::create<PJ301MPort>(
        Vec(out_port_x, y_baseline),
        Port::OUTPUT,
        module,
        SpecificValue::VALUE1_OUTPUT);

    outputs.push_back(value_out_port);
    value_out_port->box.pos = Vec(box.size.x - value_out_port->box.size.x - 2.0f, y_baseline);

    addChild(value_out_port);

    y_baseline += value_out_port->box.size.y;
    y_baseline += 16.0f;

    PurpleTrimpot *trimpot = ParamWidget::create<PurpleTrimpot>(
        Vec(middle - 24.0f, y_baseline + 2.5f),
        module,
        SpecificValue::VALUE1_PARAM,
        -10.0f, 10.0f, 0.0f);

    //debug(" trimpot: dv: %f v: %f p.value: %f", trimpot->defaultValue, trimpot->value,
    //    module->params[SpecificValue::VALUE1_PARAM].value);
    params.push_back(trimpot);
    addChild(trimpot);
}


void SpecificValueWidget::step() {
    ModuleWidget::step();

    if (prev_volts != module->params[SpecificValue::VALUE1_PARAM].value ||
        prev_octave != module->params[SpecificValue::OCTAVE_PARAM].value ||
        prev_input != module->params[SpecificValue::VALUE1_INPUT].value) {
            // debug("SpVWidget step - emitting EventChange / onChange prev_volts=%f param=%f",
            //     prev_volts, module->params[SpecificValue::VALUE1_PARAM].value);
            prev_volts = module->params[SpecificValue::VALUE1_PARAM].value;
            prev_octave = module->params[SpecificValue::OCTAVE_PARAM].value;
            prev_input = module->params[SpecificValue::VALUE1_INPUT].value;
            EventChange e;
		    onChange(e);
    }
}

void SpecificValueWidget::onChange(EventChange &e) {
    // debug("SpvWidget onChange");
    ModuleWidget::onChange(e);
    volts_field->onChange(e);
    hz_field->onChange(e);
    period_field->onChange(e);
    note_name_field->onChange(e);
    cents_field->onChange(e);

}

} // namespace rack_plugin_Alikins

using namespace rack_plugin_Alikins;

RACK_PLUGIN_MODEL_INIT(Alikins, SpecificValue) {
   Model *modelSpecificValue = Model::create<SpecificValue, SpecificValueWidget>(
      "Alikins", "SpecificValue", "Specific Values", UTILITY_TAG);
   return modelSpecificValue;
}
