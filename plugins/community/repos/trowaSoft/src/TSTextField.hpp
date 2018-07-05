#ifndef TSTEXTFIELD_HPP
#define TSTEXTFIELD_HPP

#include "rack.hpp"
#include "widgets.hpp"

using namespace rack;

#include <iostream>
#include <regex>
#include <sstream>
#include <string>


// Integer/Digits: Entire string validation
#define TROWA_REGEX_NUMERIC_STR_ONLY		"^[0-9]*$"
// Integer/Digits: Single char validation
#define TROWA_REGEX_NUMERIC_CHAR_ONLY		"^[0-9]$"
// Integer/Digits: Not an digit
#define TROWA_REGEX_NUMERIC_CHAR_NOT		"[^0-9]"

// Floating Point/Real: Entire string validation -- Format String. The precision should be injected (%d)!!!
//^-?[0-9]+(\\.[0-9]{0,%d})?$
// Now just allow or more decimals
#define TROWA_REGEX_FLOAT_STR_ONLY_FORMAT		"^[+-]?([0-9]*[.])?[0-9]+$"//"^-?[0-9]+(\\.[0-9]*)?$"
// Floating Point/Real: Single char validation
#define TROWA_REGEX_FLOAT_CHAR_ONLY				"^[\\-0-9\\.]$"
// Floating Point/Real: Not a valid char
#define TROWA_REGEX_FLOAT_CHAR_NOT				"[^\\-0-9\\.]"

// IP Address: Entire string validation
#define TROWA_REGEX_IP_ADDRESS			 "^(?:(?:25[0-5]|2[0-4][0-9]|[01]?[0-9][0-9]?)\\.){3}(?:25[0-5]|2[0-4][0-9]|[01]?[0-9][0-9]?)$" 
// IP Address: Single char validation
#define TROWA_REGEX_IP_CHAR				"^([0-9]|\\.)$"
// IP Address: Not a valid character 
#define TROWA_REGEX_IP_CHAR_NOT		"[^0-9\\.]"

//-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-
// isPrintableKey()
// @keyCode : (IN) The key that is pressed.
// @returns: True if the key represents a printable character, false if not.
//-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-
bool isPrintableKey(int keyCode);


//-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-
// TSTextField - overload rack TextField
// trowaSoft text field with some basic validation and character limiting enforced.
// 1. Consume invalid characters and throw away.
// 2. Validate (regex) input (can be checked with IsValid()).
// 3. Limit input length (with maxLength).
/// TODO: Implement scrolling and cropping so that characters can't break out of the field box.
//-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-
/// TODO: Redo this struct completely.
struct TSTextField : TextField {
	// Maximum length allowed.
	uint16_t maxLength = 50;
	// IF text input is enabled.
	bool enabled = true;
	// The id of this widget. For use like tab order or something.
	int id = 0;
	// The call back if any (parameter is id). i.e. For tabbing to another field.
	// So parent widget should do this since it is aware of any other field siblings, 
	// or alternatively parent should point to the next guy's requestFocus()....
	// EDIT: Realized you can't do function pointer to member procedure in C++, so this may be of limited use.
	void (*onTabCallback)(int id) = NULL;

	// The call back if any (parameter is id). i.e. For tabbing to the previous field if any.
	// So parent widget should do this since it is aware of any other field siblings, 
	// or alternatively parent should point to the next guy's requestFocus()....
	// EDIT: Realized you can't do function pointer to member procedure in C++, so this may be of limited use.
	void(*onShiftTabCallback)(int id) = NULL;

	// Previous field to focus if Shift-Tab (in lieu of callback).
	TSTextField* nextField = NULL;
	// Next field to focus if Tab (in lieu of callback).
	TSTextField* prevField = NULL;
	// If the tab to field is hidden, should we show on tab or go to the next one?
	enum TabFieldHiddenAction {
		DoNothing,
		ShowHiddenTabToField,
		MoveToNextVisibleTabField
	};
	TabFieldHiddenAction tabNextHiddenAction = TabFieldHiddenAction::MoveToNextVisibleTabField;

	// Text type for validation of input.
	enum TextType {
		// Any text allowed (i.e. no regex check).
		Any,
		// Digits only (no decimals even).
		DigitsOnly,
		// IP address (digits and periods), IPv4.
		IpAddress,
		// Real number (or integer), specify precision (max # of decimals).
		RealNumberOnly
	};
	// Text type for validation of input.
	TextType allowedTextType = TextType::Any;
	// Regex for a single char.
	std::regex regexChar;
	// Regex for the entire string.
	std::regex regexStr;
	// Regex for invalid characters (to match on invalid and remove).
	std::regex regexInvalidChar;
	// The background color.
	NVGcolor backgroundColor;
	// Font
	std::shared_ptr<Font> font;
	// Text offset
	Vec textOffset;
	// Text color
	NVGcolor color;
	// Font size
	float fontSize;
	// The caret color - Actually the highlight color.
	NVGcolor caretColor;
	// The number of decmals allowed.
	int realNumberPrecision = 2;

	std::string displayStr;

	// If this field can be tabbed to.
	bool canTabToThisEnabled = true;

	int borderWidth = 0;
	NVGcolor borderColor;

	int getTextPosition(Vec mousePos) override;

	TSTextField(TextType textType);
	TSTextField(TextType textType, int maxLength);
	//-----------------------------------------------------------------------------------------------
	// TSTextField()
	// @id : (IN) Hopefully unique id for the form. Should indicate tab order.
	// @textType: (IN) Text type for this field (for validation).
	// @maxLength: (IN) Max length for this field.
	// @onTabHandler: (IN) Callback/Event handler for tab (i.e. focus on the next field).
	// @onShiftTabHandler: (IN) Callback/Event handler for shift-tab (i.e. focus on the previous field).
	//-----------------------------------------------------------------------------------------------
	TSTextField(int id, TextType textType, int maxLength, void (*onTabHandler)(int),  void (*onShiftTabHandler)(int)) : TSTextField(textType, maxLength) {
		this->onTabCallback = onTabHandler;
		this->onShiftTabCallback = onShiftTabHandler;
		return;
	}

	// If the text is valid.
	bool isValid() {
		return (allowedTextType == TextType::Any
			|| std::regex_match(text, regexStr));
	}
	// Set the text type/validation
	void setTextType(TextType validationType)
	{
		this->allowedTextType = validationType;

		//char buffer[50] = { '\0' };

		switch (allowedTextType)
		{
		case TextType::DigitsOnly:
			regexChar = std::regex(TROWA_REGEX_NUMERIC_CHAR_ONLY);
			regexStr = std::regex(TROWA_REGEX_NUMERIC_STR_ONLY);
			regexInvalidChar = std::regex(TROWA_REGEX_NUMERIC_CHAR_NOT);
			break;
		case TextType::IpAddress:
			regexChar = std::regex(TROWA_REGEX_IP_CHAR);
			regexStr = std::regex(TROWA_REGEX_IP_ADDRESS);
			regexInvalidChar = std::regex(TROWA_REGEX_IP_CHAR_NOT);
			break;
		case TextType::RealNumberOnly:
			regexChar = std::regex(TROWA_REGEX_FLOAT_CHAR_ONLY);
			// Add our number of decimals to the regex:
			//sprintf(buffer, TROWA_REGEX_FLOAT_STR_ONLY_FORMAT, this->realNumberPrecision);
			//regexStr = std::regex(buffer);
			regexStr = std::regex(TROWA_REGEX_FLOAT_STR_ONLY_FORMAT);
			regexInvalidChar = std::regex(TROWA_REGEX_FLOAT_CHAR_NOT);
			break;
		case TextType::Any:
		default:
			break;
		}
		return;
	}
	// Set the decimal precision allowed for the text box.
	void setRealPrecision(int precision) {
		this->realNumberPrecision = precision;
		if (allowedTextType == TextType::RealNumberOnly) {
			setTextType(allowedTextType); // Redo our regex.
		}
	}



	// Remove invalid chars.
	std::string cleanseString(std::string newText);
	void draw(NVGcontext *vg) override;
	void insertText(std::string newText);
	void onTextChange() override;
	void onKey(EventKey &e) override;
	// Set the text
	void setText(std::string text);
	// On key
	void onText(EventText &e) override;
	// Request focus on this field from the Rack engine.
	void requestFocus();
	//void onDefocus(EventDefocus &e) override;

	// -- TRY TO NOT RESPOND TO EVENTS IF WE ARE HIDING --

	/** Called when a mouse button is pressed over this widget
	0 for left, 1 for right, 2 for middle.
	Return `this` to accept the event.
	Return NULL to reject the event and pass it to the widget behind this one.
	*/
	void onMouseDown(EventMouseDown &e) override {
		if (visible) {
			TextField::onMouseDown(e);
		}
	};
	void onMouseUp(EventMouseUp &e) override {
		if (visible) {
			TextField::onMouseUp(e);
		}
	};
	/** Called on every frame, even if mouseRel = Vec(0, 0) */
	void onMouseMove(EventMouseMove &e) override {
		if (visible) {
			TextField::onMouseMove(e);
		}
	}
	void onHoverKey(EventHoverKey &e) override {
		if (visible) {
			TextField::onHoverKey(e);
		}
	};
	///** Called when this widget begins responding to `onMouseMove` events */
	//virtual void onMouseEnter(EventMouseEnter &e) {}
	///** Called when another widget begins responding to `onMouseMove` events */
	//virtual void onMouseLeave(EventMouseLeave &e) {}
	//virtual void onFocus(EventFocus &e) {}
	//virtual void onDefocus(EventDefocus &e) {}
	//virtual void onScroll(EventScroll &e);

	/** Called when a widget responds to `onMouseDown` for a left button press */
	void onDragStart(EventDragStart &e) override {
		if (visible) {
			TextField::onDragStart(e);
		}
	}
	/** Called when the left button is released and this widget is being dragged */
	void onDragEnd(EventDragEnd &e) override {
		if (visible) {
			TextField::onDragEnd(e);
		}
	}
	/** Called when a widget responds to `onMouseMove` and is being dragged */
	void onDragMove(EventDragMove &e) override {
		if (visible) {
			TextField::onDragMove(e);
		}
	}
	/** Called when a widget responds to `onMouseUp` for a left button release and a widget is being dragged */
	void onDragEnter(EventDragEnter &e) override {
		if (visible) {
			TextField::onDragEnter(e);
		}
	}
	void onDragLeave(EventDragEnter &e) override {
		if (visible) {
			TextField::onDragLeave(e);
		}
	}
	void onDragDrop(EventDragDrop &e) override {
		if (visible) {
			TextField::onDragDrop(e);
		}
	}
	void onPathDrop(EventPathDrop &e)override {
		if (visible) {
			TextField::onPathDrop(e);
		}
	}

	void onAction(EventAction &e) override {
		if (visible) {
			TextField::onAction(e);
		}
	}
	//void onChange(EventChange &e) override {
	//}
}; // end struct TSTextField




#endif // end if not defined
