#include "app.hpp"
#include "plugin.hpp"
#include "window.hpp"
#include <set>


#define BND_LABEL_FONT_SIZE 13


namespace rack {


static std::string sSearch;
static std::set<Model*> sFavoriteModels;


struct FavoriteRadioButton : RadioButton {
	Model *model = NULL;
	void onChange(EventChange &e) override {
		debug("HI");
		if (!model)
			return;
		if (value) {
			sFavoriteModels.insert(model);
		}
		else {
			auto it = sFavoriteModels.find(model);
			if (it != sFavoriteModels.end())
				sFavoriteModels.erase(it);
		}
	}
};


struct ModuleListItem : OpaqueWidget {
	bool selected = false;
	FavoriteRadioButton *favoriteButton;

	ModuleListItem() {
		box.size.y = 3 * BND_WIDGET_HEIGHT;

		favoriteButton = new FavoriteRadioButton();
		favoriteButton->box.pos = Vec(7, BND_WIDGET_HEIGHT);
		favoriteButton->box.size.x = 20;
		favoriteButton->label = "★";
		addChild(favoriteButton);
	}

	void draw(NVGcontext *vg) override {
		BNDwidgetState state = selected ? BND_HOVER : BND_DEFAULT;
		bndMenuItem(vg, 0.0, 0.0, box.size.x, box.size.y, state, -1, "");
		Widget::draw(vg);
	}

	void onDragDrop(EventDragDrop &e) override {
		if (e.origin != this)
			return;

		EventAction eAction;
		eAction.consumed = true;
		onAction(eAction);
		if (eAction.consumed) {
			// deletes `this`
			gScene->setOverlay(NULL);
		}
	}
};

struct ModelItem : ModuleListItem {
	Model *model;

	void setModel(Model *model) {
		this->model = model;
		auto it = sFavoriteModels.find(model);
		if (it != sFavoriteModels.end())
			favoriteButton->setValue(1);
		favoriteButton->model = model;
	}

	void draw(NVGcontext *vg) override {
		ModuleListItem::draw(vg);

		// bndMenuItem(vg, 0.0, 0.0, box.size.x, box.size.y, BND_DEFAULT, -1, model->name.c_str());

		float x = box.size.x - bndLabelWidth(vg, -1, model->manufacturer.c_str());
		NVGcolor rightColor = bndGetTheme()->menuTheme.textColor;
		bndIconLabelValue(vg, x, 0.0, box.size.x, box.size.y, -1, rightColor, BND_LEFT, BND_LABEL_FONT_SIZE, model->manufacturer.c_str(), NULL);
	}

	void onAction(EventAction &e) override {
		ModuleWidget *moduleWidget = model->createModuleWidget();
		gRackWidget->moduleContainer->addChild(moduleWidget);
		// Move module nearest to the mouse position
		// Rect box;
		// box.size = moduleWidget->box.size;
		// AddModuleWindow *w = getAncestorOfType<AddModuleWindow>();
		// box.pos = w->modulePos.minus(box.getCenter());
		// gRackWidget->requestModuleBoxNearest(moduleWidget, box);
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
	List *moduleList;

	ModuleBrowser() {
		box.size.x = 300;

		// Search
		searchField	= new SearchModuleField();
		searchField->box.size.x = box.size.x;
		searchField->moduleBrowser = this;
		addChild(searchField);

		moduleList = new List();
		moduleList->box.size = Vec(box.size.x, 0.0);

		// Module Scroll
		moduleScroll = new ScrollWidget();
		moduleScroll->box.pos.y = searchField->box.size.y;
		moduleScroll->box.size.x = box.size.x;
		moduleScroll->container->addChild(moduleList);
		addChild(moduleScroll);

		// Focus search
		searchField->setText(sSearch);
		EventFocus eFocus;
		searchField->onFocus(eFocus);
	}

	void setSearch(std::string search) {
		moduleList->clearChildren();

		// Favorites
		for (Model *model : sFavoriteModels) {
			ModelItem *item = new ModelItem();
			item->setModel(model);
			moduleList->addChild(item);
		}

		// Models
		for (Plugin *plugin : gPlugins) {
			for (Model *model : plugin->models) {
				ModelItem *item = new ModelItem();
				item->setModel(model);
				moduleList->addChild(item);
			}
		}
	}

	void step() override {
		box.pos = parent->box.size.minus(box.size).div(2).round();
		box.pos.y = 40;
		box.size.y = parent->box.size.y - 2 * box.pos.y;

		moduleScroll->box.size.y = box.size.y - moduleScroll->box.pos.y;
		gFocusedWidget = searchField;
		Widget::step();
	}
};


void SearchModuleField::onTextChange() {
	sSearch = text;
	moduleBrowser->setSearch(text);
}

void SearchModuleField::onKey(EventKey &e) {
	switch (e.key) {
		case GLFW_KEY_ESCAPE: {
			gScene->setOverlay(NULL);
			e.consumed = true;
			return;
		} break;
	}

	if (!e.consumed) {
		TextField::onKey(e);
	}
}


void appModuleBrowserCreate() {
	MenuOverlay *overlay = new MenuOverlay();

	ModuleBrowser *moduleBrowser = new ModuleBrowser();
	overlay->addChild(moduleBrowser);
	gScene->setOverlay(overlay);
}

json_t *appModuleBrowserToJson() {
	// TODO
	return json_object();
}

void appModuleBrowserFromJson(json_t *root) {
	// TODO
}


} // namespace rack
