#pragma once
#include "app/common.hpp"


namespace rack {


struct Port : Component {
	enum PortType {
		INPUT,
		OUTPUT
	};
	PortType type = INPUT;
	int portId;
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
