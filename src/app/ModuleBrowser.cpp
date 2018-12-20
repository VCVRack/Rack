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
#include "plugin.hpp"
#include "context.hpp"
#include "logger.hpp"


static const float itemMargin = 2.0;


namespace rack {


static std::set<Model*> sFavoriteModels;
static std::string sAuthorFilter;
static std::string sTagFilter;



bool isMatch(std::string s, std::string search) {
	s = string::lowercase(s);
	search = string::lowercase(search);
	return (s.find(search) != std::string::npos);
}

static bool isModelMatch(Model *model, std::string search) {
	if (search.empty())
		return true;
	std::string s;
	s += model->plugin->slug;
	s += " ";
	s += model->plugin->author;
	s += " ";
	s += model->name;
	s += " ";
	s += model->slug;
	for (std::string tag : model->tags) {
		std::string allowedTag = plugin::getAllowedTag(tag);
		if (!allowedTag.empty()) {
			s += " ";
			s += allowedTag;
		}
	}
	return isMatch(s, search);
}


struct FavoriteQuantity : Quantity {
	std::string getString() override {
		return "★";
	}
};


struct FavoriteRadioButton : RadioButton {
	Model *model = NULL;

	FavoriteRadioButton() {
		quantity = new FavoriteQuantity;
	}

	void onAction(event::Action &e) override;
};


struct SeparatorItem : OpaqueWidget {
	SeparatorItem() {
		box.size.y = 2*BND_WIDGET_HEIGHT + 2*itemMargin;
	}

	void setText(std::string text) {
		clearChildren();
		Label *label = createWidget<Label>(math::Vec(0, 12 + itemMargin));
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

	void onDragStart(event::DragStart &e) override;

	void onDragDrop(event::DragDrop &e) override {
		if (e.origin != this)
			return;
		doAction();
	}

	void doAction() {
		event::Action eAction;
		eAction.target = this;
		onAction(eAction);
		if (eAction.target) {
			MenuOverlay *overlay = getAncestorOfType<MenuOverlay>();
			overlay->requestedDelete = true;
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

		FavoriteRadioButton *favoriteButton = createWidget<FavoriteRadioButton>(math::Vec(8, itemMargin));
		favoriteButton->box.size.x = 20;
		addChild(favoriteButton);

		// Set favorite button initial state
		auto it = sFavoriteModels.find(model);
		if (it != sFavoriteModels.end())
			favoriteButton->quantity->setValue(1);
		favoriteButton->model = model;

		Label *nameLabel = createWidget<Label>(favoriteButton->box.getTopRight());
		nameLabel->text = model->name;
		addChild(nameLabel);

		pluginLabel = createWidget<Label>(math::Vec(0, itemMargin));
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

	void onAction(event::Action &e) override {
		ModuleWidget *moduleWidget = model->createModuleWidget();
		if (!moduleWidget)
			return;
		context()->scene->rackWidget->addModule(moduleWidget);
		// Move module nearest to the mouse position
		moduleWidget->box.pos = context()->scene->rackWidget->lastMousePos.minus(moduleWidget->box.size.div(2));
		context()->scene->rackWidget->requestModuleBoxNearest(moduleWidget, moduleWidget->box);
	}
};


struct AuthorItem : BrowserListItem {
	std::string author;

	void setAuthor(std::string author) {
		clearChildren();
		this->author = author;
		Label *authorLabel = createWidget<Label>(math::Vec(0, 0 + itemMargin));
		if (author.empty())
			authorLabel->text = "Show all modules";
		else
			authorLabel->text = author;
		addChild(authorLabel);
	}

	void onAction(event::Action &e) override;
};


struct TagItem : BrowserListItem {
	std::string tag;

	void setTag(std::string tag) {
		clearChildren();
		this->tag = tag;
		Label *tagLabel = createWidget<Label>(math::Vec(0, 0 + itemMargin));
		if (tag.empty())
			tagLabel->text = "Show all tags";
		else
			tagLabel->text = tag;
		addChild(tagLabel);
	}

	void onAction(event::Action &e) override;
};


struct ClearFilterItem : BrowserListItem {
	ClearFilterItem() {
		Label *label = createWidget<Label>(math::Vec(0, 0 + itemMargin));
		label->text = "Back";
		addChild(label);
	}

	void onAction(event::Action &e) override;
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
		selected = math::clamp(selected, 0, countItems() - 1);
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
	void onChange(event::Change &e) override;
	void onSelectKey(event::SelectKey &e) override;
};


struct ModuleBrowser : OpaqueWidget {
	SearchModuleField *searchField;
	ScrollWidget *moduleScroll;
	BrowserList *moduleList;
	std::set<std::string, string::CaseInsensitiveCompare> availableAuthors;
	std::set<std::string> availableTags;

	ModuleBrowser() {
		box.size.x = 450;
		sAuthorFilter = "";
		sTagFilter = "";

		// Search
		searchField	= new SearchModuleField;
		searchField->box.size.x = box.size.x;
		searchField->moduleBrowser = this;
		addChild(searchField);

		moduleList = new BrowserList;
		moduleList->box.size = math::Vec(box.size.x, 0.0);

		// Module Scroll
		moduleScroll = new ScrollWidget;
		moduleScroll->box.pos.y = searchField->box.size.y;
		moduleScroll->box.size.x = box.size.x;
		moduleScroll->container->addChild(moduleList);
		addChild(moduleScroll);

		// Collect authors
		for (Plugin *plugin : plugin::plugins) {
			// Insert author
			if (!plugin->author.empty())
				availableAuthors.insert(plugin->author);
			for (Model *model : plugin->models) {
				// Insert tag
				for (std::string tag : model->tags) {
					std::string allowedTag = plugin::getAllowedTag(tag);
					if (!allowedTag.empty())
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
		if (!sAuthorFilter.empty() && model->plugin->author != sAuthorFilter)
			return false;
		if (!sTagFilter.empty()) {
			// TODO filter tags
		}
		return true;
	}

	void refreshSearch() {
		std::string search = searchField->text;
		moduleList->clearChildren();
		moduleList->selected = 0;
		bool filterPage = !(sAuthorFilter.empty() && sTagFilter.empty());

		if (!filterPage) {
			// Favorites
			if (!sFavoriteModels.empty()) {
				SeparatorItem *item = new SeparatorItem;
				item->setText("Favorites");
				moduleList->addChild(item);
			}
			for (Model *model : sFavoriteModels) {
				if (isModelFiltered(model) && isModelMatch(model, search)) {
					ModelItem *item = new ModelItem;
					item->setModel(model);
					moduleList->addChild(item);
				}
			}
			// Author items
			{
				SeparatorItem *item = new SeparatorItem;
				item->setText("Authors");
				moduleList->addChild(item);
			}
			for (std::string author : availableAuthors) {
				if (isMatch(author, search)) {
					AuthorItem *item = new AuthorItem;
					item->setAuthor(author);
					moduleList->addChild(item);
				}
			}
			// Tag items
			{
				SeparatorItem *item = new SeparatorItem;
				item->setText("Tags");
				moduleList->addChild(item);
			}
			for (std::string tag : availableTags) {
				if (isMatch(tag, search)) {
					TagItem *item = new TagItem;
					item->setTag(tag);
					moduleList->addChild(item);
				}
			}
		}
		else {
			// Clear filter
			ClearFilterItem *item = new ClearFilterItem;
			moduleList->addChild(item);
		}

		if (filterPage || !search.empty()) {
			if (!search.empty()) {
				SeparatorItem *item = new SeparatorItem;
				item->setText("Modules");
				moduleList->addChild(item);
			}
			else if (filterPage) {
				SeparatorItem *item = new SeparatorItem;
				if (!sAuthorFilter.empty())
					item->setText(sAuthorFilter);
				else if (!sTagFilter.empty())
					item->setText("Tag: " + sTagFilter);
				moduleList->addChild(item);
			}
			// Modules
			for (Plugin *plugin : plugin::plugins) {
				for (Model *model : plugin->models) {
					if (isModelFiltered(model) && isModelMatch(model, search)) {
						ModelItem *item = new ModelItem;
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
		moduleScroll->box.size.y = std::min(box.size.y - moduleScroll->box.pos.y, moduleList->box.size.y);
		box.size.y = std::min(box.size.y, moduleScroll->box.getBottomRight().y);

		context()->event->selectedWidget = searchField;
		Widget::step();
	}
};


// Implementations of inline methods above

void AuthorItem::onAction(event::Action &e) {
	ModuleBrowser *moduleBrowser = getAncestorOfType<ModuleBrowser>();
	sAuthorFilter = author;
	moduleBrowser->clearSearch();
	moduleBrowser->refreshSearch();
	e.target = this;
}

void TagItem::onAction(event::Action &e) {
	ModuleBrowser *moduleBrowser = getAncestorOfType<ModuleBrowser>();
	sTagFilter = tag;
	moduleBrowser->clearSearch();
	moduleBrowser->refreshSearch();
	e.target = this;
}

void ClearFilterItem::onAction(event::Action &e) {
	ModuleBrowser *moduleBrowser = getAncestorOfType<ModuleBrowser>();
	sAuthorFilter = "";
	sTagFilter = "";
	moduleBrowser->refreshSearch();
	e.target = this;
}

void FavoriteRadioButton::onAction(event::Action &e) {
	if (!model)
		return;
	if (quantity->isMax()) {
		sFavoriteModels.insert(model);
	}
	else {
		auto it = sFavoriteModels.find(model);
		if (it != sFavoriteModels.end())
			sFavoriteModels.erase(it);
	}

	ModuleBrowser *moduleBrowser = getAncestorOfType<ModuleBrowser>();
	if (moduleBrowser)
		moduleBrowser->refreshSearch();
}

void BrowserListItem::onDragStart(event::DragStart &e) {
	BrowserList *list = dynamic_cast<BrowserList*>(parent);
	if (list) {
		list->selectItem(this);
	}
}

void SearchModuleField::onChange(event::Change &e) {
	moduleBrowser->refreshSearch();
}

void SearchModuleField::onSelectKey(event::SelectKey &e) {
	if (e.action == GLFW_PRESS) {
		switch (e.key) {
			case GLFW_KEY_ESCAPE: {
				MenuOverlay *overlay = getAncestorOfType<MenuOverlay>();
				overlay->requestedDelete = true;
				e.target = this;
				return;
			} break;
			case GLFW_KEY_UP: {
				moduleBrowser->moduleList->incrementSelection(-1);
				moduleBrowser->moduleList->scrollSelected();
				e.target = this;
			} break;
			case GLFW_KEY_DOWN: {
				moduleBrowser->moduleList->incrementSelection(1);
				moduleBrowser->moduleList->scrollSelected();
				e.target = this;
			} break;
			case GLFW_KEY_PAGE_UP: {
				moduleBrowser->moduleList->incrementSelection(-5);
				moduleBrowser->moduleList->scrollSelected();
				e.target = this;
			} break;
			case GLFW_KEY_PAGE_DOWN: {
				moduleBrowser->moduleList->incrementSelection(5);
				moduleBrowser->moduleList->scrollSelected();
				e.target = this;
			} break;
			case GLFW_KEY_ENTER: {
				BrowserListItem *item = moduleBrowser->moduleList->getSelectedItem();
				if (item) {
					item->doAction();
					e.target = this;
				}
			} break;
		}
	}

	if (!e.target)
		TextField::onSelectKey(e);
}

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
