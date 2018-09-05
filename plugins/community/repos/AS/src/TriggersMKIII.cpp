//**************************************************************************************
//TriggersMKIII module for VCV Rack by Alfredo Santamaria - AS - https://github.com/AScustomWorks/AS
//
//**************************************************************************************
#include "AS.hpp"
#include "window.hpp"
#include "dsp/digital.hpp"

struct TriggersMKIII: Module {
    enum ParamIds {
        TRIGGER_SWITCH_1,
        TRIGGER_SWITCH_2,
        NUM_PARAMS
    };
    enum InputIds {
        CV_TRIG_INPUT_1_1,
        CV_TRIG_INPUT_1_2,
        CV_TRIG_INPUT_2_1,
        CV_TRIG_INPUT_2_2,
        NUM_INPUTS
    };
    enum OutputIds {
        TRIGGER_OUT1,
        TRIGGER_OUT2,
        NUM_OUTPUTS
    };
    enum LightIds {
        TRIGGER_LED_1,
        TRIGGER_LED_2,
        NUM_LIGHTS
    };

    SchmittTrigger btnTrigger1, btnTrigger2;
    SchmittTrigger extTrigger1_1, extTrigger1_2;
    SchmittTrigger extTrigger2_1, extTrigger2_2;

    TextField* textField1;
    TextField* textField2;

    const float lightLambda = 0.075f;
    float resetLight1 = 0.0f;
    float resetLight2 = 0.0f;

    int label_num1 = 0;
    int label_num2 = 0;



    TriggersMKIII() : Module(NUM_PARAMS, NUM_INPUTS, NUM_OUTPUTS, NUM_LIGHTS) {}
    void step() override;
 

	json_t *toJson()override {
		json_t *rootJ = json_object();
		// text
		json_object_set_new(rootJ, "label1", json_string(textField1->text.c_str()));
        json_object_set_new(rootJ, "label2", json_string(textField2->text.c_str()));
		return rootJ;
	}

	void fromJson(json_t *rootJ)override {
		json_t *text1J = json_object_get(rootJ, "label1");
		if (text1J){
			textField1->text = json_string_value(text1J);
        }
        json_t *text2J = json_object_get(rootJ, "label2");
		if (text2J){
			textField2->text = json_string_value(text2J);
        }
	}

};

void TriggersMKIII::step() {

    outputs[TRIGGER_OUT1].value = 0.0f;
    outputs[TRIGGER_OUT2].value = 0.0f;

    //TRIGGER 1
    if (btnTrigger1.process(params[TRIGGER_SWITCH_1].value) || extTrigger1_1.process(inputs[CV_TRIG_INPUT_1_1].value) || extTrigger1_2.process(inputs[CV_TRIG_INPUT_1_2].value)) {
        resetLight1 = 1.0;
        outputs[TRIGGER_OUT1].value = 10.0f;
    }else{
        outputs[TRIGGER_OUT1].value = 0.0f;
    }
    resetLight1 -= resetLight1 / lightLambda / engineGetSampleRate();
    lights[TRIGGER_LED_1].value = resetLight1;

    //TRIGGER 2
    //EXTERNAL TRIGGER
    if (btnTrigger2.process(params[TRIGGER_SWITCH_2].value) || extTrigger2_1.process(inputs[CV_TRIG_INPUT_2_1].value) || extTrigger2_2.process(inputs[CV_TRIG_INPUT_2_2].value)) {
        resetLight2 = 1.0;
        outputs[TRIGGER_OUT2].value = 10.0f;
    //INTERNAL TRIGGER
    }else{
        outputs[TRIGGER_OUT2].value = 0.0f;
    }
    resetLight2 -= resetLight2 / lightLambda / engineGetSampleRate();
    lights[TRIGGER_LED_2].value = resetLight2;
    
}
////////////////////////////////////
struct CustomLedDisplayTextField : TextField {
	std::shared_ptr<Font> font;
	Vec textOffset;
	NVGcolor color;
	CustomLedDisplayTextField(){
        font = Font::load(assetPlugin(plugin, "res/saxmono.ttf"));
        color = nvgRGB(0xf0, 0x00, 0x00);
        textOffset = Vec(5, 0);  
    };

    void draw(NVGcontext *vg) override{
        nvgScissor(vg, 0, 0, box.size.x, box.size.y);
        // Background
        NVGcolor backgroundColor = nvgRGB(0x20, 0x10, 0x10);
        NVGcolor borderColor = nvgRGB(0x10, 0x10, 0x10);
        nvgBeginPath(vg);
        nvgRoundedRect(vg, 0.0, 0.0, box.size.x, box.size.y, 4.0);

        nvgFillColor(vg, backgroundColor);
        nvgFill(vg);
        nvgStrokeWidth(vg, 1.5);
        nvgStrokeColor(vg, borderColor);
        nvgStroke(vg);
        nvgFontSize(vg, 20);

        // Text
        if (font->handle >= 0) {
            bndSetFont(font->handle);

            NVGcolor highlightColor = nvgRGB(0xf0, 0x00, 0x00);//color;
            highlightColor.a = 0.5;
            int begin = min(cursor, selection);
            int end = (this == RACK_PLUGIN_UI_FOCUSED_WIDGET) ? max(cursor, selection) : -1;
            bndIconLabelCaret(vg, textOffset.x, textOffset.y,
                box.size.x - 2*textOffset.x, box.size.y - 2*textOffset.y,
                -1, color, 12, text.c_str(), highlightColor, begin, end);
        }
        nvgResetScissor(vg);
        bndSetFont(rack::global_ui->window.gGuiFont->handle);
    }

    int getTextPosition(Vec mousePos)override {
        bndSetFont(font->handle);
        int textPos = bndIconLabelTextPosition(rack::global_ui->window.gVg, textOffset.x, textOffset.y,
            box.size.x - 2*textOffset.x, box.size.y - 2*textOffset.y,
            -1, 12, text.c_str(), mousePos.x, mousePos.y);
        bndSetFont(font->handle);
        return textPos;
    }
};

////////////////////////////////////

struct TriggersMKIIIWidget : ModuleWidget { 

    TextField* textField1;
    TextField* textField2;
    TriggersMKIIIWidget(TriggersMKIII *module);
};

TriggersMKIIIWidget::TriggersMKIIIWidget(TriggersMKIII *module) : ModuleWidget(module) {


   setPanel(SVG::load(assetPlugin(plugin, "res/TriggersMKIII.svg")));
   
	//SCREWS
	addChild(Widget::create<as_HexScrew>(Vec(RACK_GRID_WIDTH, 0)));
	addChild(Widget::create<as_HexScrew>(Vec(box.size.x - 2 * RACK_GRID_WIDTH, 0)));
	addChild(Widget::create<as_HexScrew>(Vec(RACK_GRID_WIDTH, RACK_GRID_HEIGHT - RACK_GRID_WIDTH)));
	addChild(Widget::create<as_HexScrew>(Vec(box.size.x - 2 * RACK_GRID_WIDTH, RACK_GRID_HEIGHT - RACK_GRID_WIDTH)));

    static const float led_offset = 3.3;
    static const float led_center = 15;
    static const float y_offset = 150;
    //TRIGGER 1

    textField1 = Widget::create<CustomLedDisplayTextField>(Vec(6, 46));
    textField1->box.size = Vec(78, 30);
    textField1->multiline = false;
    addChild(textField1);
    module->textField1 = this->textField1;
    //SWITCH
    addParam(ParamWidget::create<BigLEDBezel>(Vec(led_center, 122), module, TriggersMKIII::TRIGGER_SWITCH_1, 0.0, 1.0, 0.0));
    addChild(ModuleLightWidget::create<GiantLight<RedLight>>(Vec(led_center+led_offset, 122+led_offset), module, TriggersMKIII::TRIGGER_LED_1));
    //PORTS
    addInput(Port::create<as_PJ301MPort>(Vec(8, 90), Port::INPUT, module, TriggersMKIII::CV_TRIG_INPUT_1_1));
    addInput(Port::create<as_PJ301MPort>(Vec(33, 90), Port::INPUT, module, TriggersMKIII::CV_TRIG_INPUT_1_2));
    addOutput(Port::create<as_PJ301MPort>(Vec(58, 90), Port::OUTPUT, module, TriggersMKIII::TRIGGER_OUT1));

    //TRIGGER 2
    textField2 = Widget::create<CustomLedDisplayTextField>(Vec(6, 46+y_offset));
    textField2->box.size = Vec(78, 30);
    textField2->multiline = false;
    addChild(textField2);
    module->textField2 = this->textField2;
    //SWITCH
    addParam(ParamWidget::create<BigLEDBezel>(Vec(led_center, 122+y_offset), module, TriggersMKIII::TRIGGER_SWITCH_2, 0.0, 1.0, 0.0));
    addChild(ModuleLightWidget::create<GiantLight<RedLight>>(Vec(led_center+led_offset, 122+led_offset+y_offset), module, TriggersMKIII::TRIGGER_LED_2));
    //PORTS
    addInput(Port::create<as_PJ301MPort>(Vec(8, 90+y_offset), Port::INPUT, module, TriggersMKIII::CV_TRIG_INPUT_2_1));
    addInput(Port::create<as_PJ301MPort>(Vec(33, 90+y_offset), Port::INPUT, module, TriggersMKIII::CV_TRIG_INPUT_2_2));
    addOutput(Port::create<as_PJ301MPort>(Vec(58, 90+y_offset), Port::OUTPUT, module, TriggersMKIII::TRIGGER_OUT2));

    
}

RACK_PLUGIN_MODEL_INIT(AS, TriggersMKIII) {
   Model *modelTriggersMKIII = Model::create<TriggersMKIII, TriggersMKIIIWidget>("AS", "TriggersMKIII", "Triggers MKIII",  SWITCH_TAG, UTILITY_TAG);
   return modelTriggersMKIII;
}
