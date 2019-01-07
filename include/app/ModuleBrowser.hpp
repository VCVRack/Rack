#pragma once
#include "app/common.hpp"
#include "ui/ScrollWidget.hpp"
#include "ui/SequentialLayout.hpp"


namespace rack {


struct ModuleBrowser : OpaqueWidget {
	ScrollWidget *moduleScroll;
	SequentialLayout *moduleLayout;

	ModuleBrowser();
	void step() override;
	void draw(NVGcontext *vg) override;
	void onHoverKey(const event::HoverKey &e) override;
};


json_t *moduleBrowserToJson();
void moduleBrowserFromJson(json_t *rootJ);


} // namespace rack
