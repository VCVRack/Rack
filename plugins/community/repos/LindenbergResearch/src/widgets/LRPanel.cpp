#include "../LindenbergResearch.hpp"
#include <window.hpp>
#include <global_pre.hpp>
#include <global_ui.hpp>
#include "../LRComponents.hpp"
#include "../LRGestalt.hpp"

namespace lrt {

void LRPanel::init() {
    /* set panel svg */
    panelWidget = new SVGWidget();
    auto svg = (gestalt == nullptr) ? getSVGVariant(DARK) : getSVGVariant(*gestalt);  // INIT

    if (svg != nullptr) {
        panelWidget->setSVG(svg);
    }

    box.size = panelWidget->box.size.div(RACK_GRID_SIZE).round().mult(RACK_GRID_SIZE);
    addChild(panelWidget);

    /* setup patina widget */
    patinaWidgetWhite = new LRPatinaWidget("res/panels/WhitePatina.svg", box.size);
    patinaWidgetWhite->randomize();
    patinaWidgetWhite->visible = false;
    addChild(patinaWidgetWhite);

    patinaWidgetClassic = new LRPatinaWidget("res/panels/AlternatePatina.svg", box.size);
    patinaWidgetClassic->randomize();
    patinaWidgetClassic->strength = .4f;
    patinaWidgetClassic->visible = false;
    addChild(patinaWidgetClassic);

    /* setup gradient variants */
    auto gradientDark = new LRGradientWidget(box.size, nvgRGBAf(.6f, .6f, .6f, 0.25f), nvgRGBAf(0.0f, 0.0f, 0.0f, 0.2f), Vec(50, 20));
    gradientDark->visible = false;
    addChild(gradientDark);
    gradients[LRGestalt::DARK] = gradientDark;

    auto gradientLight = new LRGradientWidget(box.size, nvgRGBAf(0.3, 0.3, 0.3f, 0.09f), nvgRGBAf(0.f, 0.f, 0.f, 0.7f), Vec(100, -20));
    gradientLight->visible = false;
    addChild(gradientLight);
    gradients[LRGestalt::LIGHT] = gradientLight;

    auto gradientAged = new LRGradientWidget(box.size, nvgRGBAf(0.4, 0.6, 0.f, 0.1f), nvgRGBAf(0.f, 0.f, 0.f, 0.73f), Vec(100, -20));
    gradientAged->visible = false;
    addChild(gradientAged);
    gradients[LRGestalt::AGED] = gradientAged;

    /* setup panel border */
    auto *pb = new LRPanelBorder();
    pb->box.size = box.size;
    addChild(pb);

    dirty = true;
}


/**
 * @brief Set the gradient for the current variant on or off
 * @param invert Automaticaly invert state
 */
void LRPanel::setGradientVariant(bool invert) {
    *gradient = invert == !*gradient;

    gradients[LRGestalt::DARK]->visible = false;
    gradients[LRGestalt::LIGHT]->visible = false;
    gradients[LRGestalt::AGED]->visible = false;

    gradients[*gestalt]->visible = *gradient;

    dirty = true;
}


/**
 * @brief Setup patina on / off
 * @param enabled
 */
void LRPanel::setPatina(bool enabled) {
    *patina = enabled;

    patinaWidgetClassic->visible = *patina && *gestalt == LRGestalt::DARK;

    // TODO: extra patina for aged mode?
    patinaWidgetWhite->visible = *patina && *gestalt != LRGestalt::DARK;

    dirty = true;
}


void LRPanel::onGestaltChange(LREventGestaltChange &e) {
    auto svg = getSVGVariant(*gestalt);

    if (svg != nullptr) {
        panelWidget->setSVG(svg);
        box.size = panelWidget->box.size.div(RACK_GRID_SIZE).round().mult(RACK_GRID_SIZE);
    }

    setGradientVariant(false);
    setPatina(*patina);

    dirty = true;
    e.consumed = true;
}


void LRPanel::step() {
   if (isNear(rack::global_ui->window.gPixelRatio, 1.0)) {
        // Small details draw poorly at low DPI, so oversample when drawing to the framebuffer
        oversample = 2.0;
    }


    FramebufferWidget::step();
}


/**
 * @brief Constructor
 */
LRPanel::LRPanel() {

}

}
