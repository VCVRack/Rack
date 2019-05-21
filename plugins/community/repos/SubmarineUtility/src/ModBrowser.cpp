#include "SubControls.hpp"
#include <map>
#include <algorithm>
#include "global_pre.hpp"
#include "global_ui.hpp"
#include "app.hpp"
#include "window.hpp"
#include "osdialog.h"
#include "util/common.hpp"

namespace rack_plugin_SubmarineUtility {

struct ModBrowserWidget;

struct ListElement {
	ModBrowserWidget *mbw;
	virtual void onAction(EventAction &e) { debug ("Not Implemented"); }
	virtual std::string GetLabelOne() { return std::string("Label 1"); };
	virtual std::string GetLabelTwo() { return std::string("Label 2"); };
};

struct TextButton : SubControls::ButtonBase {
	std::string label1;
	std::string label2;
	std::shared_ptr<ListElement> element;
	float label1Width = 0;
	float label2Width = 0;	
	void CalculateSizes(NVGcontext *vg, float zoom) {
		nvgFontFaceId(vg, rack::global_ui->window.gGuiFont->handle);
		nvgFontSize(vg, 13 * zoom);
		float bounds[4];
		nvgTextBounds(vg, zoom, box.size.y / 2, label1.c_str(), NULL, bounds);
		label1Width = bounds[2] - bounds[0];
		nvgTextBounds(vg, zoom, box.size.y / 2, label2.c_str(), NULL, bounds);
		label2Width = bounds[2] - bounds[0];	
	}
	void draw (NVGcontext *vg) override {
		float zoom = 1.0f / clamp(RACK_PLUGIN_UI_RACKSCENE->zoomWidget->zoom, 0.25f, 1.0f);
		//if (label1Width == 0.0f)
			CalculateSizes(vg, zoom);
		if (RACK_PLUGIN_UI_DRAGGED_WIDGET == this) {
			nvgBeginPath(vg);
			nvgRect(vg, 0, 0, box.size.x - 2, box.size.y);
			nvgFillColor(vg, nvgRGB(0x40, 0x40, 0x40));
			nvgFill(vg);
		}
		nvgFontFaceId(vg, rack::global_ui->window.gGuiFont->handle);
		nvgFontSize(vg, 13 * zoom);
	// Draw secondary text
		nvgFillColor(vg, nvgRGB(0x80, 0x80, 0x80));
		nvgTextAlign(vg, NVG_ALIGN_MIDDLE | NVG_ALIGN_RIGHT);
		nvgText(vg, box.size.x - zoom, box.size.y / 2, label2.c_str(), NULL);
	// If text overlaps, feather out overlap
		if (label1Width + label2Width > box.size.x) {
			NVGpaint grad;
			if (RACK_PLUGIN_UI_DRAGGED_WIDGET == this) {
				nvgFillColor(vg, nvgRGB(0x40, 0x40, 0x40));
				grad = nvgLinearGradient(vg, label1Width, 0, label1Width + 10, 0, nvgRGBA(0x20, 0x20, 0x20, 0xff), nvgRGBA(0x20, 0x20, 0x20, 0));
			}
			else {
				nvgFillColor(vg, nvgRGB(0, 0, 0));
				grad = nvgLinearGradient(vg, label1Width, 0, label1Width + 10, 0, nvgRGBA(0, 0, 0, 0xff), nvgRGBA(0, 0, 0, 0));
			}
			nvgBeginPath(vg);
			nvgRect(vg, box.size.x - label2Width, 0, label1Width - box.size.x + label2Width, box.size.y);
			nvgFill(vg);
			nvgBeginPath(vg);
			nvgRect(vg, label1Width, 0, 10, box.size.y);
			nvgFillPaint(vg, grad);
			nvgFill(vg); 
		}
	// Draw primary text
		nvgFillColor(vg, nvgRGB(0xff, 0xff, 0xff));
		nvgTextAlign(vg, NVG_ALIGN_MIDDLE);
		nvgText(vg, zoom, box.size.y / 2, label1.c_str(), NULL);
		Component::draw(vg);
	}
	void GetLabels() {
		label1 = element->GetLabelOne();
		label2 = element->GetLabelTwo();
	}
	void onAction(EventAction &e) override {
		element->onAction(e);
	}
};

// Icons


struct MBIconWidget : SubControls::ButtonBase,SVGWidget {
	ModBrowserWidget *mbw;
};

struct PluginIcon : MBIconWidget {
	int selected = 0;
	PluginIcon() {
		box.size.x = 30;
		box.size.y = 30;
	}
	void onAction(EventAction &e) override;
};

struct TagIcon : MBIconWidget {
	int selected = 0;
	TagIcon() {
		box.size.x = 30;
		box.size.y = 30;
	}
	void onAction(EventAction &e) override;
};

struct FavIcon : MBIconWidget {
	int selected = 0;
	FavIcon() {
		box.size.x = 30;
		box.size.y = 30;
	}
	void onAction(EventAction &e) override;
};

struct LoadIcon : MBIconWidget {
	int selected = 0;
	LoadIcon() {
		box.size.x = 30;
		box.size.y = 30;
	}
	void onAction(EventAction &e) override;
};

struct MinimizeIcon : MBIconWidget {
	MinimizeIcon() {
		box.size.x = 30;
		box.size.y = 30;
	}
	void onAction(EventAction &e) override;
};

// Elements

struct ModelElement : ListElement {
	Model *model;
	std::string GetLabelOne() override {
		return model->name;
	}
	std::string GetLabelTwo() override {
#undef plugin
		return model->plugin->slug;
#define plugin "SubmarineUtility"
	}
	void onAction(EventAction &e) override;
};

struct PluginElement : ListElement {
	std::string label;
	std::string GetLabelOne() override {
		return label;
	}
	std::string GetLabelTwo() override;
	void onAction(EventAction &e) override;
};

struct TagElement : ListElement {
	unsigned int tag;
	std::string GetLabelOne() override {
		return gTagNames[tag];
	}
	std::string GetLabelTwo() override;
	void onAction(EventAction &e) override;
};

struct PluginBackElement : ListElement {
	std::string label2;
	std::string GetLabelOne() override {
		return std::string("\xe2\x86\x90 Back");
	}
	std::string GetLabelTwo() override {
		return label2;
	}
	void onAction(EventAction &e) override;
};

struct TagBackElement : ListElement {
	std::string label2;
	std::string GetLabelOne() override {
		return std::string("\xe2\x86\x90 Back");
	}
	std::string GetLabelTwo() override {
		return label2;
	}
	void onAction(EventAction &e) override;
};

struct ModBrowserWidget : SubControls::SizeableModuleWidget {
	Widget *scrollContainer;
	ScrollWidget *scrollWidget;
	PluginIcon *pluginIcon;
	TagIcon *tagIcon;
	FavIcon *favIcon;
	LoadIcon *loadIcon;
	MinimizeIcon *minimizeIcon;
	float width;
	float zoom = 1.0f;
	std::list<std::shared_ptr<PluginElement>> pluginList;
	std::list<std::shared_ptr<TagElement>> tagList;
	std::list<std::shared_ptr<ModelElement>> modelList;
	std::string allfilters;
	std::string lastPath;
	ModBrowserWidget(Module *module) : SubControls::SizeableModuleWidget(module) {
		moduleName = "Module Browser";
		zoom = clamp(RACK_PLUGIN_UI_RACKSCENE->zoomWidget->zoom, 0.25f, 1.0f);
		allfilters.assign(PATCH_FILTERS);
		allfilters.append(";");
		allfilters.append(PRESET_FILTERS);

		pluginIcon = Widget::create<PluginIcon>(Vec(2, 2));
		pluginIcon->selected = 1;
		pluginIcon->mbw = this;
		pluginIcon->setSVG(SVG::load(assetPlugin(plugin, "res/plugin.svg")));
		backPanel->addChild(pluginIcon);

		tagIcon = Widget::create<TagIcon>(Vec(34, 2));
		tagIcon->mbw = this;
		tagIcon->setSVG(SVG::load(assetPlugin(plugin, "res/tag.svg")));
		backPanel->addChild(tagIcon);

		favIcon = Widget::create<FavIcon>(Vec(66, 2));
		favIcon->mbw = this;
		favIcon->setSVG(SVG::load(assetPlugin(plugin, "res/favorite.svg")));
		backPanel->addChild(favIcon);
	
		loadIcon = Widget::create<LoadIcon>(Vec(98, 2));
		loadIcon->mbw = this;
		loadIcon->setSVG(SVG::load(assetPlugin(plugin, "res/load.svg")));
		backPanel->addChild(loadIcon);
	
		minimizeIcon = Widget::create<MinimizeIcon>(Vec(130, 2));
		minimizeIcon->mbw = this;
		minimizeIcon->setSVG(SVG::load(assetPlugin(plugin, "res/min.svg")));
		backPanel->addChild(minimizeIcon);	

		scrollWidget = Widget::create<ScrollWidget>(Vec(0, 35));
		scrollWidget->box.size.x = box.size.x - 20;
		scrollWidget->box.size.y = box.size.y - 65;
		width = scrollWidget->box.size.x - 20;
		backPanel->addChild(scrollWidget);

		scrollContainer = scrollWidget->container;
		for (unsigned int i = 1; i < NUM_TAGS; i++) {
			std::shared_ptr<TagElement> te = std::make_shared<TagElement>();
			te->mbw = this;
			te->tag = i;
			tagList.push_back(te);
		}
		// Sort Tags (probably already sorted)
		tagList.sort([](std::shared_ptr<TagElement> te1, std::shared_ptr<TagElement> te2) { return gTagNames[te1->tag].compare(gTagNames[te2->tag]) < 0;  } );

#undef plugin
		for (Plugin *plugin : rack::global->plugin.gPlugins) {
			for (Model *model : plugin->models) {
				std::shared_ptr<ModelElement> me = std::make_shared<ModelElement>();
				me->mbw = this;
				me->model = model;
				modelList.push_back(me);
				int found = false;
				for (std::shared_ptr<PluginElement> pe : pluginList) {
					if (!pe->label.compare(me->model->author)) {
						found = true;
						break;
					}
				}
				if (!found) {
					std::shared_ptr<PluginElement> pe = std::make_shared<PluginElement>();
					pe->mbw = this;
					pe->label.assign(me->model->author);
					pluginList.push_back(pe);
				}
			}
		}
		// Sort Plugins/Authors
		pluginList.sort([](std::shared_ptr<PluginElement> pe1, std::shared_ptr<PluginElement> pe2) { return stringLowercase(pe1->label).compare(stringLowercase(pe2->label)) < 0; } );
		
		AddPlugins();
	}
	void ResetIcons() {
		pluginIcon->selected = 0;
		tagIcon->selected = 0;
		favIcon->selected = 0;
	}

	void onResize() override {
		scrollWidget->box.size.x = box.size.x - 20;
		SetListWidth();
	} 

	void SetListWidth() {
		float width = scrollContainer->parent->box.size.x;
		float size = 15.0f / zoom;
		if (scrollContainer->children.size() * size > scrollContainer->parent->box.size.y)
			width -= 13;
		float position = 0;
		for (Widget *w : scrollContainer->children) {
			w->box.pos.y = position;
			w->box.size.x = width;
			position += w->box.size.y = size;
		}
	}
	void AddElement(std::shared_ptr<ListElement> le, float y) {
		TextButton *tb = Widget::create<TextButton>(Vec(0, y));
		tb->element = le;
		tb->GetLabels();
		tb->box.size.x = width;
		tb->box.size.y = 15;
		scrollContainer->addChild(tb);
	}
	void AddPlugins() {
		scrollContainer->clearChildren();
		unsigned int y = 0;
		for (std::shared_ptr<PluginElement> pe : pluginList) {
			AddElement(pe, y);			
			y += 15;
		}
		SetListWidth();
	}
	void AddTags() {
		scrollContainer->clearChildren();
		unsigned int y = 0;
		for (std::shared_ptr<TagElement> te : tagList) {
			AddElement(te, y);
			y += 15;
		}
		SetListWidth();
	}
	void AddFavorites() {
		scrollContainer->clearChildren();
		unsigned int y = 0;
		FILE *file = fopen(assetLocal("settings.json").c_str(), "r");
		if (!file)
			return;
		json_error_t error;
		json_t *rootJ = json_loadf(file, 0, &error);
		if (!rootJ) {
			warn("JSON parsing error at %s %d:%d %s", error.source, error.line, error.column, error.text);
			fclose(file);
			return;
		}
		json_t *modb = json_object_get(rootJ, "moduleBrowser");
		if (modb) {
			json_t *favoritesJ = json_object_get(modb, "favorites");
			if (favoritesJ) {
				size_t i;
				json_t *favoriteJ;
				json_array_foreach(favoritesJ, i, favoriteJ) {
					json_t *pluginJ = json_object_get(favoriteJ, "plugin");
					json_t *modelJ = json_object_get(favoriteJ, "model");
					if (!pluginJ || !modelJ)
						continue;
					std::string pluginSlug = json_string_value(pluginJ);
					std::string modelSlug = json_string_value(modelJ);
					Model *model = pluginGetModel(pluginSlug, modelSlug);
					if (!model)
						continue;
					for (std::shared_ptr<ModelElement> me : modelList) {
						if (me->model == model) {
							AddElement(me, y);
							y += 15;
						}
					}
				}
			}
		}
		json_decref(rootJ);
		fclose(file);
		SetListWidth();
	}
	void AddModels(std::string author) {
		scrollContainer->clearChildren();
		std::shared_ptr<PluginBackElement> pbe = std::make_shared<PluginBackElement>();	
		pbe->mbw = this;
		pbe->label2 = author;
		AddElement(pbe, 0);
		unsigned int y = 15;
		for (std::shared_ptr<ModelElement> me : modelList) {
			if (!me->model->author.compare(author)) {
				AddElement(me, y);
				y += 15;
			}
		}
		SetListWidth();
	}
	void AddModels(unsigned int tag) {
		scrollContainer->clearChildren();
		std::shared_ptr<TagBackElement> tbe = std::make_shared<TagBackElement>();
		tbe->mbw = this;
		tbe->label2 = gTagNames[tag];
		AddElement(tbe, 0);
		unsigned int y = 15;
		for (std::shared_ptr<ModelElement> me : modelList) {
			for (ModelTag mt : me->model->tags) {
				if (mt == tag) {
					AddElement(me, y);
					y += 15;
				}
			}
		}	
		SetListWidth();
	}
	void Load() {
		if (lastPath.empty()) {
			if (RACK_PLUGIN_UI_RACKWIDGET->lastPath.empty()) {
				lastPath = assetLocal("patches");
				systemCreateDirectory(lastPath);
			}
			else {
				lastPath = stringDirectory(RACK_PLUGIN_UI_RACKWIDGET->lastPath);
			}
		}
		osdialog_filters *filters = osdialog_filters_parse(allfilters.c_str());
		char *path = osdialog_file(OSDIALOG_OPEN, lastPath.c_str(), NULL, filters);
			
		if (path) {
			Load(path);
			lastPath = stringDirectory(path);
			free(path);
		}
		osdialog_filters_free(filters);
	}
	void Load(std::string filename) {
		FILE *file = fopen(filename.c_str(), "r");
		if (!file) {	
			debug("Unable to open patch %s", filename.c_str());
			return;
		}

		json_error_t error;
		json_t *rootJ = json_loadf(file, 0, &error);
		if (rootJ) {
			Load(rootJ);
			json_decref(rootJ);
		}
		else {
			std::string message = stringf("JSON parsing error at %s %d:%d %s", error.source, error.line, error.column, error.text);
			osdialog_message(OSDIALOG_WARNING, OSDIALOG_OK, message.c_str());
		}		
		fclose(file);
	}
	void LoadPreset(json_t *rootJ) {
 		ModuleWidget *moduleWidget = RACK_PLUGIN_UI_RACKWIDGET->moduleFromJson(rootJ);
		if (moduleWidget) {
			moduleWidget->box.pos = RACK_PLUGIN_UI_RACKWIDGET->lastMousePos.minus(moduleWidget->box.size.div(2));
			RACK_PLUGIN_UI_RACKWIDGET->requestModuleBoxNearest(moduleWidget, moduleWidget->box);
		}
	}
	void Load(json_t *rootJ) {
		std::string message;
		Rect newBox;
		newBox.pos.x = -1;
		//load modules
		std::map<int, ModuleWidget *> moduleWidgets;
		json_t *modulesJ = json_object_get(rootJ, "modules");
		if (!modulesJ) {
			LoadPreset(rootJ);
			return;
		}
		std::vector<Widget *> existingWidgets;
		for (Widget *child : RACK_PLUGIN_UI_RACKWIDGET->moduleContainer->children) {
			existingWidgets.push_back(child);
		} 
		size_t moduleId;
		json_t *moduleJ;
		json_array_foreach(modulesJ, moduleId, moduleJ) {
			ModuleWidget *moduleWidget = RACK_PLUGIN_UI_RACKWIDGET->moduleFromJson(moduleJ);
			if (moduleWidget) {
				json_t *posJ = json_object_get(moduleJ, "pos");
				double x, y;
				json_unpack(posJ, "[F, F]", &x, &y);
				Vec pos = Vec(x,y);
				moduleWidget->box.pos = pos.mult(RACK_GRID_SIZE);
				moduleWidgets[moduleId] = moduleWidget;
				if (newBox.pos.x == -1) {
					newBox.pos.x = moduleWidget->box.pos.x;
					newBox.pos.y = moduleWidget->box.pos.y;
					newBox.size.x = moduleWidget->box.size.x;
					newBox.size.y = moduleWidget->box.size.y;
				}
				else {
					Rect mbox = moduleWidget->box;
					if (mbox.pos.x < newBox.pos.x) {
						newBox.size.x += newBox.pos.x - mbox.pos.x;
						newBox.pos.x = mbox.pos.x;
					}
					if (mbox.pos.y < newBox.pos.y) {
						newBox.size.y += newBox.pos.y - mbox.pos.y;
						newBox.pos.y = mbox.pos.y;
					}
					if ((mbox.pos.x + mbox.size.x) > (newBox.pos.x + newBox.size.x)) {
						newBox.size.x = mbox.pos.x + mbox.size.x - newBox.pos.x;
					}
					if ((mbox.pos.y + mbox.size.y) > (newBox.pos.y + newBox.size.y)) {
						newBox.size.y = mbox.pos.y + mbox.size.y - newBox.pos.y;
					}
				}
			}
		}
 
		//find space for modules and arrange
		Rect space = FindSpace(existingWidgets, newBox);
		if (space.pos.x == -1) {
			// oooh noes!!! couldn't find space for these widgets
			warn("Module browser could not find space to load patch");
			for (const auto& kvp : moduleWidgets) {
				RACK_PLUGIN_UI_RACKWIDGET->deleteModule(kvp.second);
				kvp.second->finalizeEvents();
				delete kvp.second;
			}
			return;
		}
		// Move modules into space
		float dx = space.pos.x - newBox.pos.x;
		float dy = space.pos.y - newBox.pos.y;
		for (const auto& kvp : moduleWidgets) {
			kvp.second->box.pos.x += dx;
			kvp.second->box.pos.y += dy;
		}
		//wires
		json_t *wiresJ = json_object_get(rootJ, "wires");
		if (!wiresJ) return;
		size_t wireId;
		json_t *wireJ;
		json_array_foreach(wiresJ, wireId, wireJ) {
			int outputModuleId = json_integer_value(json_object_get(wireJ, "outputModuleId"));
			int outputId = json_integer_value(json_object_get(wireJ, "outputId"));
			int inputModuleId = json_integer_value(json_object_get(wireJ, "inputModuleId"));
			int inputId = json_integer_value(json_object_get(wireJ, "inputId"));
			ModuleWidget *outputModuleWidget = moduleWidgets[outputModuleId];
			if (!outputModuleWidget) continue;
			ModuleWidget *inputModuleWidget = moduleWidgets[inputModuleId];
			if (!inputModuleWidget) continue;
			Port *outputPort = NULL;
			Port *inputPort = NULL;
			for (Port *port : outputModuleWidget->outputs) {
				if (port->portId == outputId) {
					outputPort = port;
					break;
				}
			}
			for (Port *port : inputModuleWidget->inputs) {
				if (port->portId == inputId) {
					inputPort = port;
					break;
				}
			}
			if (!outputPort || !inputPort)
				continue;
			WireWidget *wireWidget = new WireWidget();
			wireWidget->fromJson(wireJ);
			wireWidget->outputPort = outputPort;
			wireWidget->inputPort = inputPort;
			wireWidget->updateWire();
			RACK_PLUGIN_UI_RACKWIDGET->wireContainer->addChild(wireWidget);
		}

	}
	Rect FindSpace(std::vector<Widget *> existingWidgets, Rect box) {
		int x0 = roundf(box.pos.x / RACK_GRID_WIDTH);
		int y0 = roundf(box.pos.y / RACK_GRID_HEIGHT);
		std::vector<Vec> positions;
		for (int y = max(0, y0 - 8); y < y0 + 8; y++) {
			for (int x = max(0, x0 - 400); x < x0 + 400; x++) {
				positions.push_back(Vec(x * RACK_GRID_WIDTH, y * RACK_GRID_HEIGHT));
			}
		}
		std::sort(positions.begin(), positions.end(), [box](Vec a, Vec b) {
			return a.minus(box.pos).norm() < b.minus(box.pos).norm();
		});
		for (Vec position : positions) {
			Rect newBox = box;
			newBox.pos = position;
			int collide = false;
			for (Widget *child : existingWidgets) {
				if (newBox.intersects(child->box)) {
					collide = true;
					break;
				}		
			}
			if (!collide) {
				return newBox;
			}
		}
		Rect failed;
		failed.pos.x = -1;
		return failed;
	}
	void step() override {
		float thisZoom = clamp(RACK_PLUGIN_UI_RACKSCENE->zoomWidget->zoom, 0.25f, 1.0f);
		if (thisZoom != zoom) {
			zoom = thisZoom;
			SetListWidth();
		}
		stabilized = true;
		ModuleWidget::step();
	} 

};

// Icon onAction

void PluginIcon::onAction(EventAction &e) {
	mbw->ResetIcons();
	mbw->pluginIcon->selected = 1;
	mbw->AddPlugins();
}

void TagIcon::onAction(EventAction &e) {
	mbw->ResetIcons();
	mbw->tagIcon->selected = 1;
	mbw->AddTags();
}

void FavIcon::onAction(EventAction &e) {
	mbw->pluginIcon->selected = 0;
	mbw->favIcon->selected = 1;
	mbw->AddFavorites();
}

void LoadIcon::onAction(EventAction &e) {
	mbw->Load();
}

void MinimizeIcon::onAction(EventAction &e) {
	mbw->Minimize(true);
}

// Element onAction

void PluginElement::onAction(EventAction &e) {
	mbw->AddModels(label);
}

std::string PluginElement::GetLabelTwo() {
	unsigned int count = 0;
	for (std::shared_ptr<ModelElement> me : mbw->modelList) {
		if (!label.compare(me->model->author))
			count++;
	}
	return std::to_string(count).append(" Modules");
} 

void TagElement::onAction(EventAction &e) {
	mbw->AddModels(tag);
}

std::string TagElement::GetLabelTwo() {
	unsigned int count = 0;
	for (std::shared_ptr<ModelElement> me : mbw->modelList) {
		for (ModelTag mt : me->model->tags) {
			if (mt == tag) {
				count++;
			}
		}
	}
	return std::to_string(count).append(" Modules");
}

void ModelElement::onAction(EventAction &e) {
	ModuleWidget *moduleWidget = model->createModuleWidget();
	if (!moduleWidget)
		return;
	RACK_PLUGIN_UI_RACKWIDGET->addModule(moduleWidget);
	moduleWidget->box.pos = RACK_PLUGIN_UI_RACKWIDGET->lastMousePos.minus(moduleWidget->box.size.div(2));
	RACK_PLUGIN_UI_RACKWIDGET->requestModuleBoxNearest(moduleWidget, moduleWidget->box);
}

void PluginBackElement::onAction(EventAction &e) {
	mbw->AddPlugins();
}

void TagBackElement::onAction(EventAction &e) {
	mbw->AddTags();
}

struct Blank2 : ModuleWidget {
	Blank2(Module *module) : ModuleWidget(module) {
		box.size = Vec(RACK_GRID_WIDTH * 5, RACK_GRID_HEIGHT * 2);
		{
			Panel *panel = new LightPanel();
			panel->box.size = box.size;
			addChild(panel);
		}
	}
};

struct Blank3 : ModuleWidget {
	Blank3(Module *module) : ModuleWidget(module) {
		box.size = Vec(RACK_GRID_WIDTH * 5, RACK_GRID_HEIGHT * 3);
		{
			Panel *panel = new LightPanel();
			panel->box.size = box.size;
			addChild(panel);
		}
	}
};

struct Blank5 : ModuleWidget {
	Blank5(Module *module) : ModuleWidget(module) {
		box.size = Vec(RACK_GRID_WIDTH * 5, RACK_GRID_HEIGHT * 5);
		{
			Panel *panel = new LightPanel();
			panel->box.size = box.size;
			addChild(panel);
		}
	}
};

} // namespace rack_plugin_SubmarineUtility

using namespace rack_plugin_SubmarineUtility;

RACK_PLUGIN_MODEL_INIT(SubmarineUtility, ModBrowser) {
   Model *modelModBrowser = Model::create<Module, ModBrowserWidget>("Submarine (Utilities)", "ModBrowser", "Module Browser", UTILITY_TAG);
   return modelModBrowser;
}
