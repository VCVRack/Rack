#include "app/ModuleBrowser.hpp"
#include "widget/OpaqueWidget.hpp"
#include "widget/TransparentWidget.hpp"
#include "widget/ZoomWidget.hpp"
#include "ui/ScrollWidget.hpp"
#include "ui/SequentialLayout.hpp"
#include "ui/MarginLayout.hpp"
#include "ui/Label.hpp"
#include "ui/TextField.hpp"
#include "ui/MenuOverlay.hpp"
#include "ui/List.hpp"
#include "ui/MenuItem.hpp"
#include "ui/Button.hpp"
#include "ui/ChoiceButton.hpp"
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


static float modelScore(plugin::Model *model, const std::string &search) {
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
	// for (const std::string &tag : model->tags) {
	// 	s += " ";
	// 	s += tag;
	// }
	float score = string::fuzzyScore(s, search);
	DEBUG("%s %f", s.c_str(), score);
	return score;
}


struct BrowserOverlay : widget::OpaqueWidget {
	void step() override {
		box = parent->box.zeroPos();
		// Only step if visible, since there are potentially thousands of descendants that don't need to be stepped.
		if (visible)
			OpaqueWidget::step();
	}

	void onButton(const event::Button &e) override {
		OpaqueWidget::onButton(e);
		if (e.getConsumed() != this)
			return;

		if (e.action == GLFW_PRESS && e.button == GLFW_MOUSE_BUTTON_LEFT) {
			hide();
		}
	}
};


static const float MODEL_BOX_ZOOM = 1.0f;


struct ModelBox : widget::OpaqueWidget {
	plugin::Model *model;
	widget::Widget *infoWidget;
	/** Lazily created */
	widget::Widget *previewWidget = NULL;
	/** Number of frames since draw() has been called */
	int visibleFrames = 0;
	bool selected = false;

	ModelBox() {
		// Approximate size as 10HP before we know the actual size.
		// We need a nonzero size, otherwise the parent widget will consider it not in the draw bounds, so its preview will not be lazily created.
		box.size.x = 10 * RACK_GRID_WIDTH * MODEL_BOX_ZOOM;
		box.size.y = RACK_GRID_HEIGHT * MODEL_BOX_ZOOM;
		box.size = box.size.ceil();
	}

	void setModel(plugin::Model *model) {
		this->model = model;

		infoWidget = new widget::Widget;
		infoWidget->hide();
		addChild(infoWidget);

		math::Vec pos;

		// Name label
		ui::Label *nameLabel = new ui::Label;
		// nameLabel->box.size.x = infoWidget->box.size.x;
		nameLabel->box.pos = pos;
		nameLabel->text = model->name;
		infoWidget->addChild(nameLabel);
		pos = nameLabel->box.getBottomLeft();

		// Plugin label
		ui::Label *pluginLabel = new ui::Label;
		// pluginLabel->box.size.x = infoWidget->box.size.x;
		pluginLabel->box.pos = pos;
		pluginLabel->text = model->plugin->name;
		infoWidget->addChild(pluginLabel);
		pos = pluginLabel->box.getBottomLeft();

		ui::Label *descriptionLabel = new ui::Label;
		descriptionLabel->box.size.x = infoWidget->box.size.x;
		descriptionLabel->box.pos = pos;
		descriptionLabel->text = model->description;
		infoWidget->addChild(descriptionLabel);
		pos = descriptionLabel->box.getBottomLeft();

		// for (const std::string &tag : model->tags) {
		// 	ui::Button *tagButton = new ui::Button;
		// 	tagButton->box.size.x = infoWidget->box.size.x;
		// 	tagButton->box.pos = pos;
		// 	tagButton->text = tag;
		// 	infoWidget->addChild(tagButton);
		// 	pos = tagButton->box.getTopLeft();
		// }

		// // Favorite button
		// ui::Button *favoriteButton = new ui::Button;
		// favoriteButton->box.size.x = box.size.x;
		// favoriteButton->box.pos = pos;
		// favoriteButton->box.pos.y -= favoriteButton->box.size.y;
		// favoriteButton->text = "★";
		// addChild(favoriteButton);
		// pos = favoriteButton->box.getTopLeft();
	}

	void createPreview() {
		assert(!previewWidget);
		previewWidget = new widget::TransparentWidget;
		previewWidget->box.size.y = std::ceil(RACK_GRID_HEIGHT * MODEL_BOX_ZOOM);
		addChild(previewWidget);

		widget::FramebufferWidget *fbWidget = new widget::FramebufferWidget;
		if (math::isNear(APP->window->pixelRatio, 1.0)) {
			// Small details draw poorly at low DPI, so oversample when drawing to the framebuffer
			fbWidget->oversample = 2.0;
		}
		previewWidget->addChild(fbWidget);

		widget::ZoomWidget *zoomWidget = new widget::ZoomWidget;
		zoomWidget->setZoom(MODEL_BOX_ZOOM);
		fbWidget->addChild(zoomWidget);

		ModuleWidget *moduleWidget = model->createModuleWidgetNull();
		zoomWidget->addChild(moduleWidget);

		zoomWidget->box.size.x = moduleWidget->box.size.x * MODEL_BOX_ZOOM;
		zoomWidget->box.size.y = RACK_GRID_HEIGHT * MODEL_BOX_ZOOM;
		previewWidget->box.size.x = std::ceil(zoomWidget->box.size.x);

		infoWidget->box.size = previewWidget->box.size;
		box.size.x = previewWidget->box.size.x;
	}

	void deletePreview() {
		assert(previewWidget);
		removeChild(previewWidget);
		delete previewWidget;
		previewWidget = NULL;
	}

	void step() override {
		if (previewWidget && ++visibleFrames >= 60) {
			deletePreview();
		}
		OpaqueWidget::step();
	}

	void draw(const DrawArgs &args) override {
		visibleFrames = 0;

		// Lazily create preview when drawn
		if (!previewWidget) {
			createPreview();
		}

		// Draw shadow
		nvgBeginPath(args.vg);
		float r = 10; // Blur radius
		float c = 10; // Corner radius
		nvgRect(args.vg, -r, -r, box.size.x + 2*r, box.size.y + 2*r);
		NVGcolor shadowColor = nvgRGBAf(0, 0, 0, 0.5);
		NVGcolor transparentColor = nvgRGBAf(0, 0, 0, 0);
		nvgFillPaint(args.vg, nvgBoxGradient(args.vg, 0, 0, box.size.x, box.size.y, c, r, shadowColor, transparentColor));
		nvgFill(args.vg);

		nvgScissor(args.vg, RECT_ARGS(args.clipBox));
		OpaqueWidget::draw(args);

		// Translucent overlay when selected
		if (selected) {
			nvgBeginPath(args.vg);
			nvgRect(args.vg, 0.0, 0.0, box.size.x, box.size.y);
			nvgFillColor(args.vg, nvgRGBAf(1, 1, 1, 0.25));
			nvgFill(args.vg);
		}

		nvgResetScissor(args.vg);
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
	void step() override {
		// Steal focus when step is called
		APP->event->setSelected(this);
		TextField::step();
	}
	void onSelectKey(const event::SelectKey &e) override {
		if (e.action == GLFW_PRESS) {
			if (e.key == GLFW_KEY_ESCAPE) {
				BrowserOverlay *overlay = getAncestorOfType<BrowserOverlay>();
				overlay->hide();
				e.consume(this);
			}
		}

		if (!e.getConsumed())
			ui::TextField::onSelectKey(e);
	}
	void onChange(const event::Change &e) override;
	void onHide(const event::Hide &e) override {
		setText("");
		APP->event->setSelected(NULL);
		ui::TextField::onHide(e);
	}
};


struct BrowserSidebar : widget::Widget {
	BrowserSearchField *searchField;
	ui::List *pluginList;
	ui::ScrollWidget *pluginScroll;
	ui::List *tagList;
	ui::ScrollWidget *tagScroll;

	BrowserSidebar() {
		searchField = new BrowserSearchField;
		addChild(searchField);

		// Plugin list
		pluginScroll = new ui::ScrollWidget;
		addChild(pluginScroll);

		pluginList = new ui::List;
		pluginScroll->container->addChild(pluginList);

		std::vector<std::string> pluginNames;
		for (plugin::Plugin *plugin : plugin::plugins) {
			pluginNames.push_back(plugin->name);
		}
		std::sort(pluginNames.begin(), pluginNames.end(), string::CaseInsensitiveCompare());

		for (const std::string &pluginName : pluginNames) {
			ui::MenuItem *item = new ui::MenuItem;
			item->text = pluginName;
			pluginList->addChild(item);
		}

		// Tag list
		tagScroll = new ui::ScrollWidget;
		addChild(tagScroll);

		tagList = new ui::List;
		tagScroll->container->addChild(tagList);

		for (const std::string &tag : plugin::allowedTags) {
			ui::MenuItem *item = new ui::MenuItem;
			item->text = tag;
			tagList->addChild(item);
		}
	}

	void step() override {
		searchField->box.size.x = box.size.x;
		pluginScroll->box.pos = searchField->box.getBottomLeft();
		pluginScroll->box.size.y = (box.size.y - searchField->box.size.y) / 2;
		pluginScroll->box.size.x = box.size.x;
		pluginList->box.size.x = pluginScroll->box.size.x;
		tagScroll->box.pos = pluginScroll->box.getBottomLeft().floor();
		tagScroll->box.size.y = (box.size.y - searchField->box.size.y) / 2;
		tagScroll->box.size.x = box.size.x;
		tagList->box.size.x = tagScroll->box.size.x;
		Widget::step();
	}
};


struct ModuleBrowser : widget::OpaqueWidget {
	BrowserSidebar *sidebar;
	ui::ScrollWidget *modelScroll;
	ui::MarginLayout *modelMargin;
	ui::SequentialLayout *modelContainer;

	ModuleBrowser() {
		sidebar = new BrowserSidebar;
		sidebar->box.size.x = 200;
		addChild(sidebar);

		modelScroll = new ui::ScrollWidget;
		addChild(modelScroll);

		modelMargin = new ui::MarginLayout;
		modelMargin->margin = math::Vec(10, 10);
		modelScroll->container->addChild(modelMargin);

		modelContainer = new ui::SequentialLayout;
		modelContainer->spacing = math::Vec(10, 10);
		modelMargin->addChild(modelContainer);

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
		modelMargin->box.size.x = modelScroll->box.size.x;
		modelMargin->box.size.y = modelContainer->getChildrenBoundingBox().size.y + 2 * modelMargin->margin.y;

		OpaqueWidget::step();
	}

	void draw(const DrawArgs &args) override {
		bndMenuBackground(args.vg, 0.0, 0.0, box.size.x, box.size.y, 0);
		Widget::draw(args);
	}

	void setSearch(const std::string &search) {
		std::string searchTrimmed = string::trim(search);
		std::map<const Widget*, float> scores;
		// Compute scores and set visibility
		for (Widget *w : modelContainer->children) {
			ModelBox *modelBox = dynamic_cast<ModelBox*>(w);
			assert(modelBox);
			float score = modelScore(modelBox->model, searchTrimmed);
			scores[modelBox] = score;
			modelBox->visible = (score > 0);
		}
		DEBUG("");
		// Sort by score
		modelContainer->children.sort([&](const Widget *w1, const Widget *w2) {
			return scores[w1] > scores[w2];
		});
		// Reset scroll position
		modelScroll->offset = math::Vec();
	}
};


// Implementations to resolve dependencies


void ModelBox::onButton(const event::Button &e) {
	OpaqueWidget::onButton(e);
	if (e.getConsumed() != this)
		return;

	if (e.action == GLFW_PRESS && e.button == GLFW_MOUSE_BUTTON_LEFT) {
		// Create module
		ModuleWidget *moduleWidget = model->createModuleWidget();
		assert(moduleWidget);
		APP->scene->rackWidget->addModuleAtMouse(moduleWidget);

		// Pretend the moduleWidget was clicked so it can be dragged in the RackWidget
		e.consume(moduleWidget);

		// Hide Module Browser
		BrowserOverlay *overlay = getAncestorOfType<BrowserOverlay>();
		overlay->hide();

		// Push ModuleAdd history action
		history::ModuleAdd *h = new history::ModuleAdd;
		h->name = "create module";
		h->setModule(moduleWidget);
		APP->history->push(h);
	}
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
