#include "TSParamTextField.hpp"
#include <string>

//-----------------------------------------------------------------------------------------------
// TSParamTextField()
// @textType: (IN) Text type for this field (for validation). Should be a value type.
// @maxLength: (IN) Max length for this field.
// @paramCtl: (IN) The widget for control.
// @formatStr: (IN) The format string.
//-----------------------------------------------------------------------------------------------
TSParamTextField::TSParamTextField(TextType textType, int maxLength, ParamWidget* paramCtl, const char* formatStr) : TSTextField(textType, maxLength)
{
	this->control = paramCtl;
	FramebufferWidget* fbw = dynamic_cast<FramebufferWidget*>(paramCtl);
	if (fbw != NULL)
	{
		isDirty = &(fbw->dirty);
		isBufferedCtrl = true;
	}
	this->formatString = formatStr;
	return;
}

//-----------------------------------------------------------------------------------------------
// saveValue()
// Validate input and save value (valid values only).
//-----------------------------------------------------------------------------------------------
void TSParamTextField::saveValue()
{
	isEditing = 2; // Wait 2 cycles before setting knob -> text again in step()
	char buffer[50] = { 0 };
	if (control != NULL)
	{
		float controlVal = control->value;
		if (isValid()) {
			// Set the value on the control:
			float val = (text.length() > 0) ? std::stof(text.c_str()) : 0.0f;
			if (text2KnobVal != NULL)
				controlVal = text2KnobVal(val);
			else
				controlVal = val;
			if (controlVal < control->minValue)
			{
				val = (knob2TextVal == NULL) ? control->minValue : knob2TextVal(control->minValue);
				controlVal = control->minValue;
			}
			else if (controlVal > control->maxValue)
			{
				val = (knob2TextVal == NULL) ? control->maxValue : knob2TextVal(control->maxValue);
				controlVal = control->maxValue;
			}
			control->setValue(controlVal);
			if (isBufferedCtrl && isDirty != NULL)
			{
				*isDirty = true; // Set dirty flag to redraw
			}
		}
		lastControlVal = controlVal;
		if (knob2TextVal != NULL)
			sprintf(buffer, formatString, knob2TextVal(controlVal));
		else
			sprintf(buffer, formatString, controlVal);
		text = buffer;
	}
	return;
}

//-----------------------------------------------------------------------------------------------
// onAction()
// Save value if valid.
//-----------------------------------------------------------------------------------------------
void TSParamTextField::onAction(EventAction &e)
{
	//debug("onAction() - visible = %d!", visible);
	if (visible)
	{
		saveValue();
		e.consumed = true;
	}
	return;
}
//-----------------------------------------------------------------------------------------------
// onDefocus()
// Validate input, set control value to match, format the text field number.
//-----------------------------------------------------------------------------------------------
void TSParamTextField::onDefocus(EventDefocus &e)
{
	saveValue();
	if (autoHideMode == AutoHideMode::AutoHideOnDefocus) {
		visible = false;
	}
	isEditing = 2; // Wait one cycle before setting knob -> text again in step()
	e.consumed = true;
	return; 
} // end onDefocus()
//-----------------------------------------------------------------------------------------------
// step()
// Set value to match the control.
//-----------------------------------------------------------------------------------------------
void TSParamTextField::step()
{
	if (control != NULL && !isEditing)
	{
		if (control->value != lastControlVal)
		{
			char buffer[50] = { 0 };

			if (knob2TextVal != NULL)
				sprintf(buffer, formatString, knob2TextVal(control->value));
			else
				sprintf(buffer, formatString, control->value);
			text = buffer;

			lastControlVal = control->value;
		}
	}
	else if (isEditing < 3 && isEditing > 0)
		isEditing--;
	return;
}
//-----------------------------------------------------------------------------------------------
// setText()
// @val : (IN) Float value to set the text.
// Uses the format string.
//-----------------------------------------------------------------------------------------------
void TSParamTextField::setText(float val)
{
	char buffer[50] = { 0 };
	float controlVal = val;
	if (control != NULL)
	{
		if (text2KnobVal != NULL)
			controlVal = text2KnobVal(val);
		else
			controlVal = val;
		if (controlVal < control->minValue)
			val = (knob2TextVal == NULL) ? control->minValue : knob2TextVal(control->minValue);
		else if (controlVal > control->maxValue)
			val = (knob2TextVal == NULL) ? control->maxValue : knob2TextVal(control->maxValue);
	}
	// Format the text
	sprintf(buffer, formatString, val);
	text = buffer;
	return;
}
