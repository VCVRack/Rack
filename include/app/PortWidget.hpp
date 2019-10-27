#pragma once
#include <app/common.hpp>
#include <widget/OpaqueWidget.hpp>
#include <ui/Tooltip.hpp>
#include <app/MultiLightWidget.hpp>
#include <engine/Module.hpp>
#include <engine/PortInfo.hpp>


namespace rack {
namespace app {


/** Manages an engine::Port on a ModuleWidget. */
struct PortWidget : widget::OpaqueWidget {
	engine::Module* module = NULL;
	engine::Port::Type type = engine::Port::INPUT;
	int portId = 0;

	ui::Tooltip* tooltip = NULL;
	bool hovered = false;

	MultiLightWidget* plugLight;

	PortWidget();
	~PortWidget();
	engine::Port* getPort();
	engine::PortInfo* getPortInfo();
	void createTooltip();
	void destroyTooltip();

	void step() override;
	void draw(const DrawArgs& args) override;

	void onButton(const event::Button& e) override;
	void onEnter(const event::Enter& e) override;
	void onLeave(const event::Leave& e) override;
	void onDragStart(const event::DragStart& e) override;
	void onDragEnd(const event::DragEnd& e) override;
	void onDragDrop(const event::DragDrop& e) override;
	void onDragEnter(const event::DragEnter& e) override;
	void onDragLeave(const event::DragLeave& e) override;
};


} // namespace app
} // namespace rack
