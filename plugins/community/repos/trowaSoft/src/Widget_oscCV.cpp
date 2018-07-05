#include "global_pre.hpp"
#include "global_ui.hpp"
#include "Widget_oscCV.hpp"

#include "widgets.hpp"
using namespace rack;
#include "trowaSoft.hpp"
#include "dsp/digital.hpp"
#include "trowaSoftComponents.hpp"
#include "trowaSoftUtilities.hpp"
#include "Module_oscCV.hpp"

// Channel colors
const NVGcolor oscCVWidget::CHANNEL_COLORS[TROWA_OSCCV_NUM_COLORS] = {
	COLOR_TS_RED, COLOR_DARK_ORANGE, COLOR_YELLOW, COLOR_TS_GREEN,
	COLOR_CYAN, COLOR_TS_BLUE, COLOR_PURPLE, COLOR_PINK
};


//-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-
// oscCVWidget()
// Instantiate a oscCV widget. v0.60 must have module as param.
// @oscModule : (IN) Pointer to the osc module.
//-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-
oscCVWidget::oscCVWidget(oscCV* oscModule) : TSSModuleWidgetBase(oscModule)
{
	box.size = Vec(26 * RACK_GRID_WIDTH, RACK_GRID_HEIGHT);
	bool isPreview = oscModule == NULL; // If this is null, this for a preview??? Just get the controls layed out
	this->module = oscModule;
	this->numberChannels = (isPreview) ? TROWA_OSCCV_DEFAULT_NUM_CHANNELS : oscModule->numberChannels; 

	Vec topScreenSize = Vec(363, 48);

	//////////////////////////////////////////////
	// Background
	//////////////////////////////////////////////	
	{
		SVGPanel *panel = new SVGPanel();
		panel->box.size = box.size;
		panel->setBackground(SVG::load(assetPlugin(plugin, "res/cvOSCcv.svg")));
		addChild(panel);
	}

	////////////////////////////////////
	// Top Screen
	////////////////////////////////////
	{
		this->display = new TSOscCVTopDisplay(this);
		this->display->showDisplay = true;
		display->box.pos = Vec(TROWA_HORIZ_MARGIN, 24);
		display->box.size = topScreenSize;
		addChild(display);
	}

	////////////////////////////////////
	// OSC configuration screen.
	////////////////////////////////////
	if (!isPreview)
	{
		TSOSCConfigWidget* oscConfig = new TSOSCConfigWidget(oscModule, oscCV::ParamIds::OSC_SAVE_CONF_PARAM, oscCV::ParamIds::OSC_AUTO_RECONNECT_PARAM,
			oscModule->currentOSCSettings.oscTxIpAddress.c_str(), oscModule->currentOSCSettings.oscTxPort, oscModule->currentOSCSettings.oscRxPort,
			false, OSCClient::GenericClient, true, TROWA_OSCCV_DEFAULT_NAMESPACE);
		oscConfig->setVisible(false);
		oscConfig->box.pos = Vec(TROWA_HORIZ_MARGIN, 24);
		oscConfig->box.size = topScreenSize;

		this->oscConfigurationScreen = oscConfig;
		addChild(oscConfig);
	}
	
	//////////////////////////////////////
	// Labels
	//////////////////////////////////////
	{
		TSOscCVLabels* labelArea = new TSOscCVLabels();
		labelArea->box.pos = Vec(TROWA_HORIZ_MARGIN, topScreenSize.y + 24);
		labelArea->box.size = Vec(box.size.x - TROWA_HORIZ_MARGIN * 2, box.size.y - labelArea->box.pos.y - 15);
		addChild(labelArea);
	}

	int x, y, dx, dy;
	int xStart, yStart;


	//////////////////////////////////////
	// Parameters / UI Buttons
	//////////////////////////////////////
	Vec ledSize = Vec(15, 15);
	dx = 28;

	//---------------------------
	// Button: Enable OSC button
	//---------------------------
	LEDButton* btn;
	y = topScreenSize.y + 30;
	x = 76; // 80
	Vec btnSize = Vec(ledSize.x - 2, ledSize.y - 2);
	btn = dynamic_cast<LEDButton*>(ParamWidget::create<LEDButton>(Vec(x, y), oscModule, oscCV::ParamIds::OSC_SHOW_CONF_PARAM, 0, 1, 0));
	btn->box.size = btnSize;
	addParam(btn);
	addChild(TS_createColorValueLight<ColorValueLight>(Vec(x, y), oscModule, oscCV::LightIds::OSC_CONFIGURE_LIGHT, ledSize, COLOR_WHITE));
	addChild(TS_createColorValueLight<ColorValueLight>(Vec(x + 2, y + 2), oscModule, oscCV::LightIds::OSC_ENABLED_LIGHT, Vec(ledSize.x - 4, ledSize.y - 4), TSOSC_STATUS_COLOR));


	xStart = TROWA_HORIZ_MARGIN;
	yStart = 98; // 88
	dx = 28;
	dy = 30; // 32
	const float tbYOffset = 6.0;
	const float tbXOffset = 6.0;

	////////////////////////////////////
	// Middle Screen
	////////////////////////////////////
	x = xStart + dx * 2 + tbXOffset / 2.0;
	y = yStart;
	{
		int xEnd = box.size.x - xStart - dx * 2 - tbXOffset / 2.0;
		middleDisplay = new TSOscCVMiddleDisplay(this);
		middleDisplay->setDisplayMode(TSOscCVMiddleDisplay::DisplayMode::Default);
		middleDisplay->box.size = Vec(xEnd - x, numberChannels* dy);
		middleDisplay->box.pos = Vec(x, y);
		addChild(middleDisplay);
	}

	////////////////////////////////////
	// Channel Configuration
	////////////////////////////////////
	{
		this->oscChannelConfigScreen = new TSOscCVChannelConfigScreen(this, Vec(x, y), middleDisplay->box.size);
		//debug("Hiding screen");
		oscChannelConfigScreen->setVisibility(false);
		//oscChannelConfigScreen->box.size = middleDisplay->box.size;
		//oscChannelConfigScreen->box.pos = Vec(x, y);
		addChild(oscChannelConfigScreen);
	}

	////////////////////////////////////
	// Input Ports
	////////////////////////////////////
	// Trigger and Value CV inputs for each channel
	//debug("Starting ports");
	y = yStart;
	ledSize = Vec(5, 5);
	float ledYOffset = 12.5;
#if TROWA_OSSCV_SHOW_ADV_CH_CONFIG
	Vec tbPathSize = Vec(92, 20); // 88, 20 // 108, 20
#else
	Vec tbPathSize = Vec(108, 20); // 88, 20 // 108, 20
#endif
	btnSize = Vec(24, 20); 
	for (int r = 0; r < numberChannels; r++)
	{
		// Light (to indicate when we send OSC)
		addChild(TS_createColorValueLight<ColorValueLight>(Vec(xStart - ledSize.x, y + ledYOffset), oscModule, oscCV::LightIds::CH_LIGHT_START + r * TROWA_OSCCV_NUM_LIGHTS_PER_CHANNEL, ledSize, CHANNEL_COLORS[r]));

		// Trigger Input:
		x = xStart;
		if (colorizeChannels)
			addInput(TS_createInput<TS_Port>(Vec(x, y), oscModule, oscCV::InputIds::CH_INPUT_START + r * 2, !plugLightsEnabled, CHANNEL_COLORS[r]));
		else
			addInput(TS_createInput<TS_Port>(Vec(x, y), oscModule, oscCV::InputIds::CH_INPUT_START + r * 2, !plugLightsEnabled));

		// Value input:
		x += dx;
		if (colorizeChannels)
			addInput(TS_createInput<TS_Port>(Vec(x, y), oscModule, oscCV::InputIds::CH_INPUT_START + r * 2 + 1, !plugLightsEnabled, CHANNEL_COLORS[r]));
		else
			addInput(TS_createInput<TS_Port>(Vec(x, y), oscModule, oscCV::InputIds::CH_INPUT_START + r * 2 + 1, !plugLightsEnabled));


		//---* OSC Channel Configuration *---
		// OSC Input Path (this is OSC outgoing message, our input)
		x += dx + tbXOffset;
		std::string path = (isPreview) ? "/ch/" + std::to_string(r + 1) : oscModule->inputChannels[r].path;
		TSTextField* txtField = new TSTextField(TSTextField::TextType::Any, 50);
		txtField->box.size = tbPathSize;
		txtField->box.pos = Vec(x, y + tbYOffset);
		txtField->text = path;
		if (colorizeChannels) {
			txtField->borderColor = CHANNEL_COLORS[r];
			txtField->caretColor = CHANNEL_COLORS[r];
			txtField->caretColor.a = 0.70;
		}
		tbOscInputPaths.push_back(txtField);
		addChild(txtField);

#if TROWA_OSSCV_SHOW_ADV_CH_CONFIG
		// OSC Advanced Channel Config Button (INPUTS)
		x += txtField->box.size.x;
		//int paramId = oscCV::ParamIds::CH_PARAM_START + (r*TSOSCCVChannel::BaseParamIds::CH_NUM_PARAMS + TSOSCCVChannel::BaseParamIds::CH_SHOW_CONFIG) * 2;
		int paramId = oscCV::ParamIds::CH_PARAM_START 
			+ r*TSOSCCVChannel::BaseParamIds::CH_NUM_PARAMS
			+ TSOSCCVChannel::BaseParamIds::CH_SHOW_CONFIG;

		//debug("Input Ch %d, paramId = %d", r, paramId);

		TS_ScreenBtn* btn = new TS_ScreenBtn(btnSize, oscModule, paramId, std::string( "ADV" ), /*minVal*/ 0.0f, /*maxVal*/ 1.0f, /*defVal*/ 0.0f);
		btn->box.pos = Vec(x, y + tbYOffset);
		btn->borderColor = CHANNEL_COLORS[r];
		btn->color = CHANNEL_COLORS[r];
		btn->borderWidth = 1;
		btn->backgroundColor = nvgRGB(clamp(CHANNEL_COLORS[r].r - 0.3f, 0.0f, 1.0f), clamp(CHANNEL_COLORS[r].g - 0.3f, 0.0f, 1.0f), clamp(CHANNEL_COLORS[r].b - 0.3f, 0.0f, 1.0f));
		btn->setVisible(false);
		addParam(btn);	
		btnDrawInputAdvChConfig.push_back(btn);
#endif

		y += dy;
	} // end input channels


	////////////////////////////////////
	// Output Ports
	////////////////////////////////////
	// Trigger and Value CV inputs for each channel
	xStart = box.size.x - TROWA_HORIZ_MARGIN - dx * 2;
	y = yStart;
	for (int r = 0; r < numberChannels; r++)
	{
		//---* OSC Channel Configuration *---
		// OSC Output Path (this is OSC incoming message, our output)
		x = xStart - tbXOffset - tbPathSize.x;
		std::string path = (isPreview) ? "/ch/" + std::to_string(r + 1) : oscModule->outputChannels[r].path;
		TSTextField* txtField = new TSTextField(TSTextField::TextType::Any, 50);
		txtField->box.size = tbPathSize;
		txtField->box.pos = Vec(x, y + tbYOffset);
		txtField->text = path;
		if (colorizeChannels) {
			txtField->borderColor = CHANNEL_COLORS[r];
			txtField->caretColor = CHANNEL_COLORS[r];
			txtField->caretColor.a = 0.70;
		}
		tbOscOutputPaths.push_back(txtField);
		addChild(txtField);

#if TROWA_OSSCV_SHOW_ADV_CH_CONFIG
		// OSC Advanced Channel Config (OUTPUT)
		x -= btnSize.x;
		//int paramId = oscCV::ParamIds::CH_PARAM_START + (r*TSOSCCVChannel::BaseParamIds::CH_NUM_PARAMS + TSOSCCVChannel::BaseParamIds::CH_SHOW_CONFIG) * 2 + 1;
		//int paramId = oscCV::ParamIds::CH_PARAM_START 
		//	+ ( *TSOSCCVChannel::BaseParamIds::CH_NUM_PARAMS + TSOSCCVChannel::BaseParamIds::CH_SHOW_CONFIG;
		int paramId = oscCV::ParamIds::CH_PARAM_START
			+ (numberChannels + r) * TSOSCCVChannel::BaseParamIds::CH_NUM_PARAMS + TSOSCCVChannel::BaseParamIds::CH_SHOW_CONFIG;

		//debug("Output Ch %d, paramId = %d", r, paramId);

		TS_ScreenBtn* btn = new TS_ScreenBtn(btnSize, oscModule, paramId, std::string("ADV"), /*minVal*/ 0.0f, /*maxVal*/ 1.0f, /*defVal*/ 0.0f);
		btn->box.pos = Vec(x, y + tbYOffset);
		btn->borderColor = CHANNEL_COLORS[r];
		btn->color = CHANNEL_COLORS[r];
		btn->borderWidth = 1;
		btn->backgroundColor = nvgRGB(clamp(CHANNEL_COLORS[r].r - 0.3f, 0.0f, 1.0f), clamp(CHANNEL_COLORS[r].g - 0.3f, 0.0f, 1.0f), clamp(CHANNEL_COLORS[r].b - 0.3f, 0.0f, 1.0f));
		btn->setVisible(false);
		addParam(btn);
		btnDrawOutputAdvChConfig.push_back(btn);
#endif

		// Trigger Input:
		x = xStart;
		if (colorizeChannels)
			addOutput(TS_createOutput<TS_Port>(Vec(x, y), oscModule, oscCV::OutputIds::CH_OUTPUT_START + r * 2, !plugLightsEnabled, CHANNEL_COLORS[r]));
		else
			addOutput(TS_createOutput<TS_Port>(Vec(x, y), oscModule, oscCV::OutputIds::CH_OUTPUT_START + r * 2, !plugLightsEnabled));

		// Value input:
		x += dx;
		if (colorizeChannels)
			addOutput(TS_createOutput<TS_Port>(Vec(x, y), oscModule, oscCV::OutputIds::CH_OUTPUT_START + r * 2 + 1, !plugLightsEnabled, CHANNEL_COLORS[r]));
		else
			addOutput(TS_createOutput<TS_Port>(Vec(x, y), oscModule, oscCV::OutputIds::CH_OUTPUT_START + r * 2 + 1, !plugLightsEnabled));

		// Light (to indicate when we receive OSC)
		addChild(TS_createColorValueLight<ColorValueLight>(Vec(x + dx + ledSize.x/2.0, y + ledYOffset), oscModule, oscCV::LightIds::CH_LIGHT_START + r * TROWA_OSCCV_NUM_LIGHTS_PER_CHANNEL + 1, ledSize, CHANNEL_COLORS[r]));


		y += dy;
	} // end output channels

	// Set Tab Order:
	for (int c = 0; c < numberChannels; c++)
	{
		tbOscInputPaths[c]->nextField = tbOscOutputPaths[c];
		tbOscOutputPaths[c]->prevField = tbOscInputPaths[c];
		if (c > 0)
		{
			tbOscInputPaths[c]->prevField = tbOscOutputPaths[c - 1];
		}
		else
		{
			tbOscInputPaths[c]->prevField = tbOscOutputPaths[numberChannels - 1];
		}
		if (c < numberChannels - 1)
		{
			tbOscOutputPaths[c]->nextField = tbOscInputPaths[c + 1];
		}
		else
		{
			tbOscOutputPaths[c]->nextField = tbOscInputPaths[0]; // Loop back around
		}
	} // end loop through channels - put tab order on text fields


	// Screws:
	addChild(Widget::create<ScrewBlack>(Vec(0, 0)));
	addChild(Widget::create<ScrewBlack>(Vec(box.size.x - 15, 0)));
	addChild(Widget::create<ScrewBlack>(Vec(0, box.size.y - 15)));
	addChild(Widget::create<ScrewBlack>(Vec(box.size.x - 15, box.size.y - 15)));

	if (oscModule != NULL)
	{
		oscModule->isInitialized = true;
	}

	toggleChannelPathConfig(false);
	return;
} //end oscCVWidget()

//-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-
// step(void)
// Handle UI controls.
//-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-
void oscCVWidget::step()
{
	if (this->module == NULL)
		return;

	oscCV* thisModule = dynamic_cast<oscCV*>(module);

	if (thisModule->oscConfigTrigger.process(thisModule->params[oscCV::ParamIds::OSC_SHOW_CONF_PARAM].value))
	{
		//debug("Button clicked");
		thisModule->oscShowConfigurationScreen = !thisModule->oscShowConfigurationScreen;
		thisModule->lights[oscCV::LightIds::OSC_CONFIGURE_LIGHT].value = (thisModule->oscShowConfigurationScreen) ? 1.0 : 0.0;
		this->oscConfigurationScreen->setVisible(thisModule->oscShowConfigurationScreen);
		this->display->showDisplay = !thisModule->oscShowConfigurationScreen;
		if (thisModule->oscShowConfigurationScreen) {
			this->middleDisplay->setDisplayMode(TSOscCVMiddleDisplay::DisplayMode::None);
		}
		else {
			this->middleDisplay->setDisplayMode(TSOscCVMiddleDisplay::DisplayMode::Default);
		}
		if (thisModule->oscShowConfigurationScreen)
		{
			if (!thisModule->oscInitialized)
			{
				// Make sure the ports are available
				int p = TSOSCConnector::PortInUse(thisModule->currentOSCSettings.oscTxPort);
				if (p > 0 && p != thisModule->oscId)
					thisModule->currentOSCSettings.oscTxPort = TSOSCConnector::GetAvailablePort(thisModule->oscId, thisModule->currentOSCSettings.oscTxPort);
				p = TSOSCConnector::PortInUse(thisModule->currentOSCSettings.oscRxPort);
				if (p > 0 && p != thisModule->oscId)
					thisModule->currentOSCSettings.oscRxPort = TSOSCConnector::GetAvailablePort(thisModule->oscId, thisModule->currentOSCSettings.oscRxPort);

			}
			this->oscConfigurationScreen->setValues(thisModule->currentOSCSettings.oscTxIpAddress, thisModule->currentOSCSettings.oscTxPort, thisModule->currentOSCSettings.oscRxPort, thisModule->oscNamespace);
			this->oscConfigurationScreen->ckAutoReconnect->checked = thisModule->oscReconnectAtLoad;
			//this->oscConfigurationScreen->setSelectedClient(thisModule->oscCurrentClient); // OSC Client
			this->oscConfigurationScreen->btnActionEnable = !thisModule->oscInitialized;

			setChannelPathConfig(); // Set the channel text boxes
			toggleChannelPathConfig(true);

			this->oscConfigurationScreen->errorMsg = "";
			if (thisModule->oscError)
			{
				this->oscConfigurationScreen->errorMsg = "Error connecting to " + thisModule->currentOSCSettings.oscTxIpAddress;
			}
			this->oscConfigurationScreen->setVisible(true);
		}
		else
		{
			this->oscConfigurationScreen->setVisible(false);
			toggleChannelPathConfig(false);
			readChannelPathConfig(); // Read any changes that may have happened. (we don't have room for a save button)
			oscChannelConfigScreen->setVisibility(false); // Hide
		}
	}
	if (thisModule->oscShowConfigurationScreen)
	{
		//------------------------------------------
		// Check for show channel config buttons
		//------------------------------------------
		TSOSCCVChannel* editChannelPtr = NULL;
		bool isInput = false;
		for (int c = 0; c < thisModule->numberChannels; c++) {
			// Input Channel:
			int paramId = oscCV::ParamIds::CH_PARAM_START 
				+ c*TSOSCCVChannel::BaseParamIds::CH_NUM_PARAMS + TSOSCCVChannel::BaseParamIds::CH_SHOW_CONFIG;
			if (thisModule->inputChannels[c].showChannelConfigTrigger.process(thisModule->params[paramId].value)) {
				//debug("btnClick: Input Ch %d, paramId = %d", c, paramId);
				editChannelPtr = &(thisModule->inputChannels[c]);
				isInput = true;
				break;
			}
			paramId = oscCV::ParamIds::CH_PARAM_START
				+ (thisModule->numberChannels + c) * TSOSCCVChannel::BaseParamIds::CH_NUM_PARAMS + TSOSCCVChannel::BaseParamIds::CH_SHOW_CONFIG;
			// Output Channel:
			if (thisModule->outputChannels[c].showChannelConfigTrigger.process(thisModule->params[paramId].value)) {
				//debug("btnClick: Output Ch %d, paramId = %d", c, paramId);
				editChannelPtr = &(thisModule->outputChannels[c]);
				isInput = false;
				break;
			}
		} // end for (loop through channels)
		if (editChannelPtr != NULL) 
		{
			this->toggleChannelPathConfig(false); // Hide the channel paths
			this->oscChannelConfigScreen->showControl(editChannelPtr, isInput);
		}
		else if (this->oscChannelConfigScreen->visible)
		{
			// Check for enable/disable data massaging
			// Check for Save or Cancel
			if (oscChannelConfigScreen->saveTrigger.process(thisModule->params[oscCV::ParamIds::OSC_CH_SAVE_PARAM].value)) {
				//debug("Save Clicked");
				// Validate form & save
				if (oscChannelConfigScreen->saveValues()) {
					oscChannelConfigScreen->setVisibility(false); // Hide
					this->toggleChannelPathConfig(true); // Show paths again
					//thisModule->params[oscCV::ParamIds::OSC_CH_SAVE_PARAM].value = 0.0f; // Reset
				}
			}
			else if (oscChannelConfigScreen->cancelTrigger.process(thisModule->params[oscCV::ParamIds::OSC_CH_CANCEL_PARAM].value)) {
				// Just hide and go back to paths
				oscChannelConfigScreen->setVisibility(false); // Hide
				this->toggleChannelPathConfig(true); // Show paths again
				//thisModule->params[oscCV::ParamIds::OSC_CH_CANCEL_PARAM].value = 0.0f; // Reset
			}
		}

		//------------------------------------------
		// Check for enable/disable OSC
		//------------------------------------------
		if (thisModule->oscConnectTrigger.process(thisModule->params[oscCV::ParamIds::OSC_SAVE_CONF_PARAM].value))
		{
			if (oscConfigurationScreen->btnActionEnable)
			{
				// Enable OSC ------------------------------------------------------------------------
				// User checked to connect
				if (!this->oscConfigurationScreen->isValidIpAddress())
				{
#if TROWA_DEBUG_MSGS >= TROWA_DEBUG_LVL_MED
					debug("IP Address is not valid.");
#endif
					this->oscConfigurationScreen->errorMsg = "Invalid IP Address.";
					this->oscConfigurationScreen->tbIpAddress->requestFocus();
				}
				else if (!this->oscConfigurationScreen->isValidTxPort())
				{
#if TROWA_DEBUG_MSGS >= TROWA_DEBUG_LVL_MED
					debug("Tx Port is not valid.");
#endif
					this->oscConfigurationScreen->errorMsg = "Invalid Output Port (0-" + std::to_string(0xFFFF) + ").";
					this->oscConfigurationScreen->tbTxPort->requestFocus();

				}
				else if (!this->oscConfigurationScreen->isValidRxPort())
				{
#if TROWA_DEBUG_MSGS >= TROWA_DEBUG_LVL_MED
					debug("Rx Port is not valid.");
#endif
					this->oscConfigurationScreen->errorMsg = "Invalid Input Port (0-" + std::to_string(0xFFFF) + ").";
					this->oscConfigurationScreen->tbRxPort->requestFocus();
				}
				else
				{
					// Try to connect
#if TROWA_DEBUG_MSGS >= TROWA_DEBUG_LVL_MED
					debug("Save OSC Configuration clicked, save information for module.");
#endif
					readChannelPathConfig();
					this->oscConfigurationScreen->errorMsg = "";
					thisModule->oscNewSettings.oscTxIpAddress = this->oscConfigurationScreen->tbIpAddress->text.c_str();
					thisModule->oscNewSettings.oscTxPort = this->oscConfigurationScreen->getTxPort();
					thisModule->oscNewSettings.oscRxPort = this->oscConfigurationScreen->getRxPort();
					//thisModule->oscCurrentClient = this->oscConfigurationScreen->getSelectedClient();
					//debug("Setting namespace");
					thisModule->setOscNamespace(this->oscConfigurationScreen->tbNamespace->text);
					thisModule->oscCurrentAction = oscCV::OSCAction::Enable;
					thisModule->oscReconnectAtLoad = this->oscConfigurationScreen->ckAutoReconnect->checked;
				}
			} // end if enable osc
			else
			{
				// Disable OSC ------------------------------------------------------------------
#if TROWA_DEBUG_MSGS >= TROWA_DEBUG_LVL_MED
				debug("Disable OSC clicked.");
#endif
				this->oscConfigurationScreen->errorMsg = "";
				thisModule->oscCurrentAction = oscCV::OSCAction::Disable;
			} // end else disable OSC
		} // end if OSC Save btn pressed
		else
		{
			if (thisModule->oscError)
			{
				if (this->oscConfigurationScreen->errorMsg.empty())
					this->oscConfigurationScreen->errorMsg = "Error connecting to " + thisModule->currentOSCSettings.oscTxIpAddress + ".";
			}
		}
		// Current status of OSC		
		if (thisModule->oscInitialized)
		{
			this->oscConfigurationScreen->statusMsg2 = thisModule->currentOSCSettings.oscTxIpAddress;
			this->oscConfigurationScreen->btnActionEnable = false;
		}
		else
		{
			this->oscConfigurationScreen->successMsg = "";
			this->oscConfigurationScreen->statusMsg2 = "OSC Not Connected";
			this->oscConfigurationScreen->btnActionEnable = true;
		}
	} // end if show OSC config screen

	ModuleWidget::step();
	return;
}


// Show or hide the channel configuration
void oscCVWidget::toggleChannelPathConfig(bool show)
{
	for (int i = 0; i < this->numberChannels; i++)
	{
		this->tbOscInputPaths[i]->visible = show;
		this->tbOscOutputPaths[i]->visible = show;
#if TROWA_OSSCV_SHOW_ADV_CH_CONFIG
		btnDrawInputAdvChConfig[i]->setVisible(show);
		btnDrawOutputAdvChConfig[i]->setVisible(show);
#endif
	}
	return;
}
// Read the channel path configs and store in module's channels.
void oscCVWidget::readChannelPathConfig()
{
	if (module != NULL)
	{
		oscCV* thisModule = dynamic_cast<oscCV*>(module);
		try
		{
			for (int i = 0; i < this->numberChannels; i++)
			{
				thisModule->inputChannels[i].setPath(this->tbOscInputPaths[i]->text);
				thisModule->outputChannels[i].setPath(this->tbOscOutputPaths[i]->text);
			}
		}
		catch (const std::exception& e)
		{
			warn("Error %s.", e.what());
		}
	}
	return;
} // end readChannelPathConfig()
// Set the channel path text boxes.
void oscCVWidget::setChannelPathConfig() {
	if (module != NULL)
	{
		oscCV* thisModule = dynamic_cast<oscCV*>(module);
		try
		{
			for (int i = 0; i < this->numberChannels; i++)
			{
				this->tbOscInputPaths[i]->text = thisModule->inputChannels[i].getPath();
				this->tbOscOutputPaths[i]->text = thisModule->outputChannels[i].getPath();
			}
		}
		catch (const std::exception& e)
		{
			warn("Error %s.", e.what());
		}
	}
	return;
} // end setChannelPathConfig()



//-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-
// draw()
// @vg : (IN) NVGcontext to draw on
// Draw labels on our widget.
//-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-
void TSOscCVLabels::draw(/*in*/ NVGcontext *vg) {
	// Default Font:
	nvgFontSize(vg, fontSize);
	nvgFontFaceId(vg, font->handle);
	nvgTextLetterSpacing(vg, 1);

	NVGcolor textColor = nvgRGB(0xee, 0xee, 0xee);
	nvgFillColor(vg, textColor);
	nvgFontSize(vg, fontSize);

	int x, y, dx;// , dy;
	int xStart, yStart;
	xStart = 0;
	yStart = 25; // 17
	dx = 28;
	//dy = 32;

	//-- * Top Buttons *--//
	x = 84;
	y = 18;
	nvgTextAlign(vg, NVG_ALIGN_LEFT);
	nvgText(vg, x, y, "CONFIG", NULL);


	//--- * Inputs *---//
	// (Left hand side)
	// TRIG:
	x = xStart + dx / 2;
	y = yStart;
	nvgTextAlign(vg, NVG_ALIGN_CENTER);
	nvgText(vg, x, y, "TRG", NULL);
	// VAL:
	x += dx;
	y = yStart;
	nvgTextAlign(vg, NVG_ALIGN_CENTER);
	nvgText(vg, x, y, "VAL", NULL);
	// Bottom (INPUTS)
	x = xStart + dx;
	y = box.size.y - 13;
	nvgText(vg, x, y, "INPUTS", NULL);

	//--- * Outputs *---//
	// (Right hand side)
	// TRIG:
	x = box.size.x - dx / 2 - dx;
	y = yStart;
	nvgTextAlign(vg, NVG_ALIGN_CENTER);
	nvgText(vg, x, y, "TRG", NULL);
	// VAL:
	x += dx;
	y = yStart;
	nvgTextAlign(vg, NVG_ALIGN_CENTER);
	nvgText(vg, x, y, "VAL", NULL);
	// Bottom (INPUTS)
	x = box.size.x - dx;
	y = box.size.y - 13;
	nvgText(vg, x, y, "OUTPUTS", NULL);
} // end TSOscCVLabels::draw()


//-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-
// step()
// Calculate scrolling and stuff?
//-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-
void TSOscCVTopDisplay::step() {
	//debug("Top Display: step(%8.6f)", dt);

	bool connected = false;
	bool isPreview = parentWidget->module == NULL;
	std::string thisIp = std::string("NO CONNECTION ");
	oscCV* thisModule = NULL;
	if (!isPreview)
	{
		thisModule = dynamic_cast<oscCV*>(parentWidget->module);
		connected = thisModule->oscInitialized;
		if (connected)
			thisIp = thisModule->currentOSCSettings.oscTxIpAddress
			+ std::string(" Tx:") + std::to_string(thisModule->currentOSCSettings.oscTxPort)
			+ std::string(" Rx:") + std::to_string(thisModule->currentOSCSettings.oscRxPort)
			+ ((thisModule->oscNamespace.at(0) == '/') ? " " : " /") + thisModule->oscNamespace + " ";
	}



	if (thisIp.compare(lastIp) != 0)
	{
		sprintf(scrollingMsg, "trowaSoft - %s - cv<->OSC<->cv - ", thisIp.c_str());
	}

	//dt += engineGetSampleTime() / scrollTime_sec;
	//if (dt > 1.0f)
	dt += 100.0 / engineGetSampleRate();
	if (dt > scrollTime_sec) 
	{
		//debug("Dt has grown. Increment Ix: %d", scrollIx);
		dt = 0.0f;
		if (static_cast<size_t>(scrollIx) == strlen(scrollingMsg) - 1)
			scrollIx = 0;
		else
			scrollIx++;
	}

	lastIp = thisIp;
	TransparentWidget::step(); // parent whatever he does
	return;
}

//-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-
// draw()
// @vg : (IN) NVGcontext to draw on
// Top display
//-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-
void TSOscCVTopDisplay::draw(/*in*/ NVGcontext *vg)
{
	//bool isPreview = parentWidget->module == NULL; // May get a NULL module for preview

	// Background Colors:
	NVGcolor backgroundColor = nvgRGB(0x20, 0x20, 0x20);
	NVGcolor borderColor = nvgRGB(0x10, 0x10, 0x10);

	// Screen:
	nvgBeginPath(vg);
	nvgRoundedRect(vg, 0.0, 0.0, box.size.x, box.size.y, 5.0);
	nvgFillColor(vg, backgroundColor);
	nvgFill(vg);
	nvgStrokeWidth(vg, 1.0);
	nvgStrokeColor(vg, borderColor);
	nvgStroke(vg);

	if (!showDisplay)
		return;

	Rect b = Rect(Vec(0, 0), Vec(box.size.x - 13, box.size.y));
	nvgScissor(vg, b.pos.x, b.pos.y, b.size.x, b.size.y);

	//oscCV* thisModule = NULL;
	//if (!isPreview)
	//	thisModule = dynamic_cast<oscCV*>(parentWidget->module);

	int x, y;

	// Default Font:
	nvgFontSize(vg, fontSize);
	nvgFontFaceId(vg, font->handle);
	nvgTextLetterSpacing(vg, 2.5);
	NVGcolor textColor = nvgRGB(0xee, 0xee, 0xee);
	nvgFillColor(vg, textColor);

	// SCROLLING MESSAGE ====================================
	x = 13;// -scrollIx * 0.5;
	y = 13;
	nvgFontSize(vg, fontSize * 1.5);	// Large font
	//nvgFontFaceId(vg, font->handle);
	nvgTextAlign(vg, NVG_ALIGN_LEFT | NVG_ALIGN_TOP);

	// Start (left on screen) of scrolling message:
	const char * subStr = scrollingMsg + scrollIx;
	nvgText(vg, x, y, subStr, NULL);
	// Get circular wrap (right part of screen) - start of message again:
	float txtBounds[4] = { 0,0,0,0 };
	float nextX = nvgTextBounds(vg, x, y, subStr, NULL, txtBounds);
	x = nextX; // +24
	if (x < b.size.x) {
		// Wrap the start of the string around
		nvgText(vg, x, y, scrollingMsg, subStr);
	}
	//// Measures the specified text string. Parameter bounds should be a pointer to float[4],
	//// if the bounding box of the text should be returned. The bounds value are [xmin,ymin, xmax,ymax]
	//// Returns the horizontal advance of the measured text (i.e. where the next character should drawn).
	//// Measured values are returned in local coordinate space.
	//float nvgTextBounds(NVGcontext* ctx, float x, float y, const char* string, const char* end, float* bounds);
	nvgResetScissor(vg);

	//const char* asciiArt[] = {
	//	"♫♪.ılılıll|̲̅̅●̲̅̅|̲̅̅=̲̅̅|̲̅̅●̲̅̅|llılılı.♫♪",
	//	"°º¤ø,¸¸,ø¤º°`°º¤ø,¸,ø¤°º¤ø,¸¸,ø¤º°`°º¤ø,¸"
	//}


	// Always display connected 

	return;
} // end TSOscCVTopDisplay::draw()


//-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-
// Process
// Middle display -- If we have to do scrolling, calculate it.
//-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-
void TSOscCVMiddleDisplay::step() {
	if (displayMode == DisplayMode::Default) {
		dt += 100.0 / engineGetSampleRate();
		if (dt > scrollTime)
		{
			dt = 0.0f;
			chPathPosition += 0.05f;
			if (chPathPosition > 1.0f)
				chPathPosition = 0.0f;
		}

	}
	TransparentWidget::step(); // parent whatever he does
	return;
} // end step()
//-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-
// draw()
// @vg : (IN) NVGcontext to draw on
// Middle display drawing.
//-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-
void TSOscCVMiddleDisplay::draw(/*in*/ NVGcontext *vg) {
	bool isPreview = parentWidget->module == NULL; // May get a NULL module for preview

	// Background Colors:
	NVGcolor backgroundColor = nvgRGB(0x20, 0x20, 0x20);
	NVGcolor borderColor = nvgRGB(0x10, 0x10, 0x10);

	// Screen:
	nvgBeginPath(vg);
	nvgRoundedRect(vg, 0.0, 0.0, box.size.x, box.size.y, 5.0);
	nvgFillColor(vg, backgroundColor);
	nvgFill(vg);
	nvgStrokeWidth(vg, 1.0);
	nvgStrokeColor(vg, borderColor);
	nvgStroke(vg);

	if (!displayMode)
		return;

	if (!isPreview)
	{
		oscCV* thisModule = dynamic_cast<oscCV*>(parentWidget->module);
		// Default Font:
		nvgFontSize(vg, 9);
		nvgFontFaceId(vg, font->handle);
		nvgTextLetterSpacing(vg, 1);
		NVGcolor textColor = nvgRGB(0xee, 0xee, 0xee);
		nvgFillColor(vg, textColor);

		int dy = 2;
		int dx = 4;
		int height = 28;
		int width = box.size.x / 2.0 - 4;
		int y = 2;
		float txtBounds[4] = { 0,0,0,0 };
		const int txtPadding = 4;
		int txtWidth = width - txtPadding;
		bool drawBoxes = false;
		for (int c = 0; c < thisModule->numberChannels; c++) {
			int ix = 0;
			float nextX;

			//---- INPUT ----
			int x = 2;
			drawChannelChart(vg, &(thisModule->inputChannels[c]), x, y, width, height, oscCVWidget::CHANNEL_COLORS[c]);
			if (drawBoxes) {
				// Debug draw box:
				nvgBeginPath(vg);
				nvgRect(vg, x, y, width, height);
				nvgStrokeColor(vg, oscCVWidget::CHANNEL_COLORS[c]);
				nvgStrokeWidth(vg, 1.0);
				nvgStroke(vg);
			}

			// Label:
			nextX = nvgTextBounds(vg, x, y, thisModule->inputChannels[c].path.c_str(), NULL, txtBounds);
			ix = 0;
			if (nextX > txtWidth) {
				ix = chPathPosition * thisModule->inputChannels[c].path.length();
			}
			nvgScissor(vg, x, y, txtWidth, height);
			nvgTextAlign(vg, NVG_ALIGN_LEFT | NVG_ALIGN_TOP);
			nvgText(vg, x, y + 1, &(thisModule->inputChannels[c].path.c_str()[ix]), NULL);
			nvgResetScissor(vg);

			//---- OUTPUT ----
			x += dx + width;
			drawChannelChart(vg, &(thisModule->outputChannels[c]), x, y, width, height, oscCVWidget::CHANNEL_COLORS[c]);
			if (drawBoxes) {
				// Debug draw box:
				nvgBeginPath(vg);
				nvgRect(vg, x, y, width, height);
				nvgStrokeColor(vg, oscCVWidget::CHANNEL_COLORS[thisModule->numberChannels - c - 1]);
				nvgStrokeWidth(vg, 1.0);
				nvgStroke(vg);
			}

			// Label:
			nextX = nvgTextBounds(vg, x, y, thisModule->outputChannels[c].path.c_str(), NULL, txtBounds);
			ix = thisModule->outputChannels[c].path.length();
			if (nextX > txtWidth) {
				ix = thisModule->outputChannels[c].path.length() - chPathPosition * thisModule->outputChannels[c].path.length();
			}
			nvgScissor(vg, x + txtPadding, y, txtWidth, height);
			nvgTextAlign(vg, NVG_ALIGN_RIGHT | NVG_ALIGN_TOP);
			nvgText(vg, x + width, y + 1, thisModule->outputChannels[c].path.c_str(), &(thisModule->outputChannels[c].path.c_str()[ix]));
			nvgResetScissor(vg);

			y += dy + height;
		}
	} // end if we actually have a module
	return;
} // end draw()
//-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-
// drawChannelChart()
// Draw the channel data.
//-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-
void TSOscCVMiddleDisplay::drawChannelChart(/*in*/ NVGcontext *vg, /*in*/ TSOSCCVChannel* channelData,  
	/*in*/ int x, /*in*/ int y, /*in*/ int width, /*in*/ int height,
	/*in*/ NVGcolor lineColor)
{
	nvgScissor(vg, x, y, width, height);
	nvgBeginPath(vg);
	float dx = static_cast<float>(width) / TROWA_OSCCV_VAL_BUFFER_SIZE;
	//if (dx < 0.5)
	//	dx = 0.5;
	float px = x;
	nvgMoveTo(vg, x, y);
	float startY = y + height;
	for (int i = 0; i < TROWA_OSCCV_VAL_BUFFER_SIZE; i++)
	{
		float py = startY - rescale(channelData->valBuffer[i], channelData->minVoltage, channelData->maxVoltage, 0, height);
		nvgLineTo(vg, px, py);
		px += dx;
	}
	nvgStrokeColor(vg, lineColor);
	nvgStrokeWidth(vg, 1.0);
	nvgStroke(vg);

	nvgResetScissor(vg);
	return;
}
//-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-
// drawChannelChart()
// Draw the channel data.
//-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-
void TSOscCVMiddleDisplay::drawChannelBar(/*in*/ NVGcontext *vg, /*in*/ TSOSCCVChannel* channelData,
	/*in*/ int x, /*in*/ int y, /*in*/ int width, /*in*/ int height,
	/*in*/ NVGcolor lineColor)
{
	nvgScissor(vg, x, y, width, height);
	nvgBeginPath(vg);
	float dx = static_cast<float>(width) / TROWA_OSCCV_VAL_BUFFER_SIZE;
	float px = x;
	nvgMoveTo(vg, x, y);
	for (int i = 0; i < TROWA_OSCCV_VAL_BUFFER_SIZE; i++)
	{
		float py = y + rescale(channelData->valBuffer[i], channelData->minVoltage, channelData->maxVoltage, 0, height);
		nvgLineTo(vg, px, py);
		px += dx;
	}
	nvgStrokeColor(vg, lineColor);
	nvgStrokeWidth(vg, 1.0);
	nvgStroke(vg);

	nvgResetScissor(vg);
	return;
}
// On click
void TSOscCVDataTypeSelectBtn::onAction(EventAction &e) {
	if (visible)
	{
		Menu *menu = rack::global_ui->ui.gScene->createMenu();
		menu->box.pos = getAbsoluteOffset(Vec(0, box.size.y)).round();
		menu->box.size.x = box.size.x;
		for (int i = 0; i < numVals; i++) {
			TSOscCVDataTypeItem *option = new TSOscCVDataTypeItem(this, i);
			option->itemVal = itemVals[i];
			option->text = itemStrs[i];
			menu->addChild(option);
		}
	}
	return;
}
// Draw if visible
void TSOscCVDataTypeSelectBtn::draw(NVGcontext *vg) {
	if (visible) {
		nvgScissor(vg, 0, 0, box.size.x, box.size.y);

		// Background
		nvgBeginPath(vg);
		nvgRoundedRect(vg, 0, 0, box.size.x, box.size.y, 5.0);
		nvgFillColor(vg, backgroundColor);
		nvgFill(vg);

		// Border
		if (borderWidth > 0) {
			nvgStrokeWidth(vg, borderWidth);
			nvgStrokeColor(vg, borderColor);
			nvgStroke(vg);
		}
		nvgResetScissor(vg);

		if (font->handle >= 0) {
			nvgScissor(vg, 0, 0, box.size.x - 5, box.size.y);

			nvgFillColor(vg, color);
			nvgFontFaceId(vg, font->handle);
			//nvgTextLetterSpacing(vg, 0.0);

			nvgFontSize(vg, fontSize);
			nvgText(vg, textOffset.x, textOffset.y, text.c_str(), NULL);

			nvgResetScissor(vg);

			bndUpDownArrow(vg, box.size.x - 10, 10, 5, color);
		}
	}
	return;
} // end draw()
// When the selected item chagnes.
void TSOscCVDataTypeSelectBtn::onSelectedIndexChanged() {
	if (parentScreen != NULL)
	{
		parentScreen->setDataType(static_cast<TSOSCCVChannel::ArgDataType>(this->selectedVal));
	}
	return;
}


//-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-
// TSOscCVChannelConfigScreen()
//-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-
TSOscCVChannelConfigScreen::TSOscCVChannelConfigScreen(oscCVWidget* widget, Vec pos, Vec boxSize)
{
	visible = false;
	box.size = boxSize;
	parentWidget = widget;
	Module* thisModule = (widget != NULL) ? widget->module : NULL;
	font = Font::load(assetPlugin(plugin, TROWA_DIGITAL_FONT));
	labelFont = Font::load(assetPlugin(plugin, TROWA_LABEL_FONT));
	fontSize = 10;
	box.pos = pos;
	//visible = true;

	//debug("Init Channel Config screen");
	int x, y;
	Vec tbSize = Vec(180, 20);
	int dx = 20;
	int dy = 15;

	// -- Toggle Translate Buton and Light
	x = startX;
	y = startY;
	Vec ledSize = Vec(15, 15);

	//NVGcolor backgroundColor = nvgRGBA(0, 0, 0, 0);
	//NVGcolor color = COLOR_TS_GRAY;
	//NVGcolor borderColor = COLOR_TS_GRAY;
	Vec btnSize = Vec(100, 20);
	x = box.size.x - startX - btnSize.x;
	btnToggleTranslateVals = new TS_ScreenCheckBox(/*size*/ btnSize, /*module*/ thisModule, /*paramId*/ oscCV::ParamIds::OSC_CH_TRANSLATE_VALS_PARAM, /*text*/ "Convert Values", /*minVal*/ 0.0f, /*maxVal*/ 1.0f, /*defVal*/ 0.0f);
	btnToggleTranslateVals->fontSize = 9;
	btnToggleTranslateVals->color = COLOR_TS_GRAY;
	btnToggleTranslateVals->borderWidth = 0;
	btnToggleTranslateVals->padding = 2;
	btnToggleTranslateVals->textAlign = TS_ScreenBtn::TextAlignment::Right;
	btnToggleTranslateVals->box.pos = Vec(x, y);
	//btnToggleTranslateVals->minValue = 0.0f;
	//btnToggleTranslateVals->maxValue = 1.0f;
	addChild(btnToggleTranslateVals);


	//if (widget != NULL)
	//{
	//	// Offset the position.
	//	btnToggleTranslateVals->box.pos.x += box.pos.x;
	//	btnToggleTranslateVals->box.pos.y += box.pos.y;
	//	widget->addParam(btnToggleTranslateVals); //widget->addParam(btnToggleTranslateVals);
	//}
	//else
	//	addChild(btnToggleTranslateVals);

	//debug("Light at (%d, %d). %.2fx%.2f.", x, y, ledSize.x, ledSize.y);
	//lightTranslateVals = dynamic_cast<ColorValueLight*>(TS_createColorValueLight<ColorValueLight>(Vec(x+3, y), thisModule, oscCV::LightIds::OSC_CH_TRANSLATE_LIGHT, ledSize, COLOR_WHITE));
	//if (widget != NULL)
	//{
	//	lightTranslateVals->box.pos.x += this->box.pos.x;
	//	lightTranslateVals->box.pos.y += this->box.pos.y;
	//	widget->addChild(lightTranslateVals);
	//}
	//else
	//	addChild(lightTranslateVals);


	//debug("Starting text boxes");
	// -- Min/Max Text Boxes
	btnSize = Vec(70, 20);
	tbSize = Vec(70, 20);
	x = startX;
	y = startY + ledSize.y + dy + fontSize * 2 + 5; // 13 + 15 + 20 + fontSize
	for (int i = 0; i < TextBoxIx::NumTextBoxes; i++) {
		tbNumericBounds[i] = new TSTextField(TSTextField::TextType::RealNumberOnly, 25);
		tbNumericBounds[i]->setRealPrecision(3); // mV? why not
		tbNumericBounds[i]->box.size = tbSize;
		tbNumericBounds[i]->box.pos = Vec(x, y);
		tbNumericBounds[i]->id = i;
		//debug("TextBox %d at (%d, %d)", i, x, y);

		// Next Field
		if (i > 0)
		{
			tbNumericBounds[i]->prevField = tbNumericBounds[i-1];
			tbNumericBounds[i-1]->nextField = tbNumericBounds[i];
		}
		addChild(tbNumericBounds[i]);

		// Position:
		if (i % 2)
		{
			if (i < TextBoxIx::NumTextBoxes - 1) {
				x = startX; // New row
				y += dy + tbSize.y + fontSize * 2 + 5;
			}
		}
		else
		{
			x += tbSize.x + dx; // Next column
		}
	} // end for
	//TextBoxIx::NumTextBoxes
	tbNumericBounds[TextBoxIx::NumTextBoxes - 1]->nextField = tbNumericBounds[0]; // Loop back around


	//debug("Starting btn select");
	// Data Type
	x = startX;
	y += tbSize.y + dy + dy;
	//debug("Select at (%d, %d). %.2fx%.2f.", x, y, btnSize.x, btnSize.y);
	btnSelectDataType = new TSOscCVDataTypeSelectBtn(numDataTypes, reinterpret_cast<int*>(oscDataTypeVals), oscDataTypeStr, static_cast<int>(selectedDataType));
	btnSelectDataType->box.size = btnSize;
	btnSelectDataType->box.pos = Vec(x, y);
	btnSelectDataType->parentScreen = this;
	addChild(btnSelectDataType);

	//debug("Save Btn");
	// Save Button
	y += btnSelectDataType->box.size.y + dy;
	//debug("Save Button at (%d, %d). %.2fx%.2f.", x, y, btnSize.x, btnSize.y);
	//Vec size, Module* module, int paramId, std::string text, float minVal, float maxVal, float defVal
	btnSave = new TS_ScreenBtn(/*size*/ btnSize, /*module*/ thisModule, /*paramId*/ oscCV::ParamIds::OSC_CH_SAVE_PARAM, /*text*/ "Save", /*minVal*/ 0.0f, /*maxVal*/ 1.0f, /*defVal*/ 0.0f);
	btnSave->box.pos = Vec(x, y);
	if (widget != NULL)
		addChild(btnSave);// widget->addParam(btnSave);
	else
		addChild(btnSave);

	//debug("Cancel Btn");
	// Cancel button
	x += btnSize.x + dx;
	//debug("Cancel Button at (%d, %d). %.2fx%.2f.", x, y, btnSize.x, btnSize.y);
	btnCancel = new TS_ScreenBtn(/*size*/ btnSize, /*module*/ thisModule, /*paramId*/ oscCV::ParamIds::OSC_CH_CANCEL_PARAM, /*text*/ "Cancel", /*minVal*/ 0.0f, /*maxVal*/ 1.0f, /*defVal*/ 0.0f);
	btnCancel->box.pos = Vec(x, y);
	if (widget != NULL)
		addChild(btnCancel);// widget->addParam(btnCancel);
	else
		addChild(btnCancel);
	return;
} // end TSOscCVChannelConfigScreen()

  //-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-
  // draw()
  // @vg : (IN) NVGcontext to draw on
  //-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-
void TSOscCVChannelConfigScreen::draw(/*in*/ NVGcontext *vg)
{
	if (this->visible) {
		// Default Font:
		nvgFontSize(vg, fontSize);
		nvgFontFaceId(vg, font->handle);
		nvgTextLetterSpacing(vg, 1);
		NVGcolor textColor = nvgRGB(0xee, 0xee, 0xee);
		NVGcolor errorColor = COLOR_TS_RED;

		// Draw labels
		int x, y;
		Vec tbSize = Vec(70, 20);
		//Vec ledSize = Vec(15, 15);
		int dx = 20;
		int dy = 15;
		const int errorDy = 32;

		NVGcolor channelColor = this->parentWidget->CHANNEL_COLORS[this->currentChannelPtr->channelNum - 1];

		// -- Heading --
		// Channel Number and address
		x = startX;// +ledSize.x + 10;
		y = startY;
		sprintf(buffer, "CH %d %sPUT", this->currentChannelPtr->channelNum, (isInput) ? "IN" : "OUT");
		float txtBounds[4] = { 0,0,0,0 };
		const float padding = 2.0f;
		nvgFontSize(vg, fontSize*1.1);
		nvgFontFaceId(vg, font->handle);
		nvgTextBounds(vg, x, y, buffer, NULL, txtBounds); //float txtWidth = 
		nvgBeginPath(vg);
		nvgRect(vg, txtBounds[0], y, txtBounds[2] - txtBounds[0] + padding*2, txtBounds[3] - txtBounds[1] + padding*2);
		nvgFillColor(vg, channelColor);
		nvgFill(vg);
		nvgTextAlign(vg, NVG_ALIGN_LEFT | NVG_ALIGN_TOP);
		nvgFillColor(vg, COLOR_BLACK);
		nvgText(vg, x + padding, y + padding, buffer, NULL);


		nvgFillColor(vg, textColor);
		// -- Min/Max Text Boxes
		x = startX;
		y = startY + 15 + dy;
		//	y = startY + ledSize.y + dy + fontSize*2; // 13 + 15 + 20 + fontSize
		nvgFontSize(vg, fontSize*0.9);
		sprintf(buffer, "Control Voltage (%s)", (isInput) ? "IN" : "OUT");
		nvgText(vg, x, y, buffer, NULL);
		y += fontSize + 1;
		const char* labels[] = { "Min", "Max", "Min", "Max" };
		for (int i = 0; i < TextBoxIx::NumTextBoxes; i++) {
			if (i == 2) {
				nvgFontSize(vg, fontSize*0.9);
				nvgFontFaceId(vg, font->handle);

				sprintf(buffer, "OSC Value (%s)", (isInput) ? "OUT" : "IN");
				nvgText(vg, x, y, buffer, NULL);
				y += fontSize + 1;
			}

			nvgFontSize(vg, fontSize);
			nvgFontFaceId(vg, labelFont->handle);
			nvgText(vg, x, y, labels[i], NULL);

			// Errors if any
			if (tbErrors[i].length() > 0) {
				nvgFontFaceId(vg, labelFont->handle);
				nvgFillColor(vg, errorColor);
				nvgText(vg, x, y + errorDy, tbErrors[i].c_str(), NULL);
				nvgFillColor(vg, textColor);
			}

			// Position:
			if (i % 2)
			{
				x = startX; // New row
				y += dy + tbSize.y + fontSize + 5;
			}
			else
			{
				x += tbSize.x + dx; // Next column
			}
		} // end for

		// Data Type
		nvgFontSize(vg, fontSize);
		nvgFontFaceId(vg, labelFont->handle);
		nvgText(vg, x, y, "Data Type", NULL);

		OpaqueWidget::draw(vg); // Parent
	}
	return;
} // end draw()

//-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-
// showControl()
// @channel: (IN) The channel which we are configuring.
// @isInput: (IN) If this an input or output.
//-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-
void TSOscCVChannelConfigScreen::showControl(TSOSCCVChannel* channel, bool isInput)
{
	this->currentChannelPtr = channel;
	this->isInput = isInput;

	// Translation On/Off
	translateValsEnabled = currentChannelPtr->convertVals;
	if (currentChannelPtr->convertVals)
	{		
		this->btnToggleTranslateVals->value = 1.0f;
		//parentWidget->module->lights[oscCV::LightIds::OSC_CH_TRANSLATE_LIGHT].value = 1.0f;
	}
	else
	{
		this->btnToggleTranslateVals->value = 0.0f;
		//parentWidget->module->lights[oscCV::LightIds::OSC_CH_TRANSLATE_LIGHT].value = 0.0f;
	}
	btnToggleTranslateVals->checked = translateValsEnabled;

	for (int i = 0; i < TextBoxIx::NumTextBoxes; i++)
	{
		tbErrors[i] = std::string("");
	}

	// Text Box Values
	char buffer[50] = { '\0' };
	sprintf(buffer, "%.3f", currentChannelPtr->minVoltage);
	tbNumericBounds[TextBoxIx::MinCVVolt]->text = std::string(buffer);
	sprintf(buffer, "%.3f", currentChannelPtr->maxVoltage);
	tbNumericBounds[TextBoxIx::MaxCVVolt]->text = std::string(buffer);

	tbNumericBounds[TextBoxIx::MinOSCVal]->enabled = true;
	tbNumericBounds[TextBoxIx::MaxOSCVal]->enabled = true;
	switch (currentChannelPtr->dataType)
	{
	case TSOSCCVChannel::ArgDataType::OscInt:
	{
		const char * format = "%.0f";
		sprintf(buffer, format, currentChannelPtr->minOscVal);
		tbNumericBounds[TextBoxIx::MinOSCVal]->text = std::string(buffer);
		sprintf(buffer, format, currentChannelPtr->maxOscVal);
		tbNumericBounds[TextBoxIx::MaxOSCVal]->text = std::string(buffer);
		break;
	}
	case TSOSCCVChannel::ArgDataType::OscBool:
	{
		tbNumericBounds[TextBoxIx::MinOSCVal]->enabled = false;
		tbNumericBounds[TextBoxIx::MaxOSCVal]->enabled = false;

		tbNumericBounds[TextBoxIx::MinOSCVal]->text = std::string("0");
		tbNumericBounds[TextBoxIx::MaxOSCVal]->text = std::string("1");
		//const char * format = "%.0f";
		//sprintf(buffer, format, currentChannelPtr->minOscVal);
		//tbNumericBounds[TextBoxIx::MinOSCVal]->text = std::string(buffer);
		//sprintf(buffer, format, currentChannelPtr->maxOscVal);
		//tbNumericBounds[TextBoxIx::MaxOSCVal]->text = std::string(buffer);
		break;
	}
	case TSOSCCVChannel::ArgDataType::OscFloat:
	default:
	{
		const char * format = "%.3f";
		sprintf(buffer, format, currentChannelPtr->minOscVal);
		tbNumericBounds[TextBoxIx::MinOSCVal]->text = std::string(buffer);
		sprintf(buffer, format, currentChannelPtr->maxOscVal);
		tbNumericBounds[TextBoxIx::MaxOSCVal]->text = std::string(buffer);
		break;
	}
	} // end switch (data type)

	// Data Type
	selectedDataType = currentChannelPtr->dataType;
	this->btnSelectDataType->setSelectedValue(static_cast<int>(currentChannelPtr->dataType));

	setVisibility(true);
	return;
} // end showControl()

//-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-
// Process
//-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-
void TSOscCVChannelConfigScreen::step()
{
	if (this->visible && parentWidget != NULL) {
		oscCV* thisModule = dynamic_cast<oscCV*>( parentWidget->module );

		if (thisModule) {
			// Check for enable/disable data massaging
			if (translateTrigger.process(thisModule->params[oscCV::ParamIds::OSC_CH_TRANSLATE_VALS_PARAM].value)) {
				//debug("Translate button clicked");
				translateValsEnabled = !translateValsEnabled;
			}
		}
		btnToggleTranslateVals->checked = translateValsEnabled;
		OpaqueWidget::step(); // Parent
	}
	return;
}

//-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-
// validateValues(void)
// @returns : True if valid, false if not.
// POST CONDITION: tbErrors is set and errors may be displayed on the screen.
//-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-
bool TSOscCVChannelConfigScreen::validateValues()
{
	//debug("TSOscCVChannelConfigScreen::validateValues()");
	bool isValid = true;
	for (int i = 0; i < TextBoxIx::NumTextBoxes; i++)
	{
		bool valid = tbNumericBounds[i]->isValid();
		tbErrors[i] = (valid) ? std::string("") : std::string("Invalid value.");
		isValid = isValid && valid;
	}
	if (isValid)
	{
		//---------------------
		// Check Min < Max
		//---------------------
		isValid = false;
		try
		{
			// 1. Rack voltage
			bool valid = std::stof(tbNumericBounds[TextBoxIx::MinCVVolt]->text, NULL) < std::stof(tbNumericBounds[TextBoxIx::MaxCVVolt]->text, NULL);
			if (valid)
				isValid = true;
			else
				tbErrors[TextBoxIx::MinCVVolt] = std::string("Min should be < Max.");
			// 2. OSC Values
			valid = std::stof(tbNumericBounds[TextBoxIx::MinOSCVal]->text, NULL) < std::stof(tbNumericBounds[TextBoxIx::MaxOSCVal]->text, NULL);
			if (!valid)
				tbErrors[TextBoxIx::MinOSCVal] = std::string("Min should be < Max.");
			isValid = isValid && valid;
		}
		catch (const std::exception& e)
		{
			warn("Error %s.", e.what());
		}
	} // end if valid values
	return isValid;
} // end validateValues()

//-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-
// Save the values to the ptr.
// @channelPtr : (OUT) Place to save the values.
// @returns : True if saved, false if there was an error.
//-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-
bool TSOscCVChannelConfigScreen::saveValues(/*out*/ TSOSCCVChannel* channelPtr)
{
	bool saved = false;
		
	if (channelPtr != NULL && validateValues())
	{
		channelPtr->convertVals = translateValsEnabled;// btnToggleTranslateVals->value > 0;
		channelPtr->dataType = static_cast<TSOSCCVChannel::ArgDataType>(btnSelectDataType->selectedVal);
		try
		{
			channelPtr->minVoltage = std::stof(tbNumericBounds[TextBoxIx::MinCVVolt]->text);
			channelPtr->maxVoltage = std::stof(tbNumericBounds[TextBoxIx::MaxCVVolt]->text);
			channelPtr->minOscVal = std::stof(tbNumericBounds[TextBoxIx::MinOSCVal]->text);
			channelPtr->maxOscVal = std::stof(tbNumericBounds[TextBoxIx::MaxOSCVal]->text);
			saved = true;
		}
		catch (const std::exception& e)
		{
			warn("Error %s.", e.what());
		}
	}
	return saved;
} // end saveValues()
