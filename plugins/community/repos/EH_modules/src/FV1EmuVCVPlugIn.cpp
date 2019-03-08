/* FV1 VCV PlugIn
 * Copyright (C)2018 - Eduard Heidt
 *
 *   This program is free software: you can redistribute it and/or modify
 *   it under the terms of the GNU General Public License as published by
 *   the Free Software Foundation, either version 3 of the License, or
 *   (at your option) any later version.
 *
 *   This program is distributed in the hope that it will be useful,
 *   but WITHOUT ANY WARRANTY; without even the implied warranty of
 *   MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 *   GNU General Public License for more details.
 *
 *   You should have received a copy of the GNU General Public License
 *   along with this program.  If not, see <http://www.gnu.org/licenses/>.
 *
 */

#include "EH_modules.h"
#include "FV1emu.hpp"
#include <rack.hpp>
#include <dsp/digital.hpp>
#include <osdialog.h>
#ifdef WIN32
#include "dirent_win32/dirent.h"
#else
#include <dirent.h>
#endif // WIN32
#include <iterator>
#include <thread>

using namespace rack;

namespace rack_plugin_EH_modules {

struct FV1EmuModule : Module
{
	enum ParamIds
	{
		POT0_PARAM,
		POT1_PARAM,
		POT2_PARAM,
		TPOT0_PARAM,
		TPOT1_PARAM,
		TPOT2_PARAM,
		FX_PREV,
		FX_NEXT,
		DRYWET_PARAM,
		NUM_PARAMS
	};
	enum InputIds
	{
		POT_0,
		POT_1,
		POT_2,
		INPUT_L,
		INPUT_R,
		NUM_INPUTS
	};
	enum OutputIds
	{
		OUTPUT_L,
		OUTPUT_R,
		NUM_OUTPUTS
	};

	FV1emu fx;

	SchmittTrigger nextTrigger;
	SchmittTrigger prevTrigger;

	FV1EmuModule() : Module(NUM_PARAMS, NUM_INPUTS, NUM_OUTPUTS)
	{
		loadFx(assetPlugin(plugin, "fx/demo.spn"));
		info("FV1EmuModule()");
	}

	~FV1EmuModule()
	{
		info("~FV1EmuModule()");
	}

	void step() override
	{
		if (filesInPath.size() > 0)
		{
			if (nextTrigger.process(params[FX_NEXT].value))
			{
				auto it = std::find(filesInPath.cbegin(), filesInPath.cend(), lastPath);
				;
				if (it == filesInPath.cend() || ++it == filesInPath.cend())
					it = filesInPath.cbegin();

				loadFx(*it);
			}

			if (prevTrigger.process(params[FX_PREV].value))
			{
				auto it = std::find(filesInPath.crbegin(), filesInPath.crend(), lastPath);

				if (it == filesInPath.crend() || ++it == filesInPath.crend())
					it = filesInPath.crbegin();

				loadFx(*it);
			}
		}

		//float deltaTime = engineGetSampleTime();
		auto inL = inputs[INPUT_L].value;
		auto inR = inputs[INPUT_R].value;
		auto outL = 0.0f;
		auto outR = 0.0f;

		auto p0 = params[POT0_PARAM].value;
		auto p1 = params[POT1_PARAM].value;
		auto p2 = params[POT2_PARAM].value;

		p0 += inputs[POT_0].value * 0.1f * params[TPOT0_PARAM].value;
		p1 += inputs[POT_1].value * 0.1f * params[TPOT1_PARAM].value;
		p2 += inputs[POT_2].value * 0.1f * params[TPOT2_PARAM].value;

		float mix = params[DRYWET_PARAM].value;
		float d = clamp(1.f - mix, 0.0f, 1.0f);
		float w = clamp(1.f + mix, 0.0f, 1.0f);

		if (w > 0)
		{
			fx.run(inL * 0.1, inR * 0.1, p0, p1, p2, outL, outR);
			outL *= 10;
			outR *= 10;
		}

		outputs[OUTPUT_L].value = clamp(inputs[INPUT_L].value * d + outL * w, -10.0f, 10.0f);
		outputs[OUTPUT_R].value = clamp(inputs[INPUT_R].value * d + outR * w, -10.0f, 10.0f);
	}

	json_t *toJson() override
	{
		json_t *rootJ = json_object();

		json_object_set_new(rootJ, "lastPath", json_string(lastPath.c_str()));
		return rootJ;
	}

	void fromJson(json_t *rootJ) override
	{
		if (json_t *lastPathJ = json_object_get(rootJ, "lastPath"))
		{
			std::string file = json_string_value(lastPathJ);
			loadFx(file);
		}
	}

	std::string display;
	std::string lastPath;
	std::vector<std::string> filesInPath;

	void loadFx(const std::string &file)
	{
		info(file.c_str());

		this->lastPath = file;
		this->fx.load(file);

		filesInPath.clear();
		auto dir = stringDirectory(this->lastPath);
		if (auto rep = opendir(dir.c_str()))
		{
			while (auto dirp = readdir(rep))
			{
				std::string name = dirp->d_name;

				std::size_t found = name.find(".spn", name.length() - 5);
				if (found == std::string::npos)
					found = name.find(".spn", name.length() - 5);

				if (found != std::string::npos)
				{
#ifdef _WIN32
					filesInPath.push_back(dir + "\\" + name);
#else
					filesInPath.push_back(dir + "/" + name);
#endif
					info(name.c_str());
				}
			}

			closedir(rep);
		}

		std::sort(filesInPath.begin(), filesInPath.end());
		auto it = std::find(filesInPath.cbegin(), filesInPath.cend(), lastPath);
		auto fxIndex = it - filesInPath.cbegin();

		display = std::to_string(fxIndex) + ": " + this->fx.getDisplay();
	}

	struct MyWidget : ModuleWidget
	{
		struct MyMenuItem : MenuItem
		{
			std::function<void()> action;

			MyMenuItem(const char *text, std::function<void()> action)
			{
				this->text = text;
				this->action = action;
			}
			void onAction(EventAction &e) override
			{
				this->action();
			}
		};

		Menu *createContextMenu() override
		{
			Menu *menu = ModuleWidget::createContextMenu();

			menu->addChild(new MenuLabel());

			auto module = dynamic_cast<FV1EmuModule *>(this->module);

			menu->addChild(new MyMenuItem("LoadFx..", [module]() {
				auto dir = module->lastPath.empty() ? assetLocal("") : stringDirectory(module->lastPath);
				auto *filters = osdialog_filters_parse("FV1-FX Asm:spn");
				char *path = osdialog_file(OSDIALOG_OPEN, dir.c_str(), NULL, filters);
				if (path)
				{
					module->loadFx(path);
					free(path);
				}
				osdialog_filters_free(filters);
			}));

			menu->addChild(new MyMenuItem("HELP...", [this]() {
				std::thread t([&]() {
					systemOpenBrowser("https://github.com/eh2k/fv1-emu/blob/master/README.md");
				});
				t.detach();
			}));

			menu->addChild(new MyMenuItem("Free DSP Programs...", [this]() {
				std::thread t([&]() {
					systemOpenBrowser("https://github.com/eh2k/fv1-emu/blob/master/README.md#free-dsp-programs");
				});
				t.detach();
			}));

			menu->addChild(new MenuLabel());

			menu->addChild(new MyMenuItem("DEBUG", [this]() {
				if (this->debugText == nullptr)
				{
					this->debugText = Widget::create<LedDisplayTextField>(Vec(box.size.x, 0));
					this->debugText->box.size = Vec(250, box.size.y);
					this->debugText->multiline = true;
					this->addChild(debugText);
				}
				else
				{
					this->removeChild(this->debugText);
					auto tmp = this->debugText;
					this->debugText = nullptr;
					delete tmp;
				}
			}));

			return menu;
		}

		struct DisplayPanel : TransparentWidget
		{
			const std::string &text;
			std::shared_ptr<Font> font;

			DisplayPanel(const Vec &pos, const Vec &size, const std::string &display) : text(display)
			{
				box.pos = pos;
				box.size = size;
				font = Font::load(assetGlobal("res/fonts/ShareTechMono-Regular.ttf"));
			}

			void draw(NVGcontext *vg) override
			{
				nvgFontSize(vg, 12);
				nvgFillColor(vg, nvgRGBAf(1, 1, 1, 1));

				std::stringstream stream(text);
				std::string line;
				int y = 11;
				while (std::getline(stream, line))
				{
					nvgText(vg, 5, y, line.c_str(), NULL);

					if (y == 11)
						y += 5;
					y += 11;
				}
			}
		};

		LedDisplay *display;

		MyWidget(FV1EmuModule *module) : ModuleWidget(module)
		{
			//setWidth(7);
			setPanel(SVG::load(assetPlugin(plugin, "res/panel.svg")));

			addChild(Widget::create<ScrewSilver>(Vec(RACK_GRID_WIDTH, 0)));
			addChild(Widget::create<ScrewSilver>(Vec(box.size.x - 2 * RACK_GRID_WIDTH, 0)));
			addChild(Widget::create<ScrewSilver>(Vec(RACK_GRID_WIDTH, RACK_GRID_HEIGHT - RACK_GRID_WIDTH)));
			addChild(Widget::create<ScrewSilver>(Vec(box.size.x - 2 * RACK_GRID_WIDTH, RACK_GRID_HEIGHT - RACK_GRID_WIDTH)));

			auto display = new DisplayPanel(Vec(12, 31), Vec(100, 50), module->display);
			addChild(display);

			addParam(ParamWidget::create<TL1105>(Vec(105, 88), module, FX_PREV, 0.0f, 1.0f, 0.0f));
			addParam(ParamWidget::create<TL1105>(Vec(130, 88), module, FX_NEXT, 0.0f, 1.0f, 0.0f));

			addParam(ParamWidget::create<RoundLargeBlackKnob>(Vec(13, 115), module, POT0_PARAM, 0, 1.0, 0.0));
			addParam(ParamWidget::create<RoundLargeBlackKnob>(Vec(64, 115), module, POT1_PARAM, 0, 1.0, 0.0));
			addParam(ParamWidget::create<RoundLargeBlackKnob>(Vec(115, 115), module, POT2_PARAM, 0, 1.0, 0.0));

			addParam(ParamWidget::create<Trimpot>(Vec(21, 169), module, TPOT0_PARAM, -1.0f, 1.0f, 0.0f));
			addParam(ParamWidget::create<Trimpot>(Vec(72, 169), module, TPOT1_PARAM, -1.0f, 1.0f, 0.0f));
			addParam(ParamWidget::create<Trimpot>(Vec(123, 169), module, TPOT2_PARAM, -1.0f, 1.0f, 0.0f));

			addInput(Port::create<PJ301MPort>(Vec(18, 202), Port::INPUT, module, POT_0));
			addInput(Port::create<PJ301MPort>(Vec(69, 202), Port::INPUT, module, POT_1));
			addInput(Port::create<PJ301MPort>(Vec(120, 202), Port::INPUT, module, POT_2));

			addParam(ParamWidget::create<RoundBlackKnob>(Vec(67, 235), module, DRYWET_PARAM, -1.0f, 1.0f, 0.0f));

			addInput(Port::create<PJ301MPort>(Vec(10, 280), Port::INPUT, module, INPUT_L));
			addInput(Port::create<PJ301MPort>(Vec(10, 320), Port::INPUT, module, INPUT_R));

			addOutput(Port::create<PJ301MPort>(Vec(box.size.x - 30, 280), Port::OUTPUT, module, OUTPUT_L));
			addOutput(Port::create<PJ301MPort>(Vec(box.size.x - 30, 320), Port::OUTPUT, module, OUTPUT_R));
		}

		TextField *debugText = nullptr;

		void draw(NVGcontext *vg) override
		{
			if (debugText)
				debugText->text = ((FV1EmuModule *)module)->fx.dumpState("\n");

			ModuleWidget::draw(vg);
		}
	};
};

} // namespace rack_plugin_EH_modules

using namespace rack_plugin_EH_modules;

RACK_PLUGIN_MODEL_INIT(EH_modules, FV1Emu) {
	auto modelMyModule = Model::create<FV1EmuModule, FV1EmuModule::MyWidget>("eh", "FV-1.emu", "FV-1.emu", EFFECT_TAG);
   return modelMyModule;
}
