#include "app.hpp"
#include "plugin.hpp"
#include <thread>
#include <set>
#include <algorithm>


namespace rack {


static std::string sManufacturer;
static Model *sModel = NULL;
static std::string sFilter;


struct ListMenu : OpaqueWidget {
	void draw(NVGcontext *vg) override {
		Widget::draw(vg);
	}

	void step() override {
		Widget::step();

		box.size.y = 0;
		for (Widget *child : children) {
			if (!child->visible)
				continue;
			// Increase height, set position of child
			child->box.pos = Vec(0, box.size.y);
			box.size.y += child->box.size.y;
			child->box.size.x = box.size.x;
		}
	}
};


struct UrlItem : MenuItem {
	std::string url;
	void onAction(EventAction &e) override {
		std::thread t(openBrowser, url);
		t.detach();
	}
};


struct MetadataMenu : ListMenu {
	Model *model = NULL;

	void step() override {
		if (model != sModel) {
			model = sModel;
			clearChildren();

			if (model) {
				// Tag list
				if (!model->tags.empty()) {
					for (ModelTag tag : model->tags) {
						if (0 <= tag && tag < NUM_TAGS) {
							addChild(construct<MenuLabel>(&MenuEntry::text, gTagNames[tag]));
						}
					}
					addChild(construct<MenuEntry>());
				}

				// Plugin name
				std::string pluginName = model->plugin->slug;
				if (!model->plugin->version.empty()) {
					pluginName += " v";
					pluginName += model->plugin->version;
				}
				addChild(construct<MenuLabel>(&MenuEntry::text, pluginName));

				// Plugin metadata
				if (!model->plugin->website.empty()) {
					addChild(construct<UrlItem>(&MenuEntry::text, "Website", &UrlItem::url, model->plugin->path));
				}
				if (!model->plugin->manual.empty()) {
					addChild(construct<UrlItem>(&MenuEntry::text, "Manual", &UrlItem::url, model->plugin->manual));
				}
				if (!model->plugin->path.empty()) {
					addChild(construct<UrlItem>(&MenuEntry::text, "Browse directory", &UrlItem::url, model->plugin->path));
				}
			}
		}

		ListMenu::step();
	}
};


static bool isModelMatch(Model *model, std::string search) {
	// Build content string
	std::string str;
	str += model->manufacturer;
	str += " ";
	str += model->name;
	str += " ";
	str += model->slug;
	for (ModelTag tag : model->tags) {
		if (0 <= tag && tag < NUM_TAGS) {
			str += " ";
			str += gTagNames[tag];
		}
	}
	str = tolower(str);
	search = tolower(search);
	return (str.find(search) != std::string::npos);
}


struct ModelItem : MenuItem {
	Model *model;
	void onAction(EventAction &e) override {
		ModuleWidget *moduleWidget = model->createModuleWidget();
		gRackWidget->moduleContainer->addChild(moduleWidget);
		// Move module nearest to the mouse position
		Rect box;
		box.size = moduleWidget->box.size;
		AddModuleWindow *w = getAncestorOfType<AddModuleWindow>();
		box.pos = w->modulePos.minus(box.getCenter());
		gRackWidget->requestModuleBoxNearest(moduleWidget, box);
	}
	void onMouseEnter(EventMouseEnter &e) override {
		sModel = model;
		MenuItem::onMouseEnter(e);
	}
	void onMouseLeave(EventMouseLeave &e) override {
		sModel = NULL;
		MenuItem::onMouseLeave(e);
	}
};


struct ModelMenu : ListMenu {
	std::string manufacturer;
	std::string filter;

	void step() override {
		if (manufacturer != sManufacturer) {
			manufacturer = sManufacturer;
			filter = "";
			clearChildren();
			addChild(construct<MenuLabel>(&MenuLabel::text, manufacturer));
			// Add models for the selected manufacturer
			for (Plugin *plugin : gPlugins) {
				for (Model *model : plugin->models) {
					if (model->manufacturer == manufacturer) {
						addChild(construct<ModelItem>(&MenuEntry::text, model->name, &ModelItem::model, model));
					}
				}
			}
		}

		if (filter != sFilter) {
			filter = sFilter;
			// Make all children invisible
			for (Widget *child : children) {
				child->visible = false;
			}
			// Make children with a matching model visible
			for (Widget *child : children) {
				ModelItem *item = dynamic_cast<ModelItem*>(child);
				if (!item)
					continue;

				if (isModelMatch(item->model, filter)) {
					item->visible = true;
				}
			}
		}

		ListMenu::step();
	}
};


struct ManufacturerItem : MenuItem {
	Model *model;
	void onAction(EventAction &e) override {
		sManufacturer = text;
		e.consumed = false;
	}
};


struct ManufacturerMenu : ListMenu {
	std::string filter;

	ManufacturerMenu() {
		addChild(construct<MenuLabel>(&MenuLabel::text, "Manufacturers"));

		// Collect manufacturer names
		std::set<std::string> manufacturers;
		for (Plugin *plugin : gPlugins) {
			for (Model *model : plugin->models) {
				manufacturers.insert(model->manufacturer);
			}
		}
		// Add menu item for each manufacturer name
		for (std::string manufacturer : manufacturers) {
			addChild(construct<ManufacturerItem>(&MenuEntry::text, manufacturer));
		}
	}

	void step() override {
		if (filter != sFilter) {
			// Make all children invisible
			for (Widget *child : children) {
				child->visible = false;
			}
			// Make children with a matching model visible
			for (Widget *child : children) {
				MenuItem *item = dynamic_cast<MenuItem*>(child);
				if (!item)
					continue;

				std::string manufacturer = item->text;
				for (Plugin *plugin : gPlugins) {
					for (Model *model : plugin->models) {
						if (model->manufacturer == manufacturer) {
							if (isModelMatch(model, sFilter)) {
								item->visible = true;
							}
						}
					}
				}
			}
			filter = sFilter;
		}

		ListMenu::step();
	}
};


struct SearchModuleField : TextField {
	void onTextChange() override {
		sFilter = text;
	}
};


AddModuleWindow::AddModuleWindow() {
	box.size = Vec(600, 300);
	title = "Add module";

	float posY = BND_NODE_TITLE_HEIGHT;

	// Search
	SearchModuleField *searchField	= new SearchModuleField();
	searchField->box.pos.y = posY;
	posY += searchField->box.size.y;
	searchField->box.size.x = box.size.x;
	searchField->text = sFilter;
	gFocusedWidget = searchField;
	{
		EventFocus eFocus;
		searchField->onFocus(eFocus);
		searchField->onTextChange();
	}
	addChild(searchField);

	// Manufacturers
	ManufacturerMenu *manufacturerMenu = new ManufacturerMenu();
	manufacturerMenu->box.size.x = 200;

	ScrollWidget *manufacturerScroll = new ScrollWidget();
	manufacturerScroll->container->addChild(manufacturerMenu);
	manufacturerScroll->box.pos = Vec(0, posY);
	manufacturerScroll->box.size = Vec(200, box.size.y - posY);
	addChild(manufacturerScroll);

	// Models
	ModelMenu *modelMenu = new ModelMenu();
	modelMenu->box.size.x = 200;

	ScrollWidget *modelScroll = new ScrollWidget();
	modelScroll->container->addChild(modelMenu);
	modelScroll->box.pos = Vec(200, posY);
	modelScroll->box.size = Vec(200, box.size.y - posY);
	addChild(modelScroll);

	// Metadata
	MetadataMenu *metadataMenu = new MetadataMenu();
	metadataMenu->box.size.x = 200;

	ScrollWidget *metadataScroll = new ScrollWidget();
	metadataScroll->container->addChild(metadataMenu);
	metadataScroll->box.pos = Vec(400, posY);
	metadataScroll->box.size = Vec(200, box.size.y - posY);
	addChild(metadataScroll);
}


void AddModuleWindow::step() {
	Widget::step();
}


} // namespace rack
