#pragma once
#include "app/common.hpp"
#include "widgets/OpaqueWidget.hpp"
#include "widgets/ZoomWidget.hpp"
#include "ui/ScrollWidget.hpp"
#include "app/RackWidget.hpp"
#include "app/Toolbar.hpp"


namespace rack {


struct Scene : OpaqueWidget {
	// Convenience variables for accessing important widgets
	ScrollWidget *scrollWidget;
	ZoomWidget *zoomWidget;
	RackWidget *rackWidget;
	Toolbar *toolbar;
	Widget *moduleBrowser;

	// Version checking
	bool devMode = false;
	bool checkVersion = true;
	bool checkedVersion = false;
	std::string latestVersion;

	Scene();
	~Scene();
	void step() override;
	void draw(const DrawContext &ctx) override;
	void onHoverKey(const event::HoverKey &e) override;
	void onPathDrop(const event::PathDrop &e) override;

	void runCheckVersion();
};


} // namespace rack
