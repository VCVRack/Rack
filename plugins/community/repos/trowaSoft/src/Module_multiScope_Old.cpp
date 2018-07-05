#include "Features.hpp"

#if !USE_NEW_SCOPE

#include <string.h>
#include "trowaSoft.hpp"
#include "trowaSoftComponents.hpp"
#include "trowaSoftUtilities.hpp"
#include "dsp/digital.hpp"
#include "Module_multiScope_Old.hpp"
#include "Widget_multiScope_Old.hpp"

// multiScope model.
Model *modelMultiScope = Model::create<multiScope, multiScopeWidget>(/*manufacturer*/ TROWA_PLUGIN_NAME, /*slug*/ "multiScope", /*name*/ "multiScope", /*Tags*/ VISUAL_TAG, UTILITY_TAG);


//-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-
// multiScope()
// Multi scope.
//-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-
multiScope::multiScope() : Module(multiScope::NUM_PARAMS, multiScope::NUM_INPUTS, multiScope::NUM_OUTPUTS, multiScope::NUM_LIGHTS)
{
	initialized = false;
	firstLoad = true;
	float initColorKnobs[4] = { -10, -3.33, 3, 7.2 };

	for (int wIx = 0; wIx < TROWA_SCOPE_NUM_WAVEFORMS; wIx++)
	{
		waveForms[wIx] = new TSWaveform();
		waveForms[wIx]->setHueFromKnob(initColorKnobs[wIx]);
	}
	return;
} // end multiScope()
//-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-
// ~multiScope()
//-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-
multiScope::~multiScope()
{
	// Clean our stuff
	for (int wIx = 0; wIx < TROWA_SCOPE_NUM_WAVEFORMS; wIx++)
	{
		delete waveForms[wIx];
	}
	return;
} // end multiScope()

//-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-
// step(void)
//-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-
void multiScope::step() {
	if (!initialized)
		return;


	TSWaveform* waveForm = NULL;
	for (int wIx = 0; wIx < TROWA_SCOPE_NUM_WAVEFORMS; wIx++)
	{			
		waveForm = waveForms[wIx]; // tmp pointer
		// Lissajous:
		if (waveForm->lissajousTrigger.process(params[multiScope::LISSAJOUS_PARAM + wIx].value))
		{
			waveForm->lissajous = !waveForm->lissajous;
		}
		lights[multiScope::LISSAJOUS_LED + wIx].value = waveForm->lissajous;

		// Compute Color:
		float hue = 0;
		if(inputs[multiScope::COLOR_INPUT+wIx].active){
			hue = clamp(rescale(inputs[multiScope::COLOR_INPUT+wIx].value, TROWA_SCOPE_HUE_INPUT_MIN_V, TROWA_SCOPE_HUE_INPUT_MAX_V, 0.0, 1.0), 0.0, 1.0);
		} else {
			hue = rescale(params[multiScope::COLOR_PARAM+wIx].value, TROWA_SCOPE_HUE_KNOB_MIN, TROWA_SCOPE_HUE_KNOB_MAX, 0.0, 1.0);					
		}
		waveForm->colorChanged = hue != waveForm->waveHue || firstLoad; 
		if (waveForm->colorChanged)
		{
			waveForm->waveHue = hue;
			// nvgHSLA(waveForm->waveHue, 0.5, 0.5, 0xdd);		
			waveForm->waveColor = HueToColor(waveForm->waveHue); // Base Color (opacity full)
#if TROWA_SCOPE_USE_COLOR_LIGHTS
			// Change the light color:
			waveForm->waveLight->setColor(waveForm->waveColor);
#endif
		}

		// Opacity:
		if (inputs[multiScope::OPACITY_INPUT + wIx].active)
		{
			waveForm->waveOpacity = clamp(rescale(inputs[multiScope::OPACITY_INPUT + wIx].value, TROWA_SCOPE_OPACITY_INPUT_MIN, TROWA_SCOPE_OPACITY_INPUT_MAX, TROWA_SCOPE_MIN_OPACITY, TROWA_SCOPE_MAX_OPACITY),
									 TROWA_SCOPE_MIN_OPACITY, TROWA_SCOPE_MAX_OPACITY);
		}
		else
		{
			waveForm->waveOpacity = params[multiScope::OPACITY_PARAM + wIx].value;
		}
		
		// Compute rotation:
		waveForm->rotKnobValue = params[multiScope::ROTATION_PARAM+wIx].value;
		if (waveForm->rotModeTrigger.process(params[multiScope::ROTATION_MODE_PARAM+wIx].value))
		{
			waveForm->rotMode = !waveForm->rotMode;
		}
		lights[multiScope::ROT_LED+wIx].value = waveForm->rotMode;		
		float rot = 0;
		float rotRate = 0;
		if (waveForm->rotMode)
		{
			// Absolute position:
			rot = rescale(params[multiScope::ROTATION_PARAM+wIx].value + inputs[multiScope::ROTATION_INPUT+wIx].value, 0, 10, 0, NVG_PI);
		}
		else
		{
			// Differential rotation
			rotRate = rescale(params[multiScope::ROTATION_PARAM+wIx].value + inputs[multiScope::ROTATION_INPUT+wIx].value, 0, 10, 0, 0.5);
		}
		waveForm->rotAbsValue = rot;
		waveForm->rotDiffValue = rotRate;
		
		// Compute time:
		float deltaTime = powf(2.0, params[TIME_PARAM+wIx].value + inputs[TIME_INPUT+wIx].value);
		int frameCount = (int)ceilf(deltaTime * engineGetSampleRate());
		// Add frame to buffer
		if (waveForm->bufferIndex < BUFFER_SIZE) {
			if (++(waveForm->frameIndex) > frameCount) {
				waveForm->frameIndex = 0;
				waveForm->bufferX[waveForm->bufferIndex] = inputs[X_INPUT+wIx].value;
				waveForm->bufferY[waveForm->bufferIndex] = inputs[Y_INPUT+wIx].value;
				waveForm->bufferPenOn[waveForm->bufferIndex] = (!inputs[PEN_ON_INPUT + wIx].active || inputs[PEN_ON_INPUT + wIx].value > 0.1); // Allow some noise?
				waveForm->bufferIndex++;
			}
		}
		else {
			if (waveForm->lissajous)
			{
				// Reset
				waveForm->bufferIndex = 0;
				waveForm->frameIndex = 0;
			}
			else
			{
				// Just show stuff (no trigger inputs)
				waveForm->frameIndex++;
				float holdTime = 0.1;
				if (waveForm->frameIndex >= engineGetSampleRate() * holdTime) {
					waveForm->bufferIndex = 0; 
					waveForm->frameIndex = 0;
				}
			}
		}
	} // end loop through waveforms
	firstLoad = false;
	return;
} // end step()

#endif // end if use old scope