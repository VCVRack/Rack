#include "cf.hpp"
#include "dsp/digital.hpp"
#include "osdialog.h"
namespace rack_plugin_cf {
// (note) also used in Bidoo module(s)
#include "dr_wav.h"
}
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

struct PLAY : Module {
	enum ParamIds {
		PREV_PARAM,
		NEXT_PARAM,
		LSPEED_PARAM,
		LOAD_PARAM,
		PLAY_PARAM,
		NUM_PARAMS 
	};
	enum InputIds {
		TRIG_INPUT,
		NUM_INPUTS
	};
	enum OutputIds {
		OUT_OUTPUT,
		NUM_OUTPUTS
	};
	enum LightIds {
		NUM_LIGHTS
	};
	

	unsigned int channels;
	unsigned int sampleRate;
	drwav_uint64 totalSampleCount;

	vector<vector<float>> playBuffer;
	bool loading = false;

	bool run = false;

	string lastPath = "";
	float samplePos = 0;
	string fileDesc;
	bool fileLoaded = false;
	bool reload = false ;
	int sampnumber = 0;
	vector <string> fichier;
	SchmittTrigger loadsampleTrigger;
	SchmittTrigger playTrigger;
	SchmittTrigger nextTrigger;
	SchmittTrigger prevTrigger;

PLAY() : Module(NUM_PARAMS, NUM_INPUTS, NUM_OUTPUTS, NUM_LIGHTS) {
		playBuffer.resize(1);
		playBuffer[0].resize(0); }

	void step() override;
	
	void loadSample(std::string path);
	
	// persistence
	
	json_t *toJson() override {
		json_t *rootJ = json_object();
		// lastPath
		json_object_set_new(rootJ, "lastPath", json_string(lastPath.c_str()));	
		return rootJ;
	}

	void fromJson(json_t *rootJ) override {
		// lastPath
		json_t *lastPathJ = json_object_get(rootJ, "lastPath");
		if (lastPathJ) {
			lastPath = json_string_value(lastPathJ);
			reload = true ;
			loadSample(lastPath);
			
		}}

};

void PLAY::loadSample(std::string path) {

		loading = true;
		unsigned int c;
  		unsigned int sr;
  		drwav_uint64 sc;
		float* pSampleData;
		pSampleData = drwav_open_and_read_file_f32(path.c_str(), &c, &sr, &sc);

	if (pSampleData != NULL) {
		channels = c;
		sampleRate = sr;
		playBuffer[0].clear();
		for (unsigned int i=0; i < sc; i = i + c) {
			playBuffer[0].push_back(pSampleData[i]);
			
		}
		totalSampleCount = playBuffer[0].size();
		drwav_free(pSampleData);
loading = false;

		fileLoaded = true;
		
		fileDesc = stringFilename(path);

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


void PLAY::step() {


	if (fileLoaded) {
		if (nextTrigger.process(params[NEXT_PARAM].value))
			{
			std::string dir = lastPath.empty() ? assetLocal("") : stringDirectory(lastPath);
			if (sampnumber < int(fichier.size()-1)) sampnumber=sampnumber+1; else sampnumber =0;
			loadSample(dir + "/" + fichier[sampnumber]);
			}
				
			
		if (prevTrigger.process(params[PREV_PARAM].value))
			{
			std::string dir = lastPath.empty() ? assetLocal("") : stringDirectory(lastPath);
			if (sampnumber > 0) sampnumber=sampnumber-1; else sampnumber =int(fichier.size()-1);
			loadSample(dir + "/" + fichier[sampnumber]);
			} 
	} else fileDesc = "R.Click to load";



////////////////////////////////////////////////////////////////// Play   
	if (inputs[TRIG_INPUT].active) {
		if (playTrigger.process(inputs[TRIG_INPUT].value)) 
			{
			run = true;
			samplePos = 0;
			}
		}
    
	if ((!loading) && (run) && ((abs(floor(samplePos)) < totalSampleCount))) 
		{ if (samplePos>=0) 
			outputs[OUT_OUTPUT].value = 5 * playBuffer[0][floor(samplePos)];
		  else outputs[OUT_OUTPUT].value = 5 * playBuffer[0][floor(totalSampleCount-1+samplePos)];
		samplePos = samplePos+1+(params[LSPEED_PARAM].value) /3;
		}
		else
		{ 
		run = false;
		outputs[OUT_OUTPUT].value = 0;
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

struct PLAYDisplay : TransparentWidget {
	PLAY *module;

	int frame = 0;
	shared_ptr<Font> font;

	PLAYDisplay() {
		font = Font::load(assetPlugin(plugin, "res/LEDCalculator.ttf"));
	}
	
	void draw(NVGcontext *vg) override {
		std::string to_display = "";
		for (int i=0; i<14; i++) to_display = to_display + module->fileDesc[i];
		nvgFontSize(vg, 24);
		nvgFontFaceId(vg, font->handle);
		nvgTextLetterSpacing(vg, 0);
		nvgFillColor(vg, nvgRGBA(0x4c, 0xc7, 0xf3, 0xff));
		nvgRotate(vg, -M_PI / 2.0f);	
		nvgTextBox(vg, 5, 5,350, to_display.c_str(), NULL);
	}
};


struct PLAYWidget : ModuleWidget {
	PLAYWidget(PLAY *module);
//void step() override;
Menu *createContextMenu() override;
};

PLAYWidget::PLAYWidget(PLAY *module) : ModuleWidget(module) {
	setPanel(SVG::load(assetPlugin(plugin, "res/PLAY.svg")));

	
	{
		PLAYDisplay *gdisplay = new PLAYDisplay();
		gdisplay->module = module;
		gdisplay->box.pos = Vec(18, 253);
		gdisplay->box.size = Vec(130, 250);
		addChild(gdisplay);
	}

	addChild(Widget::create<ScrewSilver>(Vec(15, 0)));
	addChild(Widget::create<ScrewSilver>(Vec(box.size.x-30, 365)));

	addParam(ParamWidget::create<Trimpot>(Vec(6, 298), module, PLAY::LSPEED_PARAM, -5.0f, 5.0f, 0.0f));
	addInput(Port::create<PJ301MPort>(Vec(3, 31), Port::INPUT, module, PLAY::TRIG_INPUT));
	addOutput(Port::create<PJ301MPort>(Vec(3, 321), Port::OUTPUT, module, PLAY::OUT_OUTPUT));

	addParam(ParamWidget::create<upButton>(Vec(6, 276), module, PLAY::PREV_PARAM, 0.0f, 1.0f, 0.0f));
	addParam(ParamWidget::create<downButton>(Vec(6, 256), module, PLAY::NEXT_PARAM, 0.0f, 1.0f, 0.0f));
}

struct PLAYItem : MenuItem {
	PLAY *play;
	void onAction(EventAction &e) override {
		
		std::string dir = play->lastPath.empty() ? assetLocal("") : stringDirectory(play->lastPath);
		char *path = osdialog_file(OSDIALOG_OPEN, dir.c_str(), NULL, NULL);
		if (path) {
			play->run = false;
			play->reload = true;
			play->loadSample(path);

			play->samplePos = 0;
			play->lastPath = path;
			free(path);
		}

	}

};

Menu *PLAYWidget::createContextMenu() {
	Menu *menu = ModuleWidget::createContextMenu();

	MenuLabel *spacerLabel = new MenuLabel();
	menu->addChild(spacerLabel);

	PLAY *play = dynamic_cast<PLAY*>(module);
	assert(play);

	PLAYItem *sampleItem = new PLAYItem();
	sampleItem->text = "Load sample";
	sampleItem->play = play ;
	menu->addChild(sampleItem);

	return menu;
}

} // namespace rack_plugin_cf

using namespace rack_plugin_cf;

RACK_PLUGIN_MODEL_INIT(cf, PLAY) {
   Model *modelPLAY = Model::create<PLAY, PLAYWidget>("cf", "PLAY", "Play", SAMPLER_TAG);
   return modelPLAY;
}

