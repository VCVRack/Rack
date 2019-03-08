#include "recorder-module-widget.hh"
#include "recorder-module.hh"

recorder_module_widget::recorder_module_widget(Module* module) : ModuleWidget(module) {
	setPanel(SVG::load(assetPlugin(plugin, "res/Recorder.svg")));

	addChild(Widget::create<ScrewSilver>(Vec(RACK_GRID_WIDTH, 0)));
	addChild(Widget::create<ScrewSilver>(Vec(box.size.x - 2 * RACK_GRID_WIDTH, 0)));
	addChild(Widget::create<ScrewSilver>(Vec(RACK_GRID_WIDTH, RACK_GRID_HEIGHT - RACK_GRID_WIDTH)));
	addChild(Widget::create<ScrewSilver>(Vec(box.size.x - 2 * RACK_GRID_WIDTH, RACK_GRID_HEIGHT - RACK_GRID_WIDTH)));

	addInput(Port::create<PJ301MPort>(mm2px(Vec(3.7069211, 10.530807)), Port::INPUT, module, recorder_module::AUDIO_INPUT + 0));
	addInput(Port::create<PJ301MPort>(mm2px(Vec(3.7069211, 23.530807)), Port::INPUT, module, recorder_module::AUDIO_INPUT + 1));
	addInput(Port::create<PJ301MPort>(mm2px(Vec(3.7069211, 36.530807)), Port::INPUT, module, recorder_module::AUDIO_INPUT + 2));
	addInput(Port::create<PJ301MPort>(mm2px(Vec(3.7069211, 49.530807)), Port::INPUT, module, recorder_module::AUDIO_INPUT + 3));

}

// Specify the Module and ModuleWidget subclass, human-readable
// author name for categorization per plugin, module slug (should never
// change), human-readable module name, and any number of tags
// (found in `include/tags.hpp`) separated by commas.
Model *recorder_model = Model::create<recorder_module, recorder_module_widget>("Skylights", "SkRecorder", "SK Recorder", RECORDING_TAG);
