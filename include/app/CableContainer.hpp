#pragma once
#include "app/common.hpp"
#include "widgets/TransparentWidget.hpp"
#include "app/CableWidget.hpp"
#include "app/PortWidget.hpp"


namespace rack {


struct CableContainer : TransparentWidget {
	CableWidget *activeCable = NULL;
	/** Takes ownership of `w` and adds it as a child if it isn't already */
	void setActiveCable(CableWidget *w);
	/** "Drops" the cable onto the port, making an engine connection if successful */
	void commitActiveCable();
	void removeTopCable(PortWidget *port);
	void removeAllCables(PortWidget *port);
	/** Returns the most recently added cable connected to the given Port, i.e. the top of the stack */
	CableWidget *getTopCable(PortWidget *port);
	void draw(NVGcontext *vg) override;
};


} // namespace rack
