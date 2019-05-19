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
#include "ui/RadioButton.hpp"
#include "ui/ChoiceButton.hpp"
#include "ui/Tooltip.hpp"
#include "app/ModuleWidget.hpp"
#include "app/Scene.hpp"
#include "plugin.hpp"
#include "app.hpp"
#include "plugin/Model.hpp"
#include "string.hpp"
#include "history.hpp"
#include "settings.hpp"

#include <set>
#include <algorithm>


namespace rack {
namespace app {


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


struct BrowserOverlay : widget::OpaqueWidget {
	void step() override {
		box = parent->box.zeroPos();
		// Only step if visible, since there are potentially thousands of descendants that don't need to be stepped.
		if (visible)
			OpaqueWidget::step();
	}

	void onButton(const event::Button &e) override {
		OpaqueWidget::onButton(e);
		if (e.getTarget() != this)
			return;

		if (e.action == GLFW_PRESS && e.button == GLFW_MOUSE_BUTTON_LEFT) {
			hide();
			e.consume(this);
		}
	}
};


struct ModelFavoriteQuantity : Quantity {
	plugin::Model *model;
	std::string getLabel() override {return "★";}
	void setValue(float value) override {
		if (value) {
			settings::favoriteModels.insert(model);
		}
		else {
			auto it = settings::favoriteModels.find(model);
			if (it != settings::favoriteModels.end())
				settings::favoriteModels.erase(it);
		}
	}
	float getValue() override {
		auto it = settings::favoriteModels.find(model);
		return (it != settings::favoriteModels.end());
	}
};


struct ModelFavoriteButton : ui::RadioButton {
	ModelFavoriteButton() {
		quantity = new ModelFavoriteQuantity;
	}
	~ModelFavoriteButton() {
		delete quantity;
	}
};


static const float MODEL_BOX_ZOOM = 0.5f;


struct ModelBox : widget::OpaqueWidget {
	plugin::Model *model;
	widget::Widget *previewWidget;
	ModelFavoriteButton *favoriteButton;
	ui::Tooltip *tooltip = NULL;
	/** Lazily created */
	widget::FramebufferWidget *previewFb = NULL;
	/** Number of frames since draw() has been called */
	int visibleFrames = 0;

	ModelBox() {
		// Approximate size as 10HP before we know the actual size.
		// We need a nonzero size, otherwise the parent widget will consider it not in the draw bounds, so its preview will not be lazily created.
		box.size.x = 10 * RACK_GRID_WIDTH * MODEL_BOX_ZOOM;
		box.size.y = RACK_GRID_HEIGHT * MODEL_BOX_ZOOM;
		box.size = box.size.ceil();
	}

	void setModel(plugin::Model *model) {
		this->model = model;

		previewWidget = new widget::TransparentWidget;
		previewWidget->box.size.y = std::ceil(RACK_GRID_HEIGHT * MODEL_BOX_ZOOM);
		addChild(previewWidget);

		// Favorite button
		favoriteButton = new ModelFavoriteButton;
		dynamic_cast<ModelFavoriteQuantity*>(favoriteButton->quantity)->model = model;
		favoriteButton->box.pos.y = box.size.y;
		box.size.y += favoriteButton->box.size.y;
		addChild(favoriteButton);
	}

	void createPreview() {
		previewFb = new widget::FramebufferWidget;
		if (math::isNear(APP->window->pixelRatio, 1.0)) {
			// Small details draw poorly at low DPI, so oversample when drawing to the framebuffer
			previewFb->oversample = 2.0;
		}
		previewWidget->addChild(previewFb);

		widget::ZoomWidget *zoomWidget = new widget::ZoomWidget;
		zoomWidget->setZoom(MODEL_BOX_ZOOM);
		previewFb->addChild(zoomWidget);

		ModuleWidget *moduleWidget = model->createModuleWidgetNull();
		zoomWidget->addChild(moduleWidget);

		zoomWidget->box.size.x = moduleWidget->box.size.x * MODEL_BOX_ZOOM;
		zoomWidget->box.size.y = RACK_GRID_HEIGHT * MODEL_BOX_ZOOM;
		previewWidget->box.size.x = std::ceil(zoomWidget->box.size.x);

		favoriteButton->box.size.x = previewWidget->box.size.x;
		box.size.x = previewWidget->box.size.x;
	}

	void deletePreview() {
		assert(previewFb);
		previewWidget->removeChild(previewFb);
		delete previewFb;
		previewFb = NULL;
	}

	void step() override {
		if (previewFb && ++visibleFrames >= 60) {
			deletePreview();
		}
		OpaqueWidget::step();
	}

	void draw(const DrawArgs &args) override {
		visibleFrames = 0;

		// Lazily create preview when drawn
		if (!previewFb) {
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

		OpaqueWidget::draw(args);
	}

	void setTooltip(ui::Tooltip *tooltip) {
		if (this->tooltip) {
			this->tooltip->parent->removeChild(this->tooltip);
			delete this->tooltip;
			this->tooltip = NULL;
		}

		if (tooltip) {
			APP->scene->addChild(tooltip);
			this->tooltip = tooltip;
		}
	}

	void onButton(const event::Button &e) override;

	void onEnter(const event::Enter &e) override {
		ui::Tooltip *tooltip = new ui::Tooltip;
		tooltip->text = model->plugin->name;
		tooltip->text += " " + model->name;
		if (model->description != "")
			tooltip->text += "\n" + model->description;
		setTooltip(tooltip);
	}

	void onLeave(const event::Leave &e) override {
		setTooltip(NULL);
	}
};


struct AuthorItem : ui::MenuItem {
	void onAction(const event::Action &e) override;
	void step() override;
};


struct TagItem : ui::MenuItem {
	void onAction(const event::Action &e) override;
	void step() override;
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
				if (text != "") {
					setText("");
				}
				else {
					BrowserOverlay *overlay = getAncestorOfType<BrowserOverlay>();
					overlay->hide();
				}
				e.consume(this);
			}
		}

		if (!e.getTarget())
			ui::TextField::onSelectKey(e);
	}

	void onChange(const event::Change &e) override;

	void onHide(const event::Hide &e) override {
		APP->event->setSelected(NULL);
		ui::TextField::onHide(e);
	}

	void onShow(const event::Show &e) override {
		selectAll();
		TextField::onShow(e);
	}
};


struct ClearButton : ui::Button {
	void onAction(const event::Action &e) override;
};


struct ShowFavoritesQuantity : Quantity {
	widget::Widget *widget;
	std::string getLabel() override {
		int favoritesLen = settings::favoriteModels.size();
		return string::f("Only show favorites (%d)", favoritesLen);
	}
	void setValue(float value) override;
	float getValue() override;
};


struct ShowFavoritesButton : ui::RadioButton {
	ShowFavoritesButton() {
		quantity = new ShowFavoritesQuantity;
	}
	~ShowFavoritesButton() {
		delete quantity;
	}
};


struct BrowserSidebar : widget::Widget {
	BrowserSearchField *searchField;
	ClearButton *clearButton;
	ShowFavoritesButton *favoriteButton;
	ui::Label *authorLabel;
	ui::List *authorList;
	ui::ScrollWidget *authorScroll;
	ui::Label *tagLabel;
	ui::List *tagList;
	ui::ScrollWidget *tagScroll;

	BrowserSidebar() {
		searchField = new BrowserSearchField;
		addChild(searchField);

		clearButton = new ClearButton;
		clearButton->text = "Reset filters";
		addChild(clearButton);

		favoriteButton = new ShowFavoritesButton;
		dynamic_cast<ShowFavoritesQuantity*>(favoriteButton->quantity)->widget = favoriteButton;
		addChild(favoriteButton);

		authorLabel = new ui::Label;
		// authorLabel->fontSize = 16;
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
		// tagLabel->fontSize = 16;
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
		searchField->box.size.x = box.size.x;
		clearButton->box.pos = searchField->box.getBottomLeft();
		clearButton->box.size.x = box.size.x;
		favoriteButton->box.pos = clearButton->box.getBottomLeft();
		favoriteButton->box.size.x = box.size.x;

		float listHeight = (box.size.y - favoriteButton->box.getBottom()) / 2;
		listHeight = std::floor(listHeight);

		authorLabel->box.pos = favoriteButton->box.getBottomLeft();
		authorLabel->box.size.x = box.size.x;
		authorScroll->box.pos = authorLabel->box.getBottomLeft();
		authorScroll->box.size.y = listHeight - authorLabel->box.size.y;
		authorScroll->box.size.x = box.size.x;
		authorList->box.size.x = authorScroll->box.size.x;

		tagLabel->box.pos = authorScroll->box.getBottomLeft();
		tagLabel->box.size.x = box.size.x;
		tagScroll->box.pos = tagLabel->box.getBottomLeft();
		tagScroll->box.size.y = listHeight - tagLabel->box.size.y;
		tagScroll->box.size.x = box.size.x;
		tagList->box.size.x = tagScroll->box.size.x;

		Widget::step();
	}
};


struct ModuleBrowser : widget::OpaqueWidget {
	BrowserSidebar *sidebar;
	ui::ScrollWidget *modelScroll;
	ui::Label *modelLabel;
	ui::MarginLayout *modelMargin;
	ui::SequentialLayout *modelContainer;

	std::string search;
	std::string author;
	std::string tag;
	bool favorites = false;

	ModuleBrowser() {
		sidebar = new BrowserSidebar;
		sidebar->box.size.x = 200;
		addChild(sidebar);

		modelScroll = new ui::ScrollWidget;
		addChild(modelScroll);

		modelLabel = new ui::Label;
		// modelLabel->fontSize = 16;
		modelLabel->box.pos = math::Vec(10, 10);
		modelScroll->container->addChild(modelLabel);

		modelMargin = new ui::MarginLayout;
		modelMargin->box.pos = modelLabel->box.getBottomLeft();
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

		refresh();
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

	void refresh() {
		// Reset scroll position
		modelScroll->offset = math::Vec();

		// Show all or only favorites
		for (Widget *w : modelContainer->children) {
			if (favorites) {
				ModelBox *m = dynamic_cast<ModelBox*>(w);
				auto it = settings::favoriteModels.find(m->model);
				w->visible = (it != settings::favoriteModels.end());
			}
			else {
				w->visible = true;
			}
		}

		if (search.empty()) {
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
				float score = 0.f;
				if (m->visible) {
					score = modelScore(m->model, search);
					m->visible = (score > 0);
				}
				scores[m] = score;
			}
			// Sort by score
			modelContainer->children.sort([&](Widget *w1, Widget *w2) {
				return scores[w1] > scores[w2];
			});
		}

		// Filter ModelBoxes by author
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

		// Filter ModelBoxes by tag
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

		std::set<std::string> enabledAuthors;
		std::set<std::string> enabledTags;

		// Get list of enabled authors and tags for sidebar
		for (Widget *w : modelContainer->children) {
			ModelBox *m = dynamic_cast<ModelBox*>(w);
			assert(m);
			if (!m->visible)
				continue;
			enabledAuthors.insert(m->model->plugin->author);
			for (const std::string &tag : m->model->tags) {
				enabledTags.insert(tag);
			}
		}

		// Count models
		int modelsLen = 0;
		for (Widget *w : modelContainer->children) {
			if (w->visible)
				modelsLen++;
		}
		modelLabel->text = string::f("Modules (%d)", modelsLen);

		// Enable author and tag items that are available in visible ModelBoxes
		int authorsLen = 0;
		for (Widget *w : sidebar->authorList->children) {
			AuthorItem *item = dynamic_cast<AuthorItem*>(w);
			assert(item);
			auto it = enabledAuthors.find(item->text);
			item->disabled = (it == enabledAuthors.end());
			if (!item->disabled)
				authorsLen++;
		}
		sidebar->authorLabel->text = string::f("Authors (%d)", authorsLen);

		int tagsLen = 0;
		for (Widget *w : sidebar->tagList->children) {
			TagItem *item = dynamic_cast<TagItem*>(w);
			assert(item);
			auto it = enabledTags.find(item->text);
			item->disabled = (it == enabledTags.end());
			if (!item->disabled)
				tagsLen++;
		}
		sidebar->tagLabel->text = string::f("Tags (%d)", tagsLen);
	}

	void clear() {
		search = "";
		sidebar->searchField->setText("");
		author = "";
		tag = "";
		refresh();
	}
};


// Implementations to resolve dependencies


inline void ModelBox::onButton(const event::Button &e) {
	OpaqueWidget::onButton(e);
	if (e.getTarget() != this)
		return;

	if (e.action == GLFW_PRESS && e.button == GLFW_MOUSE_BUTTON_LEFT) {
		// Hide tooltip
		setTooltip(NULL);

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


inline void AuthorItem::onAction(const event::Action &e) {
	ModuleBrowser *browser = getAncestorOfType<ModuleBrowser>();
	if (browser->author == text)
		browser->author = "";
	else
		browser->author = text;
	browser->refresh();
}

inline void AuthorItem::step() {
	MenuItem::step();
	ModuleBrowser *browser = getAncestorOfType<ModuleBrowser>();
	active = (browser->author == text);
}

inline void TagItem::onAction(const event::Action &e) {
	ModuleBrowser *browser = getAncestorOfType<ModuleBrowser>();
	if (browser->tag == text)
		browser->tag = "";
	else
		browser->tag = text;
	browser->refresh();
}

inline void TagItem::step() {
	MenuItem::step();
	ModuleBrowser *browser = getAncestorOfType<ModuleBrowser>();
	active = (browser->tag == text);
}

inline void BrowserSearchField::onChange(const event::Change &e) {
	ModuleBrowser *browser = getAncestorOfType<ModuleBrowser>();
	browser->search = string::trim(text);
	browser->refresh();
}

inline void ClearButton::onAction(const event::Action &e) {
	ModuleBrowser *browser = getAncestorOfType<ModuleBrowser>();
	browser->clear();
}

inline void ShowFavoritesQuantity::setValue(float value) {
	ModuleBrowser *browser = widget->getAncestorOfType<ModuleBrowser>();
	browser->favorites = (bool) value;
	browser->refresh();
}

inline float ShowFavoritesQuantity::getValue() {
	ModuleBrowser *browser = widget->getAncestorOfType<ModuleBrowser>();
	return browser->favorites;
}


// Global functions


widget::Widget *moduleBrowserCreate() {
	BrowserOverlay *overlay = new BrowserOverlay;

	ModuleBrowser *browser = new ModuleBrowser;
	overlay->addChild(browser);

	return overlay;
}


} // namespace app
} // namespace rack
