#include <string.h>
#include <stdio.h>
#include <math.h> 
#include "trowaSoft.hpp"
#include "dsp/digital.hpp"
#include "trowaSoftComponents.hpp"
#include "trowaSoftUtilities.hpp"
#include "TSSequencerModuleBase.hpp"
#include "Module_voltSeq.hpp"

#define TROWA_VOLTSEQ_OSC_ROUND_VAL					 100   // Mult & Divisor for rounding.
#define TROWA_VOLTSEQ_KNOB_CHANGED_THRESHOLD		0.01  // Value must change at least this much to send changed value over OSC


// Round the value for OSC. We will match what VOLT mode shows
inline float roundValForOSC(float val) {
	return round(val * TROWA_VOLTSEQ_OSC_ROUND_VAL) / (float)(TROWA_VOLTSEQ_OSC_ROUND_VAL);
}

//-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-
// voltSeq::randomize()
// Only randomize the current gate/trigger steps.
//-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-
void voltSeq::randomize()
{
	int r, c;
	valuesChanging = true;
	for (int s = 0; s < maxSteps; s++) 
	{
		// randomUniform() - [0.0, 1.0)
		triggerState[currentPatternEditingIx][currentChannelEditingIx][s] = voltSeq_STEP_KNOB_MIN + randomUniform()*(voltSeq_STEP_KNOB_MAX - voltSeq_STEP_KNOB_MIN);		
		r = s / this->numCols; // TROWA_SEQ_STEP_NUM_COLS;
		c = s % this->numCols; // TROWA_SEQ_STEP_NUM_COLS;
		this->params[CHANNEL_PARAM + s].value = this->triggerState[currentPatternEditingIx][currentChannelEditingIx][s];
		knobStepMatrix[r][c]->setKnobValue(this->triggerState[currentPatternEditingIx][currentChannelEditingIx][s]);			
	}	
	reloadEditMatrix = true;
	valuesChanging = false;
	return;
} // end randomize()

//-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-
// Toggle the single step value
// (i.e. this command probably comes from an external source)
// @step : (IN) The step number to edit (0 to maxSteps).
// @val : (IN) The step value.
// @channel : (IN) The channel to edit (0 to TROWA_SEQ_NUM_CHNLS - 1).
// @pattern: (IN) The pattern to edit (0 to TROWA_SEQ_NUM_PATTERNS - 1).
//-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-
float voltSeq::getToggleStepValue(int step, float val, int channel, int pattern)
{
	return -triggerState[pattern][channel][step];
} // end getToggleStepValue()

//-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-
// Calculate a representation of all channels for this step
//-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-
float voltSeq::getPlayingStepValue(int step, int pattern)
{
	int count = 0;
	for (int c = 0; c < TROWA_SEQ_NUM_CHNLS; c++)
	{
		count += (this->triggerState[pattern][c][step] > 0.05 || this->triggerState[pattern][c][step] < -0.05);
	} // end for
	return (float)(count) / (float)(TROWA_SEQ_NUM_CHNLS);
} // end getPlayingStepValue()

//-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-
// Set a single the step value
// (i.e. this command probably comes from an external source)
// >> Should set the control knob value too if applicable. <<
// @step : (IN) The step number to edit (0 to maxSteps).
// @val : (IN) The step value.
// @channel : (IN) The channel to edit (0 to TROWA_SEQ_NUM_CHNLS - 1).
// @pattern: (IN) The pattern to edit (0 to TROWA_SEQ_NUM_PATTERNS - 1).
//-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-
void voltSeq::setStepValue(int step, float val, int channel, int pattern)
{
	int r, c;
	if (channel == CURRENT_EDIT_CHANNEL_IX)
	{
		channel = currentChannelEditingIx;
	}
	if (pattern == CURRENT_EDIT_PATTERN_IX)
	{
		pattern = currentPatternEditingIx;
	}
	triggerState[pattern][channel][step] = val;
	r = step / this->numCols;
	c = step % this->numCols;
	if (pattern == currentPatternEditingIx && channel == currentChannelEditingIx)
	{
		if (triggerState[pattern][channel][step])
		{
			gateLights[r][c] = 1.0f - stepLights[r][c];
			if (gateTriggers != NULL)
				gateTriggers[step].state = SchmittTrigger::HIGH;
		}
		else
		{
			gateLights[r][c] = 0.0f; // Turn light off	
			if (gateTriggers != NULL)
				gateTriggers[step].state = SchmittTrigger::LOW;
		}
	}
	oscMutex.lock();
	if (useOSC && oscInitialized)
	{
		// Send the result back
#if TROWA_DEBUG_MSGS >= TROWA_DEBUG_LVL_MED
		debug("voltSeq:step() - Received a msg (s=%d, v=%0.2f, c=%d, p=%d), sending back (%s).",
			step, val, channel, pattern,
			oscAddrBuffer[SeqOSCOutputMsg::EditStep]);
#endif
		char valOutputBuffer[20] = { 0 };
		char addrBuff[50] = { 0 };
		float val = roundValForOSC(triggerState[pattern][channel][step]);
		ValueModes[selectedOutputValueMode]->GetDisplayString(ValueModes[selectedOutputValueMode]->GetOutputValue(triggerState[pattern][channel][step]), valOutputBuffer);

		sprintf(addrBuff, oscAddrBuffer[SeqOSCOutputMsg::EditStep], step + 1);
		osc::OutboundPacketStream oscStream(oscBuffer, OSC_OUTPUT_BUFFER_SIZE);
		oscStream << osc::BeginBundleImmediate
			<< osc::BeginMessage(addrBuff)
			<< val // Rounded value for touchOSC
			<< osc::EndMessage;
		sprintf(addrBuff, oscAddrBuffer[SeqOSCOutputMsg::EditStepString], step + 1);
		oscStream << osc::BeginMessage( addrBuff )
			<< valOutputBuffer // String version of the value (touchOSC needs this)
			<< osc::EndMessage
			<< osc::EndBundle;
		oscTxSocket->Send(oscStream.Data(), oscStream.Size());
	}
	oscMutex.unlock();

	// Set our knobs
	if (pattern == currentPatternEditingIx && channel == currentChannelEditingIx)
	{
		this->knobStepMatrix[r][c]->setKnobValue(val);
		this->params[ParamIds::CHANNEL_PARAM + step].value = val;
	}
	return;
} // end setStepValue()

//-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-
// Shift all steps (+/-) some number of volts.
// @patternIx : (IN) The index into our pattern matrix (0-15). Or TROWA_INDEX_UNDEFINED for all patterns.
// @channelIx : (IN) The index of the channel (gate/trigger/voice) if any (0-15, or TROWA_SEQ_COPY_CHANNELIX_ALL/TROWA_INDEX_UNDEFINED for all).
// @volts: (IN) The number of volts to add.
//-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-
void voltSeq::shiftValues(/*in*/ int patternIx, /*in*/ int channelIx, /*in*/ float volts)
{
	// Normal Range -10 to +10 V (20)
	// Midi: -5 to +5 V or -4 to + 6 V (10), so +1 octave will be +2V in 'Normal' range.
	float add = volts;

	if (selectedOutputValueMode == ValueMode::VALUE_MIDINOTE)
	{
		add = volts * 2.0;
	}
	else if (selectedOutputValueMode == ValueMode::VALUE_PATTERN)
	{
		add = (voltSeq_STEP_KNOB_MAX - voltSeq_STEP_KNOB_MIN) / TROWA_SEQ_NUM_PATTERNS * volts;
	}
	if (patternIx == TROWA_INDEX_UNDEFINED)
	{
		debug("shiftValues(ALL Patterns, %f) - Add %f", volts, add);
		// All patterns:
		for (int p = 0; p < TROWA_SEQ_NUM_PATTERNS; p++)
		{
			shiftValues(p, TROWA_INDEX_UNDEFINED, volts); // All channels
		}
	}
	else if (channelIx == TROWA_INDEX_UNDEFINED)
	{
		debug("shiftValues(This Pattern, %f) - Add %f", volts, add);
		// This pattern:
		for (int channelIx = 0; channelIx < TROWA_SEQ_NUM_CHNLS; channelIx++)
		{
			for (int s = 0; s < maxSteps; s++)
			{
				float tmp = clamp(triggerState[patternIx][channelIx][s] + add, /*min*/ voltSeq_STEP_KNOB_MIN,  /*max*/ voltSeq_STEP_KNOB_MAX);
				triggerState[patternIx][channelIx][s] = tmp;
				if (patternIx == currentPatternEditingIx && channelIx == currentChannelEditingIx)
				{
					int r = s / numCols;
					int c = s % numCols;
					this->params[CHANNEL_PARAM + s].value = tmp;
					knobStepMatrix[r][c]->setKnobValue(tmp);
				}
			}
		}
		//this->reloadEditMatrix = true;
	}
	else
	{
		// Just this channel
		debug("shiftValues(%d, %d, %f) - Add %f", patternIx, channelIx, volts, add);
		for (int s = 0; s < maxSteps; s++)
		{
			float tmp = clamp(triggerState[patternIx][channelIx][s] + add, /*min*/ voltSeq_STEP_KNOB_MIN,  /*max*/ voltSeq_STEP_KNOB_MAX);
			debug(" %d = %f + %fV (add %f) = %f", s, triggerState[patternIx][channelIx][s], volts, add, tmp);
			triggerState[patternIx][channelIx][s] = tmp;
			if (patternIx == currentPatternEditingIx && channelIx == currentChannelEditingIx)
			{
				int r = s / numCols;
				int c = s % numCols;
				this->params[CHANNEL_PARAM + s].value = tmp;
				knobStepMatrix[r][c]->setKnobValue(tmp);
			}
		}
		//if (patternIx == currentPatternEditingIx && channelIx == currentChannelEditingIx)
		//	this->reloadEditMatrix = true;
	}
	return;
} // end shiftValues()


//-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-
// voltSeq::step()
//-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-
void voltSeq::step()
{
	if (!initialized)
		return;
	bool gOn = true;
	bool pulse = false;
	bool reloadMatrix = false;
	bool valueModeChanged =  false;
	bool sendOSC = useOSC && oscInitialized;

	TSSequencerModuleBase::getStepInputs(&pulse, &reloadMatrix, &valueModeChanged);
	int r = 0;
	int c = 0;

	// Current output value mode	
	ValueSequencerMode* currOutputValueMode = ValueModes[selectedOutputValueMode];
	if (valueModeChanged)
	{
		modeString = currOutputValueMode->displayName;
		// Change our lights 
		for (r = 0; r < this->numRows; r++)
		{
			for (c = 0; c < this->numCols; c++)
			{
				dynamic_cast<TS_LightArc*>(padLightPtrs[r][c])->zeroAnglePoint = currOutputValueMode->zeroPointAngle_radians;
				dynamic_cast<TS_LightArc*>(padLightPtrs[r][c])->valueMode = currOutputValueMode;
				knobStepMatrix[r][c]->defaultValue = currOutputValueMode->zeroValue;
#if TROWA_DEBUG_MSGS >= TROWA_DEBUG_LVL_MED
				debug("Setting Knob %d default value to %.2f", knobStepMatrix[r][c]->id, knobStepMatrix[r][c]->defaultValue);
#endif
			}
		}
	}
	lastOutputValueMode = selectedOutputValueMode;
		
	// Only send OSC if it is enabled, initialized, and we are in EDIT mode.
	sendOSC = useOSC && currentCtlMode == ExternalControllerMode::EditMode && oscInitialized;
	//-- * Load the trigger we are editing into our button matrix for display:
	// This is what we are showing not what we playing
	char valOutputBuffer[20] = { 0 };
	char addrBuff[100] = { 0 };
	std::string stepStringAddr = std::string(oscAddrBuffer[SeqOSCOutputMsg::EditStepString]);
	if (reloadMatrix || reloadEditMatrix || valueModeChanged)
	{
		reloadEditMatrix = false;
		oscMutex.lock();
		osc::OutboundPacketStream oscStream(oscBuffer, OSC_OUTPUT_BUFFER_SIZE);
		if (sendOSC && oscInitialized)
		{
#if TROWA_DEBUG_MSGS >= TROWA_DEBUG_LVL_MED
			debug("Sending reload matrix: %s.", oscAddrBuffer[SeqOSCOutputMsg::EditStep]);
#endif
			oscStream << osc::BeginBundleImmediate;
		}
		oscMutex.unlock();
		// Load this channel into our 4x4 matrix
		for (int s = 0; s < maxSteps; s++) 
		{
			r = s / this->numCols; // TROWA_SEQ_STEP_NUM_COLS;
			c = s % this->numCols; // TROWA_SEQ_STEP_NUM_COLS;
			padLightPtrs[r][c]->setColor(voiceColors[currentChannelEditingIx]);
			gateLights[r][c] = 1.0 - stepLights[r][c];
			this->params[CHANNEL_PARAM + s].value = this->triggerState[currentPatternEditingIx][currentChannelEditingIx][s];
			knobStepMatrix[r][c]->setKnobValue(this->triggerState[currentPatternEditingIx][currentChannelEditingIx][s]);			
			lights[PAD_LIGHTS + s].value = gateLights[r][c];
			oscMutex.lock();
			if (sendOSC && oscInitialized)
			{
				// Each step may have up to 4-ish messages, so send 4 or 8 steps at a time.
				if (s > 0 && s % 8 == 0) // There is a limit to client buffer size, so let's not make the bundles too large. Hopefully they can take this many steps at a time.
				{
					// Send this bundle and then start a new one
					oscStream << osc::EndBundle;
					oscTxSocket->Send(oscStream.Data(), oscStream.Size());
					oscStream.Clear();
					// Start new bundle:
					oscStream << osc::BeginBundleImmediate;
				}
				oscLastSentVals[s] = roundValForOSC(triggerState[currentPatternEditingIx][currentChannelEditingIx][s]);
				currOutputValueMode->GetDisplayString(currOutputValueMode->GetOutputValue(triggerState[currentPatternEditingIx][currentChannelEditingIx][s]), valOutputBuffer);
				// Step value:
				sprintf(addrBuff, oscAddrBuffer[SeqOSCOutputMsg::EditStep], s+1);
				oscStream << osc::BeginMessage(addrBuff)
					<< oscLastSentVals[s]
					<< osc::EndMessage;
				if (oscCurrentClient == OSCClient::touchOSCClient)
				{
					// Change color
					sprintf(addrBuff, OSC_TOUCH_OSC_CHANGE_COLOR_FS, addrBuff);
					oscStream << osc::BeginMessage(addrBuff)
						<< touchOSC::ChannelColors[currentChannelEditingIx]
						<< osc::EndMessage;
					// LED Color (current step LED):
					sprintf(addrBuff, oscAddrBuffer[SeqOSCOutputMsg::PlayStepLed], s + 1);
					sprintf(addrBuff, OSC_TOUCH_OSC_CHANGE_COLOR_FS, addrBuff);
					oscStream << osc::BeginMessage(addrBuff)
						<< touchOSC::ChannelColors[currentChannelEditingIx]
						<< osc::EndMessage;
				}
				// Step String
				sprintf(addrBuff, oscAddrBuffer[SeqOSCOutputMsg::EditStepString], s+1);
				oscStream << osc::BeginMessage( addrBuff )
					<< valOutputBuffer // String version of the value (touchOSC needs this)
					<< osc::EndMessage;
			}
			oscMutex.unlock();
		} // end for
		oscMutex.lock();
		if (sendOSC && oscInitialized)
		{
			if (oscCurrentClient == OSCClient::touchOSCClient)
			{
				// Also change color on the Channel control:
				sprintf(addrBuff, OSC_TOUCH_OSC_CHANGE_COLOR_FS, oscAddrBuffer[SeqOSCOutputMsg::EditChannel]);
				oscStream << osc::BeginMessage(addrBuff)
					<< touchOSC::ChannelColors[currentChannelEditingIx]
					<< osc::EndMessage;
			}

			// End last bundle and send:
			oscStream << osc::EndBundle;
			oscTxSocket->Send(oscStream.Data(), oscStream.Size());
		}
		oscMutex.unlock();
	} // end if reload edit matrix
	//-- * Read the buttons
	else if (!valuesChanging) // Only read in if another thread isn't changing the values
	{		
		oscMutex.lock();
		osc::OutboundPacketStream oscStream(oscBuffer, OSC_OUTPUT_BUFFER_SIZE);
		if (sendOSC && oscInitialized)
		{
			oscStream << osc::BeginBundleImmediate;
		}
		oscMutex.unlock();

		int numChanged = 0;
		const float threshold = TROWA_VOLTSEQ_KNOB_CHANGED_THRESHOLD;
		// Channel step knobs - Read Inputs
		for (int s = 0; s < maxSteps; s++) 
		{
			bool sendLightVal = false;
			this->triggerState[currentPatternEditingIx][currentChannelEditingIx][s] = this->params[ParamIds::CHANNEL_PARAM + s].value;
			float dv = roundValForOSC(this->triggerState[currentPatternEditingIx][currentChannelEditingIx][s]) - oscLastSentVals[s];
			sendLightVal = sendOSC && (dv > threshold || -dv > threshold); // Let's not send super tiny changes
			r = s / this->numCols;
			c = s % this->numCols;			
			stepLights[r][c] -= stepLights[r][c] / lightLambda / engineGetSampleRate();
			gateLights[r][c] = stepLights[r][c];			
			lights[PAD_LIGHTS + s].value = gateLights[r][c];

			oscMutex.lock();
			// This step has changed and we are doing OSC
			if (sendLightVal && oscInitialized)
			{		
				oscLastSentVals[s] = roundValForOSC(triggerState[currentPatternEditingIx][currentChannelEditingIx][s]);
				// voltSeq should send the actual values.
#if TROWA_DEBUG_MSGS >= TROWA_DEBUG_LVL_MED
				debug("Step changed %d (new val is %.4f), dv = %.4f, sending OSC %s", s, 
					oscLastSentVals[s],
					dv,
					oscAddrBuffer[SeqOSCOutputMsg::EditStep]);
#endif
				// Now also send the equivalent string:
				currOutputValueMode->GetDisplayString(currOutputValueMode->GetOutputValue( triggerState[currentPatternEditingIx][currentChannelEditingIx][s] ), valOutputBuffer);
				sprintf(addrBuff, oscAddrBuffer[SeqOSCOutputMsg::EditStep], s + 1);
				debug("Send: %s -> %s : %s", oscAddrBuffer[SeqOSCOutputMsg::EditStepString], addrBuff, valOutputBuffer);
				oscStream << osc::BeginMessage(addrBuff)
					<< oscLastSentVals[s]
					<< osc::EndMessage;
				sprintf(addrBuff, oscAddrBuffer[SeqOSCOutputMsg::EditStepString], s + 1);
				oscStream << osc::BeginMessage(addrBuff)
					<< valOutputBuffer // String version of the value (touchOSC needs this)
					<< osc::EndMessage;
				numChanged++;
			} // end if send the value over OSC
			oscMutex.unlock();
		} // end loop through step buttons
		oscMutex.lock();
		if (sendOSC && oscInitialized && numChanged > 0)
		{
			oscStream << osc::EndBundle;
			oscTxSocket->Send(oscStream.Data(), oscStream.Size());
		}
		oscMutex.unlock();
	} // end else (read button matrix)
	
	// Set Outputs (16 triggers)	
	for (int g = 0; g < TROWA_SEQ_NUM_CHNLS; g++) 
	{		
		float gate = (running && gOn) ? currOutputValueMode->GetOutputValue( triggerState[currentPatternPlayingIx][g][index] ) : 0.0; //***********VOLTAGE OUTPUT
		outputs[CHANNELS_OUTPUT + g].value= gate;
		// Output lights (around output jacks for each gate/trigger):
		gateLightsOut[g] = (gate < 0) ? -gate : gate;
		lights[CHANNEL_LIGHTS + g].value = gate / currOutputValueMode->outputVoltageMax;
	}
	return;
} // end step()


//-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-
// voltSeqWidget()
// Widget for the trowaSoft 16-step voltage/knobby sequencer.
// @seqModule : (IN) Pointer to the sequencer module.
//-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-
voltSeqWidget::voltSeqWidget(voltSeq* seqModule) : TSSequencerWidgetBase(seqModule)
{		
	bool isPreview = seqModule == NULL; 
	//voltSeq *module = new voltSeq();
	//setModule(module);

	//////////////////////////////////////////////
	// Background
	//////////////////////////////////////////////
	{
		SVGPanel *panel = new SVGPanel();
		panel->box.size = box.size;
		panel->setBackground(SVG::load(assetPlugin(plugin, "res/voltSeq.svg")));
		addChild(panel);
	}
	
	TSSequencerWidgetBase::addBaseControls(false);
	
	// (User) Input KNOBS ==================================================	
	int y = 115;
	int x = 79;
	int dx = 0;
	//int lightSize = 50 - 2*dx;
	Vec lSize = Vec(50, 50);
	int v = 0;
	ValueSequencerMode* currValueMode = seqModule->ValueModes[seqModule->selectedOutputValueMode];
	seqModule->modeString = currValueMode->displayName;
	for (int r = 0; r < seqModule->numRows; r++) //---------THE KNOBS
	{
		for (int c = 0; c < seqModule->numCols; c++)
		{						
			// Pad Knob:
			TS_LightedKnob* knobPtr = dynamic_cast<TS_LightedKnob*>(ParamWidget::create<TS_LightedKnob>(Vec(x, y), seqModule, 
				TSSequencerModuleBase::CHANNEL_PARAM + r*seqModule->numCols + c, 
				/*min*/ voltSeq_STEP_KNOB_MIN,  /*max*/ voltSeq_STEP_KNOB_MAX, /*default*/ 0.0f));
			if (!isPreview)
				seqModule->knobStepMatrix[r][c] = knobPtr;
			knobPtr->id = r * seqModule->numCols + c;
			knobPtr->value = 0.0f;// voltSeq_STEP_KNOB_MIN;
			knobPtr->defaultValue = 0.0f; // For now, assume always that Voltage is first

			
			// Keep a reference to our pad lights so we can change the colors			
			TS_LightArc* lightPtr = dynamic_cast<TS_LightArc*>(TS_createColorValueLight<TS_LightArc>(/*pos */ Vec(x+dx, y+dx), 
				/*seqModule*/ seqModule,
				/*lightId*/ TSSequencerModuleBase::PAD_LIGHTS + r*seqModule->numCols + c,								
				/* size */ lSize, /* color */ seqModule->voiceColors[seqModule->currentChannelEditingIx]));			
			lightPtr->numericValue = &(knobPtr->value);
			lightPtr->currentAngle_radians = &(knobPtr->currentAngle);
			lightPtr->zeroAnglePoint = currValueMode->zeroPointAngle_radians;
			lightPtr->valueMode = currValueMode;			

			if (!isPreview)
				seqModule->padLightPtrs[r][c] = lightPtr;			
			addChild( lightPtr );
			
			addParam(knobPtr);
			knobPtr->dirty = true;

			x+= 59;
			v++;
		}		
		y += 59; // Next row
		x = 79;
	} // end loop through 4x4 grid
	seqModule->initialized = true;
	return;
}

struct voltSeq_ShiftVoltageSubMenuItem : MenuItem {
	voltSeq* sequencerModule;
	float amount = 1.0;
	enum ShiftType {
		// Current Edit Pattern & Channel
		CurrentChannelOnly,
		// Current Edit Pattern, All Channels
		ThisPattern,
		// All patterns, all channels
		AllPatterns
	};
	ShiftType Target = ShiftType::CurrentChannelOnly;

	voltSeq_ShiftVoltageSubMenuItem(std::string text, ShiftType target, float amount, voltSeq* seqModule)
	{
		this->box.size.x = 200;
		this->text = text;
		this->Target = target;
		this->amount = amount;
		this->sequencerModule = seqModule;
	}

	void onAction(EventAction &e) override {
		if (this->Target == ShiftType::AllPatterns)
		{
			sequencerModule->shiftValues(TROWA_INDEX_UNDEFINED, TROWA_SEQ_COPY_CHANNELIX_ALL, amount);
		}
		else if (this->Target == ShiftType::ThisPattern)
		{
			sequencerModule->shiftValues(sequencerModule->currentPatternEditingIx, TROWA_SEQ_COPY_CHANNELIX_ALL, amount);
		}
		else //if (this->Target == ShiftType::CurrentChannelOnly)
		{
			sequencerModule->shiftValues(sequencerModule->currentPatternEditingIx, sequencerModule->currentChannelEditingIx, amount);
		}
	}
	void step() override {
		//rightText = (seq3->gateMode == gateMode) ? "✔" : "";
	}
};

struct voltSeq_ShiftVoltageSubMenu : Menu {
	voltSeq* sequencerModule;
	float amount = 1.0;

	voltSeq_ShiftVoltageSubMenu(float amount, voltSeq* seqModule)
	{
		this->box.size = Vec(200, 60);
		this->amount = amount;
		this->sequencerModule = seqModule;
		return;
	}

	void createChildren()
	{
		voltSeq_ShiftVoltageSubMenuItem* menuItem = new voltSeq_ShiftVoltageSubMenuItem("Current Edit Channel", voltSeq_ShiftVoltageSubMenuItem::ShiftType::CurrentChannelOnly, this->amount, this->sequencerModule);
		addChild(menuItem); //this->pushChild(menuItem);
		menuItem = new voltSeq_ShiftVoltageSubMenuItem("Current Edit Pattern", voltSeq_ShiftVoltageSubMenuItem::ShiftType::ThisPattern, this->amount, this->sequencerModule);
		addChild(menuItem);// this->pushChild(menuItem);
		menuItem = new voltSeq_ShiftVoltageSubMenuItem("ALL Patterns", voltSeq_ShiftVoltageSubMenuItem::ShiftType::AllPatterns, this->amount, this->sequencerModule);
		addChild(menuItem);// this->pushChild(menuItem);
		return;
	}
};
// First tier menu item. Create Submenu
struct voltSeq_ShiftVoltageMenuItem : MenuItem {
	voltSeq* sequencerModule;
	float amount = 1.0;

	voltSeq_ShiftVoltageMenuItem(std::string text, float amount, voltSeq* seqModule)
	{
		this->box.size.x = 200;
		this->text = text;
		this->amount = amount;
		this->sequencerModule = seqModule;
		return;
	}
	Menu *createChildMenu() override {
		voltSeq_ShiftVoltageSubMenu* menu = new voltSeq_ShiftVoltageSubMenu(amount, sequencerModule);
		menu->amount = this->amount;
		menu->sequencerModule = this->sequencerModule;
		menu->createChildren();
		menu->box.size = Vec(200, 60);
		return menu;
	}
};

//-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-
// voltSeqWidget
// Create context menu with the ability to shift 1 V (1 octave).
//-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-
Menu *voltSeqWidget::createContextMenu()
{
	Menu *menu = TSSequencerWidgetBase::createContextMenu();

	// Add voltSeq specific options:
	MenuLabel *spacerLabel = new MenuLabel();
	menu->addChild(spacerLabel); //menu->pushChild(spacerLabel);


	voltSeq* sequencerModule = dynamic_cast<voltSeq*>(module);
	assert(sequencerModule);

	//-------- Shift Values Up/Down ------- //
	// (Affects N steps).
	MenuLabel *modeLabel = new MenuLabel();
	modeLabel->text = "Shift Values";
	menu->addChild(modeLabel); //menu->pushChild(modeLabel);

	//voltSeq_ShiftVoltageMenuItem* menuItem = new voltSeq_ShiftVoltageMenuItem("> +1 V/Octave/Patt", 1.0, sequencerModule);
	menu->addChild(new voltSeq_ShiftVoltageMenuItem("> +1 V/Octave/Patt", 1.0, sequencerModule));// menu->pushChild(menuItem);
	//menuItem = new voltSeq_ShiftVoltageMenuItem("> -1 V/Octave/Patt", -1.0, sequencerModule);
	menu->addChild(new voltSeq_ShiftVoltageMenuItem("> -1 V/Octave/Patt", -1.0, sequencerModule));// menu->pushChild(menuItem);
	return menu;
}

// Single model object? https://github.com/VCVRack/Rack/issues/258:
RACK_PLUGIN_MODEL_INIT(trowaSoft, VoltSeq) {
   Model* modelVoltSeq = Model::create<voltSeq, voltSeqWidget>(/*manufacturer*/ TROWA_PLUGIN_NAME, /*slug*/ "voltSeq", /*name*/ "voltSeq", /*Tags*/ SEQUENCER_TAG, EXTERNAL_TAG);
   return modelVoltSeq;
}
