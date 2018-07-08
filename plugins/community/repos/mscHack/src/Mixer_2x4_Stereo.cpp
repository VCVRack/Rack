#include "mscHack.hpp"
//#include "mscHack_Controls.hpp"
#include "dsp/digital.hpp"
//#include "CLog.h"

namespace rack_plugin_mscHack {

#define GROUPS 2
#define CH_PER_GROUP 4
#define CHANNELS ( GROUPS * CH_PER_GROUP )
#define nAUX 4

#define GROUP_OFF_X 52
#define CHANNEL_OFF_X 34

#define FADE_MULT (0.0005f)

#define L 0
#define R 1

#define MUTE_FADE_STATE_IDLE 0
#define MUTE_FADE_STATE_INC  1
#define MUTE_FADE_STATE_DEC  2

//-----------------------------------------------------
// Module Definition
//
//-----------------------------------------------------
struct Mix_2x4_Stereo : Module 
{
	enum ParamIds 
    {
        PARAM_MAIN_LEVEL,
        PARAM_LEVEL_IN,
        PARAM_PAN_IN            = PARAM_LEVEL_IN + CHANNELS,
        PARAM_GROUP_LEVEL_IN    = PARAM_PAN_IN + CHANNELS,
        PARAM_GROUP_PAN_IN      = PARAM_GROUP_LEVEL_IN + GROUPS,
        PARAM_MUTE_BUTTON       = PARAM_GROUP_PAN_IN + GROUPS,
        PARAM_SOLO_BUTTON       = PARAM_MUTE_BUTTON + CHANNELS,
        PARAM_GROUP_MUTE        = PARAM_SOLO_BUTTON + CHANNELS,
        PARAM_GROUP_SOLO        = PARAM_GROUP_MUTE + GROUPS,

        PARAM_EQ_HI             = PARAM_GROUP_SOLO + GROUPS,
        PARAM_EQ_MD             = PARAM_EQ_HI + CHANNELS,
        PARAM_EQ_LO             = PARAM_EQ_MD + CHANNELS,

        PARAM_AUX_KNOB         = PARAM_EQ_LO + CHANNELS,
        PARAM_AUX_PREFADE      = PARAM_AUX_KNOB + (GROUPS * nAUX),
        PARAM_AUX_OUT          = PARAM_AUX_PREFADE + (GROUPS * nAUX),

        nPARAMS                = PARAM_AUX_OUT + (nAUX)
    };

	enum InputIds 
    {
        IN_LEFT,
        IN_RIGHT                = IN_LEFT + CHANNELS,
        IN_LEVEL                = IN_RIGHT + CHANNELS,
        IN_PAN                  = IN_LEVEL + CHANNELS, 
        IN_GROUP_LEVEL          = IN_PAN + CHANNELS,
        IN_GROUP_PAN            = IN_GROUP_LEVEL + GROUPS,
        nINPUTS                 = IN_GROUP_PAN + GROUPS 
	};

	enum OutputIds 
    {
		OUT_MAINL,
        OUT_MAINR,

        OUT_AUXL,
        OUT_AUXR              = OUT_AUXL + nAUX,

        nOUTPUTS              = OUT_AUXR + nAUX
	};

    bool            m_bInitialized = false;
    CLog            lg;

    // mute buttons
    bool            m_bMuteStates[ CHANNELS ] = {};
    float           m_fMuteFade[ CHANNELS ] = {1.0};
    
    int             m_FadeState[ CHANNELS ] = {MUTE_FADE_STATE_IDLE};

    // solo buttons
    bool            m_bSoloStates[ CHANNELS ] = {};

    // group mute buttons
    bool            m_bGroupMuteStates[ GROUPS ] = {};
    float           m_fGroupMuteFade[ GROUPS ] = {1.0};

    int             m_GroupFadeState[ GROUPS ] = {MUTE_FADE_STATE_IDLE};

    // group solo buttons
    bool            m_bGroupSoloStates[ GROUPS ] = {};

    // processing
    bool            m_bMono[ CHANNELS ];
    float           m_fSubMix[ GROUPS ][ 3 ] = {};

    // aux
    bool            m_bGroupPreFadeAuxStates[ GROUPS ][ nAUX ] = {};

    // LED Meters
    LEDMeterWidget  *m_pLEDMeterChannel[ CHANNELS ][ 2 ] ={};
    LEDMeterWidget  *m_pLEDMeterGroup[ GROUPS ][ 2 ] ={};
    LEDMeterWidget  *m_pLEDMeterMain[ 2 ] ={};

    // EQ Rez
    float           lp1[ CHANNELS ][ 2 ] = {}, bp1[ CHANNELS ][ 2 ] = {}; 
    float           m_hpIn[ CHANNELS ];
    float           m_lpIn[ CHANNELS ];
    float           m_mpIn[ CHANNELS ];
    float           m_rezIn[ CHANNELS ] = {0};
    float           m_Freq;

    // buttons
    MyLEDButton             *m_pButtonChannelMute[ CHANNELS ] = {};
    MyLEDButton             *m_pButtonChannelSolo[ CHANNELS ] = {};
    MyLEDButton             *m_pButtonGroupMute[ GROUPS ] = {};
    MyLEDButton             *m_pButtonGroupSolo[ GROUPS ] = {};
    MyLEDButton             *m_pButtonAuxPreFader[ GROUPS ][ nAUX ] = {};

#define L 0
#define R 1

    // Contructor
	Mix_2x4_Stereo() : Module(nPARAMS, nINPUTS, nOUTPUTS, 0 ){}

    //-----------------------------------------------------
    // MyEQHi_Knob
    //-----------------------------------------------------
    struct MyEQHi_Knob : Knob_Green1_15
    {
        Mix_2x4_Stereo *mymodule;
        int param;

        void onChange( EventChange &e ) override 
        {
            mymodule = (Mix_2x4_Stereo*)module;

            if( mymodule )
            {
                param = paramId - Mix_2x4_Stereo::PARAM_EQ_HI;

                mymodule->m_hpIn[ param ] = value; 
            }

		    RoundKnob::onChange( e );
	    }
    };

    //-----------------------------------------------------
    // MyEQHi_Knob
    //-----------------------------------------------------
    struct MyEQMid_Knob : Knob_Green1_15
    {
        Mix_2x4_Stereo *mymodule;
        int param;

        void onChange( EventChange &e ) override 
        {
            mymodule = (Mix_2x4_Stereo*)module;

            if( mymodule )
            {
                param = paramId - Mix_2x4_Stereo::PARAM_EQ_MD;
                mymodule->m_mpIn[ param ] = value; 
            }

		    RoundKnob::onChange( e );
	    }
    };

    //-----------------------------------------------------
    // MyEQHi_Knob
    //-----------------------------------------------------
    struct MyEQLo_Knob : Knob_Green1_15
    {
        Mix_2x4_Stereo *mymodule;
        int param;

        void onChange( EventChange &e ) override 
        {
            mymodule = (Mix_2x4_Stereo*)module;

            if( mymodule )
            {
                param = paramId - Mix_2x4_Stereo::PARAM_EQ_LO;
                mymodule->m_lpIn[ param ] = value; 
            }

		    RoundKnob::onChange( e );
	    }
    };

    // Overrides 
	void    step() override;
    json_t* toJson() override;
    void    fromJson(json_t *rootJ) override;
    void    onRandomize() override{}
    void    onReset() override;

    void ProcessMuteSolo( int channel, bool bMute, bool bGroup );
    void ProcessEQ( int ch, float *pL, float *pR );
};

//-----------------------------------------------------
// MyLEDButton_Aux
//-----------------------------------------------------
void Mix_2x4_Stereo_MyLEDButton_Aux( void *pClass, int id, bool bOn ) 
{
    int ch, i;

    Mix_2x4_Stereo *mymodule;
    mymodule = (Mix_2x4_Stereo*)pClass;

    ch = id / nAUX;
    i  = id - (nAUX * ch);

    mymodule->m_bGroupPreFadeAuxStates[ ch ][ i ] = !mymodule->m_bGroupPreFadeAuxStates[ ch ][ i ];
    mymodule->m_pButtonAuxPreFader[ ch ][ i ]->Set( mymodule->m_bGroupPreFadeAuxStates[ ch ][ i ] );
}

//-----------------------------------------------------
// MyLEDButton_ChMute
//-----------------------------------------------------
void Mix_2x4_Stereo_MyLEDButton_ChMute( void *pClass, int id, bool bOn ) 
{
    Mix_2x4_Stereo *mymodule;
    mymodule = (Mix_2x4_Stereo*)pClass;
    mymodule->ProcessMuteSolo( id, true, false );
}

//-----------------------------------------------------
// MyLEDButton_ChSolo
//-----------------------------------------------------
void Mix_2x4_Stereo_MyLEDButton_ChSolo( void *pClass, int id, bool bOn ) 
{
    Mix_2x4_Stereo *mymodule;
    mymodule = (Mix_2x4_Stereo*)pClass;
    mymodule->ProcessMuteSolo( id, false, false );
}

//-----------------------------------------------------
// MyLEDButton_GroupMute
//-----------------------------------------------------
void Mix_2x4_Stereo_MyLEDButton_GroupMute( void *pClass, int id, bool bOn ) 
{
    Mix_2x4_Stereo *mymodule;
    mymodule = (Mix_2x4_Stereo*)pClass;
    mymodule->ProcessMuteSolo( id, true, true );
}

//-----------------------------------------------------
// MyLEDButton_GroupSolo
//-----------------------------------------------------
void Mix_2x4_Stereo_MyLEDButton_GroupSolo( void *pClass, int id, bool bOn ) 
{
    Mix_2x4_Stereo *mymodule;
    mymodule = (Mix_2x4_Stereo*)pClass;
    mymodule->ProcessMuteSolo( id, false, true );
}

#define CUTOFF (0.025f)
//-----------------------------------------------------
// Procedure:   Widget
//
//-----------------------------------------------------
struct Mix_2x4_Stereo_Widget : ModuleWidget {
	Mix_2x4_Stereo_Widget( Mix_2x4_Stereo *module );
};

Mix_2x4_Stereo_Widget::Mix_2x4_Stereo_Widget( Mix_2x4_Stereo *module ) : ModuleWidget(module) 
{
    float fx, fx2, fx3, fx5, fx7;
    int ch, x, y, i, ybase, x2, y2;

	box.size = Vec( 15*27, 380);

	{
		SVGPanel *panel = new SVGPanel();
		panel->box.size = box.size;
		panel->setBackground(SVG::load(assetPlugin(plugin, "res/Mix_2x4_Stereo.svg")));
		addChild(panel);
	}

    //module->lg.Open("Mix_2x4_Stereo.txt");

	addChild(Widget::create<ScrewSilver>(Vec(15, 0)));
	addChild(Widget::create<ScrewSilver>(Vec(box.size.x-30, 0)));
	addChild(Widget::create<ScrewSilver>(Vec(15, 365))); 
	addChild(Widget::create<ScrewSilver>(Vec(box.size.x-30, 365)));

    //----------------------------------------------------
    // Add mix sliders
    x = 23;
    y = 38;

    // main channel
	for ( ch = 0; ch < CHANNELS; ch++ ) 
    {
        // Left channel inputs
        addInput(Port::create<MyPortInSmall>( Vec( x, y ), Port::INPUT, module, Mix_2x4_Stereo::IN_LEFT + ch ) );

        y += 25;

        // Right channel inputs
        addInput(Port::create<MyPortInSmall>( Vec( x, y ), Port::INPUT, module, Mix_2x4_Stereo::IN_RIGHT + ch ) );

        y += 26;

        // Level knobs
        addParam(ParamWidget::create<Knob_Blue2_26>( Vec( x - 5, y ), module, Mix_2x4_Stereo::PARAM_LEVEL_IN + ch, 0.0, AMP_MAX, 0.0 ) );

        y += 31;

        // Level inputs
        addInput(Port::create<MyPortInSmall>( Vec( x, y ), Port::INPUT, module, Mix_2x4_Stereo::IN_LEVEL + ch ) );

        y += 23;

        // pan knobs
        addParam(ParamWidget::create<Knob_Yellow2_26>( Vec( x - 5, y ), module, Mix_2x4_Stereo::PARAM_PAN_IN + ch, -1.0, 1.0, 0.0 ) );

        y += 31;

        // Pan inputs
        addInput(Port::create<MyPortInSmall>( Vec( x, y ), Port::INPUT, module, Mix_2x4_Stereo::IN_PAN + ch ) );

        y += 22;

        // mute buttons
        module->m_pButtonChannelMute[ ch ] = new MyLEDButton( x - 5, y, 11, 11, 8.0, DWRGB( 180, 180, 180 ), DWRGB( 255, 0, 0 ), MyLEDButton::TYPE_SWITCH, ch, module, Mix_2x4_Stereo_MyLEDButton_ChMute );
	    addChild( module->m_pButtonChannelMute[ ch ] );
        //y += 26;

        // solo buttons
        module->m_pButtonChannelSolo[ ch ] = new MyLEDButton( x + 11, y, 11, 11, 8.0, DWRGB( 180, 180, 180 ), DWRGB( 0, 255, 255 ), MyLEDButton::TYPE_SWITCH, ch, module, Mix_2x4_Stereo_MyLEDButton_ChSolo );
	    addChild( module->m_pButtonChannelSolo[ ch ] );

        y += 22;
        y2 = y;

        // eq and rez
        addParam(ParamWidget::create<Mix_2x4_Stereo::MyEQHi_Knob>( Vec( x - 5, y ), module, Mix_2x4_Stereo::PARAM_EQ_HI + ch, 0.0, 1.0, 0.5 ) );

        y += 19;

        addParam(ParamWidget::create<Mix_2x4_Stereo::MyEQMid_Knob>( Vec( x - 5, y ), module, Mix_2x4_Stereo::PARAM_EQ_MD + ch, 0.0, 1.0, 0.5 ) );
        
        y += 19;
        
        addParam(ParamWidget::create<Mix_2x4_Stereo::MyEQLo_Knob>( Vec( x - 5, y ), module, Mix_2x4_Stereo::PARAM_EQ_LO + ch, 0.0, 1.0, 0.5 ) );
        
        // LED Meters
        module->m_pLEDMeterChannel[ ch ][ 0 ] = new LEDMeterWidget( x + 13, y2 + 30, 4, 1, 1, true );
        addChild( module->m_pLEDMeterChannel[ ch ][ 0 ] );
        module->m_pLEDMeterChannel[ ch ][ 1 ] = new LEDMeterWidget( x + 18, y2 + 30, 4, 1, 1, true );
        addChild( module->m_pLEDMeterChannel[ ch ][ 1 ] );

        if( ( ch & 3 ) == 3 )
        {
            x += GROUP_OFF_X;
        }
        else
        {
            x += CHANNEL_OFF_X;
        }

        y = 38;
    }

    // group mixera
    ybase = 278;
    x = 12;
    for( i = 0; i < GROUPS; i++ )
    {
        // mute/solo buttons
        x2 = x + 81;
        y2 = ybase;

        module->m_pButtonGroupMute[ i ] = new MyLEDButton( x2, y2 + 4, 11, 11, 8.0, DWRGB( 180, 180, 180 ), DWRGB( 255, 0, 0 ), MyLEDButton::TYPE_SWITCH, i, module, Mix_2x4_Stereo_MyLEDButton_GroupMute );
	    addChild( module->m_pButtonGroupMute[ i ] );
        x2 += 28;

        module->m_pButtonGroupSolo[ i ] = new MyLEDButton( x2, y2 + 4, 11, 11, 8.0, DWRGB( 180, 180, 180 ), DWRGB( 0, 255, 255 ), MyLEDButton::TYPE_SWITCH, i, module, Mix_2x4_Stereo_MyLEDButton_GroupSolo );
	    addChild( module->m_pButtonGroupSolo[ i ] );

        // group level and pan inputs
        x2 = x + 79;
        y2 = ybase + 23;

        // group VU Meters
        module->m_pLEDMeterGroup[ i ][ 0 ] = new LEDMeterWidget( x2 + 2, y2 + 21, 5, 2, 1, true );
        addChild( module->m_pLEDMeterGroup[ i ][ 0 ] );
        module->m_pLEDMeterGroup[ i ][ 1 ] = new LEDMeterWidget( x2 + 9, y2 + 21, 5, 2, 1, true );
        addChild( module->m_pLEDMeterGroup[ i ][ 1 ] );

        // group level and pan knobs
        x2 = x + 105;
        y2 = ybase + 17;

        addParam(ParamWidget::create<Knob_Blue2_26>( Vec( x2, y2 ), module, Mix_2x4_Stereo::PARAM_GROUP_LEVEL_IN + i, 0.0, AMP_MAX, 0.0 ) );

        y2 += 32;

        addParam(ParamWidget::create<Knob_Yellow2_26>( Vec( x2, y2 ), module, Mix_2x4_Stereo::PARAM_GROUP_PAN_IN + i, -1.0, 1.0, 0.0 ) );

        // aux 1/3
#define AUX_H 29
        x2 = x + 6;
        y2 = ybase + 20;
        
        module->m_pButtonAuxPreFader[ i ][ 0 ] = new MyLEDButton( x2, y2, 11, 11, 8.0, DWRGB( 180, 180, 180 ), DWRGB( 255, 255, 0 ), MyLEDButton::TYPE_SWITCH, (i * nAUX) + 0, module, Mix_2x4_Stereo_MyLEDButton_Aux );
	    addChild( module->m_pButtonAuxPreFader[ i ][ 0 ] );

        y2 += AUX_H;

        module->m_pButtonAuxPreFader[ i ][ 2 ] = new MyLEDButton( x2, y2, 11, 11, 8.0, DWRGB( 180, 180, 180 ), DWRGB( 255, 255, 0 ), MyLEDButton::TYPE_SWITCH, (i * nAUX) + 2, module, Mix_2x4_Stereo_MyLEDButton_Aux );
	    addChild( module->m_pButtonAuxPreFader[ i ][ 2 ] );

        x2 = x + 20;
        y2 = ybase + 16;
        addParam(ParamWidget::create<Knob_Red1_20>( Vec( x2, y2 ), module, Mix_2x4_Stereo::PARAM_AUX_KNOB + (i * nAUX) + 0, 0.0, AMP_MAX, 0.0 ) );
        y2 += AUX_H;
        addParam(ParamWidget::create<Knob_Blue3_20>( Vec( x2, y2 ), module, Mix_2x4_Stereo::PARAM_AUX_KNOB + (i * nAUX) + 2, 0.0, AMP_MAX, 0.0 ) );

        // aux 2/4
        x2 = x + 38;
        y2 = ybase + 28;
        
        addParam(ParamWidget::create<Knob_Yellow3_20>( Vec( x2, y2 ), module, Mix_2x4_Stereo::PARAM_AUX_KNOB + (i * nAUX) + 1, 0.0, AMP_MAX, 0.0 ) );
        y2 += AUX_H;
        addParam(ParamWidget::create<Knob_Purp1_20>( Vec( x2, y2 ), module, Mix_2x4_Stereo::PARAM_AUX_KNOB + (i * nAUX) + 3, 0.0, AMP_MAX, 0.0 ) );

        x2 = x + 62;
        y2 = ybase + 32;
        
        module->m_pButtonAuxPreFader[ i ][ 1 ] = new MyLEDButton( x2, y2, 11, 11, 8.0, DWRGB( 180, 180, 180 ), DWRGB( 255, 255, 0 ), MyLEDButton::TYPE_SWITCH, (i * nAUX) + 1, module, Mix_2x4_Stereo_MyLEDButton_Aux );
	    addChild( module->m_pButtonAuxPreFader[ i ][ 1 ] );

        y2 += AUX_H;

        module->m_pButtonAuxPreFader[ i ][ 3 ] = new MyLEDButton( x2, y2, 11, 11, 8.0, DWRGB( 180, 180, 180 ), DWRGB( 255, 255, 0 ), MyLEDButton::TYPE_SWITCH, (i * nAUX) + 3, module, Mix_2x4_Stereo_MyLEDButton_Aux );
	    addChild( module->m_pButtonAuxPreFader[ i ][ 3 ] );

        // account for slight error in pixel conversion to svg area
        x += 155;
    }

    // main mixer knob 
    addParam(ParamWidget::create<Knob_Blue2_56>( Vec( 317, 237 ), module, Mix_2x4_Stereo::PARAM_MAIN_LEVEL, 0.0, AMP_MAX, 0.0 ) );

    module->m_pLEDMeterMain[ 0 ] = new LEDMeterWidget( 317 + 58, 242, 5, 3, 2, true );
    addChild( module->m_pLEDMeterMain[ 0 ] );
    module->m_pLEDMeterMain[ 1 ] = new LEDMeterWidget( 317 + 65, 242, 5, 3, 2, true );
    addChild( module->m_pLEDMeterMain[ 1 ] );

    // outputs
    
    addOutput(Port::create<MyPortOutSmall>( Vec( 327, 305 ), Port::OUTPUT, module, Mix_2x4_Stereo::OUT_MAINL ) );
    addOutput(Port::create<MyPortOutSmall>( Vec( 359, 335 ), Port::OUTPUT, module, Mix_2x4_Stereo::OUT_MAINR ) );

    // AUX out
#define AUX_OUT_H 42
    x2 = 340;
    y2 = 25;

    addParam(ParamWidget::create<Knob_Red1_20>( Vec( x2, y2 ), module, Mix_2x4_Stereo::PARAM_AUX_OUT + 0, 0.0, AMP_MAX, 0.0 ) ); y2 += AUX_OUT_H;
    addParam(ParamWidget::create<Knob_Yellow3_20>( Vec( x2, y2  ), module, Mix_2x4_Stereo::PARAM_AUX_OUT + 1, 0.0, AMP_MAX, 0.0 ) ); y2 += AUX_OUT_H;
    addParam(ParamWidget::create<Knob_Blue3_20>( Vec( x2, y2 ), module, Mix_2x4_Stereo::PARAM_AUX_OUT + 2, 0.0, AMP_MAX, 0.0 ) ); y2 += AUX_OUT_H;
    addParam(ParamWidget::create<Knob_Purp1_20>( Vec( x2, y2 ), module, Mix_2x4_Stereo::PARAM_AUX_OUT + 3, 0.0, AMP_MAX, 0.0 ) );

    x2 = 326;
    y2 = 45;
    addOutput(Port::create<MyPortOutSmall>( Vec( x2, y2 ), Port::OUTPUT, module, Mix_2x4_Stereo::OUT_AUXL ) ); y2 += AUX_OUT_H;
    addOutput(Port::create<MyPortOutSmall>( Vec( x2, y2 ), Port::OUTPUT, module, Mix_2x4_Stereo::OUT_AUXL + 1 ) );  y2 += AUX_OUT_H;
    addOutput(Port::create<MyPortOutSmall>( Vec( x2, y2 ), Port::OUTPUT, module, Mix_2x4_Stereo::OUT_AUXL + 2 ) ); y2 += AUX_OUT_H;
    addOutput(Port::create<MyPortOutSmall>( Vec( x2, y2 ), Port::OUTPUT, module, Mix_2x4_Stereo::OUT_AUXL + 3 ) );

    x2 = 355;
    y2 = 45;
    addOutput(Port::create<MyPortOutSmall>( Vec( x2, y2 ), Port::OUTPUT, module, Mix_2x4_Stereo::OUT_AUXR ) ); y2 += AUX_OUT_H;
    addOutput(Port::create<MyPortOutSmall>( Vec( x2, y2 ), Port::OUTPUT, module, Mix_2x4_Stereo::OUT_AUXR + 1 ) ); y2 += AUX_OUT_H;
    addOutput(Port::create<MyPortOutSmall>( Vec( x2, y2 ), Port::OUTPUT, module, Mix_2x4_Stereo::OUT_AUXR + 2 ) ); y2 += AUX_OUT_H;
    addOutput(Port::create<MyPortOutSmall>( Vec( x2, y2 ), Port::OUTPUT, module, Mix_2x4_Stereo::OUT_AUXR + 3 ) );

    // calculate eq rez freq
    fx = 3.141592 * (CUTOFF * 0.026315789473684210526315789473684) * 2 * 3.141592; 
    fx2 = fx*fx;
    fx3 = fx2*fx; 
    fx5 = fx3*fx2; 
    fx7 = fx5*fx2;

    module->m_Freq = 2.0 * (fx 
	    - (fx3 * 0.16666666666666666666666666666667) 
	    + (fx5 * 0.0083333333333333333333333333333333) 
	    - (fx7 * 0.0001984126984126984126984126984127));

    module->m_bInitialized = true;
    module->onReset();
}

//-----------------------------------------------------
// Procedure:   reset
//
//-----------------------------------------------------
void Mix_2x4_Stereo::onReset()
{
    int ch, i, aux;

    if( !m_bInitialized )
        return;

    for( ch = 0; ch < CHANNELS; ch++ )
    {
        m_FadeState[ ch ] = MUTE_FADE_STATE_IDLE;

        m_pButtonChannelMute[ ch ]->Set( false );
        m_pButtonChannelSolo[ ch ]->Set( false );

        m_bMuteStates[ ch ] = false;
        m_bSoloStates[ ch ] = false;
        m_fMuteFade[ ch ] = 1.0;
    }

    for( i = 0; i < GROUPS; i++ )
    {
        for( aux = 0; aux < nAUX; aux++ )
        {
            m_bGroupPreFadeAuxStates[ i ][ aux ] = false;
            m_pButtonAuxPreFader[ i ][ aux ]->Set( false );
        }

        m_GroupFadeState[ i ] = MUTE_FADE_STATE_IDLE;
        m_pButtonGroupMute[ i ]->Set( false );
        m_pButtonGroupSolo[ i ]->Set( false );
        m_bGroupMuteStates[ i ] = false;
        m_fGroupMuteFade[ i ] = 1.0;
    }
}

//-----------------------------------------------------
// Procedure:   
//
//-----------------------------------------------------
json_t *Mix_2x4_Stereo::toJson() 
{
    bool *pbool;
    json_t *gatesJ;
	json_t *rootJ = json_object();

	// channel mutes
    pbool = &m_bMuteStates[ 0 ];

	gatesJ = json_array();

	for (int i = 0; i < CHANNELS; i++)
    {
		json_t *gateJ = json_integer( (int) pbool[ i ] );
		json_array_append_new( gatesJ, gateJ );
	}

	json_object_set_new( rootJ, "channel mutes", gatesJ );

	// channel solos
    pbool = &m_bSoloStates[ 0 ];

	gatesJ = json_array();

	for (int i = 0; i < CHANNELS; i++)
    {
		json_t *gateJ = json_integer( (int) pbool[ i ] );
		json_array_append_new( gatesJ, gateJ );
	}

	json_object_set_new( rootJ, "channel solos", gatesJ );

	// group mutes
    pbool = &m_bGroupMuteStates[ 0 ];

	gatesJ = json_array();

	for (int i = 0; i < GROUPS; i++)
    {
		json_t *gateJ = json_integer( (int) pbool[ i ] );
		json_array_append_new( gatesJ, gateJ );
	}

	json_object_set_new( rootJ, "group mutes", gatesJ );

	// group solos
    pbool = &m_bGroupSoloStates[ 0 ];

	gatesJ = json_array();

	for (int i = 0; i < GROUPS; i++)
    {
		json_t *gateJ = json_integer( (int) pbool[ i ] );
		json_array_append_new( gatesJ, gateJ );
	}

	json_object_set_new( rootJ, "group solos", gatesJ );

	// AUX states
    pbool = &m_bGroupPreFadeAuxStates[ 0 ][ 0 ];

	gatesJ = json_array();

	for (int i = 0; i < GROUPS * nAUX; i++)
    {
		json_t *gateJ = json_integer( (int) pbool[ i ] );
		json_array_append_new( gatesJ, gateJ );
	}

	json_object_set_new( rootJ, "group AUX prefade states", gatesJ );

	return rootJ;
}

//-----------------------------------------------------
// Procedure:   fromJson
//
//-----------------------------------------------------
void Mix_2x4_Stereo::fromJson(json_t *rootJ) 
{
    int ch, i, aux;
    bool *pbool;
    json_t *StepsJ;
    bool bSolo[ GROUPS ] = {0}, bGroupSolo = false;

	// channel mutes
    pbool = &m_bMuteStates[ 0 ];

	StepsJ = json_object_get( rootJ, "channel mutes" );

	if (StepsJ) 
    {
		for ( i = 0; i < CHANNELS; i++)
        {
			json_t *gateJ = json_array_get(StepsJ, i);

			if (gateJ)
				pbool[ i ] = json_integer_value( gateJ );
		}
	}

	// channel solos
    pbool = &m_bSoloStates[ 0 ];

	StepsJ = json_object_get( rootJ, "channel solos" );

	if (StepsJ) 
    {
		for ( i = 0; i < CHANNELS; i++)
        {
			json_t *gateJ = json_array_get(StepsJ, i);

			if (gateJ)
				pbool[ i ] = json_integer_value( gateJ );
		}
	}

	// group mutes
    pbool = &m_bGroupMuteStates[ 0 ];

	StepsJ = json_object_get( rootJ, "group mutes" );

	if (StepsJ) 
    {
		for ( i = 0; i < GROUPS; i++)
        {
			json_t *gateJ = json_array_get(StepsJ, i);

			if (gateJ)
				pbool[ i ] = json_integer_value( gateJ );
		}
	}

	// group solos
    pbool = &m_bGroupSoloStates[ 0 ];

	StepsJ = json_object_get( rootJ, "group solos" );

	if (StepsJ) 
    {
		for ( i = 0; i < GROUPS; i++)
        {
			json_t *gateJ = json_array_get(StepsJ, i);

			if (gateJ)
				pbool[ i ] = json_integer_value( gateJ );
		}
	}

    // AUX states
    pbool = &m_bGroupPreFadeAuxStates[ 0 ][ 0 ];

	StepsJ = json_object_get( rootJ, "group AUX prefade states" );

	if (StepsJ) 
    {
		for ( i = 0; i < GROUPS * nAUX; i++)
        {
			json_t *gateJ = json_array_get(StepsJ, i);

			if (gateJ)
				pbool[ i ] = json_integer_value( gateJ );
		}
	}

    // anybody soloing?
    for( ch = 0; ch < CHANNELS; ch++ )
    {
        if( m_bSoloStates[ ch ] )
        {
            bSolo[ ch / CH_PER_GROUP ]  = true;
        }
    }

    for( ch = 0; ch < CHANNELS; ch++ )
    {
        if( bSolo[ ch / CH_PER_GROUP ] )
        {
            // only open soloing channels
            if( m_bSoloStates[ ch ] )
                m_fMuteFade[ ch ] = 1.0;
            else
                m_fMuteFade[ ch ] = 0.0;
        }
        else
        {
            // nobody is soloing so just open the non muted channels
            m_fMuteFade[ ch ] = m_bMuteStates[ ch ] ? 0.0: 1.0;
        }

        m_pButtonChannelMute[ ch ]->Set( m_bMuteStates[ ch ] );
        m_pButtonChannelSolo[ ch ]->Set( m_bSoloStates[ ch ] );
    }

    // anybody group soloing?
    for( i = 0; i < GROUPS; i++ )
    {
        if( m_bGroupSoloStates[ i ] )
        {
            bGroupSolo  = true;
            break;
        }
    }

    for( i = 0; i < GROUPS; i++ )
    {
        for( aux = 0; aux < nAUX; aux++ )
            m_pButtonAuxPreFader[ i ][ aux ]->Set( m_bGroupPreFadeAuxStates[ i ][ aux ] );

        if( bGroupSolo )
        {
            // only open soloing channels
            if( m_bGroupSoloStates[ i ] )
                m_fGroupMuteFade[ i ] = 1.0;
            else
                m_fGroupMuteFade[ i ] = 0.0;
        }
        else
        {
            // nobody is soloing so just open the non muted channels
            m_fGroupMuteFade[ i ] = m_bGroupMuteStates[ i ] ? 0.0: 1.0;
        }

        m_pButtonGroupMute[ i ]->Set( m_bGroupMuteStates[ i ] );
        m_pButtonGroupSolo[ i ]->Set( m_bGroupSoloStates[ i ] );
    }
}

//-----------------------------------------------------
// Procedure:   ProcessMuteSolo
//
//-----------------------------------------------------
void Mix_2x4_Stereo::ProcessMuteSolo( int index, bool bMute, bool bGroup )
{
    int i, group, si, ei;
    bool bSoloEnabled = false, bSoloOff = false;

    if( bGroup )
    {
        if( bMute )
        {
            m_bGroupMuteStates[ index ] = !m_bGroupMuteStates[ index ];

            // turn solo off
            if( m_bGroupSoloStates[ index ] )
            {
                bSoloOff = true;
                m_bGroupSoloStates[ index ] = false;
                m_pButtonGroupSolo[ index ]->Set( false );
            }

            // if mute is off then set volume
            if( m_bGroupMuteStates[ index ] )
            {
                m_pButtonGroupMute[ index ]->Set( true );
                m_GroupFadeState[ index ] = MUTE_FADE_STATE_DEC;
            }
            else
            {
                m_pButtonGroupMute[ index ]->Set( false );
                m_GroupFadeState[ index ] = MUTE_FADE_STATE_INC;
            }
        }
        else
        {
            m_bGroupSoloStates[ index ] = !m_bGroupSoloStates[ index ];

            // turn mute off
            if( m_bGroupMuteStates[ index ] )
            {
                m_bGroupMuteStates[ index ] = false;
                m_pButtonGroupMute[ index ]->Set( false );
            }

            // shut down volume of all groups not in solo
            if( !m_bGroupSoloStates[ index ] )
            {
                bSoloOff = true;
                m_pButtonGroupSolo[ index ]->Set( false );
            }
            else
            {
                m_pButtonGroupSolo[ index ]->Set( true );
            }
        }

        // is a track soloing?
        for( i = 0; i < GROUPS; i++ )
        {
            if( m_bGroupSoloStates[ i ] )
            {
                bSoloEnabled = true;
                break;
            }
        }

        if( bSoloEnabled )
        {
            // process solo
            for( i = 0; i < GROUPS; i++ )
            {
                // shut down volume of all groups not in solo
                if( !m_bGroupSoloStates[ i ] )
                {
                    m_GroupFadeState[ i ] = MUTE_FADE_STATE_DEC;
                }
                else
                {
                    m_GroupFadeState[ i ] = MUTE_FADE_STATE_INC;
                }
            }
        }
        // nobody soloing and just turned solo off then enable all channels that aren't muted
        else if( bSoloOff )
        {
            // process solo
            for( i = 0; i < GROUPS; i++ )
            {
                // bring back if not muted
                if( !m_bGroupMuteStates[ i ] )
                {
                    m_GroupFadeState[ i ] = MUTE_FADE_STATE_INC;
                }
            }
        }
    }
    // !bGroup
    else
    {
        group = index / CH_PER_GROUP;

        si = group * CH_PER_GROUP;
        ei = si + CH_PER_GROUP;
        
        if( bMute )
        {
            m_bMuteStates[ index ] = !m_bMuteStates[ index ];

            // turn solo off
            if( m_bSoloStates[ index ] )
            {
                bSoloOff = true;
                m_bSoloStates[ index ] = false;
                m_pButtonChannelSolo[ index ]->Set( false );
            }

            // if mute is off then set volume
            if( m_bMuteStates[ index ] )
            {
                m_pButtonChannelMute[ index ]->Set( true );
                m_FadeState[ index ] = MUTE_FADE_STATE_DEC;
            }
            else
            {
                m_pButtonChannelMute[ index ]->Set( false );
                m_FadeState[ index ] = MUTE_FADE_STATE_INC;
            }
        }
        else
        {
            m_bSoloStates[ index ] = !m_bSoloStates[ index ];

            // turn mute off
            if( m_bMuteStates[ index ] )
            {
                m_bMuteStates[ index ] = false;
                m_pButtonChannelMute[ index ]->Set( false );
            }

            // toggle solo
            if( !m_bSoloStates[ index ] )
            {
                bSoloOff = true;
                m_pButtonChannelSolo[ index ]->Set( false );
            }
            else
            {
                m_pButtonChannelSolo[ index ]->Set( true );
            }
        }

        // is a track soloing?
        for( i = si; i < ei; i++ )
        {
            if( m_bSoloStates[ i ] )
            {
                bSoloEnabled = true;
                break;
            }
        }

        if( bSoloEnabled )
        {
            // process solo
            for( i = si; i < ei; i++ )
            {
                // shut down volume of all not in solo
                if( !m_bSoloStates[ i ] )
                {
                    m_FadeState[ i ] = MUTE_FADE_STATE_DEC;
                }
                else
                {
                    m_FadeState[ i ] = MUTE_FADE_STATE_INC;
                }
            }
        }
        // nobody soloing and just turned solo off then enable all channels that aren't muted
        else if( bSoloOff )
        {
            // process solo
            for( i = si; i < ei; i++ )
            {
                // bring back if not muted
                if( !m_bMuteStates[ i ] )
                {
                    m_FadeState[ i ] = MUTE_FADE_STATE_INC;
                }
            }
        }
    }
}

//-----------------------------------------------------
// Procedure:   ProcessEQ
//
//-----------------------------------------------------
#define MULTI (0.33333333333333333333333333333333f)
void Mix_2x4_Stereo::ProcessEQ( int ch, float *pL, float *pR )
{
    float rez, hp1; 
    float input[ 2 ], out[ 2 ], lowpass, bandpass, highpass;

    input[ L ] = *pL / AUDIO_MAX;
    input[ R ] = *pR / AUDIO_MAX;

    rez = 1.00;

    // do left and right channels
    for( int i = 0; i < 2; i++ )
    {
        input[ i ] = input[ i ] + 0.000000001;

        lp1[ ch ][ i ] = lp1[ ch ][ i ] + m_Freq * bp1[ ch ][ i ]; 
        hp1 = input[ i ] - lp1[ ch ][ i ] - rez * bp1[ ch ][ i ]; 
        bp1[ ch ][ i ] = m_Freq * hp1 + bp1[ ch ][ i ]; 
        lowpass  = lp1[ ch ][ i ]; 
        highpass = hp1; 
        bandpass = bp1[ ch ][ i ]; 

        lp1[ ch ][ i ] = lp1[ ch ][ i ] + m_Freq * bp1[ ch ][ i ]; 
        hp1 = input[ i ] - lp1[ ch ][ i ] - rez * bp1[ ch ][ i ]; 
        bp1[ ch ][ i ] = m_Freq * hp1 + bp1[ ch ][ i ]; 
        lowpass  = lowpass  + lp1[ ch ][ i ]; 
        highpass = highpass + hp1; 
        bandpass = bandpass + bp1[ ch ][ i ]; 

        input[ i ] = input[ i ] - 0.000000001;
        lp1[ ch ][ i ] = lp1[ ch ][ i ] + m_Freq * bp1[ ch ][ i ]; 
        hp1 = input[ i ] - lp1[ ch ][ i ] - rez * bp1[ ch ][ i ]; 
        bp1[ ch ][ i ] = m_Freq * hp1 + bp1[ ch ][ i ]; 

        lowpass  = (lowpass  + lp1[ ch ][ i ]) * MULTI; 
        highpass = (highpass + hp1) * MULTI; 
        bandpass = (bandpass + bp1[ ch ][ i ]) * MULTI;

        out[ i ] = ( highpass * m_hpIn[ ch ] ) + ( lowpass * m_lpIn[ ch ] ) + ( bandpass * m_mpIn[ ch ] );
    }

    *pL = clamp( out[ L ] * AUDIO_MAX, -AUDIO_MAX, AUDIO_MAX );
    *pR = clamp( out[ R ] * AUDIO_MAX, -AUDIO_MAX, AUDIO_MAX );
}

//-----------------------------------------------------
// Procedure:   step
//
//-----------------------------------------------------
void Mix_2x4_Stereo::step() 
{
    int ch, group, aux;
    float inL = 0.0, inR = 0.0, inLClean, inRClean, outL, outR, mainL = 0.0, mainR = 0.0;
    float inLvl, inPan;
    float auxL[ nAUX ] = {}, auxR[ nAUX ] = {};
    bool bGroupActive[ GROUPS ] = {0};

    if( !m_bInitialized )
        return;

    memset( m_fSubMix, 0, sizeof(m_fSubMix) );

    // channel mixers
	for ( ch = 0; ch < CHANNELS; ch++ ) 
    {
        group = ch / CH_PER_GROUP;

        inLClean = 0.0;
        inRClean = 0.0;
        inL = 0.0;
        inR = 0.0;

        if( inputs[ IN_RIGHT + ch ].active || inputs[ IN_LEFT + ch ].active )
        {
            inLvl = clamp( ( params[ PARAM_LEVEL_IN + ch ].value + ( inputs[ IN_LEVEL + ch ].normalize( 0.0f ) / CV_MAX ) ), 0.0f, AMP_MAX ); 

            bGroupActive[ group ] = true;

            // check right channel first for possible mono
            if( inputs[ IN_RIGHT + ch ].active )
            {
                inRClean = inputs[ IN_RIGHT + ch ].value;
                inR = inRClean * inLvl;
                m_bMono[ ch ] = false;
            }
            else
                m_bMono[ ch ] = true;

            // left channel
            if( inputs[ IN_LEFT + ch ].active )
            {
                inLClean = inputs[ IN_LEFT + ch ].value;
                inL = inLClean * inLvl; 

                if( m_bMono[ ch ] )
                {
                    inRClean = inLClean;
                    inR = inL;
                }
            }

            // put output to aux if pre fader
            for ( aux = 0; aux < nAUX; aux++ )
            {
                if( m_bGroupPreFadeAuxStates[ group ][ aux ] )
                {
                    auxL[ aux ] += inLClean * params[ PARAM_AUX_KNOB + (group * nAUX) + aux ].value;
                    auxR[ aux ] += inRClean * params[ PARAM_AUX_KNOB + (group * nAUX) + aux ].value;
                }
            }

            if( m_FadeState[ ch ] == MUTE_FADE_STATE_DEC )
            {
                m_fMuteFade[ ch ] -= FADE_MULT;

                if( m_fMuteFade[ ch ] <= 0.0 )
                {
                    m_fMuteFade[ ch ] = 0.0;
                    m_FadeState[ ch ] = MUTE_FADE_STATE_IDLE;
                }
            }
            else if( m_FadeState[ ch ] == MUTE_FADE_STATE_INC )
            {
                m_fMuteFade[ ch ] += FADE_MULT;

                if( m_fMuteFade[ ch ] >= 1.0 )
                {
                    m_fMuteFade[ ch ] = 1.0;
                    m_FadeState[ ch ] = MUTE_FADE_STATE_IDLE;
                }
            }

            ProcessEQ( ch, &inL, &inR );

            inL *= m_fMuteFade[ ch ];
            inR *= m_fMuteFade[ ch ];

            // pan
            inPan = clamp( params[ PARAM_PAN_IN + ch ].value + ( inputs[ IN_PAN + ch ].normalize( 0.0f ) / CV_MAX ), -1.0f, 1.0f );

            //lg.f("pan = %.3f\n", inputs[ IN_PAN + ch ].value );

            if( inPan <= 0.0 )
                inR *= ( 1.0 + inPan );
            else
                inL *= ( 1.0 - inPan );

            // put output to aux if not pre fader
            for ( aux = 0; aux < nAUX; aux++ )
            {
                if( !m_bGroupPreFadeAuxStates[ group ][ aux ] )
                {
                    auxL[ aux ] += inL * params[ PARAM_AUX_KNOB + (group * nAUX) + aux ].value;
                    auxR[ aux ] += inR * params[ PARAM_AUX_KNOB + (group * nAUX) + aux ].value;
                }
            }
        }
        // this channel not active
        else
        {

        }

        m_fSubMix[ group ][ L ] += inL;
        m_fSubMix[ group ][ R ] += inR;

        if( m_pLEDMeterChannel[ ch ][ 0 ] )
            m_pLEDMeterChannel[ ch ][ 0 ]->Process( inL / AUDIO_MAX );
        if( m_pLEDMeterChannel[ ch ][ 1 ] )
            m_pLEDMeterChannel[ ch ][ 1 ]->Process( inR / AUDIO_MAX);
    }

    // group mixers
	for ( group = 0; group < GROUPS; group++ ) 
    {
        outL = 0.0;
        outR = 0.0;

        if( bGroupActive[ group ] )
        {
            inLvl = clamp( ( params[ PARAM_GROUP_LEVEL_IN + group ].value + ( inputs[ IN_GROUP_LEVEL + group ].normalize( 0.0f ) / CV_MAX ) ), 0.0f, AMP_MAX ); 

            outL = m_fSubMix[ group ][ L ] * inLvl;
            outR = m_fSubMix[ group ][ R ] * inLvl;

            // pan
            inPan = clamp( params[ PARAM_GROUP_PAN_IN + group ].value + ( inputs[ IN_GROUP_PAN + group ].normalize( 0.0f ) / CV_MAX ), -1.0f, 1.0f );

            if( inPan <= 0.0 )
                outR *= ( 1.0 + inPan );
            else
                outL *= ( 1.0 - inPan );

            if( m_GroupFadeState[ group ] == MUTE_FADE_STATE_DEC )
            {
                m_fGroupMuteFade[ group ] -= FADE_MULT;

                if( m_fGroupMuteFade[ group ] <= 0.0 )
                {
                    m_fGroupMuteFade[ group ] = 0.0;
                    m_GroupFadeState[ group ] = MUTE_FADE_STATE_IDLE;
                }
            }
            else if( m_GroupFadeState[ group ] == MUTE_FADE_STATE_INC )
            {
                m_fGroupMuteFade[ group ] += FADE_MULT;

                if( m_fGroupMuteFade[ group ] >= 1.0 )
                {
                    m_fGroupMuteFade[ group ] = 1.0;
                    m_GroupFadeState[ group ] = MUTE_FADE_STATE_IDLE;
                }
            }

            outL *= m_fGroupMuteFade[ group ];
            outR *= m_fGroupMuteFade[ group ];
        }

        if( m_pLEDMeterGroup[ group ][ 0 ] )
            m_pLEDMeterGroup[ group ][ 0 ]->Process( outL / AUDIO_MAX );
        if( m_pLEDMeterGroup[ group ][ 1 ] )
            m_pLEDMeterGroup[ group ][ 1 ]->Process( outR / AUDIO_MAX );

        mainL += outL;
        mainR += outR;
    }

    if( m_pLEDMeterMain[ 0 ] )
        m_pLEDMeterMain[ 0 ]->Process( ( mainL / AUDIO_MAX )  * params[ PARAM_MAIN_LEVEL ].value );
    if( m_pLEDMeterMain[ 1 ] )
        m_pLEDMeterMain[ 1 ]->Process( ( mainR / AUDIO_MAX ) * params[ PARAM_MAIN_LEVEL ].value );

    // put aux output
    for ( aux = 0; aux < nAUX; aux++ )
    {
        outputs[ OUT_AUXL + aux ].value = clamp( auxL[ aux ] * params[ PARAM_AUX_OUT + aux ].value, -AUDIO_MAX, AUDIO_MAX );
        outputs[ OUT_AUXR + aux ].value = clamp( auxR[ aux ] * params[ PARAM_AUX_OUT + aux ].value, -AUDIO_MAX, AUDIO_MAX );
    }

    outputs[ OUT_MAINL ].value = clamp( mainL * params[ PARAM_MAIN_LEVEL ].value, -AUDIO_MAX, AUDIO_MAX );
    outputs[ OUT_MAINR ].value = clamp( mainR * params[ PARAM_MAIN_LEVEL ].value, -AUDIO_MAX, AUDIO_MAX );
}

} // namespace rack_plugin_mscHack

using namespace rack_plugin_mscHack;

RACK_PLUGIN_MODEL_INIT(mscHack, Mix_2x4_Stereo) {
   Model *modelMix_2x4_Stereo = Model::create<Mix_2x4_Stereo, Mix_2x4_Stereo_Widget>( "mscHack", "Mix_2x4_Stereo", "MIXER 2x4 Stereo/Mono", MIXER_TAG, EQUALIZER_TAG, DUAL_TAG, PANNING_TAG, AMPLIFIER_TAG, MULTIPLE_TAG );
   return modelMix_2x4_Stereo;
}
