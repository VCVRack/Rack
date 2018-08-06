#include "global_pre.hpp"
#include "app.hpp"
#include "plugin.hpp"
#include "window.hpp"
#include <set>
#include <algorithm>
#include "global.hpp"
#include "global_ui.hpp"


static const float itemMargin = 2.0;


namespace rack {


bool isMatch(std::string s, std::string search) {
	s = stringLowercase(s);
	search = stringLowercase(search);
	return (s.find(search) != std::string::npos);
}

static bool isModelMatch(Model *model, std::string search) {
	if (search.empty())
		return true;
	std::string s;
	s += model->plugin->slug;
	s += " ";
	s += model->author;
	s += " ";
	s += model->name;
	s += " ";
	s += model->slug;
	for (ModelTag tag : model->tags) {
		s += " ";
		s += gTagNames[tag];
	}
	return isMatch(s, search);
}


struct FavoriteRadioButton : RadioButton {
	Model *model = NULL;

	void onAction(EventAction &e) override;
};


struct SeparatorItem : OpaqueWidget {
	SeparatorItem() {
		box.size.y = 2*BND_WIDGET_HEIGHT + 2*itemMargin;
	}

	void setText(std::string text) {
		clearChildren();
		Label *label = Widget::create<Label>(Vec(0, 12 + itemMargin));
		label->text = text;
		label->fontSize = 20;
		label->color.a *= 0.5;
		addChild(label);
	}
};


struct BrowserListItem : OpaqueWidget {
	bool selected = false;

	BrowserListItem() {
		box.size.y = BND_WIDGET_HEIGHT + 2*itemMargin;
	}

	void draw(NVGcontext *vg) override {
		BNDwidgetState state = selected ? BND_HOVER : BND_DEFAULT;
		bndMenuItem(vg, 0.0, 0.0, box.size.x, box.size.y, state, -1, "");
		Widget::draw(vg);
	}

	void onDragStart(EventDragStart &e) override;

	void onDragDrop(EventDragDrop &e) override {
		if (e.origin != this)
			return;
		doAction();
	}

	void doAction() {
		EventAction eAction;
		eAction.consumed = true;
		onAction(eAction);
		if (eAction.consumed) {
			// deletes `this`
			global_ui->ui.gScene->setOverlay(NULL);
		}
	}
};


struct ModelItem : BrowserListItem {
	Model *model;
	Label *pluginLabel = NULL;

	void setModel(Model *model) {
		clearChildren();
		assert(model);
		this->model = model;

		FavoriteRadioButton *favoriteButton = Widget::create<FavoriteRadioButton>(Vec(8, itemMargin));
		favoriteButton->box.size.x = 20;
		favoriteButton->label = "★";
		addChild(favoriteButton);

		// Set favorite button initial state
		auto it = global_ui->module_browser.sFavoriteModels.find(model);
		if (it != global_ui->module_browser.sFavoriteModels.end())
			favoriteButton->setValue(1);
		favoriteButton->model = model;

		Label *nameLabel = Widget::create<Label>(favoriteButton->box.getTopRight());
		nameLabel->text = model->name;
		addChild(nameLabel);

		pluginLabel = Widget::create<Label>(Vec(0, itemMargin));
		pluginLabel->alignment = Label::RIGHT_ALIGNMENT;
		pluginLabel->text = model->plugin->slug + " " + model->plugin->version;
		pluginLabel->color.a = 0.5;
		addChild(pluginLabel);
	}

	void step() override {
		BrowserListItem::step();
		if (pluginLabel)
			pluginLabel->box.size.x = box.size.x - BND_SCROLLBAR_WIDTH;
	}

	void onAction(EventAction &e) override {
      printf("xxx vcvrack::ModuleBrowser: model=%p, calling createModuleWidget()\n", model);
#ifdef USE_VST2
      if(NULL != model->plugin->set_tls_globals_fxn) {
         model->plugin->set_tls_globals_fxn(model->plugin);
      }
#endif // USE_VST2
		ModuleWidget *moduleWidget = model->createModuleWidget();
      printf("xxx vcvrack::ModuleBrowser: moduleWidget=%p\n", moduleWidget);
		if (!moduleWidget)
			return;
		global_ui->app.gRackWidget->addModule(moduleWidget);
      printf("xxx vcvrack::ModuleBrowser: addModule() OK\n");
		// Move module nearest to the mouse position
		moduleWidget->box.pos = global_ui->app.gRackWidget->lastMousePos.minus(moduleWidget->box.size.div(2));
      printf("xxx vcvrack::ModuleBrowser: box.pos OK\n");
		global_ui->app.gRackWidget->requestModuleBoxNearest(moduleWidget, moduleWidget->box);
      printf("xxx vcvrack::ModuleBrowser: requestModuleBoxNearest() OK\n");
	}
};


struct AuthorItem : BrowserListItem {
	std::string author;

	void setAuthor(std::string author) {
		clearChildren();
		this->author = author;
		Label *authorLabel = Widget::create<Label>(Vec(0, 0 + itemMargin));
		if (author.empty())
			authorLabel->text = "Show all modules";
		else
			authorLabel->text = author;
		addChild(authorLabel);
	}

	void onAction(EventAction &e) override;
};


struct TagItem : BrowserListItem {
	ModelTag tag;

	void setTag(ModelTag tag) {
		clearChildren();
		this->tag = tag;
		Label *tagLabel = Widget::create<Label>(Vec(0, 0 + itemMargin));
		if (tag == NO_TAG)
			tagLabel->text = "Show all tags";
		else
			tagLabel->text = gTagNames[tag];
		addChild(tagLabel);
	}

	void onAction(EventAction &e) override;
};


struct ClearFilterItem : BrowserListItem {
	ClearFilterItem() {
		Label *label = Widget::create<Label>(Vec(0, 0 + itemMargin));
		label->text = "Back";
		addChild(label);
	}

	void onAction(EventAction &e) override;
};


struct BrowserList : List {
	int selected = 0;

	void step() override {
		incrementSelection(0);
		// Find and select item
		int i = 0;
		for (Widget *child : children) {
			BrowserListItem *item = dynamic_cast<BrowserListItem*>(child);
			if (item) {
				item->selected = (i == selected);
				i++;
			}
		}
		List::step();
	}

	void incrementSelection(int delta) {
		selected += delta;
		selected = clamp(selected, 0, countItems() - 1);
	}

	int countItems() {
		int n = 0;
		for (Widget *child : children) {
			BrowserListItem *item = dynamic_cast<BrowserListItem*>(child);
			if (item) {
				n++;
			}
		}
		return n;
	}

	void selectItem(Widget *w) {
		int i = 0;
		for (Widget *child : children) {
			BrowserListItem *item = dynamic_cast<BrowserListItem*>(child);
			if (item) {
				if (child == w) {
					selected = i;
					break;
				}
				i++;
			}
		}
	}

	BrowserListItem *getSelectedItem() {
		int i = 0;
		for (Widget *child : children) {
			BrowserListItem *item = dynamic_cast<BrowserListItem*>(child);
			if (item) {
				if (i == selected) {
					return item;
				}
				i++;
			}
		}
		return NULL;
	}

	void scrollSelected() {
		BrowserListItem *item = getSelectedItem();
		if (item) {
			ScrollWidget *parentScroll = dynamic_cast<ScrollWidget*>(parent->parent);
			if (parentScroll)
				parentScroll->scrollTo(item->box);
		}
	}
};


struct ModuleBrowser;

struct SearchModuleField : TextField {
	ModuleBrowser *moduleBrowser;
	void onTextChange() override;
	void onKey(EventKey &e) override;
};


struct ModuleBrowser : OpaqueWidget {
	SearchModuleField *searchField;
	ScrollWidget *moduleScroll;
	BrowserList *moduleList;
	std::set<std::string, StringCaseInsensitiveCompare> availableAuthors;
	std::set<ModelTag> availableTags;

	ModuleBrowser() {
		box.size.x = 450;
		global_ui->module_browser.sAuthorFilter = "";
		global_ui->module_browser.sTagFilter = NO_TAG;

		// Search
		searchField	= new SearchModuleField();
		searchField->box.size.x = box.size.x;
		searchField->moduleBrowser = this;
		addChild(searchField);

		moduleList = new BrowserList();
		moduleList->box.size = Vec(box.size.x, 0.0);

		// Module Scroll
		moduleScroll = new ScrollWidget();
		moduleScroll->box.pos.y = searchField->box.size.y;
		moduleScroll->box.size.x = box.size.x;
		moduleScroll->container->addChild(moduleList);
		addChild(moduleScroll);

		// Collect authors
		for (Plugin *plugin : global->plugin.gPlugins) {
			for (Model *model : plugin->models) {
				// Insert author
				if (!model->author.empty())
					availableAuthors.insert(model->author);
				// Insert tag
				for (ModelTag tag : model->tags) {
					if (tag != NO_TAG)
						availableTags.insert(tag);
				}
			}
		}

		// Trigger search update
		clearSearch();
	}

	void draw(NVGcontext *vg) override {
		bndMenuBackground(vg, 0.0, 0.0, box.size.x, box.size.y, BND_CORNER_NONE);
		Widget::draw(vg);
	}

	void clearSearch() {
		searchField->setText("");
	}

	bool isModelFiltered(Model *model) {
		if (!global_ui->module_browser.sAuthorFilter.empty() && model->author != global_ui->module_browser.sAuthorFilter)
			return false;
		if (global_ui->module_browser.sTagFilter != NO_TAG) {
			auto it = std::find(model->tags.begin(), model->tags.end(), global_ui->module_browser.sTagFilter);
			if (it == model->tags.end())
				return false;
		}
		return true;
	}

	void refreshSearch() {
		std::string search = searchField->text;
		moduleList->clearChildren();
		moduleList->selected = 0;
		bool filterPage = !(global_ui->module_browser.sAuthorFilter.empty() && global_ui->module_browser.sTagFilter == NO_TAG);

		if (!filterPage) {
			// Favorites
			if (!global_ui->module_browser.sFavoriteModels.empty()) {
				SeparatorItem *item = new SeparatorItem();
				item->setText("Favorites");
				moduleList->addChild(item);
			}
			for (Model *model : global_ui->module_browser.sFavoriteModels) {
				if (isModelFiltered(model) && isModelMatch(model, search)) {
					ModelItem *item = new ModelItem();
					item->setModel(model);
					moduleList->addChild(item);
				}
			}
			// Author items
			{
				SeparatorItem *item = new SeparatorItem();
				item->setText("Authors");
				moduleList->addChild(item);
			}
			for (std::string author : availableAuthors) {
				if (isMatch(author, search)) {
					AuthorItem *item = new AuthorItem();
					item->setAuthor(author);
					moduleList->addChild(item);
				}
			}
			// Tag items
			{
				SeparatorItem *item = new SeparatorItem();
				item->setText("Tags");
				moduleList->addChild(item);
			}
			for (ModelTag tag : availableTags) {
				if (isMatch(gTagNames[tag], search)) {
					TagItem *item = new TagItem();
					item->setTag(tag);
					moduleList->addChild(item);
				}
			}
		}
		else {
			// Clear filter
			ClearFilterItem *item = new ClearFilterItem();
			moduleList->addChild(item);
		}

		if (filterPage || !search.empty()) {
			if (!search.empty()) {
				SeparatorItem *item = new SeparatorItem();
				item->setText("Modules");
				moduleList->addChild(item);
			}
			else if (filterPage) {
				SeparatorItem *item = new SeparatorItem();
				if (!global_ui->module_browser.sAuthorFilter.empty())
					item->setText(global_ui->module_browser.sAuthorFilter);
				else if (global_ui->module_browser.sTagFilter != NO_TAG)
					item->setText("Tag: " + gTagNames[global_ui->module_browser.sTagFilter]);
				moduleList->addChild(item);
			}
			// Modules
			for (Plugin *plugin : global->plugin.gPlugins) {
				for (Model *model : plugin->models) {
					if (isModelFiltered(model) && isModelMatch(model, search)) {
						ModelItem *item = new ModelItem();
						item->setModel(model);
						moduleList->addChild(item);
					}
				}
			}
		}
	}

	void step() override {
		box.pos = parent->box.size.minus(box.size).div(2).round();
		box.pos.y = 60;
		box.size.y = parent->box.size.y - 2 * box.pos.y;
		moduleScroll->box.size.y = min(box.size.y - moduleScroll->box.pos.y, moduleList->box.size.y);
		box.size.y = min(box.size.y, moduleScroll->box.getBottomRight().y);

		global_ui->widgets.gFocusedWidget = searchField;
		Widget::step();
	}
};


// Implementations of inline methods above

void AuthorItem::onAction(EventAction &e) {
	ModuleBrowser *moduleBrowser = getAncestorOfType<ModuleBrowser>();
	global_ui->module_browser.sAuthorFilter = author;
	moduleBrowser->clearSearch();
	moduleBrowser->refreshSearch();
	e.consumed = false;
}

void TagItem::onAction(EventAction &e) {
	ModuleBrowser *moduleBrowser = getAncestorOfType<ModuleBrowser>();
	global_ui->module_browser.sTagFilter = tag;
	moduleBrowser->clearSearch();
	moduleBrowser->refreshSearch();
	e.consumed = false;
}

void ClearFilterItem::onAction(EventAction &e) {
	ModuleBrowser *moduleBrowser = getAncestorOfType<ModuleBrowser>();
	global_ui->module_browser.sAuthorFilter = "";
	global_ui->module_browser.sTagFilter = NO_TAG;
	moduleBrowser->refreshSearch();
	e.consumed = false;
}

void FavoriteRadioButton::onAction(EventAction &e) {
	if (!model)
		return;
	if (value) {
		global_ui->module_browser.sFavoriteModels.insert(model);
	}
	else {
		auto it = global_ui->module_browser.sFavoriteModels.find(model);
		if (it != global_ui->module_browser.sFavoriteModels.end())
			global_ui->module_browser.sFavoriteModels.erase(it);
	}

	ModuleBrowser *moduleBrowser = getAncestorOfType<ModuleBrowser>();
	if (moduleBrowser)
		moduleBrowser->refreshSearch();
}

void BrowserListItem::onDragStart(EventDragStart &e) {
	BrowserList *list = dynamic_cast<BrowserList*>(parent);
	if (list) {
		list->selectItem(this);
	}
}

void SearchModuleField::onTextChange() {
	moduleBrowser->refreshSearch();
}

void SearchModuleField::onKey(EventKey &e) {
	switch (e.key) {
		case LGLW_VKEY_ESCAPE/*GLFW_KEY_ESCAPE*/: {
			global_ui->ui.gScene->setOverlay(NULL);
			e.consumed = true;
			return;
		} break;
		case LGLW_VKEY_UP/*GLFW_KEY_UP*/: {
			moduleBrowser->moduleList->incrementSelection(-1);
			moduleBrowser->moduleList->scrollSelected();
			e.consumed = true;
		} break;
		case LGLW_VKEY_DOWN/*GLFW_KEY_DOWN*/: {
			moduleBrowser->moduleList->incrementSelection(1);
			moduleBrowser->moduleList->scrollSelected();
			e.consumed = true;
		} break;
		case LGLW_VKEY_PAGEUP/*GLFW_KEY_PAGE_UP*/: {
			moduleBrowser->moduleList->incrementSelection(-5);
			moduleBrowser->moduleList->scrollSelected();
			e.consumed = true;
		} break;
		case LGLW_VKEY_PAGEDOWN/*GLFW_KEY_PAGE_DOWN*/: {
			moduleBrowser->moduleList->incrementSelection(5);
			moduleBrowser->moduleList->scrollSelected();
			e.consumed = true;
		} break;
		case LGLW_VKEY_RETURN/*GLFW_KEY_ENTER*/: {
			BrowserListItem *item = moduleBrowser->moduleList->getSelectedItem();
			if (item) {
				item->doAction();
				e.consumed = true;
				return;
			}
		} break;
	}

	if (!e.consumed) {
		TextField::onKey(e);
	}
}

// Global functions

void appModuleBrowserCreate() {
	MenuOverlay *overlay = new MenuOverlay();

	ModuleBrowser *moduleBrowser = new ModuleBrowser();
	overlay->addChild(moduleBrowser);

	global_ui->ui.gScene->setOverlay(overlay);
}

json_t *appModuleBrowserToJson() {
	json_t *rootJ = json_object();

	json_t *favoritesJ = json_array();
	for (Model *model : global_ui->module_browser.sFavoriteModels) {
		json_t *modelJ = json_object();
		json_object_set_new(modelJ, "plugin", json_string(model->plugin->slug.c_str()));
		json_object_set_new(modelJ, "model", json_string(model->slug.c_str()));
		json_array_append_new(favoritesJ, modelJ);
	}
	json_object_set_new(rootJ, "favorites", favoritesJ);

	return rootJ;
}

void appModuleBrowserFromJson(json_t *rootJ) {
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
			Model *model = pluginGetModel(pluginSlug, modelSlug);
			if (!model)
				continue;
			global_ui->module_browser.sFavoriteModels.insert(model);
		}
	}
}


} // namespace rack
