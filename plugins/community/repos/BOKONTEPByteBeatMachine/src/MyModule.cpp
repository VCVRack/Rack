#include "Template.hpp"
//#include "Fundamental.hpp"
#include "dsp/digital.hpp"
#include "duktape.h"

namespace rack_plugin_BokontepByteBeat {

struct BokontepByteBeatModule : Module {
	enum ParamIds {
		
		TRIG_PARAM,
		TRIG_PARAM_BUTTON,
		X_PARAM,
		Y_PARAM,
		NUM_PARAMS
	};
	enum InputIds {
		
		TRIG_INPUT,
		X_INPUT,
		Y_INPUT,
		NUM_INPUTS
	};
	enum OutputIds {
		BYTEBEAT_OUTPUT,
		NUM_OUTPUTS
	};
	enum LightIds {
		BLINK_LIGHT,
		TRIGGER_LIGHT,
		NUM_LIGHTS
	};
	
	
	char javascriptBuffer[512];
	const char* header = "function f(t,X,Y)	{ return (";
	const char* footer = "); } ";
	float accumulator = 0.0f;
	float timestep = 1.0/8000.0f;
	TextField* textField;
	bool running = false;
	bool compiled = false;
	SchmittTrigger trigger;
	float phase = 0.0;
	float blinkPhase = 0.0;
	int t = 0;
	duk_context *ctx = NULL;
	BokontepByteBeatModule() : Module(NUM_PARAMS, NUM_INPUTS, NUM_OUTPUTS, NUM_LIGHTS) {
		
	}
	void step() override;

	// For more advanced Module features, read Rack's engine.hpp header file
	// - toJson, fromJson: serialization of internal data
	// - onSampleRateChange: event triggered by a change of sample rate
	// - onReset, onRandomize, onCreate, onDelete: implements special behavior when user clicks these from the context menu
	void onCreate () override
	{
		if(ctx)
		{
			duk_destroy_heap(ctx);
			ctx = duk_create_heap_default();
		}
		else
		{
			ctx = duk_create_heap_default();
		}
		//textField->text = "t = t+1;";
		running = false;
		if(strlen(textField->text.c_str())<512)
		{
			sprintf(javascriptBuffer,"%s%s%s",header,textField->text.c_str(),footer);
			if (duk_pcompile_string(ctx, DUK_COMPILE_FUNCTION,javascriptBuffer)==0)
			{
				compiled = true;
			}
			else
			{
				compiled = false;
				running = false;
			}
		}
		else
		{
			compiled = false;
			running = false;
		}
		
		
			
	}
	void onDelete() override
	{
		if(ctx)
		{
			duk_destroy_heap(ctx);
		}
		if(!ctx)
		{
			ctx = NULL;
		}
	}
	void onReset () override
	{
		onCreate();
	}

};


void BokontepByteBeatModule::step() {
	// Implement a bytebeat oscillator
	float deltaTime = engineGetSampleTime();
	int x = 0;
	int y = 0;
	if((inputs[TRIG_INPUT].value || params[TRIG_PARAM_BUTTON].value) && compiled)
	{
		
		t = 0;
		
		accumulator = 0.0f;
		running = true;	
		
	}
	
	x=(uint8_t)(((inputs[X_INPUT].value+5.0f)/10.0)*255); //scale -5.0 .. +5.0 to 0-255
	y=(uint8_t)(((inputs[Y_INPUT].value+5.0f)/10.0)*255); //scale -5.0 .. +5.0 to 0-255
	
	
	
	accumulator = accumulator + deltaTime;
	t = accumulator/timestep;
	
	
	// Compute the output
	int retval = 0;
	
	if(running)
	{
		
		duk_dup(ctx, 0);
		duk_push_int(ctx, t);
		duk_push_int(ctx, x);
		duk_push_int(ctx, y);
		duk_call(ctx, 3);
		retval = (uint8_t)duk_get_int_default(ctx, 1, 0);
		duk_pop(ctx);
			
		
	
		//retval = rand()*255;
	}
	outputs[BYTEBEAT_OUTPUT].value = 5.0f * ((retval-127.0)/127.0);
	lights[TRIGGER_LIGHT].value = max(inputs[TRIG_INPUT].value , params[TRIG_PARAM_BUTTON].value);
	// Blink light at 1Hz
	blinkPhase += deltaTime;
	if (blinkPhase >= 1.0f)
		blinkPhase -= 1.0f;
	if(compiled)
	{
		lights[BLINK_LIGHT].value = 1.0f;
	}
	else
	{
		lights[BLINK_LIGHT].value = (blinkPhase < 0.5f) ? 1.0f : 0.0f;
	}
}


struct BokontepByteBeatWidget : ModuleWidget {
	TextField *textField;	
	BokontepByteBeatWidget(BokontepByteBeatModule *module) : ModuleWidget(module) {

		setPanel(SVG::load(assetPlugin(plugin, "res/MyModule.svg")));

		addChild(Widget::create<ScrewSilver>(Vec(RACK_GRID_WIDTH, 0)));
		addChild(Widget::create<ScrewSilver>(Vec(box.size.x - 2 * RACK_GRID_WIDTH, 0)));
		addChild(Widget::create<ScrewSilver>(Vec(RACK_GRID_WIDTH, RACK_GRID_HEIGHT - RACK_GRID_WIDTH)));
		addChild(Widget::create<ScrewSilver>(Vec(box.size.x - 2 * RACK_GRID_WIDTH, RACK_GRID_HEIGHT - RACK_GRID_WIDTH)));

		//addParam(ParamWidget::create<Davies1900hBlackKnob>(Vec(28, 87), module, BokontepByteBeatModule::TRIG_PARAM, -3.0, 3.0, 0.0));
		
		addParam(ParamWidget::create<LEDButton>(Vec(42, 270), module, BokontepByteBeatModule::TRIG_PARAM_BUTTON, 0.0f, 1.0f, 0.0f));
		addChild(ModuleLightWidget::create<MediumLight<GreenLight>>(Vec(46.4f, 274.4f), module, BokontepByteBeatModule::TRIGGER_LIGHT));
		addInput(Port::create<PJ301MPort>(Vec(38, 300), Port::INPUT, module, BokontepByteBeatModule::TRIG_INPUT));
		addInput(Port::create<PJ301MPort>(Vec(98, 300), Port::INPUT, module, BokontepByteBeatModule::X_INPUT));
		addInput(Port::create<PJ301MPort>(Vec(153, 300), Port::INPUT, module, BokontepByteBeatModule::Y_INPUT));
		addOutput(Port::create<PJ301MPort>(Vec(215, 300), Port::OUTPUT, module, BokontepByteBeatModule::BYTEBEAT_OUTPUT));

		addChild(ModuleLightWidget::create<MediumLight<RedLight>>(Vec(125, 100), module, BokontepByteBeatModule::BLINK_LIGHT));
		textField = Widget::create<LedDisplayTextField>(mm2px(Vec(3, 42)));
		textField->box.size = mm2px(Vec(85, 40));
		textField->multiline = true;
		
		addChild(textField);
		module->textField = this->textField;
	}
		json_t *toJson() override {
		json_t *rootJ = ModuleWidget::toJson();

		// text
		json_object_set_new(rootJ, "text", json_string(textField->text.c_str()));
		
		return rootJ;
	}

	void fromJson(json_t *rootJ) override {
		ModuleWidget::fromJson(rootJ);

		// text
		json_t *textJ = json_object_get(rootJ, "text");
		if (textJ)
		{
			textField->text = json_string_value(textJ);
			module->onCreate();
		}
	}
};

} // namespace rack_plugin_BokontepByteBeat

using namespace rack_plugin_BokontepByteBeat;

RACK_PLUGIN_MODEL_INIT(BOKONTEPByteBeatMachine, BokontepByteBeatMachine) {
   // Specify the Module and ModuleWidget subclass, human-readable
   // author name for categorization per plugin, module slug (should never
   // change), human-readable module name, and any number of tags
   // (found in `include/tags.hpp`) separated by commas.
   Model *modelBokontepByteBeatMachine = Model::create<BokontepByteBeatModule, BokontepByteBeatWidget>("BokontepByteBeat", "BokontepByteBeatMachine", "Bokontep ByteBeat Machine", OSCILLATOR_TAG);
   return modelBokontepByteBeatMachine;
}
