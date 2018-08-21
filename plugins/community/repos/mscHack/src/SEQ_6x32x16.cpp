#include "mscHack.hpp"
//#include "mscHack_Controls.hpp"
#include "dsp/digital.hpp"
//#include "CLog.h"

namespace rack_plugin_mscHack {

#define nCHANNELS 6
#define nSTEPS 32
#define nPROG 16

typedef struct
{
    bool    bPending;
    int     prog;
}PHRASE_CHANGE_STRUCT;

//-----------------------------------------------------
// Module Definition
//
//-----------------------------------------------------
struct SEQ_6x32x16 : Module 
{
#define NO_COPY -1

	enum ParamIds 
    {
        PARAM_CPY_NEXT,
        PARAM_RAND          = PARAM_CPY_NEXT + nCHANNELS,
        PARAM_PAUSE         = PARAM_RAND + nCHANNELS,
        PARAM_BILEVEL       = PARAM_PAUSE + nCHANNELS,
        PARAM_LVL1_KNOB     = PARAM_BILEVEL + nCHANNELS,
        PARAM_LVL2_KNOB     = PARAM_LVL1_KNOB + nCHANNELS,
        PARAM_LVL3_KNOB     = PARAM_LVL2_KNOB + nCHANNELS,
        PARAM_LVL4_KNOB     = PARAM_LVL3_KNOB + nCHANNELS,
        PARAM_LVL5_KNOB     = PARAM_LVL4_KNOB + nCHANNELS,
        nPARAMS             = PARAM_LVL5_KNOB + nCHANNELS
    };

	enum InputIds 
    {
        IN_GLOBAL_CLK_RESET,
        IN_GLOBAL_PAT_CHANGE,
        IN_CLK,
        IN_PAT_TRIG     	= IN_CLK + nCHANNELS,
		IN_GLOBAL_TRIG_MUTE = IN_PAT_TRIG + nCHANNELS,
        nINPUTS
	};

	enum OutputIds 
    {
        OUT_TRIG,
        OUT_LEVEL    = OUT_TRIG + nCHANNELS,
        OUT_BEAT1    = OUT_LEVEL + nCHANNELS,
        nOUTPUTS     = OUT_BEAT1 + nCHANNELS
	};

    bool            m_bInitialized = false;
    CLog            lg;

    bool            m_bPauseState[ nCHANNELS ] = {};
    bool            m_bBiLevelState[ nCHANNELS ] = {};

    SinglePatternClocked32  *m_pPatternDisplay[ nCHANNELS ] = {};
    int                     m_Pattern[ nCHANNELS ][ nPROG ][ nSTEPS ];
    int                     m_MaxPat[ nCHANNELS ][ nPROG ] = {};

    PatternSelectStrip      *m_pProgramDisplay[ nCHANNELS ] = {};
    int                     m_CurrentProg[ nCHANNELS ] = {};
    int                     m_MaxProg[ nCHANNELS ] = {};
    PHRASE_CHANGE_STRUCT    m_ProgPending[ nCHANNELS ] = {};

    SchmittTrigger          m_SchTrigClock[ nCHANNELS ];
    SchmittTrigger          m_SchTrigProg[ nCHANNELS ];
    SchmittTrigger          m_SchTrigGlobalClkReset;
    SchmittTrigger          m_SchTrigGlobalProg;

    bool                    m_bTrig[ nCHANNELS ] = {};
    PulseGenerator          m_gatePulse[ nCHANNELS ];
    PulseGenerator          m_gateBeatPulse[ nCHANNELS ];

    // swing
    //int                     m_SwingLen[ nCHANNELS ] = {0};
    //int                     m_SwingCount[ nCHANNELS ] = {0};
    int                     m_ClockTick[ nCHANNELS ] = {0};

    int                     m_CopySrc = NO_COPY;

    // trig mute
    bool            		m_bTrigMute = false;
    MyLEDButton     		*m_pButtonTrigMute = NULL;

    // buttons    
    MyLEDButton             *m_pButtonPause[ nCHANNELS ] = {};
    MyLEDButton             *m_pButtonCopy[ nCHANNELS ] = {};
    MyLEDButton             *m_pButtonBiLevel[ nCHANNELS ] = {};
    MyLEDButton             *m_pButtonRand[ nCHANNELS ] = {};
    MyLEDButton             *m_pButtonAutoPat[ nCHANNELS ] = {};
    MyLEDButton             *m_pButtonHoldCV[ nCHANNELS ] = {};
    MyLEDButton             *m_pButtonClear[ nCHANNELS ] = {};

    bool                    m_bAutoPatChange[ nCHANNELS ] = {};
    bool                    m_bHoldCVState[ nCHANNELS ] = {};

    float                   m_fCVRanges[ 3 ] = { 15.0f, 10.0f, 5.0f};
    int                     m_RangeSelect = 0;
    char                    m_strRange[ 10 ] = {0};

	 float flast[ nCHANNELS ];

    // Contructor
	SEQ_6x32x16() : Module(nPARAMS, nINPUTS, nOUTPUTS, 0){}

    // Overrides 
	void    step() override;
    json_t* toJson() override;
    void    fromJson(json_t *rootJ) override;
    void    onRandomize() override;
    void    onReset() override;

    void    JsonParams( bool bTo, json_t *root);
    void    CpyNext( int ch );
    void    Rand( int ch );
    void    ChangeProg( int ch, int prog, bool force );
    void    SetPendingProg( int ch, int prog );
    void    Copy( int kb, bool bOn );
};

//-----------------------------------------------------
// MyLEDButton_TrigMute
//-----------------------------------------------------
void MyLEDButton_TrigMute( void *pClass, int id, bool bOn )
{
	SEQ_6x32x16 *mymodule;
    mymodule = (SEQ_6x32x16*)pClass;
    mymodule->m_bTrigMute = bOn;
}

//-----------------------------------------------------
// MyLEDButton_AutoPat
//-----------------------------------------------------
void MyLEDButton_AutoPat( void *pClass, int id, bool bOn ) 
{
    SEQ_6x32x16 *mymodule;
    mymodule = (SEQ_6x32x16*)pClass;
    mymodule->m_bAutoPatChange[ id ] = bOn;
}

//-----------------------------------------------------
// MyLEDButton_Pause
//-----------------------------------------------------
void MyLEDButton_Pause( void *pClass, int id, bool bOn ) 
{
    SEQ_6x32x16 *mymodule;
    mymodule = (SEQ_6x32x16*)pClass;
    mymodule->m_bPauseState[ id ] = bOn;
}

//-----------------------------------------------------
// MyLEDButton_HoldCV
//-----------------------------------------------------
void MyLEDButton_HoldCV( void *pClass, int id, bool bOn ) 
{
    SEQ_6x32x16 *mymodule;
    mymodule = (SEQ_6x32x16*)pClass;
    mymodule->m_bHoldCVState[ id ] = bOn;
}

//-----------------------------------------------------
// MyLEDButton_BiLevel
//-----------------------------------------------------
void MyLEDButton_BiLevel( void *pClass, int id, bool bOn ) 
{
    SEQ_6x32x16 *mymodule;
    mymodule = (SEQ_6x32x16*)pClass;
    mymodule->m_bBiLevelState[ id ] = bOn;
}

//-----------------------------------------------------
// MyLEDButton_CpyNxt
//-----------------------------------------------------
void MyLEDButton_CpyNxt( void *pClass, int id, bool bOn ) 
{
    SEQ_6x32x16 *mymodule;
    mymodule = (SEQ_6x32x16*)pClass;
    mymodule->Copy( id, bOn );
}

//-----------------------------------------------------
// MyLEDButton_Rand
//-----------------------------------------------------
void MyLEDButton_Rand( void *pClass, int id, bool bOn ) 
{
    SEQ_6x32x16 *mymodule;
    mymodule = (SEQ_6x32x16*)pClass;
    mymodule->Rand( id );
}

//-----------------------------------------------------
// MyLEDButton_Rand
//-----------------------------------------------------
void MyLEDButton_Clear( void *pClass, int id, bool bOn ) 
{
    SEQ_6x32x16 *mymodule;
    mymodule = (SEQ_6x32x16*)pClass;

    for( int i = 0; i < nSTEPS; i++ )
    {
        mymodule->m_Pattern[ id ][ mymodule->m_CurrentProg[ id ] ][ i ] = 0;
    }

    mymodule->m_pPatternDisplay[ id ]->SetPatAll( mymodule->m_Pattern[ id ][ mymodule->m_CurrentProg[ id ] ] );
}

//-----------------------------------------------------
// Procedure:   PatternChangeCallback
//
//-----------------------------------------------------
void SEQ_6x32x16_PatternChangeCallback ( void *pClass, int ch, int pat, int level, int maxpat )
{
    SEQ_6x32x16 *mymodule = (SEQ_6x32x16 *)pClass;

    if( !mymodule || !mymodule->m_bInitialized )
        return;

    mymodule->m_MaxPat[ ch ][ mymodule->m_CurrentProg[ ch ] ] = maxpat;
    mymodule->m_Pattern[ ch ][ mymodule->m_CurrentProg[ ch ] ] [ pat ] = level;
}

//-----------------------------------------------------
// Procedure:   ProgramChangeCallback
//
//-----------------------------------------------------
void SEQ_6x32x16_ProgramChangeCallback ( void *pClass, int ch, int pat, int max )
{
    SEQ_6x32x16 *mymodule = (SEQ_6x32x16 *)pClass;

    if( !mymodule || !mymodule->m_bInitialized )
        return;

    if( mymodule->m_MaxProg[ ch ] != max )
    {
        mymodule->m_MaxProg[ ch ] = max;
    }
    else if( mymodule->m_CurrentProg[ ch ] == pat && mymodule->m_bPauseState[ ch ] && mymodule->m_CopySrc != NO_COPY )
    {
        mymodule->ChangeProg( ch, pat, true );
    }
    else if( mymodule->m_CurrentProg[ ch ] != pat )
    {
        if( !mymodule->m_bPauseState[ ch ] && mymodule->inputs[ SEQ_6x32x16::IN_CLK + ch ].active )
            mymodule->SetPendingProg( ch, pat );
        else
            mymodule->ChangeProg( ch, pat, false );
    }
}

//-----------------------------------------------------
// Procedure:   Widget
//
//-----------------------------------------------------

struct SEQ_6x32x16_Widget : ModuleWidget 
{
    Menu *createContextMenu() override;
	SEQ_6x32x16_Widget( SEQ_6x32x16 *module );
};

SEQ_6x32x16_Widget::SEQ_6x32x16_Widget( SEQ_6x32x16 *module ) : ModuleWidget(module) 
{
    int x, y, x2, y2;
    ParamWidget *pWidget = NULL;

	box.size = Vec( 15*41, 380);

	{
		SVGPanel *panel = new SVGPanel();
		panel->box.size = box.size;
		panel->setBackground(SVG::load(assetPlugin(plugin, "res/SEQ_6x32x16.svg")));
		addChild(panel);
	}

    //module->lg.Open("SEQ_6x32x16.txt");

    x = 7;
    y = 22;

    // global inputs
    addInput(Port::create<MyPortInSmall>( Vec( 204, 357 ), Port::INPUT, module, SEQ_6x32x16::IN_GLOBAL_CLK_RESET ) );
    addInput(Port::create<MyPortInSmall>( Vec( 90, 357 ), Port::INPUT, module, SEQ_6x32x16::IN_GLOBAL_PAT_CHANGE ) );

    // trig mute
    module->m_pButtonTrigMute = new MyLEDButton( 491, 3, 15, 15, 13.0, DWRGB( 180, 180, 180 ), DWRGB( 255, 0, 0 ), MyLEDButton::TYPE_SWITCH, 0, module, MyLEDButton_TrigMute );
    addChild( module->m_pButtonTrigMute );

    addInput(Port::create<MyPortInSmall>( Vec( 466, 1 ), Port::INPUT, module, SEQ_6x32x16::IN_GLOBAL_TRIG_MUTE ) );

    for( int ch = 0; ch < nCHANNELS; ch++ )
    {
        // inputs
        addInput(Port::create<MyPortInSmall>( Vec( x + 6, y + 7 ), Port::INPUT, module, SEQ_6x32x16::IN_CLK + ch ) );
        addInput(Port::create<MyPortInSmall>( Vec( x + 64, y + 31 ), Port::INPUT, module, SEQ_6x32x16::IN_PAT_TRIG + ch ) );

        // pattern display
        module->m_pPatternDisplay[ ch ] = new SinglePatternClocked32( x + 39, y + 2, 13, 13, 5, 2, 7, DWRGB( 255, 128, 64 ), DWRGB( 18, 9, 0 ), DWRGB( 180, 75, 180 ), DWRGB( 80, 45, 80 ), nSTEPS, ch, module, SEQ_6x32x16_PatternChangeCallback );
	    addChild( module->m_pPatternDisplay[ ch ] );

        // program display
        module->m_pProgramDisplay[ ch ] = new PatternSelectStrip( x + 106, y + 31, 9, 7, DWRGB( 180, 180, 0 ), DWRGB( 90, 90, 64 ), DWRGB( 0, 180, 200 ), DWRGB( 0, 90, 90 ), nPROG, ch, module, SEQ_6x32x16_ProgramChangeCallback );
	    addChild( module->m_pProgramDisplay[ ch ] );

        // add knobs
        y2 = y + 34;
        //pWidget = ParamWidget::create<Knob_Green1_15>( Vec( x + 374, y2 ), module, SEQ_6x32x16::PARAM_SWING_KNOB + ch, 0.0, 0.6, 0.0 );

        // removed for now
        if( pWidget )
        {
            addParam( pWidget );
            pWidget->visible = false;
        }

        x2 = x + 447;
        addParam(ParamWidget::create<Knob_Green1_15>( Vec( x2, y2 ), module, SEQ_6x32x16::PARAM_LVL1_KNOB + ch, 0.0, 1.0, 0.25 ) ); x2 += 19;
        addParam(ParamWidget::create<Knob_Green1_15>( Vec( x2, y2 ), module, SEQ_6x32x16::PARAM_LVL2_KNOB + ch, 0.0, 1.0, 0.33 ) ); x2 += 19;
        addParam(ParamWidget::create<Knob_Green1_15>( Vec( x2, y2 ), module, SEQ_6x32x16::PARAM_LVL3_KNOB + ch, 0.0, 1.0, 0.50 ) ); x2 += 19;
        addParam(ParamWidget::create<Knob_Green1_15>( Vec( x2, y2 ), module, SEQ_6x32x16::PARAM_LVL4_KNOB + ch, 0.0, 1.0, 0.75 ) ); x2 += 19;
        addParam(ParamWidget::create<Knob_Green1_15>( Vec( x2, y2 ), module, SEQ_6x32x16::PARAM_LVL5_KNOB + ch, 0.0, 1.0, 0.9 ) );

        // add buttons
        module->m_pButtonAutoPat[ ch ] = new MyLEDButton( x + 55, y + 35, 9, 9, 6.0, DWRGB( 180, 180, 180 ), DWRGB( 0, 255, 0 ), MyLEDButton::TYPE_SWITCH, ch, module, MyLEDButton_AutoPat );
	    addChild( module->m_pButtonAutoPat[ ch ] );

        module->m_pButtonPause[ ch ] = new MyLEDButton( x + 26, y + 10, 11, 11, 8.0, DWRGB( 180, 180, 180 ), DWRGB( 255, 0, 0 ), MyLEDButton::TYPE_SWITCH, ch, module, MyLEDButton_Pause );
	    addChild( module->m_pButtonPause[ ch ] );

        y2 = y + 33;
        module->m_pButtonCopy[ ch ] = new MyLEDButton( x + 290, y2, 11, 11, 8.0, DWRGB( 180, 180, 180 ), DWRGB( 0, 244, 244 ), MyLEDButton::TYPE_SWITCH, ch, module, MyLEDButton_CpyNxt );
	    addChild( module->m_pButtonCopy[ ch ] );

        module->m_pButtonRand[ ch ] = new MyLEDButton( x + 315, y2, 11, 11, 8.0, DWRGB( 180, 180, 180 ), DWRGB( 0, 244, 244 ), MyLEDButton::TYPE_MOMENTARY, ch, module, MyLEDButton_Rand );
	    addChild( module->m_pButtonRand[ ch ] );

        module->m_pButtonClear[ ch ] = new MyLEDButton( x + 340, y2, 11, 11, 8.0, DWRGB( 180, 180, 180 ), DWRGB( 0, 244, 244 ), MyLEDButton::TYPE_MOMENTARY, ch, module, MyLEDButton_Clear );
	    addChild( module->m_pButtonClear[ ch ] );

        module->m_pButtonHoldCV[ ch ] = new MyLEDButton( x + 405, y2, 11, 11, 8.0, DWRGB( 180, 180, 180 ), DWRGB( 0, 244, 244 ), MyLEDButton::TYPE_SWITCH, ch, module, MyLEDButton_HoldCV );
	    addChild( module->m_pButtonHoldCV[ ch ] );

        module->m_pButtonBiLevel[ ch ] = new MyLEDButton( x + 425, y2, 11, 11, 8.0, DWRGB( 180, 180, 180 ), DWRGB( 0, 244, 244 ), MyLEDButton::TYPE_SWITCH, ch, module, MyLEDButton_BiLevel );
	    addChild( module->m_pButtonBiLevel[ ch ] );

        // add outputs
        addOutput(Port::create<MyPortOutSmall>( Vec( x + 580, y + 7 ), Port::OUTPUT, module, SEQ_6x32x16::OUT_TRIG + ch ) );
        addOutput(Port::create<MyPortOutSmall>( Vec( x + 544, y + 33 ), Port::OUTPUT, module, SEQ_6x32x16::OUT_LEVEL + ch ) );
        addOutput(Port::create<MyPortOutSmall>( Vec( x + 37, y + 31 ), Port::OUTPUT, module, SEQ_6x32x16::OUT_BEAT1 + ch ) );

        y += 56;
    }

	addChild(Widget::create<ScrewSilver>(Vec(15, 0)));
	addChild(Widget::create<ScrewSilver>(Vec(box.size.x-30, 0)));
	addChild(Widget::create<ScrewSilver>(Vec(15, 365))); 
	addChild(Widget::create<ScrewSilver>(Vec(box.size.x-30, 365)));

    module->m_bInitialized = true;

    module->onReset();
}

//-----------------------------------------------------
// Procedure: JsonParams  
//
//-----------------------------------------------------
void SEQ_6x32x16::JsonParams( bool bTo, json_t *root) 
{
    JsonDataBool    ( bTo, "m_bPauseState",     root, &m_bPauseState[ 0 ], nCHANNELS );
    JsonDataBool    ( bTo, "m_bBiLevelState",   root, &m_bBiLevelState[ 0 ], nCHANNELS );
    JsonDataInt     ( bTo, "m_Pattern",         root, &m_Pattern[ 0 ][ 0 ][ 0 ], nCHANNELS * nSTEPS * nPROG );
    JsonDataInt     ( bTo, "m_MaxPat",          root, &m_MaxPat[ 0 ][ 0 ], nCHANNELS * nPROG );
    JsonDataInt     ( bTo, "m_CurrentProg",     root, &m_CurrentProg[ 0 ], nCHANNELS );
    JsonDataInt     ( bTo, "m_MaxProg",         root, &m_MaxProg[ 0 ], nCHANNELS );
    JsonDataBool    ( bTo, "m_bAutoPatChange",  root, &m_bAutoPatChange[ 0 ], nCHANNELS );
    JsonDataBool    ( bTo, "m_bHoldCVState",    root, &m_bHoldCVState[ 0 ], nCHANNELS );
    JsonDataInt     ( bTo, "m_RangeSelect",     root, &m_RangeSelect, 1 );
    JsonDataBool    ( bTo, "m_bTrigMute",       root, &m_bTrigMute, 1 );
}

//-----------------------------------------------------
// Procedure: toJson  
//
//-----------------------------------------------------
json_t *SEQ_6x32x16::toJson() 
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
void SEQ_6x32x16::fromJson( json_t *root ) 
{
    JsonParams( FROMJSON, root );

    for( int ch = 0; ch < nCHANNELS; ch++ )
    {
        m_pButtonAutoPat[ ch ]->Set( m_bAutoPatChange[ ch ] );
        m_pButtonPause[ ch ]->Set( m_bPauseState[ ch ] );
        m_pButtonBiLevel[ ch ]->Set( m_bBiLevelState[ ch ] );
        m_pButtonHoldCV[ ch ]->Set( m_bHoldCVState[ ch ] );

        m_pPatternDisplay[ ch ]->SetPatAll( m_Pattern[ ch ][ m_CurrentProg[ ch ] ] );
        m_pPatternDisplay[ ch ]->SetMax( m_MaxPat[ ch ][ m_CurrentProg[ ch ] ] );

        m_pProgramDisplay[ ch ]->SetPat( m_CurrentProg[ ch ], false );
        m_pProgramDisplay[ ch ]->SetMax( m_MaxProg[ ch ] );
    }

	if( m_bTrigMute )
		m_pButtonTrigMute->Set( m_bTrigMute );

    sprintf( m_strRange, "%.1fV", m_fCVRanges[ m_RangeSelect ] );
}

//-----------------------------------------------------
// Procedure:   reset
//
//-----------------------------------------------------
void SEQ_6x32x16::onReset()
{
    if( !m_bInitialized )
        return;

    for( int ch = 0; ch < nCHANNELS; ch++ )
    {
        m_pButtonPause[ ch ]->Set( false );
        m_pButtonBiLevel[ ch ]->Set( false );
    }

    memset( m_bPauseState, 0, sizeof(m_bPauseState) );
    memset( m_bBiLevelState, 0, sizeof(m_bBiLevelState) );
    memset( m_Pattern, 0, sizeof(m_Pattern) );
    memset( m_CurrentProg, 0, sizeof(m_CurrentProg) );

    for( int ch = 0; ch < nCHANNELS; ch++ )
    {
        for( int prog = 0; prog < nCHANNELS; prog++ )
            m_MaxPat[ ch ][ prog ] = nSTEPS - 1;

        m_MaxProg[ ch ] = nPROG - 1;

        m_pPatternDisplay[ ch ]->SetPatAll( m_Pattern[ ch ][ 0 ] );
        m_pPatternDisplay[ ch ]->SetMax( m_MaxPat[ ch ][ 0 ] );

        m_pProgramDisplay[ ch ]->SetPat( 0, false );
        m_pProgramDisplay[ ch ]->SetMax( m_MaxProg[ ch ] );

        m_pPatternDisplay[ ch ]->SetPatAll( m_Pattern[ ch ][ m_CurrentProg[ ch ] ] );

        ChangeProg( ch, 0, true );
    }
}

//-----------------------------------------------------
// Procedure:   randomize
//
//-----------------------------------------------------
void SEQ_6x32x16::onRandomize()
{
    for( int ch = 0; ch < nCHANNELS; ch++ )
    {
        for( int p = 0; p < nPROG; p++ )
        {
            for( int i = 0; i < nSTEPS; i++ )
            {
                m_Pattern[ ch ][ p ][ i ] = (int)(randomUniform() * 5.0 );
            }
        }

        m_pPatternDisplay[ ch ]->SetPatAll( m_Pattern[ ch ][ m_CurrentProg[ ch ] ] );
    }
}

//-----------------------------------------------------
// Procedure:   Rand
//
//-----------------------------------------------------
void SEQ_6x32x16::Rand( int ch )
{
    for( int i = 0; i < nSTEPS; i++ )
    {
        if( i <= m_MaxPat[ ch ][ m_CurrentProg[ ch ] ] && randomUniform() > 0.5f )
            m_Pattern[ ch ][ m_CurrentProg[ ch ] ][ i ] = (int)(randomUniform() * 5.0 );
        else
            m_Pattern[ ch ][ m_CurrentProg[ ch ] ][ i ] = 0;
    }

    m_pPatternDisplay[ ch ]->SetPatAll( m_Pattern[ ch ][ m_CurrentProg[ ch ] ] );
}

//-----------------------------------------------------
// Procedure:   CpyNext
//
//-----------------------------------------------------
void SEQ_6x32x16::CpyNext( int ch )
{
    int next;

    next = m_CurrentProg[ ch ] + 1;

    if( next >= nPROG )
        return;

    memcpy( m_Pattern[ ch ][ next ], m_Pattern[ ch ][ m_CurrentProg[ ch ] ], sizeof(int) * nSTEPS );

    if(m_bPauseState[ ch ] || !inputs[ SEQ_6x32x16::IN_CLK + ch ].active )
        ChangeProg( ch, next, false );
}

//-----------------------------------------------------
// Procedure:   Copy
//
//-----------------------------------------------------
void SEQ_6x32x16::Copy( int ch, bool bOn )
{
    if( !m_bPauseState[ ch ] || !bOn || m_CopySrc != NO_COPY )
    {
        if( m_CopySrc != NO_COPY )
            m_pButtonCopy[ m_CopySrc ]->Set( false );

        m_CopySrc = NO_COPY;
        m_pButtonCopy[ ch ]->Set( false );
    }
    else if( bOn )
    {
        m_CopySrc = ch;
    }
}

//-----------------------------------------------------
// Procedure:   ChangProg
//
//-----------------------------------------------------
void SEQ_6x32x16::ChangeProg( int ch, int prog, bool bforce )
{
    if( ch < 0 || ch >= nCHANNELS )
        return;

    if( !bforce && prog == m_CurrentProg[ ch ] )
        return;

    if( prog < 0 )
        prog = nPROG - 1;
    else if( prog >= nPROG )
        prog = 0;

    if( m_CopySrc != NO_COPY )
    {
        if( m_bPauseState[ ch ] )
        {
            memcpy( m_Pattern[ ch ][ prog ], m_Pattern[ m_CopySrc ][ m_CurrentProg[ m_CopySrc ] ], sizeof(int) * nSTEPS );
            m_pButtonCopy[ m_CopySrc ]->Set( false );
            m_MaxPat[ ch ][ prog ] = m_MaxPat[ m_CopySrc ][ m_CurrentProg[ m_CopySrc ] ];
            m_CopySrc = NO_COPY;
        }
    }

    m_CurrentProg[ ch ] = prog;

    m_pPatternDisplay[ ch ]->SetPatAll( m_Pattern[ ch ][ prog ] );
    m_pPatternDisplay[ ch ]->SetMax( m_MaxPat[ ch ][ prog ] );
    m_pProgramDisplay[ ch ]->SetPat( prog, false );
}

//-----------------------------------------------------
// Procedure:   SetPendingProg
//
//-----------------------------------------------------
void SEQ_6x32x16::SetPendingProg( int ch, int progIn )
{
    int prog;

    if( progIn < 0 || progIn >= nPROG )
        prog = ( m_CurrentProg[ ch ] + 1 ) & 0xF;
    else
        prog = progIn;

    if( prog > m_MaxProg[ ch ] )
        prog = 0;

    m_ProgPending[ ch ].bPending = true;
    m_ProgPending[ ch ].prog = prog;
    m_pProgramDisplay[ ch ]->SetPat( m_CurrentProg[ ch ], false );
    m_pProgramDisplay[ ch ]->SetPat( prog, true );
}

//-----------------------------------------------------
// Procedure:   step
//
//-----------------------------------------------------
void SEQ_6x32x16::step() 
{
    int ch, level, useclock;
    bool bClk, bClkTrig, bClockAtZero = false, bGlobalClk = false, bGlobalProg = false, bTrigOut, bPatTrig[ nCHANNELS ] = {};
    float fout = 0.0;

    if( !m_bInitialized )
        return;

    if( inputs[ IN_GLOBAL_TRIG_MUTE ].active )
    {
		if( inputs[ IN_GLOBAL_TRIG_MUTE ].value >= 0.00001 )
		{
			m_bTrigMute = true;
			m_pButtonTrigMute->Set( true );
		}
		else if( inputs[ IN_GLOBAL_TRIG_MUTE ].value < 0.00001 )
		{
			m_bTrigMute = false;
			m_pButtonTrigMute->Set( false );
		}
    }

    if( inputs[ IN_GLOBAL_CLK_RESET ].active )
        bGlobalClk = m_SchTrigGlobalClkReset.process( inputs[ IN_GLOBAL_CLK_RESET ].value );

    if( inputs[ IN_GLOBAL_PAT_CHANGE ].active )
        bGlobalProg= m_SchTrigGlobalProg.process( inputs[ IN_GLOBAL_PAT_CHANGE ].value );

    // get trigs
    for( ch = 0; ch < nCHANNELS; ch++ )
        bPatTrig[ ch ] = m_SchTrigClock[ ch ].process( inputs[ IN_CLK + ch ].normalize( 0.0f ) );

    for( ch = 0; ch < nCHANNELS; ch++ )
    {
        bClkTrig = false;
        bClk = false;
        bTrigOut = false;
        bClockAtZero = false;
        useclock = ch;

        // if no keyboard clock active then use kb 0's clock
        if( !inputs[ IN_CLK + ch ].active && inputs[ IN_CLK + 0 ].active )
            useclock = 0;

        if( inputs[ IN_CLK + useclock ].active )
        {
            // time the clock tick
            if( bGlobalClk )
            {
                m_pPatternDisplay[ ch ]->ClockReset();
                bClkTrig = true;
                bClockAtZero = true;
                bClk = true;
            }

            // clock
            if( bPatTrig[ useclock ] && !bGlobalClk )
                bClkTrig = true;

            bClk = bClkTrig;

            // clock the pattern
            if( !m_bPauseState[ ch ] )
            {
                if( bGlobalProg || m_SchTrigProg[ ch ].process( inputs[ IN_PAT_TRIG + ch ].value ) )
                    SetPendingProg( ch, -1 );

                // clock in
                if( bClk )
                {
                    if( !bGlobalClk )
                        bClockAtZero = m_pPatternDisplay[ ch ]->ClockInc();

                    bTrigOut = ( m_Pattern[ ch ][ m_CurrentProg[ ch ] ][ m_pPatternDisplay[ ch ]->m_PatClk ] );
                }
            }
        }
        // no clock input
        else
        {
            // resolve any left over phrase triggers
            if( m_ProgPending[ ch ].bPending )
            {
                m_ProgPending[ ch ].bPending = false;
                ChangeProg( ch, m_ProgPending[ ch ].prog, true );
            }

            continue;
        }

        // auto inc pattern
        if( m_bAutoPatChange[ ch ] && bClockAtZero )
        {
            SetPendingProg( ch, -1 );
            m_ProgPending[ ch ].bPending = false;
            ChangeProg( ch, m_ProgPending[ ch ].prog, true );
            bTrigOut = ( m_Pattern[ ch ][ m_CurrentProg[ ch ] ][ m_pPatternDisplay[ ch ]->m_PatClk ] );
        }
        // resolve pattern change
        else if( m_ProgPending[ ch ].bPending && bClockAtZero )
        {
            m_ProgPending[ ch ].bPending = false;
            ChangeProg( ch, m_ProgPending[ ch ].prog, false );
            bTrigOut = ( m_Pattern[ ch ][ m_CurrentProg[ ch ] ][ m_pPatternDisplay[ ch ]->m_PatClk ] );
        }

        // trigger the beat 0 output
        if( bClockAtZero )
            m_gateBeatPulse[ ch ].trigger(1e-3);

        outputs[ OUT_BEAT1 + ch ].value = m_gateBeatPulse[ ch ].process( 1.0 / engineGetSampleRate() ) ? CV_MAX : 0.0;

        // trigger out
        if( bTrigOut )
        {
            m_gatePulse[ ch ].trigger(1e-3);
        }

        if( !m_bTrigMute )
        	outputs[ OUT_TRIG + ch ].value = m_gatePulse[ ch ].process( 1.0 / engineGetSampleRate() ) ? CV_MAX : 0.0;
        else
        	outputs[ OUT_TRIG + ch ].value = 0.0f;

        level = m_Pattern[ ch ][ m_CurrentProg[ ch ] ][ m_pPatternDisplay[ ch ]->m_PatClk ];

        // get actual level from knobs
        switch( level )
        {
        case 1:
            fout = params[ PARAM_LVL1_KNOB + ch ].value;
            break;
        case 2:
            fout = params[ PARAM_LVL2_KNOB + ch ].value;
            break;
        case 3:
            fout = params[ PARAM_LVL3_KNOB + ch ].value;
            break;
        case 4:
            fout = params[ PARAM_LVL4_KNOB + ch ].value;
            break;
        case 5:
            fout = params[ PARAM_LVL5_KNOB + ch ].value;
            break;
        default:
            if( m_bHoldCVState[ ch ] )
                continue;
            else
                fout = 0.0f;
        }

        // bidirectional convert to -1.0 to 1.0
        if( m_bBiLevelState[ ch ] )
            fout = ( fout * 2 ) - 1.0;

        if( m_bTrigMute )
        	outputs[ OUT_LEVEL + ch ].value = flast[ ch ];
        else
        {
        	flast[ ch ] = m_fCVRanges[ m_RangeSelect ] * fout;
        	outputs[ OUT_LEVEL + ch ].value = flast[ ch ];
        }
    }
}

//-----------------------------------------------------
// Procedure:   SEQ_6x32x16_CVRange
//
//-----------------------------------------------------
struct SEQ_6x32x16_CVRange : MenuItem 
{
    SEQ_6x32x16 *module;

    void onAction(EventAction &e) override 
    {
        module->m_RangeSelect++;

        if( module->m_RangeSelect > 2 )
            module->m_RangeSelect = 0;

        sprintf( module->m_strRange, "%.1fV", module->m_fCVRanges[ module->m_RangeSelect ] );
    }

    void step() override 
    {
        rightText = module->m_strRange;
    }
};

//-----------------------------------------------------
// Procedure:   createContextMenu
//
//-----------------------------------------------------
Menu *SEQ_6x32x16_Widget::createContextMenu() 
{
    Menu *menu = ModuleWidget::createContextMenu();

    SEQ_6x32x16 *mod = dynamic_cast<SEQ_6x32x16*>(module);

    assert(mod);

    menu->addChild(construct<MenuLabel>());

    menu->addChild(construct<MenuLabel>(&MenuLabel::text, "---- CV Output Level ----"));
    menu->addChild(construct<SEQ_6x32x16_CVRange>( &MenuItem::text, "VRange (15, 10, 5):", &SEQ_6x32x16_CVRange::module, mod ) );

    return menu;
}

} // namespace rack_plugin_mscHack

using namespace rack_plugin_mscHack;

RACK_PLUGIN_MODEL_INIT(mscHack, SEQ_6x32x16) {
   Model *modelSEQ_6x32x16 = Model::create<SEQ_6x32x16, SEQ_6x32x16_Widget>( "mscHack", "Seq_6ch_32step", "SEQ 6 x 32", SEQUENCER_TAG, MULTIPLE_TAG );
   return modelSEQ_6x32x16;
}
