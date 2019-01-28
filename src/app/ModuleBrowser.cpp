#include "app/ModuleBrowser.hpp"
#include "widgets/OpaqueWidget.hpp"
#include "widgets/TransparentWidget.hpp"
#include "widgets/ZoomWidget.hpp"
#include "ui/Label.hpp"
#include "ui/MenuOverlay.hpp"
#include "app/ModuleWidget.hpp"
#include "app/Scene.hpp"
#include "plugin.hpp"
#include "app.hpp"
#include "history.hpp"

#include <set>
#include <algorithm>


namespace rack {


static std::set<Model*> sFavoriteModels;


struct ModuleBox : OpaqueWidget {
	Model *model;
	bool initialized = false;

	void setModel(Model *model) {
		this->model = model;

		box.size.x = 70.f;
		box.size.y = std::ceil(RACK_GRID_SIZE.y * 0.5f);

		math::Vec p;
		p.y = box.size.y;
		box.size.y += 40.0;

		Label *nameLabel = new Label;
		nameLabel->text = model->name;
		nameLabel->box.pos = p;
		p.y += nameLabel->box.size.y;
		addChild(nameLabel);

		Label *pluginLabel = new Label;
		pluginLabel->text = model->plugin->name;
		pluginLabel->box.pos = p;
		p.y += pluginLabel->box.size.y;
		addChild(pluginLabel);
	}

	void draw(const DrawContext &ctx) override {
		// Lazily create ModuleWidget when drawn
		if (!initialized) {
			Widget *transparentWidget = new TransparentWidget;
			addChild(transparentWidget);

			FramebufferWidget *fbWidget = new FramebufferWidget;
			if (math::isNear(app()->window->pixelRatio, 1.0)) {
				// Small details draw poorly at low DPI, so oversample when drawing to the framebuffer
				fbWidget->oversample = 2.0;
			}
			transparentWidget->addChild(fbWidget);

			ZoomWidget *zoomWidget = new ZoomWidget;
			zoomWidget->setZoom(0.5f);
			fbWidget->addChild(zoomWidget);

			ModuleWidget *moduleWidget = model->createModuleWidgetNull();
			zoomWidget->addChild(moduleWidget);

			zoomWidget->box.size.x = moduleWidget->box.size.x * zoomWidget->zoom;
			zoomWidget->box.size.y = RACK_GRID_HEIGHT;
			float width = std::ceil(zoomWidget->box.size.x);
			box.size.x = std::max(box.size.x, width);
			initialized = true;
		}

		OpaqueWidget::draw(ctx);
		if (app()->event->hoveredWidget == this) {
			nvgBeginPath(ctx.vg);
			nvgRect(ctx.vg, 0.0, 0.0, box.size.x, box.size.y);
			nvgFillColor(ctx.vg, nvgRGBAf(1, 1, 1, 0.25));
			nvgFill(ctx.vg);
		}
	}

	void onButton(const event::Button &e) override {
		if (e.action == GLFW_PRESS && e.button == GLFW_MOUSE_BUTTON_LEFT) {
			// Create module
			ModuleWidget *moduleWidget = model->createModuleWidget();
			assert(moduleWidget);
			app()->scene->rackWidget->addModuleAtMouse(moduleWidget);
			// This is a bit nonstandard/unsupported usage, but pretend the moduleWidget was clicked so it can be dragged in the RackWidget
			// e.consume(moduleWidget);
			// Close Module Browser
			ModuleBrowser *moduleBrowser = getAncestorOfType<ModuleBrowser>();
			moduleBrowser->visible = false;

			// Push ModuleAdd history action
			history::ModuleAdd *h = new history::ModuleAdd;
			h->setModule(moduleWidget);
			app()->history->push(h);
		}
		OpaqueWidget::onButton(e);
	}
};


ModuleBrowser::ModuleBrowser() {
	moduleScroll = new ScrollWidget;
	addChild(moduleScroll);

	moduleLayout = new SequentialLayout;
	moduleLayout->spacing = math::Vec(10, 10);
	moduleScroll->container->addChild(moduleLayout);

	for (Plugin *plugin : plugin::plugins) {
		for (Model *model : plugin->models) {
			ModuleBox *moduleBox = new ModuleBox;
			moduleBox->setModel(model);
			moduleLayout->addChild(moduleBox);
		}
	}
}

void ModuleBrowser::step() {
	// TODO resize sidebar
	float sidebarWidth = 300.0;

	moduleScroll->box.pos.x = sidebarWidth;
	moduleScroll->box.size.x = box.size.x - sidebarWidth;
	moduleScroll->box.size.y = box.size.y;
	moduleLayout->box.size.x = moduleScroll->box.size.x;
	moduleLayout->box.size.y = moduleLayout->getChildrenBoundingBox().getBottomRight().y;

	OpaqueWidget::step();
}

void ModuleBrowser::draw(const DrawContext &ctx) {
	bndMenuBackground(ctx.vg, 0.0, 0.0, box.size.x, box.size.y, 0);
	Widget::draw(ctx);
}

void ModuleBrowser::onHoverKey(const event::HoverKey &e) {
	if (e.action == GLFW_PRESS) {
		switch (e.key) {
			case GLFW_KEY_ESCAPE: {
				// Close menu
				this->visible = false;
				e.consume(this);
			} break;
		}
	}

	if (!e.getConsumed())
		OpaqueWidget::onHoverKey(e);
}



// Global functions

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
