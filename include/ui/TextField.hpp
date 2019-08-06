#pragma once
#include <widget/OpaqueWidget.hpp>
#include <ui/common.hpp>
#include <app.hpp>


namespace rack {
namespace ui {


struct TextField : widget::OpaqueWidget {
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
	void draw(const DrawArgs& args) override;
	void onDragHover(const event::DragHover& e) override;
	void onButton(const event::Button& e) override;
	void onSelectText(const event::SelectText& e) override;
	void onSelectKey(const event::SelectKey& e) override;

	/** Inserts text at the cursor, replacing the selection if necessary */
	void insertText(std::string text);

	/** Replaces the entire text */
	void setText(std::string text);
	void selectAll();
	virtual int getTextPosition(math::Vec mousePos);
};


} // namespace ui
} // namespace rack
