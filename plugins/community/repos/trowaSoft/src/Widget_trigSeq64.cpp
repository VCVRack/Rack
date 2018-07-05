#include <string.h>
#include <stdio.h>
#include "trowaSoft.hpp"
#include "dsp/digital.hpp"
#include "trowaSoftComponents.hpp"
#include "trowaSoftUtilities.hpp"
#include "TSSequencerModuleBase.hpp"
#include "Module_trigSeq.hpp"



//-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-
// trigSeq64Widget()
// Widget for the trowaSoft 64-step sequencer.
// @seqModule : (IN) Pointer to the sequencer module.
//-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-
trigSeq64Widget::trigSeq64Widget(trigSeq* seqModule) : TSSequencerWidgetBase(seqModule)
{
	// [02/24/2018] Adjusted for 0.60 differences. Main issue is possiblity of NULL module...
	bool isPreview = seqModule == NULL; // If this is null, then this isn't a real module instance but a 'Preview'?
	maxSteps = N64_NUM_STEPS;

	//trigSeq *module = new trigSeq(N64_NUM_STEPS, N64_NUM_ROWS, N64_NUM_COLS);
	//setModule(module);
	
	//////////////////////////////////////////////
	// Background
	//////////////////////////////////////////////	
	{
		SVGPanel *panel = new SVGPanel();
		panel->box.size = box.size;
		panel->setBackground(SVG::load(assetPlugin(plugin, "res/trigSeq.svg")));
		addChild(panel);
	}
	
	TSSequencerWidgetBase::addBaseControls(true);
	
	// (User) Input Pads ==================================================	
	int y = 115;
	int x = 79;
	int dx = 0;
	Vec padSize = Vec(24, 24);
	Vec lSize = Vec(padSize.x - 2*dx, padSize.y - 2*dx);
	int spacing = padSize.x + 5;
	NVGcolor lightColor = COLOR_TS_RED;
	int numCols = N64_NUM_COLS;
	int numRows = N64_NUM_ROWS;
	int groupId = 0;
	if (!isPreview)
	{
		numCols = seqModule->numCols;
		numRows = seqModule->numRows;
		lightColor = seqModule->voiceColors[seqModule->currentChannelEditingIx];
		groupId = seqModule->oscId; // Use this id for now since this is unique to each module instance.
	}
	int id = 0;
	for (int r = 0; r < numRows; r++) //---------THE PADS
	{
		for (int c = 0; c < numCols; c++)
		{			
			// Pad buttons:
			TS_PadSwitch* pad = new TS_PadSwitch(padSize);
			pad->box.pos = Vec(x, y);
			pad->btnId = id;
			pad->groupId = groupId;
			pad->module = seqModule;
			pad->paramId = TSSequencerModuleBase::CHANNEL_PARAM + id;
			pad->setLimits(0, 1);
			pad->setDefaultValue(0);
			pad->value = 0;
			addParam(pad);

			// Lights:
			TS_LightSquare* padLight = dynamic_cast<TS_LightSquare*>(TS_createColorValueLight<TS_LightSquare>(/*pos */ Vec(x + dx, y + dx),
				/*seqModule*/ seqModule,
				/*lightId*/ TSSequencerModuleBase::PAD_LIGHTS + id, // r * numCols + c
				/* size */ lSize, /* color */ lightColor));
			addChild(padLight);
			if (seqModule != NULL)
			{
				// Keep a reference to our pad lights so we can change the colors
				seqModule->padLightPtrs[r][c] = padLight;
			}			
			id++;
			x+= spacing;
		}		
		y += spacing; // Next row
		x = 79;
	} // end loop through NxN grid
	
	if (seqModule != NULL)
	{
		seqModule->modeString = seqModule->modeStrings[seqModule->selectedOutputValueMode];
		seqModule->initialized = true;
	}
	return;
}
