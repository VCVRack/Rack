#include "mscHack.hpp"
#include "dsp/digital.hpp"

namespace rack_plugin_mscHack {

//-----------------------------------------------------
// Module Definition
//
//-----------------------------------------------------
struct SEQ_Envelope_8 : Module 
{
#define nCHANNELS MAX_ENVELOPE_CHANNELS
#define nWAVESETS 4
#define nTIMESETS 6

	enum ParamIds 
    {
        PARAM_BAND,
        nPARAMS
    };

	enum InputIds 
    {
        INPUT_CLK_RESET,
        INPUT_CLK,
        INPUT_GLOBAL_TRIG,
        INPUT_CH_HOLD,
        INPUT_CH_TRIG           = INPUT_CH_HOLD + nCHANNELS,
        nINPUTS                 = INPUT_CH_TRIG + nCHANNELS
	};

	enum OutputIds 
    {
        OUTPUT_CV,
        nOUTPUTS                = OUTPUT_CV + nCHANNELS
	};

	enum LightIds 
    {
        nLIGHTS
	};

    bool                m_bInitialized = false;

    CLog            lg;

    // Contructor
	SEQ_Envelope_8() : Module(nPARAMS, nINPUTS, nOUTPUTS, nLIGHTS){}

    // clock     
    SchmittTrigger      m_SchTrigClk;

    // global triggers
    SchmittTrigger      m_SchTrigGlobalClkReset;
    SchmittTrigger      m_SchTrigGlobalTrig;

    // channel triggers
    SchmittTrigger      m_SchTrigChTrig[ nCHANNELS ] ={};

    int                 m_CurrentChannel = 0;
    int                 m_GraphData[ nCHANNELS ][ ENVELOPE_HANDLES ] = {};
    int                 m_Modes[ nCHANNELS ] ={};
    int                 m_Ranges[ nCHANNELS ] = {};
    int                 m_TimeDivs[ nCHANNELS ] = {};
    bool                m_bHold[ nCHANNELS ] = {};
    bool                m_bGateMode[ nCHANNELS ] = {true};
    int                 m_HoldPos[ nCHANNELS ] = {};
    bool                m_bTrig[ nCHANNELS ] = {};

    int                 m_waveSet = 0;
    bool                m_bCpy = false;

    Widget_EnvelopeEdit *m_pEnvelope = NULL;
    MyLEDButtonStrip    *m_pButtonChSelect = NULL;
    MyLEDButtonStrip    *m_pButtonModeSelect = NULL;
    MyLEDButtonStrip    *m_pButtonRangeSelect = NULL;
    MyLEDButtonStrip    *m_pButtonTimeSelect = NULL;
    MyLEDButton         *m_pButtonHold[ nCHANNELS ] = {};
    MyLEDButton         *m_pButtonTrig[ nCHANNELS ] = {};
    MyLEDButton         *m_pButtonGateMode = NULL;
    MyLEDButton         *m_pButtonWaveSetBck = NULL;
    MyLEDButton         *m_pButtonWaveSetFwd = NULL;
    MyLEDButton         *m_pButtonJoinEnds = NULL;
    MyLEDButton         *m_pButtonGlobalReset = NULL;
    MyLEDButton         *m_pButtonDraw = NULL;
    MyLEDButton         *m_pButtonCopy = NULL;
    MyLEDButton         *m_pButtonRand = NULL;
    MyLEDButton         *m_pButtonInvert = NULL;

    Label               *m_pTextLabel = NULL;

    int                 m_BeatCount = 0;

    //-----------------------------------------------------
    // Band_Knob
    //-----------------------------------------------------
    struct Band_Knob : Knob_Yellow2_26
    {
        SEQ_Envelope_8 *mymodule;
        int param;

        void onChange( EventChange &e ) override 
        {
            mymodule = (SEQ_Envelope_8*)module;

            if( mymodule )
                mymodule->m_pEnvelope->m_fband = value; 

		    RoundKnob::onChange( e );
	    }
    };

    void ChangeChannel( int ch );

    // Overrides 
	void    step() override;
    void    JsonParams( bool bTo, json_t *root);
    json_t* toJson() override;
    void    fromJson(json_t *rootJ) override;
    void    onRandomize() override;
    void    onReset() override;
    void    onCreate() override;
    void    onDelete() override;
};

//-----------------------------------------------------
// Seq_Triad2_Pause
//-----------------------------------------------------
void EnvelopeEditCALLBACK ( void *pClass, float val )
{
    char strVal[ 10 ] = {};

    SEQ_Envelope_8 *mymodule;
    mymodule = (SEQ_Envelope_8*)pClass;

    sprintf( strVal, "[%.3fV]", val );

    mymodule->m_pTextLabel->text = strVal;
}

//-----------------------------------------------------
// SEQ_Envelope_8_DrawMode
//-----------------------------------------------------
void SEQ_Envelope_8_DrawMode( void *pClass, int id, bool bOn ) 
{
    SEQ_Envelope_8 *mymodule;
    mymodule = (SEQ_Envelope_8*)pClass;
    mymodule->m_pEnvelope->m_bDraw = bOn;
}

//-----------------------------------------------------
// SEQ_Envelope_8_GateMode
//-----------------------------------------------------
void SEQ_Envelope_8_GateMode( void *pClass, int id, bool bOn ) 
{
    SEQ_Envelope_8 *mymodule;
    mymodule = (SEQ_Envelope_8*)pClass;
    mymodule->m_bGateMode[ mymodule->m_CurrentChannel ] = bOn;
    mymodule->m_pEnvelope->setGateMode( mymodule->m_CurrentChannel, bOn );
}

//-----------------------------------------------------
// Procedure:   SEQ_Envelope_8_ChSelect
//-----------------------------------------------------
void SEQ_Envelope_8_ChSelect( void *pClass, int id, int nbutton, bool bOn )
{
    SEQ_Envelope_8 *mymodule;
    mymodule = (SEQ_Envelope_8*)pClass;

    mymodule->ChangeChannel( nbutton );
}

//-----------------------------------------------------
// Procedure:   SEQ_Envelope_8_ModeSelect
//-----------------------------------------------------
void SEQ_Envelope_8_ModeSelect( void *pClass, int id, int nbutton, bool bOn )
{
    SEQ_Envelope_8 *mymodule;
    mymodule = (SEQ_Envelope_8*)pClass;

    mymodule->m_Modes[ mymodule->m_CurrentChannel ] = nbutton;
    mymodule->m_pEnvelope->setMode( mymodule->m_CurrentChannel, nbutton );
}

//-----------------------------------------------------
// Procedure:   SEQ_Envelope_8_TimeSelect
//-----------------------------------------------------
void SEQ_Envelope_8_TimeSelect( void *pClass, int id, int nbutton, bool bOn )
{
    SEQ_Envelope_8 *mymodule;
    mymodule = (SEQ_Envelope_8*)pClass;

    mymodule->m_TimeDivs[ mymodule->m_CurrentChannel ] = nbutton;
    mymodule->m_pEnvelope->setTimeDiv( mymodule->m_CurrentChannel, nbutton );
}

//-----------------------------------------------------
// Procedure:   SEQ_Envelope_8_RangeSelect
//-----------------------------------------------------
void SEQ_Envelope_8_RangeSelect( void *pClass, int id, int nbutton, bool bOn )
{
    SEQ_Envelope_8 *mymodule;
    mymodule = (SEQ_Envelope_8*)pClass;

    mymodule->m_Ranges[ mymodule->m_CurrentChannel ] = nbutton;
    mymodule->m_pEnvelope->setRange( mymodule->m_CurrentChannel, nbutton );
}

//-----------------------------------------------------
// Procedure:   SEQ_Envelope_8_Hold
//-----------------------------------------------------
void SEQ_Envelope_8_Hold( void *pClass, int id, bool bOn )
{
    SEQ_Envelope_8 *mymodule;
    mymodule = (SEQ_Envelope_8*)pClass;

    mymodule->m_bHold[ id ] = bOn;
}

//-----------------------------------------------------
// Procedure:   SEQ_Envelope_8_WaveSet
//-----------------------------------------------------
void SEQ_Envelope_8_WaveSet( void *pClass, int id, bool bOn )
{
    int i;
    float a, div;
    SEQ_Envelope_8 *mymodule;
    mymodule = (SEQ_Envelope_8*)pClass;

    if( id == 0 )
    {
        if( ++mymodule->m_waveSet > 6 )
            mymodule->m_waveSet = 0;
    }
    else
    {
        if( --mymodule->m_waveSet < 0 )
            mymodule->m_waveSet = 6;
    }

    switch( mymodule->m_waveSet )
    {
    case 0:
        mymodule->m_pEnvelope->resetValAll( mymodule->m_CurrentChannel, 0.0f );
        break;
    case 1:
        mymodule->m_pEnvelope->resetValAll( mymodule->m_CurrentChannel, 0.5f );
        break;
    case 2: // sin
        div = (float)(ENVELOPE_HANDLES - 1) / (PI * 2.0f);
        for( i = 0; i < ENVELOPE_HANDLES; i++ )
        {
            a = ( 1.0f + sinf( (float)i / div ) ) / 2.0f;
            mymodule->m_pEnvelope->setVal( mymodule->m_CurrentChannel, i, a );
        }
        break;
    case 3: // cos
        div = (float)(ENVELOPE_HANDLES - 1) / (PI * 2.0f);
        for( i = 0; i < ENVELOPE_HANDLES; i++ )
        {
            a = ( 1.0f + cosf( (float)i / div ) ) / 2.0f;
            mymodule->m_pEnvelope->setVal( mymodule->m_CurrentChannel, i, a );
        }
        break;
    case 4: // cos half
        div = (float)(ENVELOPE_HANDLES - 1) / (PI * 1.0f);
        for( i = 0; i < ENVELOPE_HANDLES; i++ )
        {
            a = ( 1.0f + cosf( (float)i / div ) ) / 2.0f;
            mymodule->m_pEnvelope->setVal( mymodule->m_CurrentChannel, i, a );
        }
        break;
    case 5: // triangle full
        div = 1.0f / 16.0f;
        a = 0;
        for( i = 0; i < ENVELOPE_HANDLES; i++ )
        {
            mymodule->m_pEnvelope->setVal( mymodule->m_CurrentChannel, i, a );
            a += div;
        }
        break;
    case 6: // triangle half
        div = 1.0f / 8.0f;
        a = 0;
        for( i = 0; i < ENVELOPE_HANDLES; i++ )
        {
            mymodule->m_pEnvelope->setVal( mymodule->m_CurrentChannel, i, a );
            a += div;

            if( i == 8 )
                a = 0.0f;
        }
        break;

    default:
        break;
    }
}

//-----------------------------------------------------
// Procedure:   SEQ_Envelope_8_WaveInvert
//-----------------------------------------------------
void SEQ_Envelope_8_WaveInvert( void *pClass, int id, bool bOn )
{
    int i;
    SEQ_Envelope_8 *mymodule;
    mymodule = (SEQ_Envelope_8*)pClass;

    for( i = 0; i < ENVELOPE_HANDLES; i++ )
        mymodule->m_pEnvelope->setVal( mymodule->m_CurrentChannel, i, 1.0f - mymodule->m_pEnvelope->m_HandleVal[ mymodule->m_CurrentChannel ][ i ] );
}

//-----------------------------------------------------
// Procedure:   SEQ_Envelope_8_WaveRand
//-----------------------------------------------------
void SEQ_Envelope_8_WaveRand( void *pClass, int id, bool bOn )
{
    int i;
    SEQ_Envelope_8 *mymodule;
    mymodule = (SEQ_Envelope_8*)pClass;

    for( i = 0; i < ENVELOPE_HANDLES; i++ )
        mymodule->m_pEnvelope->setVal( mymodule->m_CurrentChannel, i, randomUniform() );
}

//-----------------------------------------------------
// Procedure:   SEQ_Envelope_8_WaveCopy
//-----------------------------------------------------
void SEQ_Envelope_8_WaveCopy( void *pClass, int id, bool bOn )
{
    SEQ_Envelope_8 *mymodule;
    mymodule = (SEQ_Envelope_8*)pClass;

    mymodule->m_bCpy = bOn;
}

//-----------------------------------------------------
// Procedure:   SEQ_Envelope_8_Trig
//-----------------------------------------------------
void SEQ_Envelope_8_Trig( void *pClass, int id, bool bOn )
{
    SEQ_Envelope_8 *mymodule;
    mymodule = (SEQ_Envelope_8*)pClass;

    // global reset
    if( id == nCHANNELS )
    {
        // turn on all trigs
        for( int i = 0; i < nCHANNELS; i++ )
        {
            mymodule->m_pButtonTrig[ i ]->Set( bOn );
            mymodule->m_bTrig[ i ] = true;
        }
    }
    else
    {
        mymodule->m_bTrig[ id ] = true;
    }
}

//-----------------------------------------------------
// Procedure:   Widget
//
//-----------------------------------------------------

struct SEQ_Envelope_8_Widget : ModuleWidget {
	SEQ_Envelope_8_Widget( SEQ_Envelope_8 *module );
};

SEQ_Envelope_8_Widget::SEQ_Envelope_8_Widget( SEQ_Envelope_8 *module ) : ModuleWidget(module) 
{
    int ch, x=0, y=0                                       ;

	box.size = Vec( 15*36, 380);

	{
		SVGPanel *panel = new SVGPanel();
		panel->box.size = box.size;
		panel->setBackground(SVG::load(assetPlugin(plugin, "res/SEQ_Envelope_8.svg")));
		addChild(panel);
	}

    //module->lg.Open("SEQ_Envelope_8.txt");
    // input clock
    addInput(Port::create<MyPortInSmall>( Vec( 45, 18 ), Port::INPUT, module, SEQ_Envelope_8::INPUT_CLK ) );

    // input clock reset
    addInput(Port::create<MyPortInSmall>( Vec( 21, 18 ), Port::INPUT, module, SEQ_Envelope_8::INPUT_CLK_RESET ) );

    // invert
    module->m_pButtonInvert = new MyLEDButton( 95, 23, 11, 11, 8.0, DWRGB( 180, 180, 180 ), DWRGB( 0, 255, 255 ), MyLEDButton::TYPE_MOMENTARY, 0, module, SEQ_Envelope_8_WaveInvert );
	addChild( module->m_pButtonInvert );

    // envelope editor
    module->m_pEnvelope = new Widget_EnvelopeEdit( 47, 42, 416, 192, 7, module, EnvelopeEditCALLBACK );
	addChild( module->m_pEnvelope );

    // envelope select buttons
    module->m_pButtonChSelect = new MyLEDButtonStrip( 210, 23, 11, 11, 3, 8.0, nCHANNELS, false, DWRGB( 180, 180, 180 ), DWRGB( 0, 255, 255 ), MyLEDButtonStrip::TYPE_EXCLUSIVE, 0, module, SEQ_Envelope_8_ChSelect );
    addChild( module->m_pButtonChSelect );

	module->m_pTextLabel = new Label();
	module->m_pTextLabel->box.pos = Vec( 450, 10 );
	module->m_pTextLabel->text = "----";
	addChild( module->m_pTextLabel );

    // wave set buttons
    x = 364;
    y = 23;
    module->m_pButtonWaveSetBck = new MyLEDButton( x, y, 11, 11, 8.0, DWRGB( 180, 180, 180 ), DWRGB( 0, 255, 255 ), MyLEDButton::TYPE_MOMENTARY, 0, module, SEQ_Envelope_8_WaveSet );
	addChild( module->m_pButtonWaveSetBck );

    module->m_pButtonWaveSetFwd = new MyLEDButton( x + 12, y, 11, 11, 8.0, DWRGB( 180, 180, 180 ), DWRGB( 0, 255, 255 ), MyLEDButton::TYPE_MOMENTARY, 1, module, SEQ_Envelope_8_WaveSet );
	addChild( module->m_pButtonWaveSetFwd );

    x = 405;

    // random
    module->m_pButtonRand = new MyLEDButton( x, y, 11, 11, 8.0, DWRGB( 180, 180, 180 ), DWRGB( 0, 255, 255 ), MyLEDButton::TYPE_MOMENTARY, 0, module, SEQ_Envelope_8_WaveRand );
	addChild( module->m_pButtonRand );

    x += 25;

    // copy
    module->m_pButtonCopy = new MyLEDButton( x, y, 11, 11, 8.0, DWRGB( 180, 180, 180 ), DWRGB( 0, 255, 255 ), MyLEDButton::TYPE_SWITCH, 0, module, SEQ_Envelope_8_WaveCopy );
	addChild( module->m_pButtonCopy );

    // mode select
    module->m_pButtonModeSelect = new MyLEDButtonStrip( 109, 256, 11, 11, 8, 8.0, Widget_EnvelopeEdit::nMODES, true, DWRGB( 180, 180, 180 ), DWRGB( 0, 255, 255 ), MyLEDButtonStrip::TYPE_EXCLUSIVE, 0, module, SEQ_Envelope_8_ModeSelect );
    addChild( module->m_pButtonModeSelect );

    // time select
    module->m_pButtonTimeSelect = new MyLEDButtonStrip( 272, 256, 11, 11, 8, 8.0, nTIMESETS, true, DWRGB( 180, 180, 180 ), DWRGB( 0, 255, 255 ), MyLEDButtonStrip::TYPE_EXCLUSIVE, 0, module, SEQ_Envelope_8_TimeSelect );
    addChild( module->m_pButtonTimeSelect );

    // rande select
    module->m_pButtonRangeSelect = new MyLEDButtonStrip( 350, 256, 11, 11, 8, 8.0, Widget_EnvelopeEdit::nRANGES, true, DWRGB( 180, 180, 180 ), DWRGB( 0, 255, 255 ), MyLEDButtonStrip::TYPE_EXCLUSIVE, 0, module, SEQ_Envelope_8_RangeSelect );
    addChild( module->m_pButtonRangeSelect );

    // draw mode
    module->m_pButtonDraw = new MyLEDButton( 48, 237, 11, 11, 8.0, DWRGB( 180, 180, 180 ), DWRGB( 255, 128, 0 ), MyLEDButton::TYPE_SWITCH, 0, module, SEQ_Envelope_8_DrawMode );
	addChild( module->m_pButtonDraw );

    // gate mode
    module->m_pButtonGateMode = new MyLEDButton( 48, 254, 11, 11, 8.0, DWRGB( 180, 180, 180 ), DWRGB( 255, 128, 0 ), MyLEDButton::TYPE_SWITCH, 0, module, SEQ_Envelope_8_GateMode );
	addChild( module->m_pButtonGateMode );

    // global reset input
    addInput(Port::create<MyPortInSmall>( Vec( 21, 313 ), Port::INPUT, module, SEQ_Envelope_8::INPUT_GLOBAL_TRIG ) );

    // global reseet button
    module->m_pButtonGlobalReset = new MyLEDButton( 5, 315, 14, 14, 11.0, DWRGB( 180, 180, 180 ), DWRGB( 0, 255, 255 ), MyLEDButton::TYPE_MOMENTARY, nCHANNELS, module, SEQ_Envelope_8_Trig );
	addChild( module->m_pButtonGlobalReset );

    // band knob
    addParam(ParamWidget::create<SEQ_Envelope_8::Band_Knob>( Vec( 59.5 , 280 ), module, SEQ_Envelope_8::PARAM_BAND, 0.0, 0.8, 0.333 ) );

    // inputs, outputs
    x=21;
    y=53;
    for( ch = 0; ch < nCHANNELS; ch++ )
    {
        addInput(Port::create<MyPortInSmall>( Vec( x, y ), Port::INPUT, module, SEQ_Envelope_8::INPUT_CH_TRIG + ch ) );

        // trig button
        module->m_pButtonTrig[ ch ] = new MyLEDButton( x -16, y + 2, 14, 14, 11.0, DWRGB( 180, 180, 180 ), DWRGB( 0, 255, 255 ), MyLEDButton::TYPE_MOMENTARY, ch, module, SEQ_Envelope_8_Trig );
	    addChild( module->m_pButtonTrig[ ch ] );

        // hold button
        module->m_pButtonHold[ ch ] = new MyLEDButton( 470, y + 2, 14, 14, 11.0, DWRGB( 180, 180, 180 ), DWRGB( 255, 0, 0 ), MyLEDButton::TYPE_SWITCH, ch, module, SEQ_Envelope_8_Hold );
	    addChild( module->m_pButtonHold[ ch ] );

        // hold gate input
        addInput(Port::create<MyPortInSmall>( Vec( 486, y ), Port::INPUT, module, SEQ_Envelope_8::INPUT_CH_HOLD + ch ) );

        // out cv
        addOutput(Port::create<MyPortOutSmall>( Vec( 516, y ), Port::OUTPUT, module, SEQ_Envelope_8::OUTPUT_CV + ch ) );

        y += 28;
    }

	addChild(Widget::create<ScrewSilver>(Vec(15, 0)));
	addChild(Widget::create<ScrewSilver>(Vec(box.size.x-30, 0)));
	addChild(Widget::create<ScrewSilver>(Vec(15, 365))); 
	addChild(Widget::create<ScrewSilver>(Vec(box.size.x-30, 365)));

    module->m_bInitialized = true;
}

//-----------------------------------------------------
// Procedure: JsonParams  
//
//-----------------------------------------------------
void SEQ_Envelope_8::JsonParams( bool bTo, json_t *root) 
{
    JsonDataBool( bTo, "m_bHold", root, m_bHold, nCHANNELS );
    JsonDataBool( bTo, "m_bGateMode", root, m_bGateMode, nCHANNELS );
    JsonDataInt( bTo, "m_HoldPos", root, (int*)m_HoldPos, nCHANNELS );
    JsonDataInt( bTo, "m_TimeDivs", root, (int*)m_TimeDivs, nCHANNELS );
    JsonDataInt( bTo, "m_Modes", root, (int*)m_Modes, nCHANNELS );
    JsonDataInt( bTo, "m_Ranges", root, (int*)m_Ranges, nCHANNELS );
    JsonDataInt( bTo, "m_GraphData", root, (int*)m_GraphData, nCHANNELS * ENVELOPE_HANDLES );
}

//-----------------------------------------------------
// Procedure: toJson  
//
//-----------------------------------------------------
json_t *SEQ_Envelope_8::toJson() 
{
	json_t *root = json_object();

    if( !root )
        return NULL;

    m_pEnvelope->getDataAll( (int*)m_GraphData );

    for( int i = 0; i < nCHANNELS; i++ )
        m_HoldPos[ i ] = m_pEnvelope->getPos( i );

    JsonParams( TOJSON, root );
    
	return root;
}

//-----------------------------------------------------
// Procedure:   fromJson
//
//-----------------------------------------------------
void SEQ_Envelope_8::fromJson( json_t *root ) 
{
    int ch;

    JsonParams( FROMJSON, root );

    for( ch = 0; ch < nCHANNELS; ch++ )
    {

        // hold button
        m_pButtonHold[ ch ]->Set( m_bHold[ ch ] );
 
        m_pEnvelope->setGateMode( ch, m_bGateMode[ ch ] );
        m_pEnvelope->setMode( ch, m_Modes[ ch ] );
        m_pEnvelope->setRange( ch, m_Ranges[ ch ] );
        m_pEnvelope->setTimeDiv( ch, m_TimeDivs[ ch ] );
        m_pEnvelope->setPos( ch, m_HoldPos[ ch ] );
    }

    m_pEnvelope->setDataAll( (int*)m_GraphData );

    ChangeChannel( 0 );
}

//-----------------------------------------------------
// Procedure:   onCreate
//
//-----------------------------------------------------
void SEQ_Envelope_8::onCreate()
{
}

//-----------------------------------------------------
// Procedure:   onDelete
//
//-----------------------------------------------------
void SEQ_Envelope_8::onDelete()
{
}

//-----------------------------------------------------
// Procedure:   onReset
//
//-----------------------------------------------------
void SEQ_Envelope_8::onReset()
{
    int ch;

    memset( m_GraphData, 0, sizeof( m_GraphData ) );
    memset( m_bGateMode, 0, sizeof( m_bGateMode ) );
    memset( m_Modes, 0, sizeof( m_Modes ) );
    memset( m_Ranges, 0, sizeof( m_Ranges ) );
    memset( m_GraphData, 0, sizeof( m_GraphData ) );
    memset( m_bHold, 0, sizeof( m_bHold ) );

    for( ch = 0; ch < nCHANNELS; ch++ )
    {
        m_pButtonHold[ ch ]->Set( m_bHold[ ch ] );
 
        m_pEnvelope->setGateMode( ch, m_bGateMode[ ch ] );
        m_pEnvelope->setMode( ch, m_Modes[ ch ] );
        m_pEnvelope->setRange( ch, m_Ranges[ ch ] );
        m_pEnvelope->setTimeDiv( ch, m_TimeDivs[ ch ] );
        m_pEnvelope->setPos( ch, m_HoldPos[ ch ] );
    }

    m_pEnvelope->setDataAll( (int*)m_GraphData );

    ChangeChannel( 0 );
}

//-----------------------------------------------------
// Procedure:   onRandomize
//
//-----------------------------------------------------
void SEQ_Envelope_8::onRandomize()
{
    int ch, i;

    for( ch = 0; ch < nCHANNELS; ch++ )
    {
        for( i = 0; i < ENVELOPE_HANDLES; i++ )
            m_pEnvelope->setVal( ch, i, randomUniform() );
    }
}

//-----------------------------------------------------
// Procedure:   onRandomize
//
//-----------------------------------------------------
void SEQ_Envelope_8::ChangeChannel( int ch )
{
    int i;

    if( ch < 0 || ch >= nCHANNELS )
        return;

    if( m_bCpy )
    {
        m_bCpy = false;
        m_pButtonCopy->Set( false );

        for( i = 0; i < ENVELOPE_HANDLES; i++ )
        {
            m_pEnvelope->setVal( ch, i, m_pEnvelope ->m_HandleVal[ m_CurrentChannel ][ i ] );
        }

        m_TimeDivs[ ch ]  = m_TimeDivs[ m_CurrentChannel ];
        m_Modes[ ch ]     = m_Modes[ m_CurrentChannel ];
        m_Ranges[ ch ]    = m_Ranges[ m_CurrentChannel ];
        m_bGateMode[ ch ] = m_bGateMode[ m_CurrentChannel ];

        m_pEnvelope->setGateMode( ch, m_bGateMode[ ch ] );
        m_pEnvelope->setMode( ch, m_Modes[ ch ] );
        m_pEnvelope->setRange( ch, m_Ranges[ ch ] );
        m_pEnvelope->setTimeDiv( ch, m_TimeDivs[ ch ] );
    }

    m_CurrentChannel = ch;
    m_pButtonChSelect->Set( ch, true );
    m_pButtonTimeSelect->Set( m_TimeDivs[ ch ], true );
    m_pButtonModeSelect->Set( m_Modes[ ch ], true );
    m_pButtonRangeSelect->Set( m_Ranges[ ch ], true );
    m_pButtonGateMode->Set( m_bGateMode[ ch ] );
    m_pEnvelope->setView( ch );
}

//-----------------------------------------------------
// Procedure:   step
//
//-----------------------------------------------------
void SEQ_Envelope_8::step() 
{
    int ch;
    bool bHold = false, bTrig = false, bGlobalTrig = false;
    //char strVal[ 10 ] = {};

    if( !m_bInitialized || !inputs[ INPUT_CLK ].active )
        return;

    // global clock reset
    m_pEnvelope->m_bClkReset = m_SchTrigGlobalClkReset.process( inputs[ INPUT_CLK_RESET ].normalize( 0.0f ) );

    m_BeatCount++;

    // track clock period
    if( m_SchTrigClk.process( inputs[ INPUT_CLK ].normalize( 0.0f ) ) )
    {
        //sprintf( strVal, "%d", m_BeatCount );
        //m_pTextLabel->text = strVal;
        m_pEnvelope->setBeatLen( m_BeatCount );
        m_BeatCount = 0;
    }

    if( m_SchTrigGlobalTrig.process( inputs[ INPUT_GLOBAL_TRIG ].normalize( 0.0f ) ) )
    {
        bGlobalTrig = true;
    }

    // process each channel
    for( ch = 0; ch < nCHANNELS; ch++ )
    {
        // trig, clock reset
        bTrig = ( m_SchTrigChTrig[ ch ].process( inputs[ INPUT_CH_TRIG + ch ].normalize( 0.0f ) ) ) || bGlobalTrig;

        if( bTrig )
            m_pButtonTrig[ ch ]->Set( true );

        if( bTrig || m_bTrig[ ch ] )
            bTrig = true;

        bHold = ( m_bHold[ ch ] || inputs[ INPUT_CH_HOLD + ch ].normalize( 0.0f ) > 2.5f );

        if( bHold && !m_pButtonHold[ ch ]->m_bOn )
            m_pButtonHold[ ch ]->Set( true );
        else if( !bHold && m_pButtonHold[ ch ]->m_bOn )
            m_pButtonHold[ ch ]->Set( false );

        // process envelope
        outputs[ OUTPUT_CV + ch ].value = m_pEnvelope->procStep( ch, bTrig, bHold );

        m_bTrig[ ch ] = false;
    }

    m_pEnvelope->m_bClkReset = false;
}

} // namespace rack_plugin_mscHack

using namespace rack_plugin_mscHack;

RACK_PLUGIN_MODEL_INIT(mscHack, SEQ_Envelope_8) {
   Model *modelSEQ_Envelope_8 = Model::create<SEQ_Envelope_8, SEQ_Envelope_8_Widget>( "mscHack", "SEQ_Envelope_8", "ENVY - 9", ENVELOPE_GENERATOR_TAG, MULTIPLE_TAG );
   return modelSEQ_Envelope_8;
}
