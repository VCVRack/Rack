#pragma once

#include <functional>

#include "IMWidgets.hpp"

class IPanelHost
{
public:
    virtual void setExpanded(bool) = 0;
    virtual bool isExpanded() = 0;
};

/**
 * Bundles up all the handling of expanding and contracting
 * the panel. Relies on IPanelHost to talk back to the
 * main widget.
 */
class CHBPanelManager
{
public:
    CHBPanelManager(IPanelHost*);
    MenuItem*  createMenuItem();
    void makePanel(ModuleWidget* widget);
    void addMenuItems(Menu*);
    void poll();

private:
    DynamicSVGPanel* panel;
    int expWidth = 60;
    IPanelHost* const panelHost;
    ModuleWidget* widget = nullptr;

    void setPanelSize();
};

CHBPanelManager::CHBPanelManager(IPanelHost* host) : panelHost(host)
{
}

inline  void CHBPanelManager::addMenuItems(Menu* menu)
{
    menu->addChild(createMenuItem());
}

inline void CHBPanelManager::setPanelSize()
{
    widget->box.size = panel->box.size;
    const int expansionWidth = panelHost->isExpanded() ? 0 : -expWidth;
    widget->box.size.x += expansionWidth;
}

inline void CHBPanelManager::poll()
{
    setPanelSize();     // TODO: only do on change? (possible optimization)
}

inline void CHBPanelManager::makePanel(ModuleWidget* wdg)
{
    widget = wdg;
    panel = new DynamicSVGPanel();

    panel->box.size = Vec(16 * RACK_GRID_WIDTH, RACK_GRID_HEIGHT);
    panel->box.size.x += expWidth;

    panel->expWidth = &expWidth;

    // I think this dynamically adds the real size. can get rid of the other stuff.
    panel->addPanel(SVG::load(assetPlugin(plugin, "res/cheby-wide-ghost.svg")));


    widget->box.size = panel->box.size;
    // printf("widget box a = %f exp=%f\n", widget->box.size.x, expWidth);

    // TODO: use setPanelSize
    const int expansionWidth = panelHost->isExpanded() ? 0 : -expWidth;
    widget->box.size.x += expansionWidth;
    widget->addChild(panel);
}

inline MenuItem*  CHBPanelManager::createMenuItem()
{
    auto statusCB = [this]() {
        return panelHost->isExpanded();
    };
    auto actionCB = [this]() {
        bool b = !panelHost->isExpanded();
        panelHost->setExpanded(b);
        setPanelSize();
    };
    return new SQPanelItem(statusCB, actionCB);
}