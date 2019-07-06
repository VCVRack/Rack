#include <app/ModuleBrowser.hpp>
#include <widget/OpaqueWidget.hpp>
#include <widget/TransparentWidget.hpp>
#include <widget/ZoomWidget.hpp>
#include <ui/ScrollWidget.hpp>
#include <ui/SequentialLayout.hpp>
#include <ui/MarginLayout.hpp>
#include <ui/Label.hpp>
#include <ui/TextField.hpp>
#include <ui/MenuOverlay.hpp>
#include <ui/List.hpp>
#include <ui/MenuItem.hpp>
#include <ui/Button.hpp>
#include <ui/RadioButton.hpp>
#include <ui/ChoiceButton.hpp>
#include <ui/Tooltip.hpp>
#include <ui/Slider.hpp>
#include <app/ModuleWidget.hpp>
#include <app/Scene.hpp>
#include <plugin.hpp>
#include <app.hpp>
#include <plugin/Model.hpp>
#include <string.hpp>
#include <history.hpp>
#include <settings.hpp>
#include <set>
#include <algorithm>
#include <helpers.hpp>

namespace rack {
namespace app {


// Static functions


static float modelScore(plugin::Model *model, const std::string &search) {
	if (search.empty())
		return 1.f;
	std::string s;
	s += model->plugin->brand;
	s += " ";
	s += model->plugin->name;
	s += " ";
	s += model->name;
	s += " ";
	s += model->slug;
	for (const std::string &tag : model->tags) {
		s += " ";
		s += tag;
	}
	float score = string::fuzzyScore(string::lowercase(s), string::lowercase(search));
	return score;
}

static bool isModelVisible(plugin::Model *model, const std::string &search, const std::string &brand, const std::string &tag) {
	// Filter search query
	if (search != "") {
		float score = modelScore(model, search);
		if (score <= 0.f)
			return false;
	}

	// Filter brand
	if (brand != "") {
		if (model->plugin->brand != brand)
			return false;
	}

	// Filter tag
	if (tag != "") {
		bool found = false;
		for (const std::string &modelTag : model->tags) {
			if (modelTag == tag) {
				found = true;
				break;
			}
		}
		if (!found)
			return false;
	}

	return true;
}

static ModuleWidget *chooseModel(plugin::Model *model) {
	// Create module
	ModuleWidget *moduleWidget = model->createModuleWidget();
	assert(moduleWidget);
	APP->scene->rack->addModuleAtMouse(moduleWidget);

	// Push ModuleAdd history action
	history::ModuleAdd *h = new history::ModuleAdd;
	h->name = "create module";
	h->setModule(moduleWidget);
	APP->history->push(h);

	// Hide Module Browser
	APP->scene->moduleBrowser->hide();

	return moduleWidget;
}

template <typename K, typename V>
V get_default(const std::map<K, V> &m, const K &key, const V &def) {
	auto it = m.find(key);
	if (it == m.end())
		return def;
	return it->second;
}


// Widgets


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

struct ModelBox : widget::OpaqueWidget {
	plugin::Model *model;
	widget::Widget *previewWidget = NULL;
	ui::Tooltip *tooltip = NULL;
	/** Lazily created */
	widget::FramebufferWidget *previewFb = NULL;
	/** Number of frames since draw() has been called */
	int visibleFrames = 0;
	float previousZoomValue = 0;

	ModelBox() {
		// Approximate size as 10HP before we know the actual size.
		// We need a nonzero size, otherwise the parent widget will consider it not in the draw bounds, so its preview will not be lazily created.
		updateZoomLevel();
		previousZoomValue = settings::moduleBrowserZoom;
	}

	void updateZoomLevel() {

		if (previousZoomValue != settings::moduleBrowserZoom) {
			previousZoomValue = settings::moduleBrowserZoom;
			box.size.x = 10 * RACK_GRID_WIDTH * settings::moduleBrowserZoom;
			box.size.y = RACK_GRID_HEIGHT * settings::moduleBrowserZoom;
			box.size = box.size.ceil();

			if (previewFb) 
				deletePreview();	
		}
	}

	void setModel(plugin::Model *model) {
		this->model = model;

		previewWidget = new widget::TransparentWidget;
		updateZoomLevel();

		addChild(previewWidget);
	}

	void createPreview() {
		previewFb = new widget::FramebufferWidget;
		if (math::isNear(APP->window->pixelRatio, 1.0)) {
			// Small details draw poorly at low DPI, so oversample when drawing to the framebuffer
			previewFb->oversample = 2.0;
		}
		previewWidget->addChild(previewFb);
		previewWidget->box.size.y = std::ceil(RACK_GRID_HEIGHT * settings::moduleBrowserZoom);

		widget::ZoomWidget *zoomWidget = new widget::ZoomWidget;
		zoomWidget->setZoom(settings::moduleBrowserZoom);
		previewFb->addChild(zoomWidget);

		ModuleWidget *moduleWidget = model->createModuleWidgetNull();
		zoomWidget->addChild(moduleWidget);

		zoomWidget->box.size.x = moduleWidget->box.size.x * settings::moduleBrowserZoom;
		zoomWidget->box.size.y = RACK_GRID_HEIGHT * settings::moduleBrowserZoom;
		previewWidget->box.size.x = std::ceil(zoomWidget->box.size.x);

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
		
		updateZoomLevel();

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

	void onButton(const event::Button &e) override {
		OpaqueWidget::onButton(e);
		if (e.getTarget() != this)
			return;

		if (e.action == GLFW_PRESS && e.button == GLFW_MOUSE_BUTTON_LEFT) {
			ModuleWidget *mw = chooseModel(model);

			// Pretend the moduleWidget was clicked so it can be dragged in the RackWidget
			e.consume(mw);
		}
	}

	void onEnter(const event::Enter &e) override {
		std::string text;
		text = model->plugin->brand;
		text += " " + model->name;
		if (model->description != "")
			text += "\n" + model->description;
		// Tags
		if (!model->tags.empty()) {
			text += "\nTags: ";
			for (size_t i = 0; i < model->tags.size(); i++) {
				if (i > 0)
					text += ", ";
				text += model->tags[i];
			}
		}
		ui::Tooltip *tooltip = new ui::Tooltip;
		tooltip->text = text;
		setTooltip(tooltip);
	}

	void onLeave(const event::Leave &e) override {
		setTooltip(NULL);
	}

	void onHide(const event::Hide &e) override {
		// Hide tooltip
		setTooltip(NULL);
		OpaqueWidget::onHide(e);
	}
};


struct BrandItem : ui::MenuItem {
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

	void onSelectKey(const event::SelectKey &e) override;
	void onChange(const event::Change &e) override;
	void onAction(const event::Action &e) override;

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


struct BrowserSidebar : widget::Widget {
	BrowserSearchField *searchField;
	ClearButton *clearButton;
	ui::Label *brandLabel;
	ui::List *brandList;
	ui::ScrollWidget *brandScroll;
	ui::Label *tagLabel;
	ui::List *tagList;
	ui::ScrollWidget *tagScroll;

	BrowserSidebar() {
		// Search
		searchField = new BrowserSearchField;
		addChild(searchField);

		// Clear filters
		clearButton = new ClearButton;
		clearButton->text = "Reset filters";
		addChild(clearButton);

		// Bbrand label
		brandLabel = new ui::Label;
		// brandLabel->fontSize = 16;
		brandLabel->color = nvgRGB(0x80, 0x80, 0x80);
		brandLabel->text = "Brands";
		addChild(brandLabel);

		// Brand list
		brandScroll = new ui::ScrollWidget;
		addChild(brandScroll);

		brandList = new ui::List;
		brandScroll->container->addChild(brandList);

		// Collect brands from all plugins
		std::set<std::string, string::CaseInsensitiveCompare> brands;
		for (plugin::Plugin *plugin : plugin::plugins) {
			brands.insert(plugin->brand);
		}

		for (const std::string &brand : brands) {
			BrandItem *item = new BrandItem;
			item->text = brand;
			brandList->addChild(item);
		}

		// Tag label
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

		float listHeight = (box.size.y - clearButton->box.getBottom()) / 2;
		listHeight = std::floor(listHeight);

		brandLabel->box.pos = clearButton->box.getBottomLeft();
		brandLabel->box.size.x = box.size.x;
		brandScroll->box.pos = brandLabel->box.getBottomLeft();
		brandScroll->box.size.y = listHeight - brandLabel->box.size.y;
		brandScroll->box.size.x = box.size.x;
		brandList->box.size.x = brandScroll->box.size.x;

		tagLabel->box.pos = brandScroll->box.getBottomLeft();
		tagLabel->box.size.x = box.size.x;
		tagScroll->box.pos = tagLabel->box.getBottomLeft();
		tagScroll->box.size.y = listHeight - tagLabel->box.size.y;
		tagScroll->box.size.x = box.size.x;
		tagList->box.size.x = tagScroll->box.size.x;

		Widget::step();
	}
};

struct ModuleBrowserZoomQuantity : Quantity {
	void setValue(float value) override {
		settings::moduleBrowserZoom = math::clamp(value, getMinValue(), getMaxValue());
	}
	float getValue() override {
		return settings::moduleBrowserZoom;
	}
	float getMinValue() override { return 0.5; }
	float getMaxValue() override { return 2.0; }
	float getDefaultValue() override { return 0.5; }
	float getDisplayValue() override { return getValue() * 100; }
	void setDisplayValue(float displayValue) override { setValue(displayValue); }
	std::string getLabel() override { return "Zoom"; }
	std::string getUnit() override { return "%"; }
};

struct ZoomItem : public ui::MenuItem {
	float value;
	Quantity *quantity;

	ZoomItem(std::string text, float value, Quantity *quantity) 
		: value(value)
		, quantity(quantity) {
		this->text = text;
		this->setSize(math::Vec(60, 30));
	}
	
	void onAction(const event::Action &e) override {
		quantity->setValue(value);
	}
};

struct ModuleBrowserZoomButton : ui::ChoiceButton {
	ModuleBrowserZoomButton() {
		quantity  = new ModuleBrowserZoomQuantity;
	}

	~ModuleBrowserZoomButton() {
		delete quantity;
	}

	void onDragStart(const event::DragStart &e) override {}
	void onDragEnd(const event::DragEnd &e) override {}

	void onAction(const event::Action &e) override {
		auto menu = createMenu();

		menu->setSize(math::Vec(200, 200));
		menu->addChild(new ZoomItem("50%", 0.5f, quantity));
		menu->addChild(new ZoomItem("75%", 0.75f, quantity));
		menu->addChild(new ZoomItem("100%", 1.0f, quantity));
		menu->addChild(new ZoomItem("125%", 1.25f, quantity));
		menu->addChild(new ZoomItem("150%", 1.5f, quantity));
		menu->addChild(new ZoomItem("175%", 1.75f, quantity));
		menu->addChild(new ZoomItem("200%", 2.0f, quantity));
	}

	void step() override {
		text = quantity->getString();
	}
};

struct ModuleBrowser : widget::OpaqueWidget {
	BrowserSidebar *sidebar;
	ui::ScrollWidget *modelScroll;
	ui::Label *modelLabel;
	ui::MarginLayout *modelMargin;
	ui::SequentialLayout *modelContainer;
	ModuleBrowserZoomButton *moduleBrowserZoomButton;

	std::string search;
	std::string brand;
	std::string tag;

	ModuleBrowser() {

		sidebar = new BrowserSidebar;
		sidebar->box.size.x = 200;

		addChild(sidebar);

		modelLabel = new ui::Label;
		addChild(modelLabel);

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

		moduleBrowserZoomButton = new ModuleBrowserZoomButton();
		moduleBrowserZoomButton->box.size.x = 200;
		addChild(moduleBrowserZoomButton);

		refresh();
	}

	void step() override {
		box = parent->box.zeroPos().grow(math::Vec(-70, -70));

		sidebar->box.size.y = box.size.y;
		modelLabel->box.pos = sidebar->box.getTopRight().plus(math::Vec(5, 5));
		moduleBrowserZoomButton->box.pos = sidebar->box.getTopRight().plus(math::Vec(500, 5));
		modelScroll->box.pos = sidebar->box.getTopRight().plus(math::Vec(0, 30));
		modelScroll->box.size = box.size.minus(modelScroll->box.pos);
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

		// Filter ModelBoxes
		for (Widget *w : modelContainer->children) {
			ModelBox *m = dynamic_cast<ModelBox*>(w);
			assert(m);
			m->visible = isModelVisible(m->model, search, brand, tag);
		}

		// Sort ModelBoxes
		modelContainer->children.sort([&](Widget *w1, Widget *w2) {
			ModelBox *m1 = dynamic_cast<ModelBox*>(w1);
			ModelBox *m2 = dynamic_cast<ModelBox*>(w2);
			// Sort by (modifiedTimestamp descending, plugin brand, model name)
			auto t1 = std::make_tuple(-m1->model->plugin->modifiedTimestamp, m1->model->plugin->brand, m1->model->name);
			auto t2 = std::make_tuple(-m2->model->plugin->modifiedTimestamp, m2->model->plugin->brand, m2->model->name);
			return t1 < t2;
		});

		if (search.empty()) {
			// We've already sorted above
		}
		else {
			std::map<Widget*, float> scores;
			// Compute scores
			for (Widget *w : modelContainer->children) {
				ModelBox *m = dynamic_cast<ModelBox*>(w);
				assert(m);
				if (!m->visible)
					continue;
				scores[m] = modelScore(m->model, search);
			}
			// // Sort by score
			// modelContainer->children.sort([&](Widget *w1, Widget *w2) {
			// 	// If score was not computed, scores[w] returns 0, but this doesn't matter because those widgets aren't visible.
			// 	return get_default(scores, w1, 0.f) > get_default(scores, w2, 0.f);
			// });
		}

		// Filter the brand and tag lists

		// Get modules that would be filtered by just the search query
		std::vector<plugin::Model*> filteredModels;
		for (Widget *w : modelContainer->children) {
			ModelBox *m = dynamic_cast<ModelBox*>(w);
			assert(m);
			if (isModelVisible(m->model, search, "", ""))
				filteredModels.push_back(m->model);
		}

		auto hasModel = [&](const std::string &brand, const std::string &tag) -> bool {
			for (plugin::Model *model : filteredModels) {
				if (isModelVisible(model, "", brand, tag))
					return true;
			}
			return false;
		};

		// Enable brand and tag items that are available in visible ModelBoxes
		int brandsLen = 0;
		for (Widget *w : sidebar->brandList->children) {
			BrandItem *item = dynamic_cast<BrandItem*>(w);
			assert(item);
			item->disabled = !hasModel(item->text, tag);
			if (!item->disabled)
				brandsLen++;
		}
		sidebar->brandLabel->text = string::f("Brands (%d)", brandsLen);

		int tagsLen = 0;
		for (Widget *w : sidebar->tagList->children) {
			TagItem *item = dynamic_cast<TagItem*>(w);
			assert(item);
			item->disabled = !hasModel(brand, item->text);
			if (!item->disabled)
				tagsLen++;
		}
		sidebar->tagLabel->text = string::f("Tags (%d)", tagsLen);

		// Count models
		int modelsLen = 0;
		for (Widget *w : modelContainer->children) {
			if (w->visible)
				modelsLen++;
		}
		modelLabel->text = string::f("Modules (%d) Click and drag a module to place it in the rack.", modelsLen);
	}

	void clear() {
		search = "";
		sidebar->searchField->setText("");
		brand = "";
		tag = "";
		refresh();
	}

	void onShow(const event::Show &e) override {
		refresh();
		OpaqueWidget::onShow(e);
	}
};


// Implementations to resolve dependencies


inline void BrandItem::onAction(const event::Action &e) {
	ModuleBrowser *browser = getAncestorOfType<ModuleBrowser>();
	if (browser->brand == text)
		browser->brand = "";
	else
		browser->brand = text;
	browser->refresh();
}

inline void BrandItem::step() {
	MenuItem::step();
	ModuleBrowser *browser = getAncestorOfType<ModuleBrowser>();
	active = (browser->brand == text);
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

inline void BrowserSearchField::onSelectKey(const event::SelectKey &e) {
	if (e.action == GLFW_PRESS) {
		switch (e.key) {
			case GLFW_KEY_ESCAPE: {
				BrowserOverlay *overlay = getAncestorOfType<BrowserOverlay>();
				overlay->hide();
				e.consume(this);
			} break;
			case GLFW_KEY_BACKSPACE: {
				if (text == "") {
					ModuleBrowser *browser = getAncestorOfType<ModuleBrowser>();
					browser->clear();
					e.consume(this);
				}
			} break;
		}
	}

	if (!e.getTarget())
		ui::TextField::onSelectKey(e);
}

inline void BrowserSearchField::onChange(const event::Change &e) {
	ModuleBrowser *browser = getAncestorOfType<ModuleBrowser>();
	browser->search = string::trim(text);
	browser->refresh();
}

inline void BrowserSearchField::onAction(const event::Action &e) {
	// Get first ModelBox
	ModelBox *mb = NULL;
	ModuleBrowser *browser = getAncestorOfType<ModuleBrowser>();
	for (Widget *w : browser->modelContainer->children) {
		if (w->visible) {
			mb = dynamic_cast<ModelBox*>(w);
			break;
		}
	}

	if (mb) {
		chooseModel(mb->model);
	}
}

inline void ClearButton::onAction(const event::Action &e) {
	ModuleBrowser *browser = getAncestorOfType<ModuleBrowser>();
	browser->clear();
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
