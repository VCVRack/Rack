#pragma once

#include <map>
#include "rack.hpp"
#include "asset.hpp"
#include "widgets.hpp"
#include "LRGestalt.hpp"

#define LCD_FONT_DIG7 "res/digital-7.ttf"
#define LCD_FONTSIZE 11
#define LCD_LETTER_SPACING 0
#define LCD_MARGIN_VERTICAL 1.73
#define LCD_MARGIN_HORIZONTAL 1.07

#define LCD_DEFAULT_COLOR_DARK nvgRGBAf(0.23, 0.6, 0.82, 1.0)
#define LCD_DEFAULT_COLOR_LIGHT nvgRGBAf(0.23, 0.7, 1.0, 1.0)
#define LCD_DEFAULT_COLOR_AGED nvgRGBAf(0.63, 0.1, 0.0, 1.0)

#define LED_DEFAULT_COLOR_DARK nvgRGBAf(0.23, 0.5, 1.0, 1.0)
#define LED_DEFAULT_COLOR_LIGHT nvgRGBAf(1.0, 0.32, 0.12, 1.0)
#define LED_DEFAULT_COLOR_AGED nvgRGBAf(1.0, 1.0, 0.12, 1.0)


/* show values of all knobs */
#define DEBUG_VALUES false

using namespace rack;
using std::vector;
using std::shared_ptr;
using std::string;
using std::map;


#ifdef USE_VST2
#define plugin "LindenbergResearch"
#else
extern Plugin *plugin;
#endif // USE_VST2

namespace lrt {

/* Type definitions for common used data structures */
typedef std::shared_ptr<rack::Font> TrueType;
typedef std::vector<std::string> StringVector;


/**
 * @brief Emulation of a LCD monochrome display
 */
struct LRLCDWidget : FramebufferWidget, LRGestaltVariant, LRGestaltChangeAction {

    enum LCDType {
        NUMERIC,
        TEXT,
        LIST
    };

    TransformWidget *tw;
    SVGWidget *sw;

    TrueType ttfLCDDIG7;


    LCDType type;
    NVGcolor fg;

    bool active = true;
    float value = 0.0;
    unsigned char length = 0;
    string format, text;
    vector<string> items;

    float fontsize;

    string s1;
    string s2;

    /**
     * @brief Constructor
     */
    LRLCDWidget(unsigned char length, string format, LCDType type, float fontsize);

    void step() override;

    /**
     * @brief Draw LCD display
     * @param vg
     */
    void draw(NVGcontext *vg) override;


    inline void addItem(string name) {
        items.push_back(name);
    }


    void doResize(Vec v);

    void onGestaltChange(LREventGestaltChange &e) override;

    virtual void onMouseDown(EventMouseDown &e) override;

};


/**
 * @brief Indicator for control voltages on knobs
 */
struct LRCVIndicator : TransparentWidget {
    static constexpr float OVERFLOW_THRESHOLD = 0.01f;

    /** flag to control drawing */
    bool active = false;
    bool lightMode = false;

    /** color of indicator */
    NVGcolor normalColor = nvgRGBA(0x00, 0x00, 0x00, 0xBB);
    NVGcolor overflowColor = nvgRGBA(0xBB, 0x00, 0x00, 0xBB);

    NVGcolor normalColorLight = nvgRGBA(0xDD, 0xDD, 0xDD, 0xBB);
    NVGcolor overflowColorLight = nvgRGBA(0xDD, 0x00, 0x00, 0xBB);

    /** radius from middle */
    float distance;

    /** normalized control voltage. must between [0..1] */
    float cv = 0.f;

    /** indicator distances */
    float d1 = 4.f;
    float d2 = 0.1f;

    /** draw angle */
    float angle;
    float angle2;

    /** middle of parent */
    Vec middle;


    /**
     * @brief Init indicator
     * @param distance Radius viewed from the middle
     * @param angle Angle of active knob area
     */
    LRCVIndicator(float distance, float angle);


    /**
     * @brief Manipulate the indicator symbol
     * @param d1 Height of the triangle
     * @param d2 Half of the base width
     */
    void setDistances(float d1, float d2);

    /**
     * @brief Draw routine for cv indicator
     * @param vg
     */
    void draw(NVGcontext *vg) override;
};


/**
 * @brief Standard LR Shadow
 */
struct LRShadow : TransparentWidget {
private:
    Rect box;
    float size = 0.65;
    float strength = 1.f;

    /** shadow shift */
    Vec shadowPos = Vec(3, 5);
public:


    LRShadow();


    /**
     * @brief Set the new offset of the shadow gradient
     * @param x
     * @param y
     */
    void setShadowPosition(float x, float y) {
        shadowPos = Vec(x, y);
    }


    void setBox(const Rect &box);
    void setSize(float size);
    void setStrength(float strength);

    /**
    * @brief Draw shadow for circular knobs
    * @param vg NVGcontext
    * @param strength Alpha value of outside gradient
    * @param size Outer size
    * @param shift XY Offset shift from middle
    */
    void drawShadow(NVGcontext *vg, float strength, float size);

    void draw(NVGcontext *vg) override;
};


/**
 * @brief The base of all knobs used in LR panels, includes a indicator
 */
struct LRKnob : SVGKnob, LRGestaltVariant, LRGestaltChangeAction {
private:
    static constexpr float ANGLE = 0.83f;

    /** setup indicator with default values */
    LRCVIndicator *indicator;

    bool debug = DEBUG_VALUES;
    TrueType font;

    /** snap mode */
    bool snap = false;
    /** position to snap */
    float snapAt = 0.0f;
    /** snap sensitivity */
    float snapSens = 0.1;

protected:
    /** shader */
    LRShadow *shader;

public:

    LRKnob();


    /**
     * @brief Set the value of the indicator
     * @param value
     */
    void setIndicatorValue(float value) {
        indicator->cv = value;
        dirty = true;
    }


    /**
     * @brief Switch indicator on/off
     * @param active
     */
    void setIndicatorActive(bool active) {
        indicator->active = active;
        dirty = true;
    }


    /**
     * @brief Get indicator state
     * @return
     */
    bool isIndicatorActive() {
        return indicator->active;
    }


    /**
     * @brief Setup distance of indicator from middle
     * @param distance
     */
    void setIndicatorDistance(float distance) {
        indicator->distance = distance;
        dirty = true;
    }


    /**
     * @brief Setup distance of indicator from middle
     * @param distance
     */
    void setIndicatorShape(float d1, float d2) {
        indicator->setDistances(d1, d2);
        dirty = true;
    }


    /**
     * @brief Set new colors for the indicator (default red overflow)
     * @param normal Normal indicator color
     * @param overflow Optional overflow color
     */
    void setIndicatorColors(NVGcolor normal, NVGcolor overflow = nvgRGBA(0xBB, 0x00, 0x00, 0xBB)) {
        indicator->normalColor = normal;
        indicator->overflowColor = overflow;
    }


    /**
     * @brief Hook into setSVG() method to setup box dimensions correct for indicator
     * @param svg
     */
    void setSVG(shared_ptr<SVG> svg);


    /**
     * @brief Creates a new instance of a LRKnob child
     * @tparam TParamWidget Subclass of LRKnob
     * @param pos Position
     * @param module Module pointer
     * @param paramId Parameter ID
     * @param minValue Min
     * @param maxValue Max
     * @param defaultValue Default
     * @return Pointer to new subclass of LRKnob
     */
    template<class TParamWidget>
    static TParamWidget *create(Vec pos, Module *module, int paramId, float minValue, float maxValue, float
    defaultValue) {
        auto *param = new TParamWidget();
        param->box.pos = pos;
        param->module = module;
        param->paramId = paramId;
        param->setLimits(minValue, maxValue);
        param->setDefaultValue(defaultValue);
        return param;
    }


    /**
     * @brief Draw knob
     * @param vg
     */
    void draw(NVGcontext *vg) override;

    /**
     * @brief Setup knob snapping
     * @param position
     * @param sensitivity
     */
    void setSnap(float position, float sensitivity);

    /**
     * @brief Remove knob snaping
     */
    void unsetSnap();

    /**
     * @brief Snapping mode for knobs
     * @param e
     */
    void onChange(EventChange &e) override;


    void onGestaltChange(LREventGestaltChange &e) override;
};


/**
 * @brief Quantize position to odd numbers to simulate a toggle switch
 */
struct LRToggleKnob : LRKnob {
    LRToggleKnob(float length = 0.5f) {
        //TODO: parametrize start and end angle
        minAngle = -0.666666f * (float) M_PI;
        maxAngle = length * (float) M_PI;

        setSVG(SVG::load(assetPlugin(plugin, "res/knobs/ToggleKnob.svg")));

        addSVGVariant(LRGestalt::DARK, SVG::load(assetPlugin(plugin, "res/knobs/ToggleKnob.svg")));
        addSVGVariant(LRGestalt::LIGHT, SVG::load(assetPlugin(plugin, "res/knobs/AlternateToggleKnobLight.svg")));
        addSVGVariant(LRGestalt::AGED, SVG::load(assetPlugin(plugin, "res/knobs/AlternateToggleKnobLight.svg")));


        speed = 2.f;
    }


    void onChange(EventChange &e) override {
        value = round(value);
        SVGKnob::onChange(e);
    }


    void onGestaltChange(LREventGestaltChange &e) override {
        LRKnob::onGestaltChange(e);

        switch (*gestalt) {
            case LRGestalt::DARK:
                shader->setShadowPosition(3, 4);
                shader->setStrength(1.2f);
                shader->setSize(0.7f);
                break;
            case LRGestalt::LIGHT:
                shader->setShadowPosition(2, 3);
                shader->setStrength(0.5f);
                shader->setSize(0.6f);
                break;
            case LRGestalt::AGED:
                shader->setShadowPosition(2, 3);
                shader->setStrength(0.5f);
                shader->setSize(0.6f);
                break;
            default:
                break;
        }
    }
};


/**
 * @brief Quantize position to odd numbers to simulate a toggle switch
 */
struct LRMiddleIncremental : LRKnob {
    LRMiddleIncremental(float length = 0.5f) {
        minAngle = -length * (float) M_PI;
        maxAngle = length * (float) M_PI;

        setSVG(SVG::load(assetPlugin(plugin, "res/knobs/AlternateMiddleKnob.svg")));
        shader->setShadowPosition(3, 4);

        shader->setStrength(1.2f);
        shader->setSize(0.7f);

        speed = 3.f;
    }


    void onChange(EventChange &e) override {

        value = lround(value);

        //value = round(value);
        SVGKnob::onChange(e);
    }
};


/**
 * @brief LR Big Knob
 */
struct LRBigKnob : LRKnob {
    LRBigKnob() {
        setSVG(SVG::load(assetPlugin(plugin, "res/knobs/BigKnob.svg")));

        addSVGVariant(LRGestalt::DARK, SVG::load(assetPlugin(plugin, "res/knobs/BigKnob.svg")));
        addSVGVariant(LRGestalt::LIGHT, SVG::load(assetPlugin(plugin, "res/knobs/AlternateBigLight.svg")));
        addSVGVariant(LRGestalt::AGED, SVG::load(assetPlugin(plugin, "res/knobs/AlternateBigLight.svg")));
    }


    void onGestaltChange(LREventGestaltChange &e) override {
        LRKnob::onGestaltChange(e);

        switch (*gestalt) {
            case LRGestalt::DARK:
                setIndicatorDistance(15);
                setIndicatorShape(4.8, 0.12);
                shader->setShadowPosition(4, 5);
                shader->setStrength(0.8f);
                shader->setSize(.65f);
                break;
            case LRGestalt::LIGHT:
                setIndicatorDistance(17);
                setIndicatorShape(4.1, 0.08);
                shader->setShadowPosition(4, 5);
                shader->setStrength(0.5f);
                shader->setSize(0.6f);
                break;
            case LRGestalt::AGED:
                setIndicatorDistance(17);
                setIndicatorShape(4.1, 0.08);
                shader->setShadowPosition(4, 5);
                shader->setStrength(0.5f);
                shader->setSize(0.6f);
                break;
            default:
                break;
        }
    }
};


/**
 * @brief LR Middle Knob
 */
struct LRMiddleKnob : LRKnob {
    LRMiddleKnob() {
        setSVG(SVG::load(assetPlugin(plugin, "res/knobs/MiddleKnob.svg")));

        addSVGVariant(LRGestalt::DARK, SVG::load(assetPlugin(plugin, "res/knobs/MiddleKnob.svg")));
        addSVGVariant(LRGestalt::LIGHT, SVG::load(assetPlugin(plugin, "res/knobs/AlternateMiddleLight.svg")));
        addSVGVariant(LRGestalt::AGED, SVG::load(assetPlugin(plugin, "res/knobs/AlternateMiddleLight.svg")));
    }


    void onGestaltChange(LREventGestaltChange &e) override {
        LRKnob::onGestaltChange(e);

        switch (*gestalt) {
            case LRGestalt::DARK:
                setIndicatorDistance(13);
                setIndicatorShape(5, 0.13);
                shader->setShadowPosition(2, 3);
                shader->setStrength(0.8f);
                shader->setSize(.65f);
                break;
            case LRGestalt::LIGHT:
                setIndicatorDistance(11);
                setIndicatorShape(4.3, 0.11);
                shader->setShadowPosition(2, 3);
                shader->setStrength(0.5f);
                shader->setSize(0.6f);
                break;
            case LRGestalt::AGED:
                setIndicatorDistance(11);
                setIndicatorShape(4.3, 0.11);
                shader->setShadowPosition(2, 3);
                shader->setStrength(0.5f);
                shader->setSize(0.6f);
                break;
            default:
                break;
        }
    }
};


/**
 * @brief LR Small Knob
 */
struct LRSmallKnob : LRKnob {
    LRSmallKnob() {
        setSVG(SVG::load(assetPlugin(plugin, "res/knobs/SmallKnob.svg")));

        addSVGVariant(LRGestalt::DARK, SVG::load(assetPlugin(plugin, "res/knobs/SmallKnob.svg")));
        addSVGVariant(LRGestalt::LIGHT, SVG::load(assetPlugin(plugin, "res/knobs/AlternateSmallLight.svg")));
        addSVGVariant(LRGestalt::AGED, SVG::load(assetPlugin(plugin, "res/knobs/AlternateSmallLight.svg")));

        setSnap(0.0f, 0.02f);
        speed = 0.9f;
    }


    void onGestaltChange(LREventGestaltChange &e) override {
        LRKnob::onGestaltChange(e);

        switch (*gestalt) {
            case LRGestalt::DARK:
                setIndicatorDistance(13);
                setIndicatorShape(5, 0.13);
                shader->setShadowPosition(3, 3);
                shader->setStrength(1.f);
                shader->setSize(.65f);
                break;
            case LRGestalt::LIGHT:
                shader->setShadowPosition(3, 3);
                shader->setShadowPosition(2, 3);
                shader->setStrength(0.5f);
                shader->setSize(0.7f);
                break;
            case LRGestalt::AGED:
                shader->setShadowPosition(3, 3);
                shader->setShadowPosition(2, 3);
                shader->setStrength(0.5f);
                shader->setSize(0.7f);
                break;
            default:
                break;
        }
    }
};


/**
 * @brief LR Small Knob
 */
struct LRSmallToggleKnob : LRKnob {
    LRSmallToggleKnob(float length = 0.73) {
        //TODO: parametrize start and end angle
        minAngle = -length * (float) M_PI;
        maxAngle = length * (float) M_PI;

        setSVG(SVG::load(assetPlugin(plugin, "res/knobs/AlternateSmallToggleLight.svg")));

        addSVGVariant(LRGestalt::DARK, SVG::load(assetPlugin(plugin, "res/knobs/AlternateSmallToggleLight.svg")));
        addSVGVariant(LRGestalt::LIGHT, SVG::load(assetPlugin(plugin, "res/knobs/AlternateSmallToggleLight.svg")));
        addSVGVariant(LRGestalt::AGED, SVG::load(assetPlugin(plugin, "res/knobs/AlternateSmallToggleLight.svg")));

        speed = 3.0;
    }


    void onChange(EventChange &e) override {
        value = round(value);
        SVGKnob::onChange(e);
    }


    void onGestaltChange(LREventGestaltChange &e) override {
        LRKnob::onGestaltChange(e);

        switch (*gestalt) {
            case LRGestalt::DARK:
                setIndicatorDistance(13);
                setIndicatorShape(5, 0.13);
                shader->setShadowPosition(3, 3);
                shader->setStrength(1.f);
                shader->setSize(.65f);
                break;
            case LRGestalt::LIGHT:
                shader->setShadowPosition(3, 3);
                shader->setShadowPosition(2, 3);
                shader->setStrength(0.3f);
                shader->setSize(0.7f);
                break;
            case LRGestalt::AGED:
                shader->setShadowPosition(3, 3);
                shader->setShadowPosition(2, 3);
                shader->setStrength(0.5f);
                shader->setSize(0.7f);
                break;
            default:
                break;
        }
    }
};


/**
 * @brief LR Alternate Small Knob
 */
struct LRAlternateSmallKnob : LRKnob {
    LRAlternateSmallKnob() {
        setSVG(SVG::load(assetPlugin(plugin, "res/knobs/AlternateSmallKnob.svg")));
        shader->setShadowPosition(3, 3);
        setSnap(0.0f, 0.02f);

        speed = 0.9f;
    }
};


/**
 * @brief LR Middle Knob
 */
struct LRAlternateMiddleKnob : LRKnob {
    LRAlternateMiddleKnob() {
        setSVG(SVG::load(assetPlugin(plugin, "res/knobs/AlternateMiddleKnob.svg")));
        setIndicatorDistance(11);
        setIndicatorShape(5.0, 0.14);
        shader->setShadowPosition(4, 5);

        setSnap(0.0f, 0.12f);
    }
};


/**
 * @brief LR Big Knob
 */
struct LRAlternateBigKnob : LRKnob {
    LRAlternateBigKnob() {
        setSVG(SVG::load(assetPlugin(plugin, "res/knobs/AlternateBigKnob.svg")));
        setIndicatorDistance(15);
        setIndicatorShape(4.8, 0.12);
        shader->setShadowPosition(5, 6);
    }
};


/**
 * @brief LR Big Knob
 */
struct LRAlternateBigLight : LRKnob {
    LRAlternateBigLight() {
        setSVG(SVG::load(assetPlugin(plugin, "res/knobs/AlternateBigLight.svg")));
        setIndicatorDistance(17);
        setIndicatorShape(4.1, 0.08);


        shader->setShadowPosition(4, 5);

        shader->setStrength(0.5f);
        shader->setSize(0.6f);

    }
};


/**
 * @brief LR Big Knob
 */
struct LRAlternateMiddleLight : LRKnob {
    LRAlternateMiddleLight() {
        setSVG(SVG::load(assetPlugin(plugin, "res/knobs/AlternateMiddleLight.svg")));
        setIndicatorDistance(11);
        setIndicatorShape(4.3, 0.11);


        shader->setShadowPosition(2, 3);

        shader->setStrength(0.5f);
        shader->setSize(0.6f);

    }
};


/**
 * @brief LR Small Knob
 */
struct LRAlternateSmallLight : LRKnob {
    LRAlternateSmallLight() {
        setSVG(SVG::load(assetPlugin(plugin, "res/knobs/AlternateSmallLight.svg")));
        shader->setShadowPosition(3, 3);
        setSnap(0.0f, 0.02f);

        shader->setShadowPosition(2, 3);

        shader->setStrength(0.5f);
        shader->setSize(0.7f);

        speed = 0.9f;
    }
};


/**
 * @brief Alternative IO Port
 */
struct LRIOPort : SVGPort {
private:
    LRShadow *shader;

public:
    LRIOPort() {
        background->svg = SVG::load(assetPlugin(plugin, "res/elements/IOPortB.svg"));
        background->wrap();
        box.size = background->box.size;

        shader = new LRShadow();
        //  addChild(shadow);

        /** inherit dimensions */
        shader->setBox(box);
        shader->setSize(0.50);
        shader->setShadowPosition(3, 2);
    }


    /**
     * @brief Hook into draw method
     * @param vg
     */
    void draw(NVGcontext *vg) override {
        shader->draw(vg);
        SVGPort::draw(vg);
    }
};


/**
 * @brief Alternative IO Port
 */
struct LRIOPortBLight : SVGPort {
private:
    LRShadow *shader;

public:
    LRIOPortBLight() {
        background->svg = SVG::load(assetPlugin(plugin, "res/elements/IOPortBLight.svg"));
        background->wrap();
        box.size = background->box.size;

        shader = new LRShadow();
        //  addChild(shadow);

        /** inherit dimensions */
        shader->setBox(box);
        shader->setSize(0.55);
        shader->setStrength(0.3);
        shader->setShadowPosition(1, 2);
    }


    /**
     * @brief Hook into draw method
     * @param vg
     */
    void draw(NVGcontext *vg) override {
        shader->draw(vg);
        SVGPort::draw(vg);
    }
};


/**
 * @brief Alternative IO Port
 */
struct LRIOPortC : SVGPort {
private:
    LRShadow *shader;

public:
    LRIOPortC() {
        background->svg = SVG::load(assetPlugin(plugin, "res/elements/IOPortC.svg"));
        background->wrap();
        box.size = background->box.size;

        shader = new LRShadow();
        // addChild(shader);

        /** inherit dimensions */
        shader->setBox(box);
        shader->setSize(0.50);
        shader->setStrength(0.1);
        shader->setShadowPosition(3, 4);
    }


    /**
     * @brief Hook into draw method
     * @param vg
     */
    void draw(NVGcontext *vg) override {
        shader->draw(vg);
        SVGPort::draw(vg);
    }
};


/**
 * @brief Alternative IO Port
 */
struct LRIOPortD : SVGPort, LRGestaltVariant, LRGestaltChangeAction { //TODO: rename after migration
private:
    LRShadow *shader;

public:
    LRIOPortD() {
        shader = new LRShadow();
    }


    void onGestaltChange(LREventGestaltChange &e) override {
        switch (*gestalt) {
            case LRGestalt::DARK:
                background->svg = getSVGVariant(DARK);
                background->wrap();
                box.size = background->box.size;

                shader->setBox(box);
                shader->setSize(0.57);
                shader->setStrength(0.3);
                shader->setShadowPosition(2, 3);
                break;
            case LRGestalt::LIGHT:
                background->svg = getSVGVariant(LIGHT);
                background->wrap();
                box.size = background->box.size;

                shader->setBox(box);
                shader->setSize(0.52);
                shader->setStrength(0.5);
                shader->setShadowPosition(1, 2);
                break;
            case LRGestalt::AGED:
                background->svg = getSVGVariant(AGED);
                background->wrap();
                box.size = background->box.size;

                shader->setBox(box);
                shader->setSize(0.52);
                shader->setStrength(0.5);
                shader->setShadowPosition(1, 2);
                break;
            default:
                break;
        }

        dirty = true;
    }


    /**
     * @brief Hook into draw method
     * @param vg
     */
    void draw(NVGcontext *vg) override {
        shader->draw(vg);
        SVGPort::draw(vg);
    }
};


/**
 * @brief Audio variant of IO port
 */
struct LRIOPortAudio : LRIOPortD {
    LRIOPortAudio() : LRIOPortD() {
        addSVGVariant(DARK, SVG::load(assetPlugin(plugin, "res/elements/IOPortB.svg")));
        addSVGVariant(LIGHT, SVG::load(assetPlugin(plugin, "res/elements/IOPortBLight.svg")));
        addSVGVariant(AGED, SVG::load(assetPlugin(plugin, "res/elements/IOPortBLight.svg")));
    }
};


/**
 * @brief CV variant of IO port
 */
struct LRIOPortCV : LRIOPortD {
    LRIOPortCV() : LRIOPortD() {
        addSVGVariant(DARK, SVG::load(assetPlugin(plugin, "res/elements/IOPortC.svg")));
        addSVGVariant(LIGHT, SVG::load(assetPlugin(plugin, "res/elements/IOPortCLight.svg")));
        addSVGVariant(AGED, SVG::load(assetPlugin(plugin, "res/elements/IOPortCLight.svg")));
    }
};


/**
 * @brief Alternative IO Port
 */
struct LRIOPortCLight : SVGPort {
private:
    LRShadow *shader;

public:
    LRIOPortCLight() {
        background->svg = SVG::load(assetPlugin(plugin, "res/elements/IOPortCLight.svg"));
        background->wrap();
        box.size = background->box.size;

        shader = new LRShadow();
        // addChild(shader);

        /** inherit dimensions */
        shader->setBox(box);
        shader->setSize(0.55);
        shader->setStrength(0.3);
        shader->setShadowPosition(1, 2);
    }


    /**
     * @brief Hook into draw method
     * @param vg
     */
    void draw(NVGcontext *vg) override {
        shader->draw(vg);
        SVGPort::draw(vg);
    }
};


/**
 * @brief Alternative screw head A
 */
struct ScrewDarkA : SVGScrew {
    ScrewDarkA() {
        sw->svg = SVG::load(assetPlugin(plugin, "res/elements/ScrewDark.svg"));
        sw->wrap();
        box.size = sw->box.size;
    }
};


/**
 * @brief Alternative screw head A
 */
struct ScrewLight : SVGScrew, LRGestaltVariant, LRGestaltChangeAction {
    ScrewLight() {
        sw->svg = SVG::load(assetPlugin(plugin, "res/elements/ScrewLight.svg"));
        sw->wrap();
        box.size = sw->box.size;


        addSVGVariant(LRGestalt::DARK, SVG::load(assetPlugin(plugin, "res/elements/ScrewDarkC.svg")));
        addSVGVariant(LRGestalt::LIGHT, SVG::load(assetPlugin(plugin, "res/elements/ScrewDarkLightC.svg")));
        addSVGVariant(LRGestalt::AGED, SVG::load(assetPlugin(plugin, "res/elements/ScrewDarkLightC.svg")));
    }


    void onGestaltChange(LREventGestaltChange &e) override {
        LRGestaltChangeAction::onGestaltChange(e);

        sw->svg = getSVGVariant(*gestalt);
        dirty = true;
    }

};


/**
 * @brief Alternative screw head A
 */
struct AlternateScrewLight : SVGScrew {
    AlternateScrewLight() {
        sw->svg = SVG::load(assetPlugin(plugin, "res/elements/AlternateScrewLight.svg"));
        sw->wrap();
        box.size = sw->box.size;
    }
};


/**
 * @brief Alternative screw head A
 */
struct ScrewDarkB : SVGScrew {
    ScrewDarkB() {
        sw->svg = SVG::load(assetPlugin(plugin, "res/elements/ScrewDarkB.svg"));
        sw->wrap();
        box.size = sw->box.size;
    }
};


/**
 * @brief Custom switch based on original Rack files
 */
struct LRSwitch : SVGSwitch, ToggleSwitch {
    LRSwitch() {
        addFrame(SVG::load(assetPlugin(plugin, "res/elements/Switch0.svg")));
        addFrame(SVG::load(assetPlugin(plugin, "res/elements/Switch1.svg")));
    }
};


/**
 * @brief Standard LED
 */
struct LRLight : ModuleLightWidget, LRGestaltChangeAction {
    float glowIntensity = 0.25;

    LRLight();

    void draw(NVGcontext *vg) override;

    void setColor(NVGcolor color);

    void onGestaltChange(LREventGestaltChange &e) override;
};


/**
 * @brief Standard linear gradient widget
 */
struct LRGradientWidget : TransparentWidget {
private:
    /* gradient vectors */
    Vec v1, v2;

    /* standard margin */
    float margin = 10.f;

    /* gradient colors */
    NVGcolor innerColor, outerColor;

public:

    LRGradientWidget(const Vec &size) {
        box.pos = Vec(0, 0);
        box.size = size;
    }


    /**
     * @brief Create a new gradient widget
     * @param size box size (derived from parent widget)
     * @param innerColor The inner color (v1)
     * @param outerColor The outher color (v2)
     * @param offset The offset of the left-top corner (optional)
     */
    LRGradientWidget(const Vec &size, const NVGcolor &innerColor, const NVGcolor &outerColor, const Vec &offset = Vec(30, -50)) :
            innerColor(innerColor), outerColor(outerColor) {
        box.size = size;

        box.pos = Vec(0, 0);

        /* initialise with standard dimensions */
        v1 = offset;
        v2 = Vec(v1.x, size.y);
    }


    void setGradientOffset(const Vec &v1, const Vec &v2) {
        LRGradientWidget::v1 = v1;
        LRGradientWidget::v2 = v2;
    }


    const NVGcolor &getInnerColor() const;
    void setInnerColor(const NVGcolor &innerColor);
    const NVGcolor &getOuterColor() const;
    void setOuterColor(const NVGcolor &outerColor);

    void draw(NVGcontext *vg) override;
};


/**
 * @brief Widget for simulating used look
 */
struct LRPatinaWidget : TransparentWidget {
    SVGWidget *svg;

    float strength = 0.99f;

    LRPatinaWidget(const string &filename, const Vec &size);
    void draw(NVGcontext *vg) override;
    void randomize();

};


/**
 * @brief Default panel border
 */
struct LRPanelBorder : TransparentWidget {
    static constexpr float BORDER_WIDTH = 1.2f;


    inline void draw(NVGcontext *vg) override {
        NVGcolor borderColorLight = nvgRGBAf(0.9, 0.9, 0.9, 0.1);
        NVGcolor borderColorDark = nvgRGBAf(0.1, 0.1, 0.1, 0.5);

        nvgBeginPath(vg);
        nvgRect(vg, 0, BORDER_WIDTH, 0 + BORDER_WIDTH, box.size.y);
        nvgFillColor(vg, borderColorLight);
        nvgFill(vg);

        nvgBeginPath(vg);
        nvgRect(vg, 0, 0, box.size.x, BORDER_WIDTH);
        nvgFillColor(vg, borderColorLight);
        nvgFill(vg);

        nvgBeginPath(vg);
        nvgRect(vg, 0, box.size.y - BORDER_WIDTH, box.size.x, box.size.y - BORDER_WIDTH);
        nvgFillColor(vg, borderColorDark);
        nvgFill(vg);

        nvgBeginPath(vg);
        nvgRect(vg, box.size.x - BORDER_WIDTH, 0, box.size.x - BORDER_WIDTH, box.size.y);
        nvgFillColor(vg, borderColorDark);
        nvgFill(vg);

    }
};


/**
 * @brief Standard LR module Panel
 */
struct LRPanel : FramebufferWidget, LRGestaltVariant, LRGestaltChangeAction {
    SVGWidget *panelWidget;
    map<LRGestalt, LRGradientWidget *> gradients;
    LRPatinaWidget *patinaWidgetClassic, *patinaWidgetWhite;

    LRPanel();

    void setGradientVariant(bool enabled);
    void setPatina(bool enabled);
    void step() override;
    void init();
    void onGestaltChange(LREventGestaltChange &e) override;
};


/**
 * @brief Passive rotating SVG image
 */
struct SVGRotator : FramebufferWidget {
    TransformWidget *tw;
    SVGWidget *sw;

    /** angle to rotate per step */
    float angle = 0;
    float inc;
    float scale;
    float transperency;


    SVGRotator();


    /**
     * @brief Factory method
     * @param pos Position
     * @param svg Pointer to SVG image
     * @param angle Increment angle per step
     * @param scale Scaling of the SVG / default 100%
     * @param transperency Transperancy of the SVG / default 100%
     */
    SVGRotator static *create(Vec pos, shared_ptr<SVG> svg, float inc, float scale = 1.0f, float transperency = 1.f) {
        SVGRotator *rotator = FramebufferWidget::create<SVGRotator>(pos);

        rotator->setSVG(svg);
        rotator->inc = inc;
        rotator->scale = scale;
        rotator->transperency = transperency;

        return rotator;
    }


    void draw(NVGcontext *vg) override;

    void setSVG(shared_ptr<SVG> svg);
    void step() override;
};


struct FontIconWidget : FramebufferWidget {
    TrueType iconFont;
    float fontSize;
    NVGcolor color;

    explicit FontIconWidget(float fontSize = 12.f, NVGcolor color = nvgRGBAf(1.f, 1.f, 1.f, 1.f));

    void draw(NVGcontext *vg) override;
};


/**
 * Utility widget for resize action on modules
 */
struct ModuleResizeWidget : Widget {

    float minWidth;
    bool right = false;
    float dragX;
    float dragY;
    Rect originalBox;


    ModuleResizeWidget(float _minWidth) {
        box.size = Vec(RACK_GRID_WIDTH * 1, RACK_GRID_HEIGHT);
        minWidth = _minWidth;
    }


    void onMouseDown(EventMouseDown &e) override {
        if (e.button == 0) {
            e.consumed = true;
            e.target = this;
        }
    }


    void onDragStart(EventDragStart &e) override {
        dragX = RACK_PLUGIN_UI_RACKWIDGET->lastMousePos.x;
        dragY = RACK_PLUGIN_UI_RACKWIDGET->lastMousePos.y;
        ModuleWidget *m = getAncestorOfType<ModuleWidget>();
        originalBox = m->box;
    }


    void onDragMove(EventDragMove &e) override {
        ModuleWidget *m = getAncestorOfType<ModuleWidget>();

        float newDragX = RACK_PLUGIN_UI_RACKWIDGET->lastMousePos.x;
        float deltaX = newDragX - dragX;
        float newDragY = RACK_PLUGIN_UI_RACKWIDGET->lastMousePos.y;
        float deltaY = newDragY - dragY;

        Rect newBox = originalBox;

        // resize width
        if (right) {
            newBox.size.x += deltaX;
            newBox.size.x = fmaxf(newBox.size.x, minWidth);
            newBox.size.x = roundf(newBox.size.x / RACK_GRID_WIDTH) * RACK_GRID_WIDTH;
        } else {
            newBox.size.x -= deltaX;
            newBox.size.x = fmaxf(newBox.size.x, minWidth);
            newBox.size.x = roundf(newBox.size.x / RACK_GRID_WIDTH) * RACK_GRID_WIDTH;
            newBox.pos.x = originalBox.pos.x + originalBox.size.x - newBox.size.x;
        }

        // resize height
        newBox.size.y += deltaY;
        newBox.size.y = fmaxf(newBox.size.y, RACK_GRID_HEIGHT);
        newBox.size.y = roundf(newBox.size.y / RACK_GRID_HEIGHT) * RACK_GRID_HEIGHT;

        RACK_PLUGIN_UI_RACKWIDGET->requestModuleBox(m, newBox);
    }

};
}
