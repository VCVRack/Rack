#include "NauModular.hpp"

struct BitHammer : Module{
    enum ParamIds{
	    NUM_PARAMS
    };
    enum InputIds{
        A_INPUT,
        B_INPUT,
	    NUM_INPUTS
    };
    enum OutputIds{
        AND_OUTPUT,
        OR_OUTPUT,
        XOR_OUTPUT,
        LEFT_OUTPUT,
        RIGHT_OUTPUT,
        NOT_OUTPUT,
	    NUM_OUTPUTS
    };
    enum LightIds{
	    NUM_LIGHTS
    };

    union Volts2Bits{
        float volts;
        int bits;
    };

    BitHammer();
    void step() override;

    Volts2Bits inA;
    Volts2Bits inB;

    Volts2Bits outAnd;
    Volts2Bits outOr;
    Volts2Bits outXor;
    Volts2Bits outLeft;
    Volts2Bits outRight;
    Volts2Bits outNot;
};

BitHammer::BitHammer() : Module(NUM_PARAMS, NUM_INPUTS, NUM_OUTPUTS, NUM_LIGHTS){
}


void BitHammer::step(){
    inA.volts = inputs[A_INPUT].value;
    inB.volts = inputs[B_INPUT].value;

    outAnd.bits = inA.bits & inB.bits;
    outOr.bits = inA.bits | inB.bits;
    outXor.bits = inA.bits ^ inB.bits;

    outNot.bits = ~inA.bits;
    outNot.bits &= ~inB.bits;

    outLeft.bits = inA.bits &(1<< inB.bits);
    outRight.bits = inA.bits >> inB.bits;

    outputs[AND_OUTPUT].value = outAnd.volts;
    outputs[OR_OUTPUT].value = outOr.volts;
    outputs[XOR_OUTPUT].value = outXor.volts;
    outputs[NOT_OUTPUT].value = outNot.volts;
    outputs[LEFT_OUTPUT].value = outLeft.volts;
    outputs[RIGHT_OUTPUT].value = outRight.volts;
}

struct BitHammerWidget : ModuleWidget{
    BitHammerWidget(BitHammer *module);
};

BitHammerWidget::BitHammerWidget(BitHammer *module) : ModuleWidget(module){
    box.size = Vec(6*RACK_GRID_WIDTH, RACK_GRID_HEIGHT);
    {
	SVGPanel * panel = new SVGPanel();
	panel->box.size = box.size;
	panel->setBackground(SVG::load(assetPlugin(plugin, "res/BitHammer.svg")));
	addChild(panel);
    }

    addChild(Widget::create<ScrewSilver>(Vec(15,0)));
    addChild(Widget::create<ScrewSilver>(Vec(box.size.x-30,0)));
    addChild(Widget::create<ScrewSilver>(Vec(15,365)));
    addChild(Widget::create<ScrewSilver>(Vec(box.size.x-30,365)));

    addInput(Port::create<PJ301MPort>(Vec(15, 85), Port::INPUT, module, BitHammer::A_INPUT));
    addInput(Port::create<PJ301MPort>(Vec(50, 85), Port::INPUT, module, BitHammer::B_INPUT));

    addOutput(Port::create<PJ301MPort>(Vec(20, 140), Port::OUTPUT, module, BitHammer::AND_OUTPUT));
    addOutput(Port::create<PJ301MPort>(Vec(20, 175), Port::OUTPUT, module, BitHammer::OR_OUTPUT));
    addOutput(Port::create<PJ301MPort>(Vec(20, 210), Port::OUTPUT, module, BitHammer::XOR_OUTPUT));
    addOutput(Port::create<PJ301MPort>(Vec(20, 245), Port::OUTPUT, module, BitHammer::LEFT_OUTPUT));
    addOutput(Port::create<PJ301MPort>(Vec(20, 280), Port::OUTPUT, module, BitHammer::RIGHT_OUTPUT));
    addOutput(Port::create<PJ301MPort>(Vec(20, 315), Port::OUTPUT, module, BitHammer::NOT_OUTPUT));
}


Model *modelBitHammer = Model::create<BitHammer, BitHammerWidget>("NauModular", "BitHammer", "BitHammer", LOGIC_TAG);
