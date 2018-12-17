#pragma once
#include "app/common.hpp"
#include "widgets/OpaqueWidget.hpp"
#include "app/MultiLightWidget.hpp"
#include "engine/Module.hpp"


namespace rack {


struct Port : OpaqueWidget {
	Module *module = NULL;
	int portId;

	enum PortType {
		INPUT,
		OUTPUT
	};
	PortType type = INPUT;
	MultiLightWidget *plugLight;

	Port();
	~Port();
	void step() override;
	void draw(NVGcontext *vg) override;
	void onButton(event::Button &e) override;
	void onDragStart(event::DragStart &e) override;
	void onDragEnd(event::DragEnd &e) override;
	void onDragDrop(event::DragDrop &e) override;
	void onDragEnter(event::DragEnter &e) override;
	void onDragLeave(event::DragLeave &e) override;
};


} // namespace rack
