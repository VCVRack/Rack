#pragma once
#include <app/common.hpp>
#include <widget/OpaqueWidget.hpp>
#include <app/RackScrollWidget.hpp>
#include <app/RackWidget.hpp>
#include <app/MenuBar.hpp>


namespace rack {
namespace app {


struct Scene : widget::OpaqueWidget {
	// Convenience variables for accessing important widgets
	RackScrollWidget* rackScroll;
	RackWidget* rack;
	MenuBar* menuBar;
	widget::Widget* moduleBrowser;

	double lastAutosaveTime = 0.0;

	// Version checking
	bool checkVersion = true;
	bool checkedVersion = false;
	std::string latestVersion;

	Scene();
	~Scene();
	void step() override;
	void draw(const DrawArgs& args) override;
	void onHoverKey(const event::HoverKey& e) override;
	void onPathDrop(const event::PathDrop& e) override;

	void runCheckVersion();
};


} // namespace app
} // namespace rack
