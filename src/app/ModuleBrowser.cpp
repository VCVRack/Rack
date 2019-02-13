#include "app/ModuleBrowser.hpp"
#include "widget/OpaqueWidget.hpp"
#include "widget/TransparentWidget.hpp"
#include "widget/ZoomWidget.hpp"
#include "ui/ScrollWidget.hpp"
#include "ui/SequentialLayout.hpp"
#include "ui/Label.hpp"
#include "ui/TextField.hpp"
#include "ui/MenuOverlay.hpp"
#include "app/ModuleWidget.hpp"
#include "app/Scene.hpp"
#include "plugin.hpp"
#include "app.hpp"
#include "plugin/Model.hpp"
#include "string.hpp"
#include "history.hpp"

#include <set>
#include <algorithm>


namespace rack {
namespace app {


static std::set<plugin::Model*> sFavoriteModels;


bool isMatch(const std::string &s, const std::string &search) {
	std::string s2 = string::lowercase(s);
	std::string search2 = string::lowercase(search);
	return (s2.find(search2) != std::string::npos);
}

static bool isModelMatch(plugin::Model *model, const std::string &search) {
	if (search.empty())
		return true;
	std::string s;
	s += model->plugin->slug;
	s += " ";
	s += model->plugin->author;
	s += " ";
	s += model->plugin->name;
	s += " ";
	s += model->slug;
	s += " ";
	s += model->name;
	for (const std::string &tag : model->tags) {
		// TODO Normalize tag
		s += tag;
		s += " ";
	}
	return isMatch(s, search);
}


struct BrowserOverlay : widget::OpaqueWidget {
	void step() override {
		box = parent->box.zeroPos();
		// Only step if visible, since there are potentially thousands of descendants that don't need to be stepped.
		if (visible)
			widget::OpaqueWidget::step();
	}

	void onButton(const event::Button &e) override {
		widget::OpaqueWidget::onButton(e);
		if (e.getConsumed() != this)
			return;

		if (e.action == GLFW_PRESS && e.button == GLFW_MOUSE_BUTTON_LEFT) {
			this->visible = false;
		}
	}

	void onHoverKey(const event::HoverKey &e) override {
		if (e.action == GLFW_PRESS) {
			switch (e.key) {
				case GLFW_KEY_ESCAPE: {
					this->visible = false;
					e.consume(this);
				} break;
			}
		}

		if (!e.getConsumed())
			widget::OpaqueWidget::onHoverKey(e);
	}
};


struct ModelBox : widget::OpaqueWidget {
	plugin::Model *model;
	/** Lazily created */
	widget::Widget *previewWidget = NULL;
	/** Number of frames since draw() has been called */
	int visibleFrames = 0;
	bool selected = false;

	void setModel(plugin::Model *model) {
		this->model = model;

		box.size.x = 70.f;
		box.size.y = std::ceil(RACK_GRID_SIZE.y * 0.5f);

		math::Vec p;
		p.y = box.size.y;
		box.size.y += 40.0;

		ui::Label *nameLabel = new ui::Label;
		nameLabel->text = model->name;
		nameLabel->box.pos = p;
		p.y += nameLabel->box.size.y;
		addChild(nameLabel);

		ui::Label *pluginLabel = new ui::Label;
		pluginLabel->text = model->plugin->name;
		pluginLabel->box.pos = p;
		p.y += pluginLabel->box.size.y;
		addChild(pluginLabel);
	}

	void step() override {
		if (previewWidget && ++visibleFrames >= 60) {
			removeChild(previewWidget);
			delete previewWidget;
			previewWidget = NULL;
		}
	}

	void draw(const widget::DrawContext &ctx) override {
		visibleFrames = 0;

		// Lazily create ModuleWidget when drawn
		if (!previewWidget) {
			widget::Widget *transparentWidget = new widget::TransparentWidget;
			addChild(transparentWidget);

			widget::FramebufferWidget *fbWidget = new widget::FramebufferWidget;
			if (math::isNear(APP->window->pixelRatio, 1.0)) {
				// Small details draw poorly at low DPI, so oversample when drawing to the framebuffer
				fbWidget->oversample = 2.0;
			}
			transparentWidget->addChild(fbWidget);

			widget::ZoomWidget *zoomWidget = new widget::ZoomWidget;
			zoomWidget->setZoom(0.5f);
			fbWidget->addChild(zoomWidget);

			ModuleWidget *moduleWidget = model->createModuleWidgetNull();
			zoomWidget->addChild(moduleWidget);

			zoomWidget->box.size.x = moduleWidget->box.size.x * zoomWidget->zoom;
			zoomWidget->box.size.y = RACK_GRID_HEIGHT;
			float width = std::ceil(zoomWidget->box.size.x);
			box.size.x = std::max(box.size.x, width);

			previewWidget = transparentWidget;
		}

		widget::OpaqueWidget::draw(ctx);
		if (selected) {
			nvgBeginPath(ctx.vg);
			nvgRect(ctx.vg, 0.0, 0.0, box.size.x, box.size.y);
			nvgFillColor(ctx.vg, nvgRGBAf(1, 1, 1, 0.25));
			nvgFill(ctx.vg);
		}
	}

	void onButton(const event::Button &e) override;

	void onEnter(const event::Enter &e) override {
		e.consume(this);
		selected = true;
	}

	void onLeave(const event::Leave &e) override {
		selected = false;
	}
};


struct BrowserSearchField : ui::TextField {
	void onChange(const event::Change &e) override;
};


struct BrowserSidebar : widget::Widget {
	BrowserSearchField *searchField;

	BrowserSidebar() {
		searchField = new BrowserSearchField;
		addChild(searchField);
	}

	void step() override {
		searchField->box.size.x = box.size.x;
		widget::Widget::step();
	}
};


struct ModuleBrowser : widget::OpaqueWidget {
	BrowserSidebar *sidebar;
	ui::ScrollWidget *modelScroll;
	ui::SequentialLayout *modelContainer;

	ModuleBrowser() {
		sidebar = new BrowserSidebar;
		sidebar->box.size.x = 300;
		addChild(sidebar);

		modelScroll = new ui::ScrollWidget;
		addChild(modelScroll);

		modelContainer = new ui::SequentialLayout;
		modelContainer->spacing = math::Vec(10, 10);
		modelScroll->container->addChild(modelContainer);

		for (plugin::Plugin *plugin : plugin::plugins) {
			for (plugin::Model *model : plugin->models) {
				ModelBox *moduleBox = new ModelBox;
				moduleBox->setModel(model);
				modelContainer->addChild(moduleBox);
			}
		}
	}

	void step() override {
		box = parent->box.zeroPos().grow(math::Vec(-50, -50));

		sidebar->box.size.y = box.size.y;

		modelScroll->box.pos.x = sidebar->box.size.x;
		modelScroll->box.size.x = box.size.x - sidebar->box.size.x;
		modelScroll->box.size.y = box.size.y;
		modelContainer->box.size.x = modelScroll->box.size.x;
		modelContainer->box.size.y = modelContainer->getChildrenBoundingBox().getBottomRight().y;

		widget::OpaqueWidget::step();
	}

	void draw(const widget::DrawContext &ctx) override {
		bndMenuBackground(ctx.vg, 0.0, 0.0, box.size.x, box.size.y, 0);
		widget::Widget::draw(ctx);
	}

	void setSearch(const std::string &search) {
		for (Widget *w : modelContainer->children) {
			ModelBox *modelBox = dynamic_cast<ModelBox*>(w);
			assert(modelBox);
			bool match = isModelMatch(modelBox->model, search);
			modelBox->visible = match;
		}
	}
};


// Implementations to resolve dependencies


void ModelBox::onButton(const event::Button &e) {
	if (e.action == GLFW_PRESS && e.button == GLFW_MOUSE_BUTTON_LEFT) {
		// Create module
		ModuleWidget *moduleWidget = model->createModuleWidget();
		assert(moduleWidget);
		APP->scene->rackWidget->addModuleAtMouse(moduleWidget);

		// Pretend the moduleWidget was clicked so it can be dragged in the RackWidget
		e.consume(moduleWidget);

		// Close Module Browser
		BrowserOverlay *overlay = getAncestorOfType<BrowserOverlay>();
		overlay->visible = false;

		// Push ModuleAdd history action
		history::ModuleAdd *h = new history::ModuleAdd;
		h->name = "create module";
		h->setModule(moduleWidget);
		APP->history->push(h);
	}
	widget::OpaqueWidget::onButton(e);
}

void BrowserSearchField::onChange(const event::Change &e) {
	ModuleBrowser *browser = getAncestorOfType<ModuleBrowser>();
	browser->setSearch(text);
}



// Global functions


widget::Widget *moduleBrowserCreate() {
	BrowserOverlay *overlay = new BrowserOverlay;

	ModuleBrowser *browser = new ModuleBrowser;
	overlay->addChild(browser);

	return overlay;
}

json_t *moduleBrowserToJson() {
	json_t *rootJ = json_object();

	json_t *favoritesJ = json_array();
	for (plugin::Model *model : sFavoriteModels) {
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
			plugin::Model *model = plugin::getModel(pluginSlug, modelSlug);
			if (!model)
				continue;
			sFavoriteModels.insert(model);
		}
	}
}


} // namespace app
} // namespace rack
