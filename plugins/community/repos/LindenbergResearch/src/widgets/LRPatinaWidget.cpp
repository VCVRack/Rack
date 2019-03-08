/*                                                                     *\
**       __   ___  ______                                              **
**      / /  / _ \/_  __/                                              **
**     / /__/ , _/ / /    Lindenberg                                   **
**    /____/_/|_| /_/  Research Tec.                                   **
**                                                                     **
**                                                                     **
**	  https://github.com/lindenbergresearch/LRTRack	                   **
**    heapdump@icloud.com                                              **
**		                                                               **
**    Sound Modules for VCV Rack                                       **
**    Copyright 2017/2018 by Patrick Lindenberg / LRT                  **
**                                                                     **
**    For Redistribution and use in source and binary forms,           **
**    with or without modification please see LICENSE.                 **
**                                                                     **
\*                                                                     */
#include "../LRComponents.hpp"

using namespace lrt;
using std::string;


/**
 * Standard constructor with a given filename
 */
LRPatinaWidget::LRPatinaWidget(const string &filename, const Vec &size) {
    svg = new SVGWidget();
#ifdef USE_VST2
    svg->setSVG(SVG::load(assetPlugin("LindenbergResearch"/*plugin*/, filename.c_str())));
#else
    svg->setSVG(SVG::load(assetPlugin(plugin, filename)));
#endif

    addChild(svg);
    box.size = size;
}


/**
 * @brief Randomize patina svg offset and trigger redraw
 */
void LRPatinaWidget::randomize() {
    float maxx = svg->box.size.x - box.size.x;
    float maxy = svg->box.size.y - box.size.y;

    svg->box.pos = Vec(-randomUniform() * maxx, -randomUniform() * maxy);
}


/**
 * @brief Override draw to set global (widget) transparency (strength)
 * @param vg
 */
void LRPatinaWidget::draw(NVGcontext *vg) {
    nvgGlobalAlpha(vg, strength);
    TransparentWidget::draw(vg);
}
