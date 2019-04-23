#include "../LRComponents.hpp"


namespace lrt {

LRKnob::LRKnob() {
    minAngle = -ANGLE * (float) M_PI;
    maxAngle = ANGLE * (float) M_PI;

    //gestalt = module->gestalt;

    shader = new LRShadow();
    removeChild(shadow); // uninstall default

    font = Font::load(assetGlobal("res/fonts/ShareTechMono-Regular.ttf"));

    indicator = new LRCVIndicator(15.f, ANGLE);
    // addChild(indicator);
}


void LRKnob::setSVG(std::shared_ptr<SVG> svg) {
    SVGKnob::setSVG(svg);

    /** inherit dimensions after loaded svg */
    indicator->box.size = sw->box.size;
    indicator->middle = Vec(box.size.x / 2, box.size.y / 2);
    shader->setBox(box);
}


void LRKnob::draw(NVGcontext *vg) {
    /** shadow */
    shader->draw(vg);

    /** component */
    FramebufferWidget::draw(vg);

    indicator->draw(vg);

    /** debug numerical values */
    if (debug) {
        auto text = stringf("%4.3f", value);
        auto size = box.size.x / 2. > 15 ? 15 : box.size.x / 2.;
        nvgFontSize(vg, size);

        nvgFontFaceId(vg, font->handle);
        nvgTextAlign(vg, NVG_ALIGN_LEFT | NVG_ALIGN_MIDDLE);

        float bounds[4];

        nvgTextBounds(vg, 0, 0, text.c_str(), nullptr, bounds);

        nvgBeginPath(vg);
        nvgFillColor(vg, nvgRGBAf(0., 0.1, 0.2, 0.8));
        nvgRoundedRect(vg, bounds[0] - 4, bounds[1] - 2, (bounds[2] - bounds[0]) + 8, (bounds[3] - bounds[1]) + 4,
                       ((bounds[3] - bounds[1]) + 4) / 2 - 1);
        nvgFill(vg);

        nvgFillColor(vg, nvgRGBAf(1.0f, 1.0f, 1.0f, .99f));
        nvgText(vg, 0, 0, text.c_str(), NULL);
    }
}


void LRKnob::setSnap(float position, float sensitivity) {
    snap = true;
    snapSens = sensitivity;
    snapAt = position;
}


void LRKnob::unsetSnap() {
    snap = false;
}


void LRKnob::onChange(EventChange &e) {
    // if the value still inside snap-tolerance keep the value zero
    if (snap && value > -snapSens + snapAt && value < snapSens + snapAt) value = 0;
    SVGKnob::onChange(e);
}


void LRKnob::onGestaltChange(LREventGestaltChange &e) {
    auto svg = getSVGVariant(*gestalt);

    if (svg != nullptr) {
        setSVG(svg);
    }

    switch (*gestalt) {
        case DARK:
            indicator->lightMode = false;
            break;
        case LIGHT:
            indicator->lightMode = true;
            break;
        case AGED:
            indicator->lightMode = true;
            break;
        default:
            indicator->lightMode = false;
    }


    dirty = true;
    e.consumed = true;
}

}