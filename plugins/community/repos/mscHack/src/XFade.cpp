#include "mscHack.hpp"
//#include "mscHack_Controls.hpp"
#include "dsp/digital.hpp"
//#include "CLog.h"

namespace rack_plugin_mscHack {

#define CHANNELS 3

//-----------------------------------------------------
// Module Definition
//
//-----------------------------------------------------
struct XFade : Module 
{
	enum ParamIds 
    {
        PARAM_MIX,
        PARAM_LEVEL,
        nPARAMS
    };

	enum InputIds 
    {
        IN_MIXCV,
        IN_AL,
        IN_AR           = IN_AL + CHANNELS,
        IN_BL           = IN_AR + CHANNELS,
        IN_BR           = IN_BL + CHANNELS,
        nINPUTS         = IN_BR + CHANNELS
	};

	enum OutputIds 
    {
        OUT_L,
        OUT_R       = OUT_L + CHANNELS,
        nOUTPUTS    = OUT_R + CHANNELS
	};

    CLog            lg;

    // Contructor
	XFade() : Module(nPARAMS, nINPUTS, nOUTPUTS){}

    // Overrides 
	void    step() override;
    //json_t* toJson() override;
    //void    fromJson(json_t *rootJ) override;
    void    onRandomize() override;
    void    onReset() override;
};

//-----------------------------------------------------
// Procedure:   Widget
//
//-----------------------------------------------------

struct XFade_Widget : ModuleWidget {
	XFade_Widget( XFade *module );
};

XFade_Widget::XFade_Widget( XFade *module ) : ModuleWidget(module) 
{
    int i, x, y;

	box.size = Vec( 15*8, 380);

	{
		SVGPanel *panel = new SVGPanel();
		panel->box.size = box.size;
		panel->setBackground(SVG::load(assetPlugin(plugin, "res/XFade.svg")));
		addChild(panel);
	}

    //module->lg.Open("XFade.txt");

	addChild(Widget::create<ScrewSilver>(Vec(15, 0)));
	addChild(Widget::create<ScrewSilver>(Vec(box.size.x-30, 0)));
	addChild(Widget::create<ScrewSilver>(Vec(15, 365))); 
	addChild(Widget::create<ScrewSilver>(Vec(box.size.x-30, 365)));

    x = 10;
    y = 47;

    for( i = 0; i < CHANNELS; i++ )
    {
        // audio A
        addInput(Port::create<MyPortInSmall>( Vec( x, y ), Port::INPUT, module, XFade::IN_AL + i ) );
        addInput(Port::create<MyPortInSmall>( Vec( x + 28, y ), Port::INPUT, module, XFade::IN_AR + i ) );

        // audio B
        addInput(Port::create<MyPortInSmall>( Vec( x, y + 32 ), Port::INPUT, module, XFade::IN_BL + i ) );
        addInput(Port::create<MyPortInSmall>( Vec( x + 28, y + 32 ), Port::INPUT, module, XFade::IN_BR + i ) );

        // audio  outputs
        addOutput(Port::create<MyPortOutSmall>( Vec( x + 83, y ), Port::OUTPUT, module, XFade::OUT_L + i ) );
        addOutput(Port::create<MyPortOutSmall>( Vec( x + 83, y + 32 ), Port::OUTPUT, module, XFade::OUT_R + i ) );

        y += 67;
    }

    // mix CV
    addInput(Port::create<MyPortInSmall>( Vec( 4, 263 ), Port::INPUT, module, XFade::IN_MIXCV) );

    // mix knobs
    addParam(ParamWidget::create<Knob_Yellow2_56>( Vec( 30, 243 ), module, XFade::PARAM_MIX, -1.0, 1.0, 0.0 ) );
    addParam(ParamWidget::create<Knob_Yellow2_56>( Vec( 30, 313 ), module, XFade::PARAM_LEVEL, 0.0, 2.0, 1.0 ) );
}

//-----------------------------------------------------
// Procedure:   reset
//
//-----------------------------------------------------
void XFade::onReset()
{
}

//-----------------------------------------------------
// Procedure:   randomize
//
//-----------------------------------------------------
void XFade::onRandomize()
{
}

//-----------------------------------------------------
// Procedure:   step
//
//-----------------------------------------------------
void XFade::step() 
{
    int i;
    float mix, mixa, mixb;

    if( inputs[ IN_MIXCV ].active )
        mix = inputs[ IN_MIXCV ].value / AUDIO_MAX;
    else
        mix = params[ PARAM_MIX ].value;

    if( mix <= 0.0 )
    {
        mixa = 1.0;
        mixb = 1.0 + mix;
    }
    else
    {
        mixb = 1.0;
        mixa = 1.0 - mix;
    }

    for( i = 0; i < CHANNELS; i++ )
    {
        outputs[ OUT_L + i ].value = ( ( inputs[ IN_AL + i ].value * mixa ) + ( inputs[ IN_BL + i ].value * mixb ) ) * params[ PARAM_LEVEL ].value;
        outputs[ OUT_R + i ].value = ( ( inputs[ IN_AR + i ].value * mixa ) + ( inputs[ IN_BR + i ].value * mixb ) ) * params[ PARAM_LEVEL ].value;
    }
}

} // namespace rack_plugin_mscHack

using namespace rack_plugin_mscHack;

RACK_PLUGIN_MODEL_INIT(mscHack, XFade) {
   Model *modelXFade = Model::create<XFade, XFade_Widget>( "mscHack", "XFade", "MIXER Cross Fader 3 Channel", MIXER_TAG, MULTIPLE_TAG );
   return modelXFade;
}
