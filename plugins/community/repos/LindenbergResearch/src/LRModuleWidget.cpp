#include "LRComponents.hpp"
#include "LRModel.hpp"

using namespace rack;
using namespace lrt;


Menu *LRModuleWidget::createContextMenu() {
    Menu *menu = ModuleWidget::createContextMenu();

    auto count = panel->pool.size() - 1; // NIL does not count!


    if (noVariants) return menu; // if gestalt is disabled do nothing

    auto *spacerLabel = new MenuLabel();
    menu->addChild(spacerLabel);

    auto *sectionLabel = new MenuLabel();
    sectionLabel->text = "Module Gestalt";
    menu->addChild(sectionLabel);

    auto *darkGestaltItem = new GestaltItem(DARK, this);
    darkGestaltItem->text = "Classic Dark";
    menu->addChild(darkGestaltItem);

    if (count > 1) {
        auto *lightGestaltItem = new GestaltItem(LIGHT, this);
        lightGestaltItem->text = "Bright Light";
        menu->addChild(lightGestaltItem);
    } else {
        auto *lightGestaltLabel = new MenuLabel();
        lightGestaltLabel->text = "Bright Light";
        menu->addChild(lightGestaltLabel);
    }

    if (count > 2) {
        auto *agedGestaltItem = new GestaltItem(AGED, this);
        agedGestaltItem->text = "Alternate/Aged";
        menu->addChild(agedGestaltItem);
    } else {
        auto *agedGestaltLabel = new MenuLabel();
        agedGestaltLabel->text = "Alternate/Aged";
        menu->addChild(agedGestaltLabel);
    }

    auto *spacerLabelBottom = new MenuLabel();
    menu->addChild(spacerLabelBottom);

    auto *sectionLabelBottom = new MenuLabel();
    sectionLabelBottom->text = "Module Style";
    menu->addChild(sectionLabelBottom);

    auto *gradientItem = new GradientItem(this->panel);
    gradientItem->text = "Metallic Look";
    menu->addChild(gradientItem);

    auto *patinaItem = new PatinaItem(this->panel);
    patinaItem->text = "Used Look";
    menu->addChild(patinaItem);

    return menu;
}


/**
 * @brief Load UI relevant settings
 * @return
 */
json_t *LRModuleWidget::toJson() {
    auto *rootJ = ModuleWidget::toJson();

    LRGestalt gestaltid = gestalt;

    json_object_set_new(rootJ, JSON_GESTALT_KEY, json_integer(gestaltid));
    json_object_set_new(rootJ, JSON_GRADIENT_KEY, json_boolean(gradient));
    json_object_set_new(rootJ, JSON_PATINA_KEY, json_boolean(patina));

    json_object_set_new(rootJ, JSON_PATINA_A_X, json_real(panel->patinaWidgetWhite->svg->box.pos.x));
    json_object_set_new(rootJ, JSON_PATINA_A_Y, json_real(panel->patinaWidgetWhite->svg->box.pos.y));

    json_object_set_new(rootJ, JSON_PATINA_B_X, json_real(panel->patinaWidgetClassic->svg->box.pos.x));
    json_object_set_new(rootJ, JSON_PATINA_B_Y, json_real(panel->patinaWidgetClassic->svg->box.pos.y));

    return rootJ;
}


/**
 * @brief Save UI relevant settings
 * @param rootJ
 */
void LRModuleWidget::fromJson(json_t *rootJ) {
    ModuleWidget::fromJson(rootJ);

    //debug("[%p] read module json: ", this);
    json_t *gestaltID = json_object_get(rootJ, JSON_GESTALT_KEY);
    json_t *gradientJ = json_object_get(rootJ, JSON_GRADIENT_KEY);
    json_t *patinaJ = json_object_get(rootJ, JSON_PATINA_KEY);

    json_t *patina_a_xJ = json_object_get(rootJ, JSON_PATINA_A_X);
    json_t *patina_a_yJ = json_object_get(rootJ, JSON_PATINA_A_Y);

    json_t *patina_b_xJ = json_object_get(rootJ, JSON_PATINA_B_X);
    json_t *patina_b_yJ = json_object_get(rootJ, JSON_PATINA_B_Y);

    /* load gradient flag */
    gradient = gradientJ ? json_is_true(gradientJ) : true;

    /* load patina flag */
    patina = patinaJ ? json_is_true(patinaJ) : false;

    /* load coordinates of patina layers */
    panel->patinaWidgetWhite->svg->box.pos.x = (float) json_real_value(patina_a_xJ);
    panel->patinaWidgetWhite->svg->box.pos.y = (float) json_real_value(patina_a_yJ);

    panel->patinaWidgetClassic->svg->box.pos.x = (float) json_real_value(patina_b_xJ);
    panel->patinaWidgetClassic->svg->box.pos.y = (float) json_real_value(patina_b_yJ);

    /* load and assign gestalt */
    if (gestaltID) {
        switch (json_integer_value(gestaltID)) {
            case 1:
                gestalt = DARK;
                break;
            case 2:
                gestalt = LIGHT;
                break;
            case 3:
                gestalt = AGED;
                break;
            default:
                gestalt = DARK;
        }
    }

}


/**
 * @brief Randomize parameters
 */
void LRModuleWidget::randomize() {
    ModuleWidget::randomize();
    panel->patinaWidgetClassic->randomize();
    panel->patinaWidgetWhite->randomize();
    panel->dirty = true;
}


/**
 * @brief Detect gestalt change and fire event to all children
 */
void LRModuleWidget::step() {
    Widget::step();

    bool modified = gestalt != prevGestalt;

    if (modified) {
        for (Widget *child : children) {
            auto *gc = dynamic_cast<LRGestaltChangeAction *>(child);

            if (gc != nullptr) {
                /* link variables */
                gc->gestalt = &gestalt;
                gc->patina = &patina;
                gc->gradient = &gradient;

                auto *e = new LREventGestaltChange();
                gc->onGestaltChange(*e);
            }
        }

        prevGestalt = gestalt;
    }
}

