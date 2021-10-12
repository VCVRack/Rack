#pragma once
#include <app/common.hpp>
#include <widget/OpaqueWidget.hpp>
#include <ui/Tooltip.hpp>
#include <engine/Module.hpp>
#include <engine/PortInfo.hpp>


namespace rack {
namespace app {


/** Manages an engine::Port on a ModuleWidget. */
struct PortWidget : widget::OpaqueWidget {
	struct Internal;
	Internal* internal;

	engine::Module* module = NULL;
	engine::Port::Type type = engine::Port::INPUT;
	int portId = -1;

	PortWidget();
	~PortWidget();
	engine::Port* getPort();
	engine::PortInfo* getPortInfo();
	void createTooltip();
	void destroyTooltip();
	void createContextMenu();
	virtual void appendContextMenu(ui::Menu* menu) {}
	void deleteTopCableAction();

	void step() override;
	void draw(const DrawArgs& args) override;

	void onButton(const ButtonEvent& e) override;
	void onEnter(const EnterEvent& e) override;
	void onLeave(const LeaveEvent& e) override;
	void onDragStart(const DragStartEvent& e) override;
	void onDragEnd(const DragEndEvent& e) override;
	void onDragDrop(const DragDropEvent& e) override;
	void onDragEnter(const DragEnterEvent& e) override;
	void onDragLeave(const DragLeaveEvent& e) override;
};


} // namespace app
} // namespace rack
