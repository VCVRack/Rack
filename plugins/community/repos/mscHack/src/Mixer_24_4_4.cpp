#include "mscHack.hpp"

namespace rack_plugin_mscHack {

#define nCHANNELS 32
#define nINCHANNELS 24
#define nGROUPS   4
#define nAUX      4
#define nEQ       3

#define FADE_MULT (0.0005f)
#define CUTOFF (0.025f)

#define L 0
#define R 1

#define MUTE_FADE_STATE_IDLE 0
#define MUTE_FADE_STATE_INC  1
#define MUTE_FADE_STATE_DEC  2

//-----------------------------------------------------
// Module Definition
//
//-----------------------------------------------------
struct Mixer_24_4_4 : Module 
{
	enum ParamIds 
    {
        PARAM_LEVEL_OUT,
        PARAM_CHLVL,
        PARAM_CHPAN             = PARAM_CHLVL + nCHANNELS,
        PARAM_CHEQHI            = PARAM_CHPAN + nCHANNELS,
        PARAM_CHEQMD            = PARAM_CHEQHI + nCHANNELS,
        PARAM_CHEQLO            = PARAM_CHEQMD + nCHANNELS,
        PARAM_CHAUX             = PARAM_CHEQLO + nCHANNELS,
        PARAM_AUXLVL            = PARAM_CHAUX + ( (nCHANNELS - 4) * nAUX ),
        nPARAMS                 = PARAM_AUXLVL + nAUX,
    };

	enum InputIds 
    {
        IN_LEFT,
        IN_RIGHT                = IN_LEFT + nCHANNELS,
        IN_LEVELCV              = IN_RIGHT + nCHANNELS,
        IN_PANCV                = IN_LEVELCV + nINCHANNELS,
        IN_FADEX                = IN_PANCV + nINCHANNELS,
        IN_FADEY,
        nINPUTS
	};

	enum OutputIds 
    {
		OUT_MAINL,
        OUT_MAINR,

        OUT_GRPL,
        OUT_GRPR                = OUT_GRPL + nGROUPS,

        OUT_AUXL                = OUT_GRPR + nGROUPS,
        OUT_AUXR                = OUT_AUXL + nAUX,

        nOUTPUTS                = OUT_AUXR + nAUX
	};

	enum LightIds 
    {
        nLIGHTS
	};

    bool            m_bInitialized = false;
    CLog            lg;

    // Contructor
	Mixer_24_4_4() : Module(nPARAMS, nINPUTS, nOUTPUTS, nLIGHTS){}

    // mute/solo
    bool            m_bMuteStates[ nCHANNELS ] = {};
    float           m_fMuteFade[ nCHANNELS ] = {};
    int             m_FadeState[ nCHANNELS ] = {MUTE_FADE_STATE_IDLE};
    bool            m_bSoloStates[ nCHANNELS ] = {};
    bool            m_bPreFader[ nINCHANNELS + nGROUPS ] = {};

    // processing
    bool            m_bMono[ nCHANNELS ];

    // LED Meters
    LEDMeterWidget  *m_pLEDMeterChannel[ nCHANNELS ][ 2 ] ={};
    LEDMeterWidget  *m_pLEDMeterMain[ 2 ] ={};

    // EQ Rez
    float           lp1[ nCHANNELS ][ 2 ] = {}, bp1[ nCHANNELS ][ 2 ] = {}; 
    float           m_hpIn[ nCHANNELS ];
    float           m_lpIn[ nCHANNELS ];
    float           m_mpIn[ nCHANNELS ];
    float           m_Freq;

    // buttons
    MyLEDButton            *m_pButtonChannelMute[ nCHANNELS ] = {};
    MyLEDButton            *m_pButtonChannelSolo[ nCHANNELS ] = {};
    MyLEDButton            *m_pButtonPreFader[ nCHANNELS ] = {};

    // routing
    int                     m_iRouteGroup[ nINCHANNELS ] = {nGROUPS};
    MyLEDButtonStrip       *m_pMultiButtonRoute[ nINCHANNELS ] = {0};

    bool                    m_bGroupPreMute = true;
    bool                    m_bGainLevelx2 = false;

    // Overrides 
	void    step() override;
    void    JsonParams( bool bTo, json_t *root);
    json_t* toJson() override;
    void    fromJson(json_t *rootJ) override;
    void    onRandomize() override;
    void    onReset() override;
    void    onCreate() override;
    void    onDelete() override;
    
    void ProcessMuteSolo( int channel, bool bMute, bool bGroup, bool bOn );
    void ProcessEQ( int ch, float *pL, float *pR );
    void SetControls( int ch );

    //-----------------------------------------------------
    // MyEQHi_Knob
    //-----------------------------------------------------
    struct MyEQHi_Knob : Knob_Green1_15
    {
        Mixer_24_4_4 *mymodule;
        int param;

        void onChange( EventChange &e ) override 
        {
            mymodule = (Mixer_24_4_4*)module;

            if( mymodule )
            {
                param = paramId - Mixer_24_4_4::PARAM_CHEQHI;

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
        Mixer_24_4_4 *mymodule;
        int param;

        void onChange( EventChange &e ) override 
        {
            mymodule = (Mixer_24_4_4*)module;

            if( mymodule )
            {
                param = paramId - Mixer_24_4_4::PARAM_CHEQMD;
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
        Mixer_24_4_4 *mymodule;
        int param;

        void onChange( EventChange &e ) override 
        {
            mymodule = (Mixer_24_4_4*)module;

            if( mymodule )
            {
                param = paramId - Mixer_24_4_4::PARAM_CHEQLO;
                mymodule->m_lpIn[ param ] = value; 
            }

		    RoundKnob::onChange( e );
	    }
    };
};

//-----------------------------------------------------
// MyLEDButton_ChSolo
//-----------------------------------------------------
void Button_ChSolo( void *pClass, int id, bool bOn ) 
{
    Mixer_24_4_4 *mymodule;
    mymodule = (Mixer_24_4_4*)pClass;
    mymodule->ProcessMuteSolo( id, false, false, bOn );
}

//-----------------------------------------------------
// MyLEDButton_ChMute
//-----------------------------------------------------
void Button_ChMute( void *pClass, int id, bool bOn ) 
{
    Mixer_24_4_4 *mymodule;
    mymodule = (Mixer_24_4_4*)pClass;
    mymodule->ProcessMuteSolo( id, true, false, bOn );
}

//-----------------------------------------------------
// Button_ChPreFader
//-----------------------------------------------------
void Button_ChPreFader( void *pClass, int id, bool bOn ) 
{
    Mixer_24_4_4 *mymodule;
    mymodule = (Mixer_24_4_4*)pClass;
    mymodule->m_bPreFader[ id ] = bOn;
}

//-----------------------------------------------------
// Procedure:   RouteCallback
//-----------------------------------------------------
void RouteCallback( void *pClass, int id, int nbutton, bool bOn ) 
{
    Mixer_24_4_4 *mymodule;
    mymodule = (Mixer_24_4_4*)pClass;

    if( mymodule )
    {
        mymodule->m_iRouteGroup[ id ] = nbutton;
    }
}

//-----------------------------------------------------
// Procedure:   Widget
//
//-----------------------------------------------------

struct Mixer_24_4_4_Widget : ModuleWidget 
{
    Menu *createContextMenu() override;
	Mixer_24_4_4_Widget( Mixer_24_4_4 *module );
};

Mixer_24_4_4_Widget::Mixer_24_4_4_Widget( Mixer_24_4_4 *module ) : ModuleWidget(module) 
{
    Port *pPort;
    float fx, fx2, fx3, fx5, fx7;
    int ch, x, y, x2, y2;
    bool bGroup, bAux, bNormal;

	box.size = Vec( 15*57, 380);

	{
		SVGPanel *panel = new SVGPanel();
		panel->box.size = box.size;
		panel->setBackground(SVG::load(assetPlugin(plugin, "res/Mixer_24_4_4.svg")));
		addChild(panel);
	}

    //module->lg.Open("Mixer_24_4_4.txt");

    x = 47;
    y = 13;

    for( ch = 0; ch < nCHANNELS; ch++ )
    {
        bGroup = false;
        bAux   = false;
        bNormal= false;

        // groups
        if( ch < nINCHANNELS )
            bNormal = true;
        else if( ch >= nINCHANNELS && ch < ( nINCHANNELS + nGROUPS) )
            bGroup = true;
        else
            bAux = true;

        x2 = x + 3;
        y2 = y + 8;

        // inputs
        pPort = Port::create<MyPortInSmall>( Vec( x2, y2 ), Port::INPUT, module, Mixer_24_4_4::IN_LEFT + ch );
        addInput( pPort ); y2 += 23;

        if( bGroup )
            pPort->visible = false;

        pPort = Port::create<MyPortInSmall>( Vec( x2, y2 ), Port::INPUT, module, Mixer_24_4_4::IN_RIGHT + ch );
        addInput( pPort );

        if( bGroup )
            pPort->visible = false;

        x2 = x + 4;
        y2 += 20;

        // aux sends
        if( !bAux )
        {
            addParam(ParamWidget::create<Knob_Red1_15>( Vec( x2, y2 ), module, Mixer_24_4_4::PARAM_CHAUX + (ch * 4) + 0, 0.0, 1.0, 0.0 ) );
            y2 += 17;
            addParam(ParamWidget::create<Knob_Yellow3_15>( Vec( x2, y2 ), module, Mixer_24_4_4::PARAM_CHAUX + (ch * 4) + 1, 0.0, 1.0, 0.0 ) );
            y2 += 17;
            addParam(ParamWidget::create<Knob_Blue3_15>( Vec( x2, y2 ), module, Mixer_24_4_4::PARAM_CHAUX + (ch * 4) + 2, 0.0, 1.0, 0.0 ) );
            y2 += 17;
            addParam(ParamWidget::create<Knob_Purp1_15>( Vec( x2, y2 ), module, Mixer_24_4_4::PARAM_CHAUX + (ch * 4) + 3, 0.0, 1.0, 0.0 ) );

            module->m_pButtonPreFader[ ch ] = new MyLEDButton( x2 - 3, y2 + 15, 7, 7, 5.0f, DWRGB( 180, 180, 180 ), DWRGB( 255, 255, 255 ), MyLEDButton::TYPE_SWITCH, ch, module, Button_ChPreFader );
	        addChild( module->m_pButtonPreFader[ ch ] );

            y2 += 24;
        }
        else
        {
            switch( ch )
            {
            case 28:
                addParam(ParamWidget::create<Knob_Red1_15>( Vec( x2, y2 ), module, Mixer_24_4_4::PARAM_AUXLVL + 0, 0.0, 1.0, 0.0 ) );
                break;
            case 29:
                addParam(ParamWidget::create<Knob_Yellow3_15>( Vec( x2, y2 ), module, Mixer_24_4_4::PARAM_AUXLVL + 1, 0.0, 1.0, 0.0 ) );
                break;
            case 30:
                addParam(ParamWidget::create<Knob_Blue3_15>( Vec( x2, y2 ), module, Mixer_24_4_4::PARAM_AUXLVL + 2, 0.0, 1.0, 0.0 ) );
                break;
            case 31:
                addParam(ParamWidget::create<Knob_Purp1_15>( Vec( x2, y2 ), module, Mixer_24_4_4::PARAM_AUXLVL + 3, 0.0, 1.0, 0.0 ) );
                break;
            }

            addOutput(Port::create<MyPortOutSmall>( Vec( x2 - 1, y2 + 22), Port::OUTPUT, module, Mixer_24_4_4::OUT_AUXL + (ch - 28) ) );
            addOutput(Port::create<MyPortOutSmall>( Vec( x2 - 1, y2 + 47), Port::OUTPUT, module, Mixer_24_4_4::OUT_AUXR + (ch - 28) ) );

            y2 += 75;
        }

        // EQ
        addParam(ParamWidget::create<Mixer_24_4_4::MyEQHi_Knob>( Vec( x2, y2 ), module, Mixer_24_4_4::PARAM_CHEQHI + ch, 0.0, 1.0, 0.5 ) );
        y2 += 18;
        addParam(ParamWidget::create<Mixer_24_4_4::MyEQMid_Knob>( Vec( x2, y2 ), module, Mixer_24_4_4::PARAM_CHEQMD + ch, 0.0, 1.0, 0.5 ) );
        y2 += 18;
        addParam(ParamWidget::create<Mixer_24_4_4::MyEQLo_Knob>( Vec( x2, y2 ), module, Mixer_24_4_4::PARAM_CHEQLO + ch, 0.0, 1.0, 0.5 ) );
        y2 += 20;

        // CVs
        if( bNormal )
        {
            addInput(Port::create<MyPortInSmall>( Vec( x2 - 1, y2 ), Port::INPUT, module, Mixer_24_4_4::IN_LEVELCV + ch ) );
            y2 += 25;
            addInput(Port::create<MyPortInSmall>( Vec( x2 - 1, y2 ), Port::INPUT, module, Mixer_24_4_4::IN_PANCV + ch ) );
        }
        // group outs
        else if( bGroup )
        {
            addOutput(Port::create<MyPortOutSmall>( Vec( x2 - 1, y2 ), Port::OUTPUT, module, Mixer_24_4_4::OUT_GRPL + (ch - nINCHANNELS) ) );
            y2 += 25;
            addOutput(Port::create<MyPortOutSmall>( Vec( x2 - 1, y2 ), Port::OUTPUT, module, Mixer_24_4_4::OUT_GRPR + (ch - nINCHANNELS) ) );
        }
        else
            y2 += 25;

        y2 += 23;

        addParam(ParamWidget::create<Knob_Yellow1_15>( Vec( x2, y2 ), module, Mixer_24_4_4::PARAM_CHPAN + ch, -1.0, 1.0, 0.0 ) );

        y2 += 19;

        // mute/solo
        if( bNormal )
        {
            module->m_pButtonChannelMute[ ch ] = new MyLEDButton( x + 3, y2, 8, 8, 6.0f, DWRGB( 180, 180, 180 ), DWRGB( 255, 0, 0 ), MyLEDButton::TYPE_SWITCH, ch, module, Button_ChMute );
	        addChild( module->m_pButtonChannelMute[ ch ] );

            module->m_pButtonChannelSolo[ ch ] = new MyLEDButton( x + 12, y2, 8, 8, 6.0f, DWRGB( 180, 180, 180 ), DWRGB( 0, 255, 0 ), MyLEDButton::TYPE_SWITCH, ch, module, Button_ChSolo );
	        addChild( module->m_pButtonChannelSolo[ ch ] );
        }
        // mute only
        else
        {
            module->m_pButtonChannelMute[ ch ] = new MyLEDButton( x + 7, y2, 8, 8, 6.0f, DWRGB( 180, 180, 180 ), DWRGB( 255, 0, 0 ), MyLEDButton::TYPE_SWITCH, ch, module, Button_ChMute );
	        addChild( module->m_pButtonChannelMute[ ch ] );
        }

        // VUMeter
        module->m_pLEDMeterChannel[ ch ][ 0 ] = new LEDMeterWidget( x + 7, y + 260, 4, 1, 1, true );
        addChild( module->m_pLEDMeterChannel[ ch ][ 0 ] );
        module->m_pLEDMeterChannel[ ch ][ 1 ] = new LEDMeterWidget( x + 12, y + 260, 4, 1, 1, true );
        addChild( module->m_pLEDMeterChannel[ ch ][ 1 ] );

        x2 = x + 2;
        y2 = y + 290;

        // Group Route
        if( bNormal )
        {
            module->m_pMultiButtonRoute[ ch ] = new MyLEDButtonStrip( x2, y2, 6, 6, 3, 4.0f, 5, true, DWRGB( 0, 128, 128 ), DWRGB( 0, 255, 255 ), MyLEDButtonStrip::TYPE_EXCLUSIVE, ch, module, RouteCallback );
	        addChild( module->m_pMultiButtonRoute[ ch ] );
        }

        // level slider
        addParam(ParamWidget::create<Slider02_10x15>( Vec( x + 10, y2 - 8 ), module, Mixer_24_4_4::PARAM_CHLVL + ch, 0.0, 1.0, 0.0 ) );

        x += 23;
    }

    // output
    addParam(ParamWidget::create<Knob_Blue2_56>( Vec( 788, 256 ), module, Mixer_24_4_4::PARAM_LEVEL_OUT, 0.0, 1.0, 0.5 ) );

    module->m_pLEDMeterMain[ 0 ] = new LEDMeterWidget( 798, 315, 5, 3, 2, true );
    addChild( module->m_pLEDMeterMain[ 0 ] );
    module->m_pLEDMeterMain[ 1 ] = new LEDMeterWidget( 798 + 7, 315, 5, 3, 2, true );
    addChild( module->m_pLEDMeterMain[ 1 ] );

    // main outputs
    addOutput(Port::create<MyPortOutSmall>( Vec( 821, 319 ), Port::OUTPUT, module, Mixer_24_4_4::OUT_MAINL ) );
    addOutput(Port::create<MyPortOutSmall>( Vec( 821, 344 ), Port::OUTPUT, module, Mixer_24_4_4::OUT_MAINR ) );

    // xfade inputs
    addInput(Port::create<MyPortInSmall>( Vec( 833, 203 ), Port::INPUT, module, Mixer_24_4_4::IN_FADEX ) );
    addInput(Port::create<MyPortInSmall>( Vec( 833, 230 ), Port::INPUT, module, Mixer_24_4_4::IN_FADEY ) );

    //screw
	addChild(Widget::create<ScrewSilver>(Vec(15, 0)));
	addChild(Widget::create<ScrewSilver>(Vec(box.size.x-30, 0)));
	addChild(Widget::create<ScrewSilver>(Vec(15, 365))); 
	addChild(Widget::create<ScrewSilver>(Vec(box.size.x-30, 365)));

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
// Procedure: JsonParams  
//
//-----------------------------------------------------
void Mixer_24_4_4::JsonParams( bool bTo, json_t *root) 
{
    JsonDataBool( bTo, "m_bMuteStates", root, m_bMuteStates, 32 );
    JsonDataBool( bTo, "m_bSoloStates", root, m_bSoloStates, 32 );
    JsonDataInt( bTo, "m_iRouteGroup", root, &m_iRouteGroup[ 0 ], nINCHANNELS );
    JsonDataBool( bTo, "m_bGroupPreMute", root, &m_bGroupPreMute, 1 );
    JsonDataBool( bTo, "m_bGainLevelx2", root, &m_bGainLevelx2, 1 );
    JsonDataBool( bTo, "m_bPreFader", root, m_bPreFader, nINCHANNELS + nGROUPS );
}

//-----------------------------------------------------
// Procedure: toJson  
//
//-----------------------------------------------------
json_t *Mixer_24_4_4::toJson() 
{
	json_t *root = json_object();

    if( !root )
        return NULL;

    JsonParams( TOJSON, root );
    
	return root;
}

//-----------------------------------------------------
// Procedure:   fromJson
//
//-----------------------------------------------------
void Mixer_24_4_4::fromJson( json_t *root ) 
{
    int ch;
    bool bSolo = false;

    JsonParams( FROMJSON, root );

    // anybody soloing?
    for( ch = 0; ch < nCHANNELS; ch++ )
    {
        if( m_bSoloStates[ ch ] )
            bSolo = true;
    }

    for( ch = 0; ch < nCHANNELS; ch++ )
    {
        if( bSolo )
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

        SetControls( ch );
    }
}

//-----------------------------------------------------
// Procedure:   SetControls
//
//-----------------------------------------------------
void Mixer_24_4_4::SetControls( int ch )
{
    if( !m_bInitialized || ch >= nCHANNELS || ch < 0 )
        return;

    //lg.f("Here\n");

    if( m_pButtonChannelMute[ ch ] )
        m_pButtonChannelMute[ ch ]->Set( m_bMuteStates[ ch ] );

    if( m_pButtonChannelSolo[ ch ] )
        m_pButtonChannelSolo[ ch ]->Set( m_bSoloStates[ ch ] );

    if( ch < nINCHANNELS && m_pMultiButtonRoute[ ch ] )
        m_pMultiButtonRoute[ ch ]->Set( m_iRouteGroup[ ch ], true );

    if( ch < (nINCHANNELS + nGROUPS ) )
        m_pButtonPreFader[ ch ]->Set( m_bPreFader[ ch ] );
}

//-----------------------------------------------------
// Procedure:   onCreate
//
//-----------------------------------------------------
void Mixer_24_4_4::onCreate()
{
}

//-----------------------------------------------------
// Procedure:   onDelete
//
//-----------------------------------------------------
void Mixer_24_4_4::onDelete()
{
}

//-----------------------------------------------------
// Procedure:   onReset
//
//-----------------------------------------------------
void Mixer_24_4_4::onReset()
{
    int ch;

    if( !m_bInitialized )
        return;

    for( ch = 0; ch < nCHANNELS; ch++ )
    {
        m_FadeState[ ch ] = MUTE_FADE_STATE_IDLE;

        m_bMuteStates[ ch ] = false;
        m_bSoloStates[ ch ] = false;
        m_fMuteFade[ ch ] = 1.0f;

        if( ch < nINCHANNELS )
            m_iRouteGroup[ ch ] = nGROUPS;

        SetControls( ch );
    }
}

//-----------------------------------------------------
// Procedure:   onRandomize
//
//-----------------------------------------------------
void Mixer_24_4_4::onRandomize()
{
}

//-----------------------------------------------------
// Procedure:   ProcessEQ
//
//-----------------------------------------------------
#define MULTI (0.33333333333333333333333333333333f)
void Mixer_24_4_4::ProcessEQ( int ch, float *pL, float *pR )
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
// Procedure:   ProcessMuteSolo
//
//-----------------------------------------------------
void Mixer_24_4_4::ProcessMuteSolo( int index, bool bMute, bool bGroup, bool bOn )
{
    int i;
    bool bSoloEnabled = false, bSoloGroup[ nGROUPS ] = {};

    if( bMute )
    {
        m_bMuteStates[ index ] = bOn;

        // turn solo off
        if( m_bSoloStates[ index ] )
        {
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
        m_bSoloStates[ index ] = bOn;

        // turn mute off
        if( m_bMuteStates[ index ] )
        {
            m_bMuteStates[ index ] = false;
            m_pButtonChannelMute[ index ]->Set( false );
        }

        // toggle solo
        if( !m_bSoloStates[ index ] )
        {
            m_pButtonChannelSolo[ index ]->Set( false );
        }
        else
        {
            m_pButtonChannelSolo[ index ]->Set( true );
        }
    }

    // is a track soloing?
    for( i = 0; i < nINCHANNELS; i++ )
    {
        if( m_bSoloStates[ i ] )
        {
            bSoloEnabled = true;

            if( m_iRouteGroup[ i ] != 4 )
                bSoloGroup[ m_iRouteGroup[ i ] ] = true;
        }
    }

    // somebody is soloing
    if( bSoloEnabled )
    {
        // process solo
        for( i = 0; i < nINCHANNELS; i++ )
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

        // process solo for groups if one of the routed inputs is soloing
        for( i = nINCHANNELS; i < (nINCHANNELS + 4); i++ )
        {
            // shut down volume of all not in solo
            if( !bSoloGroup[ i - nINCHANNELS ] )
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
    else //if( bSoloOff )
    {
        // process solo
        for( i = 0; i < nINCHANNELS; i++ )
        {
            // bring back if not muted
            if( !m_bMuteStates[ i ] )
            {
                m_FadeState[ i ] = MUTE_FADE_STATE_INC;
            }
        }

        // process solo for groups if one of the routed inputs is soloing
        for( i = nINCHANNELS; i < (nINCHANNELS + 4); i++ )
        {
            // bring back if not muted
            if( !m_bMuteStates[ i ] )
            {
                m_FadeState[ i ] = MUTE_FADE_STATE_INC;
            }
        }
    }
}

//-----------------------------------------------------
// Procedure:   step
//
//-----------------------------------------------------
#define SNORMAL 0
#define SGROUP 1
#define SAUX   2
void Mixer_24_4_4::step() 
{
    int section = 0;
    int ch, aux, group = 0;
    float GroupMixL[ nGROUPS ] = {0}, GroupMixR[ nGROUPS ] = {0}, fMixOutL = 0.0f, fMixOutR = 0.0f, inL, inR, flevel, fx, fy, fade[ nGROUPS ];
    float auxL[ nAUX ] = {}, auxR[ nAUX ] = {};
    bool bChannelActive, bGroupActive[ nGROUPS ] = {false};
    float pan, levelmult = 1.0;

    if( !m_bInitialized )
        return;

    if( m_bGainLevelx2 )
        levelmult = 1.5f;

    fade[ 0 ] = 1.0f;
    fade[ 1 ] = 1.0f;
    fade[ 2 ] = 1.0f;
    fade[ 3 ] = 1.0f;

    // calc XFADE
    if( inputs[ IN_FADEX ].active || inputs[ IN_FADEY ].active )
    {
        fx = clamp( inputs[ IN_FADEX ].normalize( 5.0f ), 0.0f, 10.0f ) / 10.0f;
        fy = clamp( inputs[ IN_FADEY ].normalize( 5.0f ), 0.0f, 10.0f ) / 10.0f;

        if( fx > 0.5f || fy < 0.5 )
            fade[ 0 ] = min( (1.0f - fx) * 2, fy * 2 );

        if( fx < 0.5f || fy < 0.5 )
            fade[ 1 ] = min( fx * 2, fy * 2 );

        if( fx > 0.5f || fy > 0.5 )
            fade[ 2 ] = min( (1.0f - fx) * 2, (1.0f - fy) * 2 );

        if( fx < 0.5f || fy > 0.5 )
            fade[ 3 ] = min( fx * 2, (1.0f - fy) * 2 );
    }

    for( ch = 0; ch < nCHANNELS; ch++ )
    {
        inL = 0.0f;
        inR = 0.0f;

        // normal(0), group(1) or aux channels(2) section
        if( ch == nINCHANNELS || ch == ( nINCHANNELS + nGROUPS ) )
            section++;

        bChannelActive = false;

        // see if channel active
        if( section != SGROUP )
        {
            if( inputs[ IN_LEFT + ch ].active || inputs[ IN_RIGHT + ch ].active ) 
                bChannelActive = true;
        }
        else if( section == SGROUP )
        {
            group = ch - nINCHANNELS;

            if( bGroupActive[ group ] )
                bChannelActive = true;
        }

        if( bChannelActive )
        {
            if( section == SNORMAL )
                flevel = clamp( ( params[ PARAM_CHLVL + ch ].value * levelmult ) * ( inputs[ IN_LEVELCV + ch ].normalize( CV_MAX ) / CV_MAX ), 0.0f, levelmult );
            else
                flevel = params[ PARAM_CHLVL + ch ].value * levelmult;

            if( section == SGROUP )
            {
                // process xfade
                inL = GroupMixL[ group ];
                inR = GroupMixR[ group ];
            }
            else
            {
                // check right channel first for possible mono
                if( inputs[ IN_RIGHT + ch ].active )
                {
                    inR = inputs[ IN_RIGHT + ch ].value;
                    m_bMono[ ch ] = false;
                }
                else
                    m_bMono[ ch ] = true;

                // left channel
                if( inputs[ IN_LEFT + ch ].active )
                {
                    inL = inputs[ IN_LEFT + ch ].value;

                    if( m_bMono[ ch ] )
                        inR = inL;
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

            // attenuate for EQ

            ProcessEQ( ch, &inL, &inR );
            inL *= 2.0f;
            inR *= 2.0f;

            if( section == SNORMAL )
                pan = clamp( params[ PARAM_CHPAN + ch ].value + ( inputs[ IN_PANCV + ch ].normalize( 0.0 ) / CV_MAX ), -1.0f, 1.0f );
            else
                pan = params[ PARAM_CHPAN + ch ].value;

            if( pan <= 0.0 )
                inR *= ( 1.0 + pan );
            else
                inL *= ( 1.0 - pan );

            // put output to aux ( pre fader )
            if( section != SAUX )
            {
                if( m_bPreFader[ ch ] )
                {
                    for ( aux = 0; aux < nAUX; aux++ )
                    {
                        auxL[ aux ] += inL * params[ PARAM_CHAUX + (ch * 4) + aux ].value;
                        auxR[ aux ] += inR * params[ PARAM_CHAUX + (ch * 4) + aux ].value;
                    }
                }
            }

            inL *= flevel;
            inR *= flevel;

            if( section == SGROUP )
            {
                // process xfade
                inL *= fade[ group ];
                inR *= fade[ group ];
            }

            // mute comes before group outputs
            if( !m_bGroupPreMute )
            {
                inL *= m_fMuteFade[ ch ];
                inR *= m_fMuteFade[ ch ];
            }

            // group output (pre mute)
            if( section == SGROUP )
            {
                if( bGroupActive[ group ] )
                {
                    outputs[ OUT_GRPL + group ].value = clamp( inL, -AUDIO_MAX, AUDIO_MAX );
                    outputs[ OUT_GRPR + group ].value = clamp( inR, -AUDIO_MAX, AUDIO_MAX );
                }
                else
                {
                    outputs[ OUT_GRPL + group ].value = 0.0f;
                    outputs[ OUT_GRPR + group ].value = 0.0f;
                }
            }

            // mute comes after group outputs
            if( m_bGroupPreMute )
            {
                inL *= m_fMuteFade[ ch ];
                inR *= m_fMuteFade[ ch ];
            }

            // put output to aux ( post fader )
            if( section != SAUX )
            {
                if( !m_bPreFader[ ch ] )
                {
                    for ( aux = 0; aux < nAUX; aux++ )
                    {
                        auxL[ aux ] += inL * params[ PARAM_CHAUX + (ch * 4) + aux ].value;
                        auxR[ aux ] += inR * params[ PARAM_CHAUX + (ch * 4) + aux ].value;
                    }
                }
            }

            // non normal input channels go directly to output
            if( section != SNORMAL )
            {
                fMixOutL += inL;
                fMixOutR += inR;
            }
            else
            {
                // normal channel direct out
                if( m_iRouteGroup[ ch ] == 4 )
                {
                    fMixOutL += inL;
                    fMixOutR += inR;
                }
                // normal channel routed to group
                else
                {
                    GroupMixL[ m_iRouteGroup[ ch ] ] += inL;
                    GroupMixR[ m_iRouteGroup[ ch ] ] += inR;

                    bGroupActive[ m_iRouteGroup[ ch ] ] = true;
                }
            }
        }

        if( m_pLEDMeterChannel[ ch ][ 0 ] )
            m_pLEDMeterChannel[ ch ][ 0 ]->Process( inL / AUDIO_MAX );
        if( m_pLEDMeterChannel[ ch ][ 1 ] )
            m_pLEDMeterChannel[ ch ][ 1 ]->Process( inR / AUDIO_MAX);
    }

    // apply main level
    fMixOutL = clamp( fMixOutL * params[ PARAM_LEVEL_OUT ].value, -AUDIO_MAX, AUDIO_MAX );
    fMixOutR = clamp( fMixOutR * params[ PARAM_LEVEL_OUT ].value, -AUDIO_MAX, AUDIO_MAX );

    // update main VUMeters
    if( m_pLEDMeterMain[ 0 ] )
        m_pLEDMeterMain[ 0 ]->Process( fMixOutL / AUDIO_MAX );
    if( m_pLEDMeterMain[ 1 ] )
        m_pLEDMeterMain[ 1 ]->Process( fMixOutR / AUDIO_MAX );

    // put aux output
    for ( aux = 0; aux < nAUX; aux++ )
    {
        outputs[ OUT_AUXL + aux ].value = clamp( auxL[ aux ] * params[ PARAM_AUXLVL + aux ].value, -AUDIO_MAX, AUDIO_MAX );
        outputs[ OUT_AUXR + aux ].value = clamp( auxR[ aux ] * params[ PARAM_AUXLVL + aux ].value, -AUDIO_MAX, AUDIO_MAX );
    }

    outputs[ OUT_MAINL ].value = fMixOutL;
    outputs[ OUT_MAINR ].value = fMixOutR;
}

//-----------------------------------------------------
// Procedure:   Group Pre-Mute Menu Item
//
//-----------------------------------------------------
struct Mixer_24_4_4_GroupPreMute : MenuItem 
{
    Mixer_24_4_4 *module;

    void onAction(EventAction &e) override 
    {
        module->m_bGroupPreMute = !module->m_bGroupPreMute;
    }

    void step() override 
    {
        rightText = (module->m_bGroupPreMute) ? "✔" : "";
    }
};

//-----------------------------------------------------
// Procedure:   Level Gain x2 Menu Item
//
//-----------------------------------------------------
struct Mixer_24_4_4_Gainx2 : MenuItem 
{
    Mixer_24_4_4 *module;

    void onAction(EventAction &e) override 
    {
        module->m_bGainLevelx2 = !module->m_bGainLevelx2;
    }

    void step() override 
    {
        rightText = (module->m_bGainLevelx2) ? "✔" : "";
    }
};

//-----------------------------------------------------
// Procedure:   createContextMenu
//
//-----------------------------------------------------
Menu *Mixer_24_4_4_Widget::createContextMenu() 
{
    Menu *menu = ModuleWidget::createContextMenu();

    Mixer_24_4_4 *mod = dynamic_cast<Mixer_24_4_4*>(module);

    assert(mod);

    menu->addChild(construct<MenuLabel>());

    menu->addChild(construct<MenuLabel>(&MenuLabel::text, "---- Group Outputs ----"));
    menu->addChild(construct<Mixer_24_4_4_GroupPreMute>( &MenuItem::text, "Pre-Mute", &Mixer_24_4_4_GroupPreMute::module, mod ) );

    menu->addChild(construct<MenuLabel>(&MenuLabel::text, "---- Level Sliders ----"));
    menu->addChild(construct<Mixer_24_4_4_Gainx2>( &MenuItem::text, "Gain x1.5", &Mixer_24_4_4_Gainx2::module, mod ) );

    return menu;
}

} // namespace rack_plugin_mscHack

using namespace rack_plugin_mscHack;

RACK_PLUGIN_MODEL_INIT(mscHack, Mix_24_4_4) {
   Model *modelMix_24_4_4 = Model::create<Mixer_24_4_4, Mixer_24_4_4_Widget>( "mscHack", "Mix_24_4_4", "MIXER 24ch, 4 groups, 4 aux", MIXER_TAG, EQUALIZER_TAG, PANNING_TAG, AMPLIFIER_TAG, MULTIPLE_TAG );
   return modelMix_24_4_4;
}
