#pragma once
#include "app/common.hpp"
#include "widgets/TransparentWidget.hpp"
#include "app/WireWidget.hpp"
#include "app/PortWidget.hpp"


namespace rack {


struct WireContainer : TransparentWidget {
	WireWidget *activeWire = NULL;
	/** Takes ownership of `w` and adds it as a child if it isn't already */
	void setActiveWire(WireWidget *w);
	/** "Drops" the wire onto the port, making an engine connection if successful */
	void commitActiveWire();
	void removeTopWire(PortWidget *port);
	void removeAllWires(PortWidget *port);
	/** Returns the most recently added wire connected to the given Port, i.e. the top of the stack */
	WireWidget *getTopWire(PortWidget *port);
	void draw(NVGcontext *vg) override;
};


} // namespace rack
