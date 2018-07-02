#include "cf.hpp"
#include "dsp/digital.hpp"
#include "osdialog.h"
#include "AudioFile.h"
#include <vector>
#include "cmath"
#ifdef _MSC_VER
#include "dirent_win32/dirent.h"
#else
#include <dirent.h>
#endif
#include <algorithm> //----added by Joakim Lindbom


using namespace std;

namespace rack_plugin_cf {

struct PLAYER : Module {
	enum ParamIds {
		PLAY_PARAM,
		POS_PARAM,
		LSTART_PARAM,
		LSPEED_PARAM,
		TSTART_PARAM,
		TSPEED_PARAM,
		SPD_PARAM,
		NEXT_PARAM,
		PREV_PARAM,
		OSC_PARAM,
		NUM_PARAMS 
	};
	enum InputIds {
		GATE_INPUT,
		POS_INPUT,
        SPD_INPUT,
	PREV_INPUT,
	NEXT_INPUT,
		TRIG_INPUT,
		NUM_INPUTS
	};
	enum OutputIds {
		OUT_OUTPUT,
		OUT2_OUTPUT,
		NUM_OUTPUTS
	};
	enum LightIds {
		OSC_LIGHT,
		NUM_LIGHTS
	};
	
	bool play = false;
	string lastPath = "";
	AudioFile<double> audioFile;
	float samplePos = 0;
 	float startPos = 0;
	vector<double> displayBuff;
	string fileDesc;
	bool fileLoaded = false;

	SchmittTrigger playTrigger;
	SchmittTrigger playGater;
	SchmittTrigger nextTrigger;
	SchmittTrigger prevTrigger;
	SchmittTrigger nextinTrigger;
	SchmittTrigger previnTrigger;
	SchmittTrigger oscTrigger;
	vector <string> fichier;

	int sampnumber = 0;
	int retard = 0;
	bool reload = false ;
	bool oscState = false ;


	PLAYER() : Module(NUM_PARAMS, NUM_INPUTS, NUM_OUTPUTS, NUM_LIGHTS) { }

	void step() override;
	
	void loadSample(std::string path);
	
	// persistence
	
	json_t *toJson() override {
		json_t *rootJ = json_object();
		// lastPath
		json_object_set_new(rootJ, "lastPath", json_string(lastPath.c_str()));	
json_object_set_new(rootJ, "oscstate", json_integer(oscState));
		return rootJ;
	}

	void fromJson(json_t *rootJ) override {
		// lastPath
		json_t *lastPathJ = json_object_get(rootJ, "lastPath");
		if (lastPathJ) {
			lastPath = json_string_value(lastPathJ);
			reload = true ;
			loadSample(lastPath);
			
		}
	json_t *oscstateJ = json_object_get(rootJ, "oscstate");
		if (oscstateJ)
			oscState = json_integer_value(oscstateJ);
			lights[OSC_LIGHT].value=oscState;
	
	}
};

void PLAYER::loadSample(std::string path) {
	if (audioFile.load (path.c_str())) {
		fileLoaded = true;
		vector<double>().swap(displayBuff);
		for (int i=0; i < audioFile.getNumSamplesPerChannel(); i = i + floor(audioFile.getNumSamplesPerChannel()/130)) {
			displayBuff.push_back(audioFile.samples[0][i]);
		}
		fileDesc = stringFilename(path)+ "\n";
		fileDesc += std::to_string(audioFile.getSampleRate())+ " Hz" + " - ";                 //"\n";
		fileDesc += std::to_string(audioFile.getBitDepth())+ " bits" + " \n";
	//	fileDesc += std::to_string(audioFile.getNumSamplesPerChannel())+ " smp" +"\n";
	//	fileDesc += std::to_string(audioFile.getLengthInSeconds())+ " s." + "\n";
		fileDesc += std::to_string(audioFile.getNumChannels())+ " channel(s)" + "\n";
	//	fileDesc += std::to_string(audioFile.isMono())+ "\n";
	//	fileDesc += std::to_string(audioFile.isStereo())+ "\n";

		if (reload) {
			DIR* rep = NULL;
			struct dirent* dirp = NULL;
			std::string dir = path.empty() ? assetLocal("") : stringDirectory(path);

			rep = opendir(dir.c_str());
			int i = 0;
			fichier.clear();
			while ((dirp = readdir(rep)) != NULL) {
				std::string name = dirp->d_name;

				std::size_t found = name.find(".wav",name.length()-5);
				if (found==std::string::npos) found = name.find(".WAV",name.length()-5);
				if (found==std::string::npos) found = name.find(".aif",name.length()-5);
				if (found==std::string::npos) found = name.find(".AIF",name.length()-5);
				if (found==std::string::npos) found = name.find(".aiff",name.length()-5);
				if (found==std::string::npos) found = name.find(".AIFF",name.length()-5);

  				if (found!=std::string::npos) {
					fichier.push_back(name);
					if ((dir + "/" + name)==path) {sampnumber = i;}
					i=i+1;
            }
				
         }

//----added by Joakim Lindbom
         sort(fichier.begin(), fichier.end());  // Linux needs this to get files in right order
         for (int o=0;o<int(fichier.size()-1); o++) {
            if ((dir + "/" + fichier[o])==path) {
               sampnumber = o;
            }
         }
//---------------

			closedir(rep);
			reload = false;
		}
			lastPath = path;
	}
	else {
		
		fileLoaded = false;
	}
}


void PLAYER::step() {
	if (fileLoaded) {
		if (nextTrigger.process(params[NEXT_PARAM].value)+nextinTrigger.process(inputs[NEXT_INPUT].value))
			{
			std::string dir = lastPath.empty() ? assetLocal("") : stringDirectory(lastPath);
			if (sampnumber < int(fichier.size()-1)) sampnumber=sampnumber+1; else sampnumber =0;
			loadSample(dir + "/" + fichier[sampnumber]);
			}
				
			
		if (prevTrigger.process(params[PREV_PARAM].value)+previnTrigger.process(inputs[PREV_INPUT].value))
			{retard = 1000;
			std::string dir = lastPath.empty() ? assetLocal("") : stringDirectory(lastPath);
			if (sampnumber > 0) sampnumber=sampnumber-1; else sampnumber =int(fichier.size()-1);
			loadSample(dir + "/" + fichier[sampnumber]);
			} 
	} else fileDesc = "right click to load \n .wav or .aif sample \n :)";

if (oscTrigger.process(params[OSC_PARAM].value))
			{oscState =!oscState;lights[OSC_LIGHT].value=oscState;}

	// Play
if (!oscState) {
    bool gated = inputs[GATE_INPUT].value > 0;
    
    if (inputs[POS_INPUT].active)
    startPos = clamp((params[LSTART_PARAM].value + inputs[POS_INPUT].value * params[TSTART_PARAM].value),0.0f,10.0f)*audioFile.getNumSamplesPerChannel()/10;
    else {startPos = clamp((params[LSTART_PARAM].value),0.0f,10.0f)*audioFile.getNumSamplesPerChannel()/10;
        inputs[POS_INPUT].value = 0 ;
    }
    
    if (!inputs[TRIG_INPUT].active) {
	if (playGater.process(inputs[GATE_INPUT].value)) {
		play = true;
		samplePos = startPos;
		}
	} else {
	if (playTrigger.process(inputs[TRIG_INPUT].value)) {
		play = true;
		samplePos = startPos;
		}
	}
    
	if ((play) && ((floor(samplePos) < audioFile.getNumSamplesPerChannel()) && (floor(samplePos) >= 0))) {
		if (audioFile.getNumChannels() == 1) {
			outputs[OUT_OUTPUT].value = 5 * audioFile.samples[0][floor(samplePos)];
			outputs[OUT2_OUTPUT].value = 5 * audioFile.samples[0][floor(samplePos)];}
		else if (audioFile.getNumChannels() ==2) {
			outputs[OUT_OUTPUT].value = 5 * audioFile.samples[0][floor(samplePos)];
			outputs[OUT2_OUTPUT].value = 5 * audioFile.samples[1][floor(samplePos)];
        		}
		if (inputs[SPD_INPUT].active)
        samplePos = samplePos+1+(params[LSPEED_PARAM].value +inputs[SPD_INPUT].value * params[TSPEED_PARAM].value) /3;
        else {
            samplePos = samplePos+1+(params[LSPEED_PARAM].value) /3;
            inputs[SPD_INPUT].value = 0 ;}
	}
	else
	{ 
		play = false;
	    outputs[OUT_OUTPUT].value = 0;outputs[OUT2_OUTPUT].value = 0;
	}
       if (!inputs[TRIG_INPUT].active) {if (gated == false) {play = false; outputs[OUT_OUTPUT].value = 0;outputs[OUT2_OUTPUT].value = 0;}}
} else {
	
	if (((floor(samplePos) < audioFile.getNumSamplesPerChannel()) && (floor(samplePos) >= 0))) {
		if (playTrigger.process(inputs[TRIG_INPUT].value)) samplePos = 0;

		if (audioFile.getNumChannels() == 1) {
			outputs[OUT_OUTPUT].value = 5 * audioFile.samples[0][floor(samplePos)];
			outputs[OUT2_OUTPUT].value = 5 * audioFile.samples[0][floor(samplePos)];}
		else if (audioFile.getNumChannels() ==2) {
			outputs[OUT_OUTPUT].value = 5 * audioFile.samples[0][floor(samplePos)];
			outputs[OUT2_OUTPUT].value = 5 * audioFile.samples[1][floor(samplePos)];
        		}
		if (inputs[SPD_INPUT].active)
        samplePos = samplePos+1+(params[LSPEED_PARAM].value +inputs[SPD_INPUT].value * params[TSPEED_PARAM].value) /3;
        else {
            samplePos = samplePos+1+(params[LSPEED_PARAM].value) /3;
            inputs[SPD_INPUT].value = 0 ;}
	}
	else
	{ 
		samplePos=0;
	}

	}
}

struct upButton : SVGSwitch, MomentarySwitch {
	upButton() {
		addFrame(SVG::load(assetPlugin(plugin, "res/upButton.svg")));
		addFrame(SVG::load(assetPlugin(plugin, "res/upButtonDown.svg")));
		sw->wrap();
		box.size = sw->box.size;
	}
};
struct downButton : SVGSwitch, MomentarySwitch {
	downButton() {
		addFrame(SVG::load(assetPlugin(plugin, "res/downButton.svg")));
		addFrame(SVG::load(assetPlugin(plugin, "res/downButtonDown.svg")));
		sw->wrap();
		box.size = sw->box.size;
	}
};

struct PLAYERDisplay : TransparentWidget {
	PLAYER *module;
	int frame = 0;
	shared_ptr<Font> font;

	PLAYERDisplay() {
		font = Font::load(assetPlugin(plugin, "res/DejaVuSansMono.ttf"));
	}
	
	void draw(NVGcontext *vg) override {
		nvgFontSize(vg, 12);
		nvgFontFaceId(vg, font->handle);
		nvgTextLetterSpacing(vg, -2);
		nvgFillColor(vg, nvgRGBA(0xff, 0xff, 0xff, 0xff));	
		nvgTextBox(vg, 5, 5,120, module->fileDesc.c_str(), NULL);
		
		// Draw ref line
		nvgStrokeColor(vg, nvgRGBA(0xff, 0xff, 0xff, 0x40));
		{
			nvgBeginPath(vg);
			nvgMoveTo(vg, 0, 125);
			nvgLineTo(vg, 125, 125);
			nvgClosePath(vg);
		}
		nvgStroke(vg);
		
		if (module->fileLoaded) {
			// Draw play line
			nvgStrokeColor(vg, nvgRGBA(0x28, 0xb0, 0xf3, 0xff));
            nvgStrokeWidth(vg, 0.8);
			{
				nvgBeginPath(vg);
				nvgMoveTo(vg, floor(module->samplePos * 125 / module->audioFile.getNumSamplesPerChannel()) , 85);
				nvgLineTo(vg, floor(module->samplePos * 125 / module->audioFile.getNumSamplesPerChannel()) , 165);
				nvgClosePath(vg);
			}
			nvgStroke(vg);
            
            // Draw start line
			nvgStrokeColor(vg, nvgRGBA(0x28, 0xb0, 0xf3, 0xff));
            nvgStrokeWidth(vg, 1.5);
			{
				nvgBeginPath(vg);
				nvgMoveTo(vg, floor(module->startPos * 125 / module->audioFile.getNumSamplesPerChannel()) , 85);
				nvgLineTo(vg, floor(module->startPos * 125 / module->audioFile.getNumSamplesPerChannel()) , 165);
				nvgClosePath(vg);
			}
			nvgStroke(vg);
            
			
			// Draw waveform
			nvgStrokeColor(vg, nvgRGBA(0xe1, 0x02, 0x78, 0xc0));
			nvgSave(vg);
			Rect b = Rect(Vec(0, 85), Vec(125, 80));
			nvgScissor(vg, b.pos.x, b.pos.y, b.size.x, b.size.y);
			nvgBeginPath(vg);
			for (unsigned int i = 0; i < module->displayBuff.size(); i++) {
				float x, y;
				x = (float)i / (module->displayBuff.size() - 1);
				y = module->displayBuff[i] / 2.0 + 0.5;
				Vec p;
				p.x = b.pos.x + b.size.x * x;
				p.y = b.pos.y + b.size.y * (1.0 - y);
				if (i == 0)
					nvgMoveTo(vg, p.x, p.y);
				else
					nvgLineTo(vg, p.x, p.y);
			}
			nvgLineCap(vg, NVG_ROUND);
			nvgMiterLimit(vg, 2.0);
			nvgStrokeWidth(vg, 1.5);
			nvgGlobalCompositeOperation(vg, NVG_LIGHTER);
			nvgStroke(vg);			
			nvgResetScissor(vg);
			nvgRestore(vg);	
		}
	}
};



struct PLAYERWidget : ModuleWidget {
	PLAYERWidget(PLAYER *module);
	Menu *createContextMenu() override;
};

PLAYERWidget::PLAYERWidget(PLAYER *module) : ModuleWidget(module) {
	setPanel(SVG::load(assetPlugin(plugin, "res/PLAYER.svg")));

	addChild(Widget::create<ScrewSilver>(Vec(15, 0)));
	addChild(Widget::create<ScrewSilver>(Vec(box.size.x-30, 0)));
	addChild(Widget::create<ScrewSilver>(Vec(15, 365)));
	addChild(Widget::create<ScrewSilver>(Vec(box.size.x-30, 365)));
	
	{
		PLAYERDisplay *display = new PLAYERDisplay();
		display->module = module;
		display->box.pos = Vec(5, 40);
		display->box.size = Vec(130, 250);
		addChild(display);
	}
		
	static const float portX0[4] = {10, 40, 70, 100};
	

	addParam(ParamWidget::create<RoundLargeBlackKnob>(Vec(23, 230), module, PLAYER::LSTART_PARAM, 0.0f, 10.0f, 0.0f));
	addParam(ParamWidget::create<RoundLargeBlackKnob>(Vec(73, 230), module, PLAYER::LSPEED_PARAM, -5.0f, 5.0f, 0.0f));
	addParam(ParamWidget::create<Trimpot>(Vec(42, 278), module, PLAYER::TSTART_PARAM, -1.0f, 1.0f, 0.0f));
	addParam(ParamWidget::create<Trimpot>(Vec(73, 278), module, PLAYER::TSPEED_PARAM, -1.0f, 1.0f, 0.0f));

	addInput(Port::create<PJ301MPort>(Vec(portX0[0], 321), Port::INPUT, module, PLAYER::GATE_INPUT));
	addInput(Port::create<PJ301MPort>(Vec(portX0[1], 321), Port::INPUT, module, PLAYER::POS_INPUT));
  	addInput(Port::create<PJ301MPort>(Vec(portX0[2], 321), Port::INPUT, module, PLAYER::SPD_INPUT));
	addOutput(Port::create<PJ301MPort>(Vec(portX0[3], 275), Port::OUTPUT, module, PLAYER::OUT_OUTPUT));
	addOutput(Port::create<PJ301MPort>(Vec(portX0[3], 321), Port::OUTPUT, module, PLAYER::OUT2_OUTPUT));

	addInput(Port::create<PJ301MPort>(Vec(portX0[0], 91), Port::INPUT, module, PLAYER::PREV_INPUT));
	addInput(Port::create<PJ301MPort>(Vec(portX0[3], 91), Port::INPUT, module, PLAYER::NEXT_INPUT));
	addInput(Port::create<PJ301MPort>(Vec(portX0[0], 275), Port::INPUT, module, PLAYER::TRIG_INPUT));
	addParam(ParamWidget::create<upButton>(Vec(43, 95), module, PLAYER::PREV_PARAM, 0.0f, 1.0f, 0.0f));
	addParam(ParamWidget::create<downButton>(Vec(73, 95), module, PLAYER::NEXT_PARAM, 0.0f, 1.0f, 0.0f));

	addParam(ParamWidget::create<LEDButton>(Vec(11, 210), module, PLAYER::OSC_PARAM, 0.0, 1.0, 0.0));
		addChild(ModuleLightWidget::create<MediumLight<BlueLight>>(Vec(15.4, 214.4), module, PLAYER::OSC_LIGHT));

}

struct PLAYERItem : MenuItem {
	PLAYER *player;
	void onAction(EventAction &e) override {
		
		std::string dir = player->lastPath.empty() ? assetLocal("") : stringDirectory(player->lastPath);
		char *path = osdialog_file(OSDIALOG_OPEN, dir.c_str(), NULL, NULL);
		if (path) {
			player->play = false;
			player->reload = true;
			player->loadSample(path);
			player->samplePos = 0;
			player->lastPath = path;
			free(path);
		}
	}
};

Menu *PLAYERWidget::createContextMenu() {
	Menu *menu = ModuleWidget::createContextMenu();

	MenuLabel *spacerLabel = new MenuLabel();
	menu->addChild(spacerLabel);

	PLAYER *player = dynamic_cast<PLAYER*>(module);
	assert(player);

	PLAYERItem *sampleItem = new PLAYERItem();
	sampleItem->text = "Load sample";
	sampleItem->player = player;
	menu->addChild(sampleItem);

	return menu;
}

} // namespace rack_plugin_cf

using namespace rack_plugin_cf;

RACK_PLUGIN_MODEL_INIT(cf, PLAYER) {
   Model *modelPLAYER = Model::create<PLAYER, PLAYERWidget>("cf", "PLAYER", "Player", SAMPLER_TAG);
   return modelPLAYER;
}
