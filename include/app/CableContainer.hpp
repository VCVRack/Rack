#pragma once
#include "app/common.hpp"
#include "widgets/TransparentWidget.hpp"
#include "app/CableWidget.hpp"
#include "app/PortWidget.hpp"
#include <map>


namespace rack {


struct CableContainer : TransparentWidget {
	CableWidget *incompleteCable = NULL;

	~CableContainer();
	void clear();
	/** Removes all complete cables connected to the port */
	void clearPort(PortWidget *port);
	/** Adds a complete cable and adds it to the Engine.
	Ownership rules work like add/removeChild()
	*/
	void addCable(CableWidget *w);
	void removeCable(CableWidget *w);
	/** Takes ownership of `w` and adds it as a child if it isn't already */
	void setIncompleteCable(CableWidget *w);
	CableWidget *releaseIncompleteCable();
	/** Returns the most recently added complete cable connected to the given Port, i.e. the top of the stack */
	CableWidget *getTopCable(PortWidget *port);
	CableWidget *getCable(int cableId);

	json_t *toJson();
	void fromJson(json_t *rootJ, const std::map<int, ModuleWidget*> &moduleWidgets);
	void draw(NVGcontext *vg) override;
};


} // namespace rack
