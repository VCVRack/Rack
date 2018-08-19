#include "../LRComponents.hpp"


namespace lrt {

    LRKnob::LRKnob() {
        minAngle = -ANGLE * (float) M_PI;
        maxAngle = ANGLE * (float) M_PI;

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
            auto text = stringf("%4.2f", value);
            nvgFontSize(vg, 15);
            nvgFontFaceId(vg, font->handle);

            nvgFillColor(vg, nvgRGBAf(1.f, 1.f, 1.0f, 1.0f));
            nvgText(vg, box.size.x - 5, box.size.y + 10, text.c_str(), NULL);
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

}