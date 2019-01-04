#include <set>
#include <algorithm>
#include "window.hpp"
#include "helpers.hpp"
#include "event.hpp"
#include "ui/Quantity.hpp"
#include "ui/RadioButton.hpp"
#include "ui/Label.hpp"
#include "app/ModuleBrowser.hpp"
#include "app/Scene.hpp"
#include "ui/List.hpp"
#include "ui/TextField.hpp"
#include "widgets/ObstructWidget.hpp"
#include "widgets/ZoomWidget.hpp"
#include "plugin.hpp"
#include "context.hpp"


namespace rack {


static std::set<Model*> sFavoriteModels;
static std::string sAuthorFilter;
static std::string sTagFilter;



struct ModuleWidgetWrapper : ObstructWidget {
	Model *model;

	void onDragDrop(const event::DragDrop &e) override {
		if (e.origin == this) {
			// Create module
			ModuleWidget *moduleWidget = model->createModuleWidget();
			assert(moduleWidget);
			context()->scene->rackWidget->addModuleAtMouse(moduleWidget);
			// Close Module Browser
			MenuOverlay *menuOverlay = getAncestorOfType<MenuOverlay>();
			menuOverlay->requestedDelete = true;
		}
	}
};


struct ModuleBrowser : OpaqueWidget {
	ModuleBrowser() {
		math::Vec p;
		for (Plugin *plugin : plugin::plugins) {
			for (Model *model : plugin->models) {
				ModuleWidgetWrapper *wrapper = new ModuleWidgetWrapper;
				wrapper->box.pos = p;
				wrapper->model = model;
				addChild(wrapper);

				ZoomWidget *zoomWidget = new ZoomWidget;
				zoomWidget->setZoom(0.5);
				wrapper->addChild(zoomWidget);

				ModuleWidget *moduleWidget = model->createModuleWidgetNull();
				zoomWidget->addChild(moduleWidget);
				wrapper->box.size = moduleWidget->box.size.mult(zoomWidget->zoom);
				p = wrapper->box.getTopRight().plus(math::Vec(20, 0));
			}
		}
	}

	void step() override {
		assert(parent);

		box = parent->box.zeroPos().grow(math::Vec(-100, -100));

		OpaqueWidget::step();
	}

	void draw(NVGcontext *vg) override {
		bndTooltipBackground(vg, 0.0, 0.0, box.size.x, box.size.y);
		Widget::draw(vg);
	}
};



// Global functions

void moduleBrowserCreate() {
	MenuOverlay *overlay = new MenuOverlay;

	ModuleBrowser *moduleBrowser = new ModuleBrowser;
	overlay->addChild(moduleBrowser);

	context()->scene->addChild(overlay);
}

json_t *moduleBrowserToJson() {
	json_t *rootJ = json_object();

	json_t *favoritesJ = json_array();
	for (Model *model : sFavoriteModels) {
		json_t *modelJ = json_object();
		json_object_set_new(modelJ, "plugin", json_string(model->plugin->slug.c_str()));
		json_object_set_new(modelJ, "model", json_string(model->slug.c_str()));
		json_array_append_new(favoritesJ, modelJ);
	}
	json_object_set_new(rootJ, "favorites", favoritesJ);

	return rootJ;
}

void moduleBrowserFromJson(json_t *rootJ) {
	json_t *favoritesJ = json_object_get(rootJ, "favorites");
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
			Model *model = plugin::getModel(pluginSlug, modelSlug);
			if (!model)
				continue;
			sFavoriteModels.insert(model);
		}
	}
}


} // namespace rack
