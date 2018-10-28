#pragma once

#include "widgets.hpp"

class ButtonCell
{
public:
    friend class WaveformSelector;
    ButtonCell(const ButtonCell&) = delete;
    ButtonCell& operator = (const ButtonCell&) = delete;

    ButtonCell(float x) : value(x) {}
    void loadSVG(const char* res, const char* resOn);

    const float value;
    rack::Rect box;

    void dump(const char*);
private:
    SVGWidget svg;
    SVGWidget svgOn;
};

inline void ButtonCell::loadSVG(const char* res, const char* resOn)
{
    svg.setSVG(SVG::load (assetPlugin(plugin, res)));
    svgOn.setSVG(SVG::load (assetPlugin(plugin, resOn)));
    this->box.size = svg.box.size;
}

inline void ButtonCell::dump(const char* label)
{
    printf("cell(%.2f) {%s} box size=%f, %f po %f, %f\n",
        value,
        label,
        box.size.x,
        box.size.y,
        box.pos.x,
        box.pos.y); 
}

using CellPtr = std::shared_ptr<ButtonCell>;

struct WaveformSelector  : ParamWidget
{
    WaveformSelector();
    void draw(NVGcontext *vg) override;
    ~WaveformSelector() override;

    std::vector< std::vector< CellPtr>> svgs;
    void addSvg(int row, const char* res, const char* resOn);
    void drawSVG(NVGcontext *vg, SVGWidget&, float x, float y);
    void onMouseDown( EventMouseDown &e ) override;
    CellPtr hitTest(float x, float y);
    //
    float nextValue = 0;
};

 CellPtr WaveformSelector::hitTest(float x, float y)
 {
    const Vec pos(x, y);
    for (auto& r : svgs) {
        for (auto& s : r) {
            if (s->box.contains(pos)) {
                return s;
            }
        }
    }
     return nullptr;
 }

inline void WaveformSelector::addSvg(int row, const char* res, const char* resOn)
{
    if ((int)svgs.size() < row+1) {
        svgs.resize(row+1);
    }
   
    // make a new cell, put the SVGs in it
    CellPtr cell = std::make_shared<ButtonCell>(nextValue++);
    cell->loadSVG(res, resOn);
    svgs[row].push_back(cell);

    // now set the box for cell
    float y = 0;
    if (row > 0) {
        // if we are going in the second row, y = height of first
        assert(!svgs[row-1].empty());
        CellPtr otherCell = svgs[row-1][0];
        y = otherCell->box.pos.y + otherCell->box.size.y;
    }
    cell->box.pos.y = y;

    const int cellsInRow = (int) svgs[row].size();
    if (cellsInRow == 1) {
        cell->box.pos.x = 0;
    } else {
        cell->box.pos.x =
            svgs[row][cellsInRow-2]->box.pos.x + 
            svgs[row][cellsInRow-2]->box.size.x;
    }
}

inline WaveformSelector::WaveformSelector()
{
    addSvg(0, "res/waveforms-6-08.svg","res/waveforms-6-07.svg");   
    addSvg(0, "res/waveforms-6-06.svg","res/waveforms-6-05.svg");   
    addSvg(0, "res/waveforms-6-02.svg","res/waveforms-6-01.svg");   
    addSvg(1, "res/waveforms-6-04.svg","res/waveforms-6-03.svg");   
    addSvg(1, "res/waveforms-6-12.svg","res/waveforms-6-11.svg");   
    addSvg(1, "res/waveforms-6-10.svg","res/waveforms-6-09.svg");   
}

inline WaveformSelector::~WaveformSelector()
{
}

inline void WaveformSelector::drawSVG(NVGcontext *vg, SVGWidget& svg, float x, float y)
{
    nvgSave(vg);
    float transform[6];
    nvgTransformIdentity(transform);
    nvgTransformTranslate(transform, x, y);
    nvgTransform(vg, transform[0], transform[1], transform[2], transform[3], transform[4], transform[5]);
    svg.draw(vg);
    nvgRestore(vg);
}

void inline WaveformSelector::draw(NVGcontext *vg)
{
    for (auto& r : svgs) {
        for (auto& s : r) {
            const bool on = (this->value == s->value);
            drawSVG(vg, on ? s->svgOn : s->svg, s->box.pos.x, s->box.pos.y);
        }
    }
}

inline void WaveformSelector::onMouseDown( EventMouseDown &e )
{
    e.consumed = false;
 
    CellPtr hit = hitTest(e.pos.x, e.pos.y);
    if (hit) {
        e.consumed = true;
        if (hit->value == this->value) {
            printf("value same\n"); fflush(stdout);
            return;
        }
        setValue(hit->value);
    }
}