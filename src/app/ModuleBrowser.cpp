#include "app/ModuleBrowser.hpp"
#include "widget/OpaqueWidget.hpp"
#include "widget/OverlayWidget.hpp"
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
	return score;
}


struct BrowserOverlay : widget::OverlayWidget {
	void step() override {
		box = parent->box.zeroPos();
		// Only step if visible, since there are potentially thousands of descendants that don't need to be stepped.
		if (visible)
			OverlayWidget::step();
	}

	void onButton(const widget::ButtonEvent &e) override {
		OverlayWidget::onButton(e);
		if (e.getConsumed() != this)
			return;

		if (e.action == GLFW_PRESS && e.button == GLFW_MOUSE_BUTTON_LEFT) {
			hide();
		}
	}
};


static const float MODEL_BOX_ZOOM = 0.5f;


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

	void onButton(const widget::ButtonEvent &e) override;

	void onEnter(const widget::EnterEvent &e) override {
		e.consume(this);
		selected = true;
	}

	void onLeave(const widget::LeaveEvent &e) override {
		selected = false;
	}
};


struct AuthorItem : ui::MenuItem {
	void onAction(const widget::ActionEvent &e) override;
};


struct TagItem : ui::MenuItem {
	void onAction(const widget::ActionEvent &e) override;
};


struct BrowserSearchField : ui::TextField {
	void step() override {
		// Steal focus when step is called
		APP->event->setSelected(this);
		TextField::step();
	}

	void onSelectKey(const widget::SelectKeyEvent &e) override {
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

	void onChange(const widget::ChangeEvent &e) override;

	void onHide(const widget::HideEvent &e) override {
		APP->event->setSelected(NULL);
		ui::TextField::onHide(e);
	}

	void onShow(const widget::ShowEvent &e) override {
		selectAll();
		TextField::onShow(e);
	}
};


struct BrowserSidebar : widget::Widget {
	BrowserSearchField *searchField;
	ui::Label *authorLabel;
	ui::List *authorList;
	ui::ScrollWidget *authorScroll;
	ui::Label *tagLabel;
	ui::List *tagList;
	ui::ScrollWidget *tagScroll;

	BrowserSidebar() {
		searchField = new BrowserSearchField;
		addChild(searchField);

		authorLabel = new ui::Label;
		authorLabel->color = nvgRGB(0x80, 0x80, 0x80);
		authorLabel->text = "Authors";
		addChild(authorLabel);

		// Plugin list
		authorScroll = new ui::ScrollWidget;
		addChild(authorScroll);

		authorList = new ui::List;
		authorScroll->container->addChild(authorList);

		std::set<std::string, string::CaseInsensitiveCompare> authorNames;
		for (plugin::Plugin *plugin : plugin::plugins) {
			authorNames.insert(plugin->author);
		}

		for (const std::string &authorName : authorNames) {
			AuthorItem *item = new AuthorItem;
			item->text = authorName;
			authorList->addChild(item);
		}

		tagLabel = new ui::Label;
		tagLabel->color = nvgRGB(0x80, 0x80, 0x80);
		tagLabel->text = "Tags";
		addChild(tagLabel);

		// Tag list
		tagScroll = new ui::ScrollWidget;
		addChild(tagScroll);

		tagList = new ui::List;
		tagScroll->container->addChild(tagList);

		for (const std::string &tag : plugin::allowedTags) {
			TagItem *item = new TagItem;
			item->text = tag;
			tagList->addChild(item);
		}
	}

	void step() override {
		float listHeight = (box.size.y - searchField->box.size.y) / 2 - authorLabel->box.size.y;
		listHeight = std::floor(listHeight);

		searchField->box.size.x = box.size.x;

		authorLabel->box.pos = searchField->box.getBottomLeft();
		authorLabel->box.size.x = box.size.x;
		authorScroll->box.pos = authorLabel->box.getBottomLeft();
		authorScroll->box.size.y = listHeight;
		authorScroll->box.size.x = box.size.x;
		authorList->box.size.x = authorScroll->box.size.x;

		tagLabel->box.pos = authorScroll->box.getBottomLeft();
		tagLabel->box.size.x = box.size.x;
		tagScroll->box.pos = tagLabel->box.getBottomLeft();
		tagScroll->box.size.y = listHeight;
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

	std::string search;
	std::string author;
	std::string tag;

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

		// Add ModelBoxes for each Model
		for (plugin::Plugin *plugin : plugin::plugins) {
			for (plugin::Model *model : plugin->models) {
				ModelBox *moduleBox = new ModelBox;
				moduleBox->setModel(model);
				modelContainer->addChild(moduleBox);
			}
		}

		refreshModels();
	}

	void step() override {
		box = parent->box.zeroPos().grow(math::Vec(-70, -70));

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

	void refreshModels() {
		// Reset scroll position
		modelScroll->offset = math::Vec();

		if (search.empty()) {
			// Make all ModelBoxes visible
			for (Widget *w : modelContainer->children) {
				w->visible = true;
			}
			// Sort by plugin name and then module name
			modelContainer->children.sort([&](Widget *w1, Widget *w2) {
				ModelBox *m1 = dynamic_cast<ModelBox*>(w1);
				ModelBox *m2 = dynamic_cast<ModelBox*>(w2);
				if (m1->model->plugin->name != m2->model->plugin->name)
					return m1->model->plugin->name < m2->model->plugin->name;
				return m1->model->name < m2->model->name;
			});
		}
		else {
			std::map<Widget*, float> scores;
			// Compute scores and filter visibility
			for (Widget *w : modelContainer->children) {
				ModelBox *m = dynamic_cast<ModelBox*>(w);
				assert(m);
				float score = modelScore(m->model, search);
				scores[m] = score;
				m->visible = (score > 0);
			}
			// Sort by score
			modelContainer->children.sort([&](Widget *w1, Widget *w2) {
				return scores[w1] > scores[w2];
			});
		}

		// Filter authors
		if (!author.empty()) {
			for (Widget *w : modelContainer->children) {
				if (!w->visible)
					continue;
				ModelBox *m = dynamic_cast<ModelBox*>(w);
				assert(m);
				if (m->model->plugin->author != author)
					m->visible = false;
			}
		}

		// Filter tags
		if (!tag.empty()) {
			for (Widget *w : modelContainer->children) {
				if (!w->visible)
					continue;
				ModelBox *m = dynamic_cast<ModelBox*>(w);
				assert(m);
				bool found = false;
				for (const std::string &tag : m->model->tags) {
					if (tag == this->tag) {
						found = true;
						break;
					}
				}
				if (!found)
					m->visible = false;
			}
		}
	}
};


// Implementations to resolve dependencies


inline void ModelBox::onButton(const widget::ButtonEvent &e) {
	OpaqueWidget::onButton(e);
	if (e.getConsumed() != this)
		return;

	if (e.action == GLFW_PRESS && e.button == GLFW_MOUSE_BUTTON_LEFT) {
		// Create module
		ModuleWidget *moduleWidget = model->createModuleWidget();
		assert(moduleWidget);
		APP->scene->rack->addModuleAtMouse(moduleWidget);

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


inline void AuthorItem::onAction(const widget::ActionEvent &e) {
	ModuleBrowser *browser = getAncestorOfType<ModuleBrowser>();
	if (browser->author == text)
		browser->author = "";
	else
		browser->author = text;
	browser->refreshModels();
}


inline void TagItem::onAction(const widget::ActionEvent &e) {
	ModuleBrowser *browser = getAncestorOfType<ModuleBrowser>();
	if (browser->tag == text)
		browser->tag = "";
	else
		browser->tag = text;
	browser->refreshModels();
}


inline void BrowserSearchField::onChange(const widget::ChangeEvent &e) {
	ModuleBrowser *browser = getAncestorOfType<ModuleBrowser>();
	browser->search = string::trim(text);
	browser->refreshModels();
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
