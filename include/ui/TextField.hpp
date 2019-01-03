#pragma once
#include "widgets/OpaqueWidget.hpp"
#include "ui/common.hpp"
#include "event.hpp"
#include "context.hpp"


namespace rack {


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

	TextField();
	void draw(NVGcontext *vg) override;
	void onButton(const event::Button &e) override;
	void onHover(const event::Hover &e) override;
	void onEnter(const event::Enter &e) override;
	void onSelectText(const event::SelectText &e) override;
	void onSelectKey(const event::SelectKey &e) override;

	/** Inserts text at the cursor, replacing the selection if necessary */
	void insertText(std::string text);

	/** Replaces the entire text */
	void setText(std::string text);
	virtual int getTextPosition(math::Vec mousePos);
};


} // namespace rack
