#include "Features.hpp"

#if USE_NEW_SCOPE


#include <string.h>
#include "trowaSoft.hpp"
#include "trowaSoftComponents.hpp"
#include "trowaSoftUtilities.hpp"
#include "dsp/digital.hpp"
#include "Module_multiScope.hpp"
#include "TSScopeBase.hpp"
#include "Widget_multiScope.hpp"



//-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-
// multiScope()
// Multi scope.
//-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-
multiScope::multiScope() : Module(multiScope::NUM_PARAMS, multiScope::NUM_INPUTS, multiScope::NUM_OUTPUTS, multiScope::NUM_LEDS)
{
	initialized = false;
	firstLoad = true;
	plotBackgroundColor = COLOR_BLACK;
	float initColorKnobs[4] = { -10, -3.33, 3, 7.2 };

	for (int wIx = 0; wIx < TROWA_SCOPE_NUM_WAVEFORMS; wIx++)
	{
		waveForms[wIx] = new TSWaveform();
		waveForms[wIx]->setHueFromKnob(initColorKnobs[wIx]);
		waveForms[wIx]->setFillHueFromKnob(initColorKnobs[wIx]);
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

#if ENABLE_BG_COLOR_PICKER
	if (plotBackgroundDisplayOnTrigger.process(params[multiScope::BGCOLOR_DISPLAY_PARAM].value))
	{
		showColorPicker = !showColorPicker;
		if (showColorPicker)
		{
			// Load Background color:
			editColorPointer = &(this->plotBackgroundColor);
		}
	}
	lights[multiScope::BGCOLOR_DISPLAY_LED].value = showColorPicker;
#endif

	//TSWaveform* waveForm = NULL;
	for (int wIx = 0; wIx < TROWA_SCOPE_NUM_WAVEFORMS; wIx++)
	{			
		//waveForm = waveForms[wIx]; // tmp pointer

		// Effect:
		waveForms[wIx]->gEffectIx = (int)clamp(static_cast<int>(roundf(params[multiScope::EFFECT_PARAM+wIx].value)), 0, TROWA_SCOPE_NUM_EFFECTS - 1);

		// Lissajous:
		if (waveForms[wIx]->lissajousTrigger.process(params[multiScope::LISSAJOUS_PARAM + wIx].value))
		{
			waveForms[wIx]->lissajous = !waveForms[wIx]->lissajous;
		}
		lights[multiScope::LISSAJOUS_LED + wIx].value = waveForms[wIx]->lissajous;

		// Compute Color:
		float hue = 0;
		if(inputs[multiScope::COLOR_INPUT+wIx].active){
			hue = clamp(rescale(inputs[multiScope::COLOR_INPUT+wIx].value, TROWA_SCOPE_HUE_INPUT_MIN_V, TROWA_SCOPE_HUE_INPUT_MAX_V, 0.0, 1.0), 0.0, 1.0);
		} else {
			hue = rescale(params[multiScope::COLOR_PARAM+wIx].value, TROWA_SCOPE_HUE_KNOB_MIN, TROWA_SCOPE_HUE_KNOB_MAX, 0.0, 1.0);					
		}
		waveForms[wIx]->colorChanged = hue != waveForms[wIx]->waveHue || firstLoad; 
		if (waveForms[wIx]->colorChanged)
		{
			waveForms[wIx]->waveHue = hue;
			if (hue > 0.99)
			{
				// Inject white
				waveForms[wIx]->waveColor = COLOR_WHITE;
			}
			else
			{
				waveForms[wIx]->waveColor = HueToColor(waveForms[wIx]->waveHue); // Base Color (opacity full)
			}
#if TROWA_SCOPE_USE_COLOR_LIGHTS
			// Change the light color:
			waveForms[wIx]->waveLight->setColor(waveForms[wIx]->waveColor);
#endif
		} // end if color changed		
		// Opacity:
		if (inputs[multiScope::OPACITY_INPUT + wIx].active)
		{
			waveForms[wIx]->waveOpacity = clamp(rescale(inputs[multiScope::OPACITY_INPUT + wIx].value, TROWA_SCOPE_OPACITY_INPUT_MIN, TROWA_SCOPE_OPACITY_INPUT_MAX, TROWA_SCOPE_MIN_OPACITY, TROWA_SCOPE_MAX_OPACITY),
									 TROWA_SCOPE_MIN_OPACITY, TROWA_SCOPE_MAX_OPACITY);
		}
		else
		{
			waveForms[wIx]->waveOpacity = params[multiScope::OPACITY_PARAM + wIx].value;
		}
		// Line Thickness
		if (inputs[multiScope::THICKNESS_INPUT + wIx].active)
		{
			waveForms[wIx]->lineThickness = clamp(rescale(inputs[multiScope::THICKNESS_INPUT + wIx].value, TROWA_SCOPE_THICKNESS_INPUT_MIN, TROWA_SCOPE_THICKNESS_INPUT_MAX, TROWA_SCOPE_THICKNESS_MIN, TROWA_SCOPE_THICKNESS_MAX),
				TROWA_SCOPE_THICKNESS_MIN, TROWA_SCOPE_THICKNESS_MAX);
		}
		else
		{
			waveForms[wIx]->lineThickness = params[multiScope::THICKNESS_PARAM + wIx].value;
		}

		// Compute Fill :::::::::::::::::::::::::::::::::::::::::
		if (waveForms[wIx]->fillOnTrigger.process(params[multiScope::FILL_ON_PARAM + wIx].value))
		{
			waveForms[wIx]->doFill = !waveForms[wIx]->doFill;
			//debug("Waveform %d: Fill On Clicked : %d (ParamId: %d).", wIx, waveForms[wIx]->doFill, multiScope::FILL_ON_PARAM + wIx);
		}
		lights[multiScope::FILL_ON_LED + wIx].value = waveForms[wIx]->doFill;
		hue = 0;
		if (inputs[multiScope::FILL_COLOR_INPUT + wIx].active) {
			hue = clamp(rescale(inputs[multiScope::FILL_COLOR_INPUT + wIx].value, TROWA_SCOPE_HUE_INPUT_MIN_V, TROWA_SCOPE_HUE_INPUT_MAX_V, 0.0, 1.0), 0.0, 1.0);
		}
		else {
			hue = rescale(params[multiScope::FILL_COLOR_PARAM + wIx].value, TROWA_SCOPE_HUE_KNOB_MIN, TROWA_SCOPE_HUE_KNOB_MAX, 0.0, 1.0);
		}
		if (hue != waveForms[wIx]->fillHue || firstLoad)
		{
			waveForms[wIx]->fillHue = hue;
			if (hue > 0.99)
			{
				// Inject White
				waveForms[wIx]->fillColor = COLOR_WHITE;
			}
			else
			{
				waveForms[wIx]->fillColor = HueToColor(waveForms[wIx]->fillHue); // Base Color (opacity full)
			}
		} // end if color changed		
		// Opacity:
		if (inputs[multiScope::FILL_OPACITY_INPUT + wIx].active)
		{
			waveForms[wIx]->fillOpacity = clamp(rescale(inputs[multiScope::FILL_OPACITY_INPUT + wIx].value, TROWA_SCOPE_OPACITY_INPUT_MIN, TROWA_SCOPE_OPACITY_INPUT_MAX, TROWA_SCOPE_MIN_OPACITY, TROWA_SCOPE_MAX_OPACITY),
				TROWA_SCOPE_MIN_OPACITY, TROWA_SCOPE_MAX_OPACITY);
		}
		else
		{
			waveForms[wIx]->fillOpacity = params[multiScope::FILL_OPACITY_PARAM + wIx].value;
		}


		// Compute rotation:
		waveForms[wIx]->rotKnobValue = params[multiScope::ROTATION_PARAM+wIx].value;
		if (waveForms[wIx]->rotModeTrigger.process(params[multiScope::ROTATION_MODE_PARAM+wIx].value))
		{
			waveForms[wIx]->rotMode = !waveForms[wIx]->rotMode;
			//debug("Waveform %d: Rotation Mode On Clicked : %d.", wIx, waveForms[wIx]->rotMode);
		}
		lights[multiScope::ROT_LED+wIx].value = waveForms[wIx]->rotMode;		
		float rot = 0;
		float rotRate = 0;
		if (waveForms[wIx]->rotMode)
		{
			// Absolute position:
			rot = rescale(params[multiScope::ROTATION_PARAM+wIx].value + inputs[multiScope::ROTATION_INPUT+wIx].value, 0, 10, 0, NVG_PI);
		}
		else
		{
			// Differential rotation
			rotRate = rescale(params[multiScope::ROTATION_PARAM+wIx].value + inputs[multiScope::ROTATION_INPUT+wIx].value, 0, 10, 0, 0.5);
		}
		waveForms[wIx]->rotAbsValue = rot;
		waveForms[wIx]->rotDiffValue = rotRate;
		
		// Compute time:
		float deltaTime = powf(2.0, params[TIME_PARAM+wIx].value + inputs[TIME_INPUT+wIx].value);
		int frameCount = (int)ceilf(deltaTime * engineGetSampleRate());
		// Add frame to buffer
		if (waveForms[wIx]->bufferIndex < BUFFER_SIZE) {
			if (++(waveForms[wIx]->frameIndex) > frameCount) {
				waveForms[wIx]->frameIndex = 0;
				waveForms[wIx]->bufferX[waveForms[wIx]->bufferIndex] = inputs[X_INPUT+wIx].value;
				waveForms[wIx]->bufferY[waveForms[wIx]->bufferIndex] = inputs[Y_INPUT+wIx].value;
				waveForms[wIx]->bufferPenOn[waveForms[wIx]->bufferIndex] = (!inputs[PEN_ON_INPUT + wIx].active || inputs[PEN_ON_INPUT + wIx].value > 0.1); // Allow some noise?
				waveForms[wIx]->bufferIndex++;
			}
		}
		else {
			if (waveForms[wIx]->lissajous)
			{
				// Reset
				waveForms[wIx]->bufferIndex = 0;
				waveForms[wIx]->frameIndex = 0;
			}
			else
			{
				// Just show stuff (no trigger inputs)
				waveForms[wIx]->frameIndex++;
				float holdTime = 0.1;
				if (waveForms[wIx]->frameIndex >= engineGetSampleRate() * holdTime) {
					waveForms[wIx]->bufferIndex = 0; 
					waveForms[wIx]->frameIndex = 0;
				}
			}
		}
	} // end loop through waveforms
	firstLoad = false;
	return;
} // end step()


//-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-
// drawWaveform()
// @vg : (IN) NVGcontext
// @valX: (IN) Pointer to x values.
// @valY: (IN) Pointer to y values.
// @rotRate: (IN) Rotation rate in radians
// @lineThickness: (IN) Line thickness
// @compositeOp: (IN) Some global effect if any
// @flipX: (IN) Flip along x (at x=0)
// @flipY: (IN) Flip along y
//-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-
void multiScopeDisplay::drawWaveform(NVGcontext *vg, float *valX, float *valY, bool* penOn,
	float rotRate, float lineThickness, NVGcolor lineColor,
	bool doFill, NVGcolor fillColor,
	NVGcompositeOperation compositeOp, bool flipX, bool flipY)
{
	if (!valX)
		return;
	nvgSave(vg);
	Rect b = Rect(Vec(0, 0), box.size);
	nvgScissor(vg, b.pos.x, b.pos.y, b.size.x, b.size.y);
	//nvgTranslate(vg, box.size.x / 2.0, box.size.y / 2.0);
	//nvgRotate(vg, rot += rotRate);
	rot += rotRate;
	if (flipX || flipY)
	{
		// Sets the transform to scale matrix.
		// void nvgTransformScale(float* dst, float sx, float sy);
		nvgScale(vg, ((flipX) ? -1 : 1), (flipY) ? -1 : 1); // flip
	}

	// Draw maximum display left to right
	nvgBeginPath(vg);
	float xOffset = 0;// -box.size.x / 2.0;
	float yOffset = 0;// -box.size.y / 2.0;
	float minX = b.pos.x + xOffset + lineThickness / 2.0;
	float maxX = minX + b.size.x - lineThickness;
	float minY = b.pos.y + yOffset + lineThickness / 2.0;
	float maxY = minY + box.size.y - lineThickness;
	bool doTrim = false;
	switch (compositeOp)
	{
	case NVG_DESTINATION_OVER:
	case NVG_SOURCE_IN:
	case NVG_SOURCE_OUT:
	case NVG_DESTINATION_IN:
	case NVG_DESTINATION_ATOP:
	case NVG_COPY:
		doTrim = true;
		break;
	case NVG_SOURCE_OVER:
	case NVG_ATOP:
	case NVG_DESTINATION_OUT:
	case NVG_LIGHTER:
	case NVG_XOR:
	default:
		break;
	}
	float ox = b.pos.x + b.size.x / 2.0; // Center of box
	float oy = b.pos.y + b.size.y / 2.0;
	float s = sin(rot);
	float c = cos(rot);

	bool lastPointStarted = false; // If the last point was actually plotted
								   //bool lastPointInBounds = false; // If the last point was in bounds
	Vec lastPoint;
	Vec lastPointRaw;
	uint8_t lastLocCodeRaw = POINT_POS_INSIDE;
	bool lastPointExists = false; // If the last point was actually calculated (i.e. false if pen is off)
	uint8_t lastLocCode = POINT_POS_INSIDE;
	for (int i = 0; i < BUFFER_SIZE; i++) {
		if (penOn[i])
		{
			float x, y;
			if (valY) {
				x = valX[i] / 2.0 + 0.5;
				y = valY[i] / 2.0 + 0.5;
			}
			else {
				x = (float)i / (BUFFER_SIZE - 1);
				y = valX[i] / 2.0 + 0.5;
			}


			Vec p;
			//p.x = b.pos.x + xOffset + b.size.x * x;
			//p.y = b.pos.y + yOffset + b.size.y * (1.0 - y);
			p.x = b.size.x * x;
			p.y = b.size.y * (1.0 - y);
			// Rotate ourselves so we can cull easily.
			double dx = p.x - ox;
			double dy = p.y - oy;
			p.x =  ox + dx * c - dy * s;
			p.y =  oy + dx * s + dy * c;

			bool plotPoint = true;
			if (doTrim)
			{
				Vec origPoint = p;
				// Do some cropping/clipping if needed
				uint8_t locCode = GetPointLocationCode(p, minX, maxX, minY, maxY);
				uint8_t origLocCode = locCode;
				//bool inBounds = !locCode;
				bool doSearch = false;
				if (locCode)
				{
					// Outside of bounds
					if (lastPointExists && !LINE_OUT_OF_BOUNDS(locCode, lastLocCodeRaw)) // If there was a point last time and both prev and this one don't make a line totally outside of bounds.
					{
						// Check the last point calculated (it may be out of bounds too)
						doSearch = true;
					}
					else
					{
						// Just save this for next time. Do not plot
						plotPoint = false;
					}
				} // end if this point is out of bounds
				else if (lastPointExists && lastLocCodeRaw)
				{
					// Last point wasn't valid although this one is, so we will have to inject both points.
					doSearch = true;
				}
				if (doSearch)
				{
					Vec p1 = lastPointRaw;
					Vec p2 = p;
					uint8_t outcode0 = lastLocCodeRaw;
					uint8_t outcode1 = locCode;
					while (doSearch)
					{
						if (LINE_IS_IN_BOUNDS(outcode0, outcode1))
						{ // Bitwise OR is 0. Trivially accept and get out of loop
							plotPoint = true;
							doSearch = false;
						}
						else if (LINE_OUT_OF_BOUNDS(outcode0, outcode1))
						{ // Bitwise AND is not 0. (implies both end points are in the same region outside the window). Reject and get out of loop
							doSearch = false;
							plotPoint = false;
						}
						else {
							// failed both tests, so calculate the line segment to clip
							// from an outside point to an intersection with clip edge
							double x, y;

							// At least one endpoint is outside the clip rectangle; pick it.
							uint8_t outcodeOut = outcode0 ? outcode0 : outcode1;

							// Now find the intersection point;
							// use formulas:
							//   slope = (y1 - y0) / (x1 - x0)
							//   x = x0 + (1 / slope) * (ym - y0), where ym is ymin or ymax
							//   y = y0 + slope * (xm - x0), where xm is xmin or xmax
							if (outcodeOut & POINT_POS_TOP) {           // point is above the clip rectangle
								x = p1.x + (p2.x - p1.x) * (maxY - p1.y) / (p2.y - p1.y);
								y = maxY;
							}
							else if (outcodeOut & POINT_POS_BOTTOM) { // point is below the clip rectangle
								x = p1.x + (p2.x - p1.x) * (minY - p1.y) / (p2.y - p1.y);
								y = minY;
							}
							else if (outcodeOut & POINT_POS_RIGHT) {  // point is to the right of clip rectangle
								y = p1.y + (p2.y - p1.y) * (maxX - p1.x) / (p2.x - p1.x);
								x = maxX;
							}
							else if (outcodeOut & POINT_POS_LEFT) {   // point is to the left of clip rectangle
								y = p1.y + (p2.y - p1.y) * (minX - p1.x) / (p2.x - p1.x);
								x = minX;
							}

							// Now we move outside point to intersection point to clip
							// and get ready for next pass.
							if (outcodeOut == outcode0) {
								p1.x = x;
								p1.y = y;
								outcode0 = GetPointLocationCode(p1, minX, maxX, minY, maxY);
							}
							else {
								p2.x = x;
								p2.y = y;
								outcode1 = GetPointLocationCode(p2, minX, maxX, minY, maxY);
							}
						} // end else (check bounds)
					} // end while


					// See if we should plot the last point (now that it's fixed)
					if (lastLocCode && !outcode0)
					{
						// Last point was out of bounds, but is now not out of bounds
						if (!lastPointStarted)
						{
							nvgMoveTo(vg, p1.x, p1.y);
						}
						else
						{
							nvgLineTo(vg, p1.x, p1.y);
						}
						lastPointStarted = true;
					} // end if plot prev point

					locCode = outcode1;
					p = p2;
					if (locCode)
					{
						// Still not in bounds
						plotPoint = false;
					}
				}
				lastLocCode = locCode;
				lastPointRaw = origPoint;
				lastLocCodeRaw = origLocCode;
			} // end if do trimming
			if (plotPoint)
			{
				if (!lastPointStarted)
				{
					nvgMoveTo(vg, p.x, p.y);
				}
				else
				{
					nvgLineTo(vg, p.x, p.y);
				}
				lastPointStarted = true;
			}
			else
			{
				lastPointStarted = false;
			}
			lastPoint = p;
			lastPointExists = true;
		} // end if penOn
		else
		{
			// Pen is off, ignore this point
			lastPointStarted = false;
			lastPointExists = false;
		} // end else (pen off)
	} // end loop through buffer
    
	nvgLineCap(vg, NVG_ROUND);
	nvgMiterLimit(vg, 2.0);
	nvgGlobalCompositeOperation(vg, compositeOp);
	if (doFill)
	{
		nvgFillColor(vg, fillColor);
		nvgFill(vg);
	}
	nvgStrokeColor(vg, lineColor);
	nvgStrokeWidth(vg, lineThickness);
	nvgStroke(vg);
	nvgResetScissor(vg);
	nvgRestore(vg);
	nvgGlobalCompositeOperation(vg, NVG_SOURCE_OVER); // Restore to normal
	return;
} // end drawWaveform()	  

RACK_PLUGIN_MODEL_INIT(trowaSoft, MultiScope) {
   Model *modelMultiScope = Model::create<multiScope, multiScopeWidget>(/*manufacturer*/ TROWA_PLUGIN_NAME, /*slug*/ "multiScope", /*name*/ "multiScope", /*Tags*/ VISUAL_TAG, UTILITY_TAG);
   return modelMultiScope;
}

#endif // end if use new scope
