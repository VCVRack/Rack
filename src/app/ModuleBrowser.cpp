#include <set>
#include <algorithm>
#include <thread>

#include <app/ModuleBrowser.hpp>
#include <widget/OpaqueWidget.hpp>
#include <widget/TransparentWidget.hpp>
#include <widget/ZoomWidget.hpp>
#include <ui/MenuOverlay.hpp>
#include <ui/ScrollWidget.hpp>
#include <ui/SequentialLayout.hpp>
#include <ui/MarginLayout.hpp>
#include <ui/Label.hpp>
#include <ui/Slider.hpp>
#include <ui/TextField.hpp>
#include <ui/MenuItem.hpp>
#include <ui/MenuSeparator.hpp>
#include <ui/Button.hpp>
#include <ui/ChoiceButton.hpp>
#include <ui/RadioButton.hpp>
#include <ui/Tooltip.hpp>
#include <app/ModuleWidget.hpp>
#include <app/Scene.hpp>
#include <plugin.hpp>
#include <context.hpp>
#include <engine/Engine.hpp>
#include <plugin/Model.hpp>
#include <string.hpp>
#include <history.hpp>
#include <settings.hpp>
#include <system.hpp>
#include <tag.hpp>
#include <helpers.hpp>
#include <FuzzySearchDatabase.hpp>


namespace rack {
namespace app {
namespace moduleBrowser {


static fuzzysearch::Database<plugin::Model*> modelDb;
static bool modelDbInitialized = false;


static void modelDbInit() {
	if (modelDbInitialized)
		return;
	modelDb.setWeights({1.f, 1.f, 0.25f, 1.f, 0.5f, 0.5f});
	modelDb.setThreshold(0.5f);

	// Iterate plugins
	for (plugin::Plugin* plugin : plugin::plugins) {
		// Iterate model in plugin
		for (plugin::Model* model : plugin->models) {
			// Get search fields for model
			std::string tagStr;
			for (int tagId : model->tags) {
				// Add all aliases of a tag
				for (const std::string& tagAlias : tag::tagAliases[tagId]) {
					tagStr += tagAlias;
					tagStr += ", ";
				}
			}
			std::vector<std::string> fields {
				model->plugin->brand,
				model->plugin->name,
				model->plugin->description,
				model->name,
				model->description,
				tagStr,
			};
			modelDb.addEntry(model, fields);
		}
	}

	modelDbInitialized = true;
}


static ModuleWidget* chooseModel(plugin::Model* model) {
	// Record usage
	settings::ModuleUsage& mu = settings::moduleUsages[model->plugin->slug][model->slug];
	mu.count++;
	mu.lastTime = system::getUnixTime();

	// Create Module and ModuleWidget
	engine::Module* module = model->createModule();
	APP->engine->addModule(module);

	ModuleWidget* moduleWidget = model->createModuleWidget(module);
	APP->scene->rack->addModuleAtMouse(moduleWidget);

	// Load template preset
	moduleWidget->loadTemplate();

	// history::ModuleAdd
	history::ModuleAdd* h = new history::ModuleAdd;
	h->name = "create module";
	// This serializes the module so redoing returns to the current state.
	h->setModule(moduleWidget);
	APP->history->push(h);

	// Hide Module Browser
	APP->scene->moduleBrowser->hide();

	return moduleWidget;
}


// Widgets


struct ModuleBrowser;


struct BrowserOverlay : ui::MenuOverlay {
	void step() override {
		// Only step if visible, since there are potentially thousands of descendants that don't need to be stepped.
		if (isVisible())
			MenuOverlay::step();
	}

	void onAction(const event::Action& e) override {
		hide();
	}
};


struct ModelBox : widget::OpaqueWidget {
	plugin::Model* model;
	ui::Tooltip* tooltip = NULL;
	// Lazily created widgets
	widget::Widget* previewWidget = NULL;
	widget::ZoomWidget* zoomWidget = NULL;
	widget::FramebufferWidget* fb = NULL;
	ModuleWidget* moduleWidget = NULL;

	ModelBox() {
		updateZoom();
	}

	void setModel(plugin::Model* model) {
		this->model = model;
	}

	void updateZoom() {
		float zoom = std::pow(2.f, settings::moduleBrowserZoom);

		if (previewWidget) {
			fb->setDirty();
			zoomWidget->setZoom(zoom);
			box.size.x = moduleWidget->box.size.x * zoom;
		}
		else {
			// Approximate size as 12HP before we know the actual size.
			// We need a nonzero size, otherwise too many ModelBoxes will lazily render in the same frame.
			box.size.x = 12 * RACK_GRID_WIDTH * zoom;
		}
		box.size.y = RACK_GRID_HEIGHT * zoom;
		box.size = box.size.ceil();
	}

	void createPreview() {
		if (previewWidget)
			return;

		previewWidget = new widget::TransparentWidget;
		addChild(previewWidget);

		zoomWidget = new widget::ZoomWidget;
		previewWidget->addChild(zoomWidget);

		fb = new widget::FramebufferWidget;
		if (math::isNear(APP->window->pixelRatio, 1.0)) {
			// Small details draw poorly at low DPI, so oversample when drawing to the framebuffer
			fb->oversample = 2.0;
		}
		zoomWidget->addChild(fb);

		moduleWidget = model->createModuleWidget(NULL);
		fb->addChild(moduleWidget);

		updateZoom();
	}

	void draw(const DrawArgs& args) override {
		// Lazily create preview when drawn
		createPreview();

		// Draw shadow
		nvgBeginPath(args.vg);
		float r = 10; // Blur radius
		float c = 10; // Corner radius
		nvgRect(args.vg, -r, -r, box.size.x + 2 * r, box.size.y + 2 * r);
		NVGcolor shadowColor = nvgRGBAf(0, 0, 0, 0.5);
		NVGcolor transparentColor = nvgRGBAf(0, 0, 0, 0);
		nvgFillPaint(args.vg, nvgBoxGradient(args.vg, 0, 0, box.size.x, box.size.y, c, r, shadowColor, transparentColor));
		nvgFill(args.vg);

		OpaqueWidget::draw(args);
	}

	void step() override {
		OpaqueWidget::step();
	}

	void setTooltip(ui::Tooltip* tooltip) {
		if (this->tooltip) {
			this->tooltip->requestDelete();
			this->tooltip = NULL;
		}

		if (tooltip) {
			APP->scene->addChild(tooltip);
			this->tooltip = tooltip;
		}
	}

	void onButton(const event::Button& e) override {
		OpaqueWidget::onButton(e);
		if (e.getTarget() != this)
			return;

		if (e.action == GLFW_PRESS && e.button == GLFW_MOUSE_BUTTON_LEFT) {
			ModuleWidget* mw = chooseModel(model);

			// Pretend the moduleWidget was clicked so it can be dragged in the RackWidget
			e.consume(mw);

			// Set the drag position at the center of the module
			mw->dragOffset() = mw->box.size.div(2);
			// Disable dragging temporarily until the mouse has moved a bit.
			mw->dragEnabled() = false;
		}
	}

	void onEnter(const event::Enter& e) override {
		std::string text;
		text = model->plugin->brand;
		text += " " + model->name;
		// Description
		if (model->description != "") {
			text += "\n" + model->description;
		}
		// Tags
		text += "\n\nTags: ";
		for (size_t i = 0; i < model->tags.size(); i++) {
			if (i > 0)
				text += ", ";
			int tagId = model->tags[i];
			text += tag::getTag(tagId);
		}
		ui::Tooltip* tooltip = new ui::Tooltip;
		tooltip->text = text;
		setTooltip(tooltip);
	}

	void onLeave(const event::Leave& e) override {
		setTooltip(NULL);
	}

	void onHide(const event::Hide& e) override {
		// Hide tooltip
		setTooltip(NULL);
		OpaqueWidget::onHide(e);
	}
};


struct BrowserSearchField : ui::TextField {
	ModuleBrowser* browser;

	void step() override {
		// Steal focus when step is called
		APP->event->setSelected(this);
		TextField::step();
	}

	void onSelectKey(const event::SelectKey& e) override;
	void onChange(const event::Change& e) override;
	void onAction(const event::Action& e) override;

	void onHide(const event::Hide& e) override {
		APP->event->setSelected(NULL);
		ui::TextField::onHide(e);
	}

	void onShow(const event::Show& e) override {
		selectAll();
		TextField::onShow(e);
	}
};


struct ClearButton : ui::Button {
	ModuleBrowser* browser;
	void onAction(const event::Action& e) override;
};


struct BrandItem : ui::MenuItem {
	ModuleBrowser* browser;
	std::string brand;
	void onAction(const event::Action& e) override;
	void step() override;
};


struct BrandButton : ui::ChoiceButton {
	ModuleBrowser* browser;

	void onAction(const event::Action& e) override;
	void step() override;
};


struct TagItem : ui::MenuItem {
	ModuleBrowser* browser;
	int tagId;
	void onAction(const event::Action& e) override;
	void step() override;
};


struct TagButton : ui::ChoiceButton {
	ModuleBrowser* browser;

	void onAction(const event::Action& e) override;
	void step() override;
};


static const std::string sortNames[] = {
	"Last updated",
	"Last used",
	"Most used",
	"Brand",
	"Module name",
	"Random",
};


struct SortItem : ui::MenuItem {
	ModuleBrowser* browser;
	settings::ModuleBrowserSort sort;
	void onAction(const event::Action& e) override;
	void step() override {
		rightText = CHECKMARK(settings::moduleBrowserSort == sort);
		MenuItem::step();
	}
};


struct SortButton : ui::ChoiceButton {
	ModuleBrowser* browser;

	void onAction(const event::Action& e) override {
		ui::Menu* menu = createMenu();
		menu->box.pos = getAbsoluteOffset(math::Vec(0, box.size.y));
		menu->box.size.x = box.size.x;

		for (int sortId = 0; sortId <= settings::MODULE_BROWSER_SORT_RANDOM; sortId++) {
			SortItem* sortItem = new SortItem;
			sortItem->text = sortNames[sortId];
			sortItem->sort = (settings::ModuleBrowserSort) sortId;
			sortItem->browser = browser;
			menu->addChild(sortItem);
		}
	}

	void step() override {
		text = "Sort: ";
		text += sortNames[settings::moduleBrowserSort];
		ChoiceButton::step();
	}
};


struct ZoomItem : ui::MenuItem {
	ModuleBrowser* browser;
	float zoom;
	void onAction(const event::Action& e) override;
	void step() override {
		rightText = CHECKMARK(settings::moduleBrowserZoom == zoom);
		MenuItem::step();
	}
};


struct ZoomButton : ui::ChoiceButton {
	ModuleBrowser* browser;

	void onAction(const event::Action& e) override {
		ui::Menu* menu = createMenu();
		menu->box.pos = getAbsoluteOffset(math::Vec(0, box.size.y));
		menu->box.size.x = box.size.x;

		for (float zoom = 0.f; zoom >= -2.f; zoom -= 0.5f) {
			ZoomItem* sortItem = new ZoomItem;
			sortItem->text = string::f("%.3g%%", std::pow(2.f, zoom) * 100.f);
			sortItem->zoom = zoom;
			sortItem->browser = browser;
			menu->addChild(sortItem);
		}
	}

	void step() override {
		text = "Zoom: ";
		text += string::f("%.3g%%", std::pow(2.f, settings::moduleBrowserZoom) * 100.f);
		ChoiceButton::step();
	}
};


struct UrlButton : ui::Button {
	std::string url;
	void onAction(const event::Action& e) override {
		std::thread t([=] {
			system::openBrowser(url);
		});
		t.detach();
	}
};


struct ModuleBrowser : widget::OpaqueWidget {
	ui::SequentialLayout* headerLayout;
	BrowserSearchField* searchField;
	BrandButton* brandButton;
	TagButton* tagButton;
	ClearButton* clearButton;

	ui::ScrollWidget* modelScroll;
	ui::MarginLayout* modelMargin;
	ui::SequentialLayout* modelContainer;

	std::string search;
	std::string brand;
	std::set<int> tagIds = {};

	std::map<plugin::Model*, float> prefilteredModelScores;

	ModuleBrowser() {
		float margin = 10;

		// Header
		headerLayout = new ui::SequentialLayout;
		headerLayout->box.pos = math::Vec(0, 0);
		headerLayout->box.size.y = BND_WIDGET_HEIGHT + 2 * margin;
		headerLayout->margin = math::Vec(margin, margin);
		headerLayout->spacing = math::Vec(margin, margin);
		addChild(headerLayout);

		searchField = new BrowserSearchField;
		searchField->box.size.x = 150;
		searchField->placeholder = "Search modules";
		searchField->browser = this;
		headerLayout->addChild(searchField);

		brandButton = new BrandButton;
		brandButton->box.size.x = 150;
		brandButton->browser = this;
		headerLayout->addChild(brandButton);

		tagButton = new TagButton;
		tagButton->box.size.x = 150;
		tagButton->browser = this;
		headerLayout->addChild(tagButton);

		clearButton = new ClearButton;
		clearButton->box.size.x = 100;
		clearButton->text = "Reset filters";
		clearButton->browser = this;
		headerLayout->addChild(clearButton);

		widget::Widget* spacer1 = new widget::Widget;
		spacer1->box.size.x = 20;
		headerLayout->addChild(spacer1);

		SortButton* sortButton = new SortButton;
		sortButton->box.size.x = 150;
		sortButton->browser = this;
		headerLayout->addChild(sortButton);

		ZoomButton* zoomButton = new ZoomButton;
		zoomButton->box.size.x = 100;
		zoomButton->browser = this;
		headerLayout->addChild(zoomButton);

		UrlButton* libraryButton = new UrlButton;
		libraryButton->box.size.x = 150;
		libraryButton->text = "Browse VCV Library";
		libraryButton->url = "https://library.vcvrack.com/";
		headerLayout->addChild(libraryButton);

		// Model container
		modelScroll = new ui::ScrollWidget;
		modelScroll->box.pos.y = BND_WIDGET_HEIGHT;
		addChild(modelScroll);

		modelMargin = new ui::MarginLayout;
		modelMargin->margin = math::Vec(margin, 0);
		modelScroll->container->addChild(modelMargin);

		modelContainer = new ui::SequentialLayout;
		modelContainer->spacing = math::Vec(margin, margin);
		modelMargin->addChild(modelContainer);

		resetModelBoxes();
		clear();
	}

	void resetModelBoxes() {
		modelContainer->clearChildren();
		// Iterate plugins
		// for (int i = 0; i < 100; i++)
		for (plugin::Plugin* plugin : plugin::plugins) {
			// Get module slugs from module whitelist
			const auto& pluginIt = settings::moduleWhitelist.find(plugin->slug);
			// Iterate models in plugin
			for (plugin::Model* model : plugin->models) {
				// Don't show module if plugin whitelist exists but the module is not in it.
				if (pluginIt != settings::moduleWhitelist.end()) {
					auto moduleIt = std::find(pluginIt->second.begin(), pluginIt->second.end(), model->slug);
					if (moduleIt == pluginIt->second.end())
						continue;
				}

				// Create ModelBox
				ModelBox* modelBox = new ModelBox;
				modelBox->setModel(model);
				modelContainer->addChild(modelBox);
			}
		}
	}

	void updateZoom() {
		modelScroll->offset = math::Vec();

		for (Widget* w : modelContainer->children) {
			ModelBox* mb = reinterpret_cast<ModelBox*>(w);
			assert(mb);
			mb->updateZoom();
		}
	}

	void step() override {
		box = parent->box.zeroPos().grow(math::Vec(-40, -40));

		headerLayout->box.size.x = box.size.x;

		modelScroll->box.pos = headerLayout->box.getBottomLeft();
		modelScroll->box.size = box.size.minus(modelScroll->box.pos);
		modelMargin->box.size.x = modelScroll->box.size.x;
		modelMargin->box.size.y = modelContainer->getChildrenBoundingBox().size.y + 2 * modelMargin->margin.y;

		OpaqueWidget::step();
	}

	void draw(const DrawArgs& args) override {
		bndMenuBackground(args.vg, 0.0, 0.0, box.size.x, box.size.y, 0);
		Widget::draw(args);
	}

	bool isModelVisible(plugin::Model* model, const std::string& brand, std::set<int> tagIds) {
		// Filter brand
		if (!brand.empty()) {
			if (model->plugin->brand != brand)
				return false;
		}

		// Filter tag
		for (int tagId : tagIds) {
			auto it = std::find(model->tags.begin(), model->tags.end(), tagId);
			if (it == model->tags.end())
				return false;
		}

		return true;
	};

	// Determines if there is at least 1 visible Model with a given brand and tag
	bool hasVisibleModel(const std::string& brand, std::set<int> tagIds) {
		for (const auto& pair : prefilteredModelScores) {
			plugin::Model* model = pair.first;
			if (isModelVisible(model, brand, tagIds))
				return true;
		}
		return false;
	};

	template <typename F>
	void sortModels(F f) {
		modelContainer->children.sort([&](Widget* w1, Widget* w2) {
			ModelBox* m1 = reinterpret_cast<ModelBox*>(w1);
			ModelBox* m2 = reinterpret_cast<ModelBox*>(w2);
			return f(m1) < f(m2);
		});
	}

	void refresh() {
		// Reset scroll position
		modelScroll->offset = math::Vec();

		prefilteredModelScores.clear();

		// Filter ModelBoxes by brand and tag
		for (Widget* w : modelContainer->children) {
			ModelBox* m = reinterpret_cast<ModelBox*>(w);
			m->setVisible(isModelVisible(m->model, brand, tagIds));
		}

		// Filter and sort by search results
		if (search.empty()) {
			// Add all models to prefilteredModelScores with scores of 1
			for (Widget* w : modelContainer->children) {
				ModelBox* m = reinterpret_cast<ModelBox*>(w);
				prefilteredModelScores[m->model] = 1.f;
			}

			// Sort ModelBoxes
			if (settings::moduleBrowserSort == settings::MODULE_BROWSER_SORT_UPDATED) {
				sortModels([](ModelBox* m) {
					plugin::Plugin* p = m->model->plugin;
					return std::make_tuple(-p->modifiedTimestamp, p->brand, m->model->name);
				});
			}
			else if (settings::moduleBrowserSort == settings::MODULE_BROWSER_SORT_LAST_USED) {
				sortModels([](ModelBox* m) {
					plugin::Plugin* p = m->model->plugin;
					const settings::ModuleUsage* mu = settings::getModuleUsage(p->slug, m->model->slug);
					double lastTime = mu ? mu->lastTime : -INFINITY;
					return std::make_tuple(-lastTime, -p->modifiedTimestamp, p->brand);
				});
			}
			else if (settings::moduleBrowserSort == settings::MODULE_BROWSER_SORT_MOST_USED) {
				sortModels([](ModelBox* m) {
					plugin::Plugin* p = m->model->plugin;
					const settings::ModuleUsage* mu = settings::getModuleUsage(p->slug, m->model->slug);
					int count = mu ? mu->count : 0;
					double lastTime = mu ? mu->lastTime : -INFINITY;
					return std::make_tuple(-count, -lastTime, -p->modifiedTimestamp, p->brand);
				});
			}
			else if (settings::moduleBrowserSort == settings::MODULE_BROWSER_SORT_BRAND) {
				sortModels([](ModelBox* m) {
					return std::make_tuple(m->model->plugin->brand, m->model->name);
				});
			}
			else if (settings::moduleBrowserSort == settings::MODULE_BROWSER_SORT_NAME) {
				sortModels([](ModelBox* m) {
					return std::make_tuple(m->model->name, m->model->plugin->brand);
				});
			}
			else if (settings::moduleBrowserSort == settings::MODULE_BROWSER_SORT_RANDOM) {
				std::map<ModelBox*, uint64_t> randomOrder;
				for (Widget* w : modelContainer->children) {
					ModelBox* m = reinterpret_cast<ModelBox*>(w);
					randomOrder[m] = random::u64();
				}
				sortModels([&](ModelBox* m) {
					return get(randomOrder, m, 0);
				});
			}
		}
		else {
			// Lazily initialize search database
			modelDbInit();
			// Score results against search query
			auto results = modelDb.search(search);
			// DEBUG("=============");
			for (auto& result : results) {
				prefilteredModelScores[result.key] = result.score;
				// DEBUG("%s %s\t\t%f", result._key->plugin->slug.c_str(), result._key->slug.c_str(), result._score);
			}
			// Sort by score
			sortModels([&](ModelBox* m) {
				return get(prefilteredModelScores, m->model, 0.f);
			});
			// Filter by whether the score is above the threshold
			for (Widget* w : modelContainer->children) {
				ModelBox* m = reinterpret_cast<ModelBox*>(w);
				assert(m);
				if (m->visible) {
					if (prefilteredModelScores.find(m->model) == prefilteredModelScores.end())
						m->visible = false;
				}
			}
		}
	}

	void clear() {
		search = "";
		searchField->setText("");
		brand = "";
		tagIds = {};
		refresh();
	}

	void onShow(const event::Show& e) override {
		refresh();
		OpaqueWidget::onShow(e);
	}
};


// Implementations to resolve dependencies


inline void ClearButton::onAction(const event::Action& e) {
	browser->clear();
}

inline void BrowserSearchField::onSelectKey(const event::SelectKey& e) {
	if (e.action == GLFW_PRESS || e.action == GLFW_REPEAT) {
		if (e.key == GLFW_KEY_ESCAPE) {
			BrowserOverlay* overlay = browser->getAncestorOfType<BrowserOverlay>();
			overlay->hide();
			e.consume(this);
		}
		// Backspace when the field is empty to clear filters.
		if (e.key == GLFW_KEY_BACKSPACE) {
			if (text == "") {
				browser->clear();
				e.consume(this);
			}
		}
	}

	if (!e.getTarget())
		ui::TextField::onSelectKey(e);
}

inline void BrowserSearchField::onChange(const event::Change& e) {
	browser->search = string::trim(text);
	browser->refresh();
}

inline void BrowserSearchField::onAction(const event::Action& e) {
	// Get first ModelBox
	ModelBox* mb = NULL;
	for (Widget* w : browser->modelContainer->children) {
		if (w->isVisible()) {
			mb = reinterpret_cast<ModelBox*>(w);
			break;
		}
	}

	if (mb) {
		chooseModel(mb->model);
	}
}

inline void BrandItem::onAction(const event::Action& e) {
	if (browser->brand == brand)
		browser->brand = "";
	else
		browser->brand = brand;
	browser->refresh();
}

inline void BrandItem::step() {
	rightText = CHECKMARK(browser->brand == brand);
	MenuItem::step();
}

inline void BrandButton::onAction(const event::Action& e) {
	ui::Menu* menu = createMenu();
	menu->box.pos = getAbsoluteOffset(math::Vec(0, box.size.y));
	menu->box.size.x = box.size.x;

	BrandItem* noneItem = new BrandItem;
	noneItem->text = "All brands";
	noneItem->brand = "";
	noneItem->browser = browser;
	menu->addChild(noneItem);

	menu->addChild(new ui::MenuSeparator);

	// Collect brands from all plugins
	std::set<std::string, string::CaseInsensitiveCompare> brands;
	for (plugin::Plugin* plugin : plugin::plugins) {
		brands.insert(plugin->brand);
	}

	for (const std::string& brand : brands) {
		BrandItem* brandItem = new BrandItem;
		brandItem->text = brand;
		brandItem->brand = brand;
		brandItem->browser = browser;
		brandItem->disabled = !browser->hasVisibleModel(brand, browser->tagIds);
		menu->addChild(brandItem);
	}
}

inline void BrandButton::step() {
	text = "Brand";
	if (!browser->brand.empty()) {
		text += ": ";
		text += browser->brand;
	}
	text = string::ellipsize(text, 21);
	ChoiceButton::step();
}

inline void TagItem::onAction(const event::Action& e) {
	auto it = browser->tagIds.find(tagId);
	bool isSelected = (it != browser->tagIds.end());

	if (tagId >= 0) {
		// Actual tag
		int mods = APP->window->getMods();
		if ((mods & RACK_MOD_MASK) == RACK_MOD_CTRL) {
			// Multi select
			if (isSelected)
				browser->tagIds.erase(tagId);
			else
				browser->tagIds.insert(tagId);
			e.unconsume();
		}
		else {
			// Single select
			if (isSelected)
				browser->tagIds = {};
			else {
				browser->tagIds = {tagId};
			}
		}
	}
	else {
		// All tags
		browser->tagIds = {};
	}

	browser->refresh();
}

inline void TagItem::step() {
	// TODO Disable tags with no modules
	if (tagId >= 0) {
		auto it = browser->tagIds.find(tagId);
		bool isSelected = (it != browser->tagIds.end());
		rightText = CHECKMARK(isSelected);
	}
	else {
		rightText = CHECKMARK(browser->tagIds.empty());
	}
	MenuItem::step();
}

inline void TagButton::onAction(const event::Action& e) {
	ui::Menu* menu = createMenu();
	menu->box.pos = getAbsoluteOffset(math::Vec(0, box.size.y));
	menu->box.size.x = box.size.x;

	TagItem* noneItem = new TagItem;
	noneItem->text = "All tags";
	noneItem->tagId = -1;
	noneItem->browser = browser;
	menu->addChild(noneItem);

	menu->addChild(createMenuLabel(RACK_MOD_CTRL_NAME "+click to select multiple"));
	menu->addChild(new ui::MenuSeparator);

	for (int tagId = 0; tagId < (int) tag::tagAliases.size(); tagId++) {
		TagItem* tagItem = new TagItem;
		tagItem->text = tag::getTag(tagId);
		tagItem->tagId = tagId;
		tagItem->browser = browser;
		tagItem->disabled = !browser->hasVisibleModel(browser->brand, {tagId});
		menu->addChild(tagItem);
	}
}

inline void TagButton::step() {
	text = "Tags";
	if (!browser->tagIds.empty()) {
		text += ": ";
		bool firstTag = true;
		for (int tagId : browser->tagIds) {
			if (!firstTag)
				text += ", ";
			text += tag::getTag(tagId);
			firstTag = false;
		}
	}
	text = string::ellipsize(text, 21);
	ChoiceButton::step();
}

inline void SortItem::onAction(const event::Action& e) {
	settings::moduleBrowserSort = sort;
	browser->refresh();
}

inline void ZoomItem::onAction(const event::Action& e) {
	if (zoom != settings::moduleBrowserZoom) {
		settings::moduleBrowserZoom = zoom;
		browser->updateZoom();
	}
}


} // namespace moduleBrowser


widget::Widget* moduleBrowserCreate() {
	moduleBrowser::BrowserOverlay* overlay = new moduleBrowser::BrowserOverlay;

	moduleBrowser::ModuleBrowser* browser = new moduleBrowser::ModuleBrowser;
	overlay->addChild(browser);

	return overlay;
}


} // namespace app
} // namespace rack
