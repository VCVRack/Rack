#pragma once
#include "widgets.hpp"
#include "blendish.h"


#define CHECKMARK_STRING "✔"
#define CHECKMARK(_cond) ((_cond) ? CHECKMARK_STRING : "")


namespace rack {

////////////////////
// Layouts (layouts.cpp)
////////////////////

/** Positions children in a row/column based on their widths/heights */
struct SequentialLayout : VirtualWidget {
	enum Orientation {
		HORIZONTAL_ORIENTATION,
		VERTICAL_ORIENTATION,
	};
	Orientation orientation = HORIZONTAL_ORIENTATION;
	enum Alignment {
		LEFT_ALIGNMENT,
		CENTER_ALIGNMENT,
		RIGHT_ALIGNMENT,
	};
	Alignment alignment = LEFT_ALIGNMENT;
	/** Space between adjacent elements */
	float spacing = 0.0;
	void step() override;
};

////////////////////
// Blendish UI elements
////////////////////

struct Label : VirtualWidget {
	std::string text;
	float fontSize;
	NVGcolor color;
	enum Alignment {
		LEFT_ALIGNMENT,
		CENTER_ALIGNMENT,
		RIGHT_ALIGNMENT,
	};
	Alignment alignment = LEFT_ALIGNMENT;

	Label();
	void draw(NVGcontext *vg) override;
};

struct List : OpaqueWidget {
	void step() override;
};

/** Deletes itself from parent when clicked */
struct MenuOverlay : OpaqueWidget {
	void step() override;
	void onMouseDown(EventMouseDown &e) override;
	void onHoverKey(EventHoverKey &e) override;
};

struct MenuEntry;

struct Menu : OpaqueWidget {
	Menu *parentMenu = NULL;
	Menu *childMenu = NULL;
	/** The entry which created the child menu */
	MenuEntry *activeEntry = NULL;

	Menu() {
		box.size = Vec(0, 0);
	}
	~Menu();
	/** Deprecated. Just use addChild(child) instead */
	void pushChild(Widget *child) DEPRECATED {
		addChild(child);
	}
	void setChildMenu(Menu *menu);
	void step() override;
	void draw(NVGcontext *vg) override;
	void onScroll(EventScroll &e) override;
};

struct MenuEntry : OpaqueWidget {
	MenuEntry() {
		box.size = Vec(0, BND_WIDGET_HEIGHT);
	}
	template <typename T = MenuEntry>
	static T *create() {
		T *o = Widget::create<T>(Vec());
		return o;
	}
};

struct MenuLabel : MenuEntry {
	std::string text;
	void draw(NVGcontext *vg) override;
	void step() override;

	template <typename T = MenuLabel>
	static T *create(std::string text) {
		T *o = MenuEntry::create<T>();
		o->text = text;
		return o;
	}
};

struct MenuItem : MenuEntry {
	std::string text;
	std::string rightText;
	void draw(NVGcontext *vg) override;
	void step() override;
	virtual Menu *createChildMenu() {return NULL;}
	void onMouseEnter(EventMouseEnter &e) override;
	void onDragDrop(EventDragDrop &e) override;

	template <typename T = MenuItem>
	static T *create(std::string text, std::string rightText = "") {
		T *o = MenuEntry::create<T>();
		o->text = text;
		o->rightText = rightText;
		return o;
	}
};

struct WindowOverlay : OpaqueWidget {
};

struct WindowWidget : OpaqueWidget {
	std::string title;
	void draw(NVGcontext *vg) override;
	void onDragMove(EventDragMove &e) override;
};

struct Button : OpaqueWidget {
	std::string text;
	BNDwidgetState state = BND_DEFAULT;

	Button() {
		box.size.y = BND_WIDGET_HEIGHT;
	}
	void draw(NVGcontext *vg) override;
	void onMouseEnter(EventMouseEnter &e) override;
	void onMouseLeave(EventMouseLeave &e) override;
	void onDragStart(EventDragStart &e) override;
	void onDragEnd(EventDragEnd &e) override;
	void onDragDrop(EventDragDrop &e) override;
};

struct ChoiceButton : Button {
	void draw(NVGcontext *vg) override;
};

struct RadioButton : OpaqueWidget, QuantityWidget {
	BNDwidgetState state = BND_DEFAULT;

	RadioButton() {
		box.size.y = BND_WIDGET_HEIGHT;
	}
	void draw(NVGcontext *vg) override;
	void onMouseEnter(EventMouseEnter &e) override;
	void onMouseLeave(EventMouseLeave &e) override;
	void onDragDrop(EventDragDrop &e) override;
};

struct Slider : OpaqueWidget, QuantityWidget {
	BNDwidgetState state = BND_DEFAULT;

	Slider() {
		box.size.y = BND_WIDGET_HEIGHT;
	}
	void draw(NVGcontext *vg) override;
	void onDragStart(EventDragStart &e) override;
	void onDragMove(EventDragMove &e) override;
	void onDragEnd(EventDragEnd &e) override;
	void onMouseDown(EventMouseDown &e) override;
};

struct ScrollBar;
/** Handles a container with ScrollBar */
struct ScrollWidget : OpaqueWidget {
	Widget *container;
	ScrollBar *horizontalScrollBar;
	ScrollBar *verticalScrollBar;
	Vec offset;

	ScrollWidget();
	void scrollTo(Rect r);
	void draw(NVGcontext *vg) override;
	void step() override;
	void onMouseMove(EventMouseMove &e) override;
	void onScroll(EventScroll &e) override;
	void onHoverKey(EventHoverKey &e) override;
};

struct TextField : OpaqueWidget {
	std::string text;
	std::string placeholder;
	bool multiline = false;
	/** The index of the text cursor */
	int cursor = 0;
	/** The index of the other end of the selection.
	If nothing is selected, this is equal to `cursor`.
	*/
	int selection = 0;

	TextField() {
		box.size.y = BND_WIDGET_HEIGHT;
	}
	void draw(NVGcontext *vg) override;
	void onMouseDown(EventMouseDown &e) override;
	void onMouseMove(EventMouseMove &e) override;
	void onFocus(EventFocus &e) override;
	void onText(EventText &e) override;
	void onKey(EventKey &e) override;
	/** Inserts text at the cursor, replacing the selection if necessary */
	void insertText(std::string text);
	/** Replaces the entire text */
	void setText(std::string text);
	virtual int getTextPosition(Vec mousePos);
	virtual void onTextChange() {}
};

struct PasswordField : TextField {
	void draw(NVGcontext *vg) override;
};

struct ProgressBar : QuantityWidget {
	ProgressBar() {
		box.size.y = BND_WIDGET_HEIGHT;
	}
	void draw(NVGcontext *vg) override;
};

struct Tooltip : VirtualWidget {
	void step() override;
	void draw(NVGcontext *vg) override;
};

struct Scene : OpaqueWidget {
	Widget *overlay = NULL;
	void setOverlay(Widget *w);
	Menu *createMenu();
	void step() override;
};


////////////////////
// globals
////////////////////

extern Scene *gScene;


} // namespace rack
