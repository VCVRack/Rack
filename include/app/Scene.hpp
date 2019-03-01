#pragma once
#include "app/common.hpp"
#include "widget/OpaqueWidget.hpp"
#include "app/RackScrollWidget.hpp"
#include "app/RackWidget.hpp"
#include "app/Toolbar.hpp"


namespace rack {
namespace app {


struct Scene : widget::OpaqueWidget {
	// Convenience variables for accessing important widgets
	RackScrollWidget *rackScroll;
	RackWidget *rack;
	Toolbar *toolbar;
	widget::Widget *moduleBrowser;

	// Version checking
	bool devMode = false;
	bool checkVersion = true;
	bool checkedVersion = false;
	std::string latestVersion;

	double lastAutoSaveTime = 0.0;

	Scene();
	~Scene();
	void step() override;
	void draw(const DrawArgs &args) override;
	void onHoverKey(const widget::HoverKeyEvent &e) override;
	void onPathDrop(const widget::PathDropEvent &e) override;

	void runCheckVersion();
};


} // namespace app
} // namespace rack
