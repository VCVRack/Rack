#pragma once
#include "app/common.hpp"
#include "widget/OpaqueWidget.hpp"
#include "widget/ZoomWidget.hpp"
#include "ui/ScrollWidget.hpp"
#include "app/RackWidget.hpp"
#include "app/Toolbar.hpp"


namespace rack {
namespace app {


struct Scene : widget::OpaqueWidget {
	// Convenience variables for accessing important widgets
	ui::ScrollWidget *scrollWidget;
	widget::ZoomWidget *zoomWidget;
	RackWidget *rackWidget;
	Toolbar *toolbar;
	widget::Widget *moduleBrowser;

	// Version checking
	bool devMode = false;
	bool checkVersion = true;
	bool checkedVersion = false;
	std::string latestVersion;

	Scene();
	~Scene();
	void step() override;
	void draw(const widget::DrawContext &ctx) override;
	void onHoverKey(const event::HoverKey &e) override;
	void onPathDrop(const event::PathDrop &e) override;

	void runCheckVersion();
};


} // namespace app
} // namespace rack
