#pragma once

#include "GateTrigger.h"
#include "IComposite.h"

static const uint8_t gtable[256] =
{
0, 1, 3, 2, 6, 7, 5, 4, 12, 13, 15, 14, 10, 11, 9, 8,
24, 25, 27, 26, 30, 31, 29, 28, 20, 21, 23, 22, 18, 19, 17, 16,
48, 49, 51, 50, 54, 55, 53, 52, 60, 61, 63, 62, 58, 59, 57, 56,
40, 41, 43, 42, 46, 47, 45, 44, 36, 37, 39, 38, 34, 35, 33, 32,
96, 97, 99, 98, 102, 103, 101, 100, 108, 109, 111, 110, 106, 107, 105, 104,
120, 121, 123, 122, 126, 127, 125, 124, 116, 117, 119, 118, 114, 115, 113, 112,
80, 81, 83, 82, 86, 87, 85, 84, 92, 93, 95, 94, 90, 91, 89, 88,
72, 73, 75, 74, 78, 79, 77, 76, 68, 69, 71, 70, 66, 67, 65, 64,
192, 193, 195, 194, 198, 199, 197, 196, 204, 205, 207, 206, 202, 203, 201, 200,
216, 217, 219, 218, 222, 223, 221, 220, 212, 213, 215, 214, 210, 211, 209, 208,
240, 241, 243, 242, 246, 247, 245, 244, 252, 253, 255, 254, 250, 251, 249, 248,
232, 233, 235, 234, 238, 239, 237, 236, 228, 229, 231, 230, 226, 227, 225, 224,
160, 161, 163, 162, 166, 167, 165, 164, 172, 173, 175, 174, 170, 171, 169, 168,
184, 185, 187, 186, 190, 191, 189, 188, 180, 181, 183, 182, 178, 179, 177, 176,
144, 145, 147, 146, 150, 151, 149, 148, 156, 157, 159, 158, 154, 155, 153, 152,
136, 137, 139, 138, 142, 143, 141, 140, 132, 133, 135, 134, 130, 131, 129, 128
};

static const uint8_t bgtable[256] =
{
0x00, 0x01, 0x03, 0x02, 0x06, 0x0E, 0x0A, 0x0B, 0x09, 0x0D, 0x0F, 0x07, 0x05, 0x04, 0x0C, 0x08,
0x18, 0x1C, 0x14, 0x15, 0x17, 0x1F, 0x3F, 0x37, 0x35, 0x34, 0x3C, 0x38, 0x28, 0x2C, 0x24, 0x25,
0x27, 0x2F, 0x2D, 0x29, 0x39, 0x3D, 0x1D, 0x19, 0x1B, 0x3B, 0x2B, 0x2A, 0x3A, 0x1A, 0x1E, 0x16,
0x36, 0x3E, 0x2E, 0x26, 0x22, 0x32, 0x12, 0x13, 0x33, 0x23, 0x21, 0x31, 0x11, 0x10, 0x30, 0x20,
0x60, 0x70, 0x50, 0x51, 0x71, 0x61, 0x63, 0x73, 0x53, 0x52, 0x72, 0x62, 0x66, 0x6E, 0x7E, 0x76,
0x56, 0x5E, 0x5A, 0x7A, 0x6A, 0x6B, 0xEB, 0xEA, 0xFA, 0xDA, 0xDE, 0xD6, 0xF6, 0xFE, 0xEE, 0xE6,
0xE2, 0xF2, 0xD2, 0xD3, 0xF3, 0xE3, 0xE1, 0xF1, 0xD1, 0xD0, 0xF0, 0xE0, 0xA0, 0xB0, 0x90, 0x91,
0xB1, 0xA1, 0xA3, 0xB3, 0x93, 0x92, 0xB2, 0xA2, 0xA6, 0xAE, 0xBE, 0xB6, 0x96, 0x9E, 0x9A, 0xBA,
0xAA, 0xAB, 0xBB, 0x9B, 0x99, 0x9D, 0xDD, 0xD9, 0xDB, 0xFB, 0x7B, 0x5B, 0x59, 0x5D, 0x7D, 0x79,
0xF9, 0xFD, 0xBD, 0xB9, 0xA9, 0xE9, 0x69, 0x6D, 0x6F, 0x67, 0x65, 0x64, 0xE4, 0xE5, 0xE7, 0xEF,
0xED, 0xAD, 0xAF, 0xA7, 0xA5, 0xA4, 0xAC, 0xEC, 0x6C, 0x68, 0xE8, 0xA8, 0xB8, 0xF8, 0x78, 0x7C,
0xFC, 0xBC, 0xB4, 0xB5, 0xB7, 0xF7, 0xF5, 0xF4, 0x74, 0x75, 0x77, 0x7F, 0xFF, 0xBF, 0x9F, 0xDF,
0x5F, 0x57, 0x55, 0x54, 0xD4, 0xD5, 0xD7, 0x97, 0x95, 0x94, 0x9C, 0xDC, 0x5C, 0x58, 0xD8, 0x98,
0x88, 0xC8, 0x48, 0x4C, 0xCC, 0x8C, 0x84, 0xC4, 0x44, 0x45, 0xC5, 0x85, 0x87, 0xC7, 0x47, 0x4F,
0xCF, 0x8F, 0x8D, 0xCD, 0x4D, 0x49, 0xC9, 0x89, 0x8B, 0xCB, 0x4B, 0x4A, 0xCA, 0x8A, 0x8E, 0xCE,
0x4E, 0x46, 0xC6, 0x86, 0x82, 0xC2, 0x42, 0x43, 0xC3, 0x83, 0x81, 0xC1, 0x41, 0x40, 0xC0, 0x80
};

template <class TBase>
class GrayDescription : public IComposite
{
public:
    Config getParam(int i) override;
    int getNumParams() override;
};

template <class TBase>
class Gray : public TBase
{
public:
    Gray(struct Module * module) : TBase(module), gateTrigger(true)
    {
        init();
    }
    Gray() : TBase(), gateTrigger(true)
    {
        init();
    }

    enum ParamIds
    {
        PARAM_CODE,
        NUM_PARAMS
    };

    enum InputIds
    {
        INPUT_CLOCK,
        NUM_INPUTS
    };

    enum OutputIds
    {
        OUTPUT_MIXED,
        OUTPUT_0,
        OUTPUT_1,
        OUTPUT_2,
        OUTPUT_3,
        OUTPUT_4,
        OUTPUT_5,
        OUTPUT_6,
        OUTPUT_7,
        NUM_OUTPUTS
    };

    enum LightIds
    {
        LIGHT_0,
        LIGHT_1,
        LIGHT_2,
        LIGHT_3,
        LIGHT_4,
        LIGHT_5,
        LIGHT_6,
        LIGHT_7,
        NUM_LIGHTS
    };

    /** Implement IComposite
     */
    static std::shared_ptr<IComposite> getDescription()
    {
        return std::make_shared<GrayDescription<TBase>>();
    }

    /**
     * Main processing entry point. Called every sample
     */
    void step() override;

private:
    uint8_t counterValue = 0;
    GateTrigger gateTrigger;
    int c = 0;
    void init();
};


template <class TBase>
void  Gray<TBase>::init()
{
    // Init all the outputs to zero,
    // since they don't all get update until a clock.
    for (int i = 0; i < NUM_OUTPUTS; ++i) {
        TBase::outputs[i].value = 0;
    }
}


template <class TBase>
void  Gray<TBase>::step()
{
    gateTrigger.go(TBase::inputs[INPUT_CLOCK].value);
    if (!gateTrigger.trigger()) {
        return;
    }
    ++counterValue;

    const uint8_t* table = TBase::params[PARAM_CODE].value > .5 ? gtable : bgtable;

    const auto g0 = table[counterValue];
    auto g = g0;
    for (int i = 0; i < 8; ++i) {
        bool b = g & 1;
        TBase::lights[i + LIGHT_0].value = b ? 10.f : 0.f;
        TBase::outputs[i + OUTPUT_0].value = b ? 10.f : 0.f;
        g >>= 1;
    }
    TBase::outputs[OUTPUT_MIXED].value = (float) g0 / 25.f;
}

template <class TBase>
int GrayDescription<TBase>::getNumParams()
{
    return Gray<TBase>::NUM_PARAMS;
}

template <class TBase>
IComposite::Config GrayDescription<TBase>::getParam(int i)
{
    assert(i == Gray<TBase>::PARAM_CODE);
    return {0.0f, 1.0f, 0.0f, "Code type"};
}