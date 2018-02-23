#pragma once
#include "widgets.hpp"
#include "../ext/oui-blendish/blendish.h"


namespace rack {


struct Label : Widget {
	std::string text;
	Label() {
		box.size.y = BND_WIDGET_HEIGHT;
	}
	void draw(NVGcontext *vg) override;
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
	// Resizes menu and calls addChild()
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

struct Window : OpaqueWidget {
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

/** Parent must be a ScrollWidget */
struct ScrollBar : OpaqueWidget {
	enum { VERTICAL, HORIZONTAL } orientation;
	BNDwidgetState state = BND_DEFAULT;
	float offset = 0.0;
	float size = 0.0;

	ScrollBar() {
		box.size = Vec(BND_SCROLLBAR_WIDTH, BND_SCROLLBAR_HEIGHT);
	}
	void draw(NVGcontext *vg) override;
	void onDragStart(EventDragStart &e) override;
	void onDragMove(EventDragMove &e) override;
	void onDragEnd(EventDragEnd &e) override;
};

/** Handles a container with ScrollBar */
struct ScrollWidget : OpaqueWidget {
	Widget *container;
	ScrollBar *horizontalScrollBar;
	ScrollBar *verticalScrollBar;
	Vec offset;

	ScrollWidget();
	void draw(NVGcontext *vg) override;
	void step() override;
	void onMouseMove(EventMouseMove &e) override;
	void onScroll(EventScroll &e) override;
};

struct TextField : OpaqueWidget {
	std::string text;
	std::string placeholder;
	bool multiline = false;
	int begin = 0;
	int end = 0;
	int dragPos = 0;

	TextField() {
		box.size.y = BND_WIDGET_HEIGHT;
	}
	void draw(NVGcontext *vg) override;
	void onMouseDown(EventMouseDown &e) override;
	void onMouseMove(EventMouseMove &e) override;
	void onFocus(EventFocus &e) override;
	void onText(EventText &e) override;
	void onKey(EventKey &e) override;
	void insertText(std::string newText);
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

struct Tooltip : Widget {
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
