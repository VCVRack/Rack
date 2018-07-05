#ifndef TROWASOFT_MODULE_TSSEQUENCERWIDGETBASE_HPP
#define TROWASOFT_MODULE_TSSEQUENCERWIDGETBASE_HPP

#include "rack.hpp"
using namespace rack;

#include "TSSModuleWidgetBase.hpp"
#include "TSOSCConfigWidget.hpp"

struct TSSeqDisplay;
struct TSSequencerModuleBase;

//-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-
// TSSequencerWidgetBase
// Sequencer Widget Base Class (adds common UI controls).
//-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-
struct TSSequencerWidgetBase : TSSModuleWidgetBase {
	// Top digital display for sequencer.
	TSSeqDisplay *display;
	// OSC configuration widget.
	TSOSCConfigWidget* oscConfigurationScreen;
	// Numer of steps this should have (for when we get a NULL module).
	int maxSteps = 16;
	//-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-
	// TSSequencerWidgetBase() - Base constructor.
	// Instantiate a trowaSoft Sequencer widget. v0.60 must have module as param.
	// @seqModule : (IN) Pointer to the sequencer module.
	//-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-
	TSSequencerWidgetBase(TSSequencerModuleBase* seqModule);
	// Step
	void step() override;
	// Add base controls.
	void addBaseControls() { addBaseControls(false); }
	// Add base controls.
	void addBaseControls(bool addGridLines);
	// Create context menu with common adds to sequencers.
	Menu *createContextMenu() override;
};
#endif