#pragma once

#include "rack.hpp"
#include "asset.hpp"
#include "widgets.hpp"

#define LCD_FONT_DIG7 "res/digital-7.ttf"
#define LCD_FONTSIZE 11
#define LCD_LETTER_SPACING 0

/* show values of all knobs */
#define DEBUG_VALUES false

typedef std::shared_ptr<rack::Font> TrueType;
using namespace rack;

#ifdef USE_VST2
#define plugin "LindenbergResearch"
#else
extern Plugin *plugin;
#endif // USE_VST2


/**
 * @brief Standard LRT module
 */
struct LRModule : Module {
    long cnt = 0;


    /**
     * @brief Overtake default constructor for module to be compatible
     * @param numParams
     * @param numInputs
     * @param numOutputs
     * @param numLights
     */
    LRModule(int numParams, int numInputs, int numOutputs, int numLights = 0) :
            Module(numParams, numInputs, numOutputs, numLights) {}


    void step() override {
        Module::step();

        // increment counter
        cnt++;
    }
};


/**
 * @brief Standard LRT ModuleWidget definition
 */
struct LRModuleWidget : ModuleWidget {
    explicit LRModuleWidget(Module *module);
};


/**
 * @brief Emulation of a LCD monochrome display
 */
struct LCDWidget : Label {

    enum LCDType {
        NUMERIC,
        TEXT,
        LIST
    };

    TrueType ttfLCDDig7;
    float fontsize;

    LCDType type;

    NVGcolor fg;
    NVGcolor bg;

    bool active = true;
    float value = 0.0;
    unsigned char length = 0;
    std::string format;
    std::vector<std::string> items;

    std::string s1;
    std::string s2;

    /**
     * @brief Constructor
     */
    LCDWidget(NVGcolor fg, unsigned char length, std::string format, LCDType type, float fontsize = LCD_FONTSIZE);

    /**
     * @brief Draw LCD display
     * @param vg
     */
    void draw(NVGcontext *vg) override;


    inline void addItem(std::string name) {
        items.push_back(name);
    }
};


/**
 * @brief Indicator for control voltages on knobs
 */
struct Indicator {
    static constexpr float OVERFLOW_THRESHOLD = 0.01f;

    /** flag to control drawing */
    bool active = false;

    /** color of indicator */
    NVGcolor normalColor = nvgRGBA(0x00, 0x00, 0x00, 0xBB);
    NVGcolor overflowColor = nvgRGBA(0xBB, 0x00, 0x00, 0xBB);

    /** radius from middle */
    float distance;

    /** normalized control voltage. must between [0..1] */
    float cv = 0.f;

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
    Indicator(float distance, float angle) {
        Indicator::distance = distance;
        Indicator::angle = angle;

        /** for optimization */
        angle2 = 2 * angle;
    }


    /**
     * @brief Draw routine for cv indicator
     * @param vg
     */
    void draw(NVGcontext *vg);

};


/**
 * @brief Standard LR Shadow
 */
struct LRShadow {
private:
    Rect box;
    float size = 0.65;
    float strength = 1.f;

    /** shadow shift */
    Vec shadowPos = Vec(3, 5);
public:
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

    void draw(NVGcontext *vg);
};


/**
 * @brief The base of all knobs used in LR panels, includes a indicator
 */
struct LRKnob : SVGKnob {
private:
    static constexpr float ANGLE = 0.83f;

    /** setup indicator with default values */
    Indicator idc = Indicator(15.f, ANGLE);

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
    LRShadow shadow = LRShadow();


public:
    /**
     * @brief Default constructor
     */
    LRKnob() {
        minAngle = -ANGLE * (float) M_PI;
        maxAngle = ANGLE * (float) M_PI;

        font = Font::load(assetGlobal("res/fonts/ShareTechMono-Regular.ttf"));
    }


    /**
     * @brief Set the value of the indicator
     * @param value
     */
    void setIndicatorValue(float value) {
        idc.cv = value;
    }


    /**
     * @brief Switch indicator on/off
     * @param active
     */
    void setIndicatorActive(bool active) {
        idc.active = active;
    }


    /**
     * @brief Get indicator state
     * @return
     */
    bool isIndicatorActive() {
        return idc.active;
    }


    /**
     * @brief Setup distance of indicator from middle
     * @param distance
     */
    void setIndicatorDistance(float distance) {
        idc.distance = distance;
    }


    /**
     * @brief Hook into setSVG() method to setup box dimensions correct for indicator
     * @param svg
     */
    void setSVG(std::shared_ptr<SVG> svg) {
        SVGKnob::setSVG(svg);

        /** inherit dimensions after loaded svg */
        idc.middle = Vec(box.size.x / 2, box.size.y / 2);
        shadow.setBox(box);
    }


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
    static TParamWidget *create(Vec pos, Module *module, int paramId, float minValue, float maxValue, float defaultValue) {
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
    void draw(NVGcontext *vg) override {
        /** shadow */
        shadow.draw(vg);

        /** component */
        FramebufferWidget::draw(vg);

/*
        nvgBeginPath(vg);
        nvgRect(vg, -30, -30, box.size.x + 60, box.size.y + 60);

        NVGcolor icol = nvgRGBAf(0.0f, 0.0f, 0.0f, 0.3f);
        NVGcolor ocol = nvgRGBAf(0.0f, 0.0f, 0.0f, 0.0f);;

        NVGpaint paint = nvgRadialGradient(vg, box.size.x / 2, box.size.y / 2,
                                           0.f, box.size.x /2.f * 0.9f, icol, ocol);
        nvgFillPaint(vg, paint);
        nvgFill(vg);*/

        /** indicator */
        idc.draw(vg);

        if (debug) {
            auto text = stringf("%4.2f", value);
            nvgFontSize(vg, 15);
            nvgFontFaceId(vg, font->handle);

            nvgFillColor(vg, nvgRGBAf(1.f, 1.f, 1.0f, 1.0f));
            nvgText(vg, box.size.x - 5, box.size.y + 10, text.c_str(), NULL);
        }


    }


    /**
     * @brief Setup knob snapping
     * @param position
     * @param sensitivity
     */
    void setSnap(float position, float sensitivity) {
        snap = true;
        snapSens = sensitivity;
        snapAt = position;
    }


    /**
     * @brief Remove knob snaping
     */
    void unsetSnap() {
        snap = false;
    }


    /**
     * @brief Snapping mode for knobs
     * @param e
     */
    void onChange(EventChange &e) override {
        // if the value still inside snap-tolerance keep the value zero
        if (snap && value > -snapSens + snapAt && value < snapSens + snapAt) value = 0;
        SVGKnob::onChange(e);
    }
};


/**
 * @brief Quantize position to odd numbers to simulate a toggle switch
 */
struct LRToggleKnob : LRKnob {
    LRToggleKnob(float length = 0.5f) {
        //TODO: parametrize start and end angle
        minAngle = -0.666666f * (float) M_PI;
        maxAngle = length * (float) M_PI;

        setSVG(SVG::load(assetPlugin(plugin, "res/ToggleKnob.svg")));
        shadow.setShadowPosition(3, 4);

        shadow.setStrength(1.2f);
        shadow.setSize(0.7f);

        speed = 2.f;
    }


    void onChange(EventChange &e) override {
        value = round(value);
        SVGKnob::onChange(e);
    }
};


/**
 * @brief Quantize position to odd numbers to simulate a toggle switch
 */
struct LRMiddleIncremental : LRKnob {
    LRMiddleIncremental(float length = 0.5f) {
        minAngle = -length * (float) M_PI;
        maxAngle = length * (float) M_PI;

        setSVG(SVG::load(assetPlugin(plugin, "res/MiddleIncremental.svg")));
        shadow.setShadowPosition(3, 4);

        shadow.setStrength(1.2f);
        shadow.setSize(0.7f);

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
        setSVG(SVG::load(assetPlugin(plugin, "res/BigKnob.svg")));
        setIndicatorDistance(15);
        shadow.setShadowPosition(5, 6);
    }
};


/**
 * @brief LR Middle Knob
 */
struct LRMiddleKnob : LRKnob {
    LRMiddleKnob() {
        setSVG(SVG::load(assetPlugin(plugin, "res/MiddleKnob.svg")));
        setIndicatorDistance(12);
        shadow.setShadowPosition(4, 4);
    }
};


/**
 * @brief LR Small Knob
 */
struct LRSmallKnob : LRKnob {
    LRSmallKnob() {
        setSVG(SVG::load(assetPlugin(plugin, "res/SmallKnob.svg")));
        shadow.setShadowPosition(3, 3);
        setSnap(0.0f, 0.03f);


        speed = 0.9f;
    }
};


/**
 * @brief LR Alternate Small Knob
 */
struct LRAlternateSmallKnob : LRKnob {
    LRAlternateSmallKnob() {
        setSVG(SVG::load(assetPlugin(plugin, "res/AlternateSmallKnob.svg")));
        shadow.setShadowPosition(3, 3);
        setSnap(0.0f, 0.03f);


        speed = 0.9f;
    }
};

/**
 * @brief LR Middle Knob
 */
struct LRAlternateMiddleKnob : LRKnob {
    LRAlternateMiddleKnob() {
        setSVG(SVG::load(assetPlugin(plugin, "res/AlternateMiddleKnob.svg")));
        setIndicatorDistance(12);
        shadow.setShadowPosition(4, 4);
    }
};


/**
 * @brief LR Big Knob
 */
struct LRAlternateBigKnob : LRKnob {
    LRAlternateBigKnob() {
        setSVG(SVG::load(assetPlugin(plugin, "res/AlternateBigKnob.svg")));
        setIndicatorDistance(15);
        shadow.setShadowPosition(5, 6);
    }
};


/**
 * @brief Alternative IO Port
 */
struct LRIOPort : SVGPort {
private:
    LRShadow shadow = LRShadow();

public:
    LRIOPort() {
        background->svg = SVG::load(assetPlugin(plugin, "res/IOPortB.svg"));
        background->wrap();
        box.size = background->box.size;

        /** inherit dimensions */
        shadow.setBox(box);
        shadow.setSize(0.50);
        shadow.setShadowPosition(2, 1);
    }


    /**
     * @brief Hook into draw method
     * @param vg
     */
    void draw(NVGcontext *vg) override {
        shadow.draw(vg);
        SVGPort::draw(vg);
    }
};


/**
 * @brief Alternative IO Port
 */
struct LRIOPortC : SVGPort {
private:
    LRShadow shadow = LRShadow();

public:
    LRIOPortC() {
        background->svg = SVG::load(assetPlugin(plugin, "res/IOPortC.svg"));
        background->wrap();
        box.size = background->box.size;

        /** inherit dimensions */
        shadow.setBox(box);
        shadow.setSize(0.50);
        shadow.setShadowPosition(2, 1);
    }


    /**
     * @brief Hook into draw method
     * @param vg
     */
    void draw(NVGcontext *vg) override {
        shadow.draw(vg);
        SVGPort::draw(vg);
    }
};

/**
 * @brief Alternative screw head A
 */
struct ScrewDarkA : SVGScrew {
    ScrewDarkA() {
        sw->svg = SVG::load(assetPlugin(plugin, "res/ScrewDark.svg"));
        sw->wrap();
        box.size = sw->box.size;
    }
};


/**
 * @brief Alternative screw head A
 */
struct ScrewLight : SVGScrew {
    ScrewLight() {
        sw->svg = SVG::load(assetPlugin(plugin, "res/ScrewLight.svg"));
        sw->wrap();
        box.size = sw->box.size;
    }
};

/**
 * @brief Custom switch based on original Rack files
 */
struct LRSwitch : SVGSwitch, ToggleSwitch {
    LRSwitch() {
        addFrame(SVG::load(assetPlugin(plugin, "res/Switch0.svg")));
        addFrame(SVG::load(assetPlugin(plugin, "res/Switch1.svg")));
    }
};


/**
 * @brief Standard LED Redlight
 */
struct LRLight : SmallLight<ModuleLightWidget> {
    LRLight();

    void draw(NVGcontext *vg) override;
};


/**
 * @brief Standard LR module Panel
 */
struct LRPanel : SVGPanel {
private:
    /** margin of gradient box */
    static constexpr float MARGIN = 10;

    /** gradient colors */
    NVGcolor inner = nvgRGBAf(1.5f * .369f, 1.5f * 0.357f, 1.5f * 0.3333f, 0.25f);
    NVGcolor outer = nvgRGBAf(0.0f, 0.0f, 0.0f, 0.34f);;

    /** gradient offset */
    Vec offset = Vec(30, -50);


public:
    LRPanel();


    LRPanel(float x, float y) {
        offset.x = x;
        offset.y = y;
    }


    void setInner(const NVGcolor &inner);
    void setOuter(const NVGcolor &outer);
    const NVGcolor &getInner() const;
    const NVGcolor &getOuter() const;

    void draw(NVGcontext *vg) override;
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


    SVGRotator();


    /**
     * @brief Factory method
     * @param pos Position
     * @param svg Pointer to SVG image
     * @param angle Increment angle per step
     */
    SVGRotator static *create(Vec pos, std::shared_ptr<SVG> svg, float inc) {
        SVGRotator *rotator = FramebufferWidget::create<SVGRotator>(pos);

        rotator->setSVG(svg);
        rotator->inc = inc;

        return rotator;
    }


    void setSVG(std::shared_ptr<SVG> svg);
    void step() override;
};
