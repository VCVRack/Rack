#include "mscHack.hpp"
//#include "mscHack_Controls.hpp"
#include "dsp/digital.hpp"
//#include "CLog.h"

namespace rack_plugin_mscHack {

#define nCHANNELS 4

#define MID_INDEX 12
#define CLOCK_DIVS 25
const int multdisplayval[ CLOCK_DIVS ] = { 32, 24, 16, 12, 9, 8, 7, 6, 5, 4, 3, 2, 1, 2, 3, 4, 5, 6, 7, 8, 9, 12, 16, 24, 32 };

//-----------------------------------------------------
// Module Definition
//
//-----------------------------------------------------
struct MasterClockx4 : Module 
{
	enum ParamIds 
    {
        PARAM_BPM,
        PARAM_MULT,
        PARAM_HUMANIZE  = PARAM_MULT + nCHANNELS,
        nPARAMS
    };

	enum InputIds 
    {
        INPUT_EXT_SYNC,
        INPUT_CHAIN     = INPUT_EXT_SYNC + nCHANNELS,
        nINPUTS         
	};

	enum OutputIds 
    {
        OUTPUT_CLK,
        OUTPUT_TRIG     = OUTPUT_CLK + ( nCHANNELS * 4 ),
        OUTPUT_CHAIN    = OUTPUT_TRIG + ( nCHANNELS * 4 ),
        nOUTPUTS
	};

    bool            m_bInitialized = false;
    CLog            lg;

    float                m_fBPM = 120;

    MyLED7DigitDisplay  *m_pDigitDisplayMult[ nCHANNELS ] = {};
    MyLED7DigitDisplay  *m_pDigitDisplayBPM  = NULL;

    MyLEDButton         *m_pButtonGlobalStop = NULL;
    MyLEDButton         *m_pButtonGlobalTrig = NULL;
    MyLEDButton         *m_pButtonStop[ nCHANNELS ] = {};
    MyLEDButton         *m_pButtonTrig[ nCHANNELS ] = {};
    MyLEDButton         *m_pButtonTimeX2[ nCHANNELS ] = {};

    bool                m_bGlobalSync = false;
    bool                m_bStopState[ nCHANNELS ] = {};
    bool                m_bGlobalStopState = false;
    bool                m_bChannelSyncPending[ nCHANNELS ] = {};
    bool                m_bTimeX2[ nCHANNELS ] = {};

    int                 m_ChannelDivBeatCount[ nCHANNELS ] = {};
    float               m_fChannelBeatsPers[ nCHANNELS ] = {};
    float               m_fChannelClockCount[ nCHANNELS ] = {};
    int                 m_ChannelMultSelect[ nCHANNELS ] = {};

    float               m_fHumanize = 0;
    float               m_bWasChained = false;

    float               m_fBeatsPers;
    float               m_fMainClockCount;

    PulseGenerator      m_PulseClock[ nCHANNELS ];
    PulseGenerator      m_PulseSync[ nCHANNELS ];
    SchmittTrigger      m_SchmittSyncIn[ nCHANNELS ];

    ParamWidget        *m_pBpmKnob = NULL;
    ParamWidget        *m_pHumanKnob = NULL;

    // Contructor
	MasterClockx4() : Module(nPARAMS, nINPUTS, nOUTPUTS){}

    //-----------------------------------------------------
    // MyHumanize_Knob
    //-----------------------------------------------------
    struct MyHumanize_Knob : Knob_Yellow2_26
    {
        MasterClockx4 *mymodule;

        void onChange( EventChange &e ) override 
        {
            mymodule = (MasterClockx4*)module;

            if( mymodule && !mymodule->inputs[ INPUT_CHAIN ].active )
                mymodule->GetNewHumanizeVal();

		    RoundKnob::onChange( e );
	    }
    };

    //-----------------------------------------------------
    // MyBPM_Knob
    //-----------------------------------------------------
    struct MyBPM_Knob : Knob_Yellow2_56
    {
        MasterClockx4 *mymodule;

        void onChange( EventChange &e ) override 
        {
            mymodule = (MasterClockx4*)module;

            if( mymodule && !mymodule->inputs[ INPUT_CHAIN ].active  )
                mymodule->BPMChange( value, false );

		    RoundKnob::onChange( e );
	    }
    };

    //-----------------------------------------------------
    // MyMult_Knob
    //-----------------------------------------------------
    
    struct MyMult_Knob : Knob_Yellow2_26_Snap
    {
        MasterClockx4 *mymodule;
        int ch, col;

        void onChange( EventChange &e ) override 
        {
            mymodule = (MasterClockx4*)module;

            if( mymodule )
            {
                //if( !mymodule->m_bInitialized )
                    //return;

                ch = paramId - MasterClockx4::PARAM_MULT;

                //if( ch >= 0 && ch <= 3 )
                //{
                    mymodule->SetDisplayLED( ch, (int)value );
                //}
            }

		    RoundKnob::onChange( e );
	    }
    };

    // Overrides 
	void    step() override;
    void    JsonParams( bool bTo, json_t *root);
    json_t* toJson() override;
    void    fromJson(json_t *rootJ) override;
    void    onReset() override;

    void    GetNewHumanizeVal( void );
    void    BPMChange( float fbmp, bool bforce );
    void    CalcChannelClockRate( int ch );
    void    SetDisplayLED( int ch, int val );
};

//-----------------------------------------------------
// MyLEDButton_GlobalStop
//-----------------------------------------------------
void MyLEDButton_GlobalStop( void *pClass, int id, bool bOn ) 
{
    MasterClockx4 *mymodule;
    mymodule = (MasterClockx4*)pClass;
    mymodule->m_bGlobalStopState = bOn;
}

//-----------------------------------------------------
// MyLEDButton_TimeX2
//-----------------------------------------------------
void MyLEDButton_TimeX2( void *pClass, int id, bool bOn ) 
{
    MasterClockx4 *mymodule;
    mymodule = (MasterClockx4*)pClass;
    mymodule->m_bTimeX2[ id ] = bOn;
    mymodule->SetDisplayLED( id, (int)mymodule->params[ MasterClockx4::PARAM_MULT + id ].value );
}

//-----------------------------------------------------
// MyLEDButton_GlobalTrig
//-----------------------------------------------------
void MyLEDButton_GlobalTrig( void *pClass, int id, bool bOn ) 
{
    MasterClockx4 *mymodule;
    mymodule = (MasterClockx4*)pClass;
    mymodule->m_bGlobalSync = true;
}

//-----------------------------------------------------
// MyLEDButton_ChannelStop
//-----------------------------------------------------
void MyLEDButton_ChannelStop ( void *pClass, int id, bool bOn ) 
{
    MasterClockx4 *mymodule;
    mymodule = (MasterClockx4*)pClass;
    mymodule->m_bStopState[ id ] = bOn;
}

//-----------------------------------------------------
// MyLEDButton_ChannelSync
//-----------------------------------------------------
void MyLEDButton_ChannelSync( void *pClass, int id, bool bOn ) 
{
    MasterClockx4 *mymodule;
    mymodule = (MasterClockx4*)pClass;
    mymodule->m_bChannelSyncPending[ id ] = true;

    if( mymodule->m_pButtonTrig[ id ] )
        mymodule->m_pButtonTrig[ id ]->Set( true );
}

//-----------------------------------------------------
// Procedure:   Widget
//
//-----------------------------------------------------

struct MasterClockx4_Widget : ModuleWidget {
	MasterClockx4_Widget( MasterClockx4 *module );
};

MasterClockx4_Widget::MasterClockx4_Widget( MasterClockx4 *module ) : ModuleWidget(module) 
{
    int ch, x, y;

	box.size = Vec( 15*18, 380);

	{
		SVGPanel *panel = new SVGPanel();
		panel->box.size = box.size;
		panel->setBackground(SVG::load(assetPlugin( plugin, "res/MasterClockx4.svg")));
		addChild(panel);
	}

    //module->lg.Open("MasterClockx4.txt");

    // bpm knob
    module->m_pBpmKnob = ParamWidget::create<MasterClockx4::MyBPM_Knob>( Vec( 9, 50 ), module, MasterClockx4::PARAM_BPM, 60.0, 220.0, 120.0 );
    addParam( module->m_pBpmKnob );

    // bpm display
    module->m_pDigitDisplayBPM = new MyLED7DigitDisplay( 5, 115, 0.055, DWRGB( 0, 0, 0 ), DWRGB( 0xFF, 0xFF, 0xFF ), MyLED7DigitDisplay::TYPE_FLOAT, 5 );
    addChild( module->m_pDigitDisplayBPM );

    // global stop switch
    module->m_pButtonGlobalStop = new MyLEDButton( 22, 144, 25, 25, 20.0, DWRGB( 180, 180, 180 ), DWRGB( 255, 0, 0 ), MyLEDButton::TYPE_SWITCH, 0, module, MyLEDButton_GlobalStop );
	addChild( module->m_pButtonGlobalStop );

    // global sync button
    module->m_pButtonGlobalTrig = new MyLEDButton( 22, 202, 25, 25, 20.0, DWRGB( 180, 180, 180 ), DWRGB( 0, 255, 255 ), MyLEDButton::TYPE_MOMENTARY, 0, module, MyLEDButton_GlobalTrig );
	addChild( module->m_pButtonGlobalTrig );

    // humanize knob
    module->m_pHumanKnob = ParamWidget::create<MasterClockx4::MyHumanize_Knob>( Vec( 22, 235 ), module, MasterClockx4::PARAM_HUMANIZE, 0.0, 1.0, 0.0 );
    addParam( module->m_pHumanKnob );

    // add chain out
    addOutput(Port::create<MyPortOutSmall>( Vec( 30, 345 ), Port::OUTPUT, module, MasterClockx4::OUTPUT_CHAIN ) );

    // chain in
    addInput(Port::create<MyPortInSmall>( Vec( 30, 13 ), Port::INPUT, module, MasterClockx4::INPUT_CHAIN ) );

    x = 91;
    y = 39;

    for( ch = 0; ch < nCHANNELS; ch++ )
    {
        // x2
        module->m_pButtonTimeX2[ ch ] = new MyLEDButton( x + 2, y + 2, 11, 11, 9.0, DWRGB( 180, 180, 180 ), DWRGB( 0, 255, 255 ), MyLEDButton::TYPE_SWITCH, ch, module, MyLEDButton_TimeX2 );
	    addChild( module->m_pButtonTimeX2[ ch ] );

        // clock mult knob
        addParam(ParamWidget::create<MasterClockx4::MyMult_Knob>( Vec( x + 13, y + 13 ), module, MasterClockx4::PARAM_MULT + ch, 0.0f, (float)(CLOCK_DIVS - 1), (float)(MID_INDEX) ) );

        // mult display
        module->m_pDigitDisplayMult[ ch ] = new MyLED7DigitDisplay( x + 10, y + 48, 0.07, DWRGB( 0, 0, 0 ), DWRGB( 0xFF, 0xFF, 0xFF ), MyLED7DigitDisplay::TYPE_INT, 2 );
        addChild( module->m_pDigitDisplayMult[ ch ] );

        // sync triggers
        module->m_pButtonTrig[ ch ] = new MyLEDButton( x + 76, y + 4, 19, 19, 15.0, DWRGB( 180, 180, 180 ), DWRGB( 0, 255, 255 ), MyLEDButton::TYPE_MOMENTARY, ch, module, MyLEDButton_ChannelSync );
	    addChild( module->m_pButtonTrig[ ch ] );

        addInput(Port::create<MyPortInSmall>( Vec( x + 54, y + 5 ), Port::INPUT, module, MasterClockx4::INPUT_EXT_SYNC + ch ) );
        addOutput(Port::create<MyPortOutSmall>( Vec( x + 54, y + 31 ), Port::OUTPUT, module, MasterClockx4::OUTPUT_TRIG  + (ch * 4) + 0 ) );
        addOutput(Port::create<MyPortOutSmall>( Vec( x + 54, y + 53 ), Port::OUTPUT, module, MasterClockx4::OUTPUT_TRIG + (ch * 4) + 1 ) );
        addOutput(Port::create<MyPortOutSmall>( Vec( x + 77, y + 31 ), Port::OUTPUT, module, MasterClockx4::OUTPUT_TRIG + (ch * 4) + 2 ) );
        addOutput(Port::create<MyPortOutSmall>( Vec( x + 77, y + 53 ), Port::OUTPUT, module, MasterClockx4::OUTPUT_TRIG + (ch * 4) + 3 ) );

        // clock out
        module->m_pButtonStop[ ch ] = new MyLEDButton( x + 132, y + 4, 19, 19, 15.0, DWRGB( 180, 180, 180 ), DWRGB( 255, 0, 0 ), MyLEDButton::TYPE_SWITCH, ch, module, MyLEDButton_ChannelStop );
	    addChild( module->m_pButtonStop[ ch ] );

        addOutput(Port::create<MyPortOutSmall>( Vec( x + 107, y + 31 ), Port::OUTPUT, module, MasterClockx4::OUTPUT_CLK + (ch * 4) + 0 ) );
        addOutput(Port::create<MyPortOutSmall>( Vec( x + 107, y + 53 ), Port::OUTPUT, module, MasterClockx4::OUTPUT_CLK + (ch * 4) + 1 ) );
        addOutput(Port::create<MyPortOutSmall>( Vec( x + 130, y + 31 ), Port::OUTPUT, module, MasterClockx4::OUTPUT_CLK + (ch * 4) + 2 ) );
        addOutput(Port::create<MyPortOutSmall>( Vec( x + 130, y + 53 ), Port::OUTPUT, module, MasterClockx4::OUTPUT_CLK + (ch * 4) + 3 ) );

        y += 80;
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
void MasterClockx4::JsonParams( bool bTo, json_t *root) 
{
    JsonDataBool( bTo, "m_bGlobalStopState", root, &m_bGlobalStopState, 1 );
    JsonDataBool( bTo, "m_bStopState", root, m_bStopState, nCHANNELS );
    JsonDataBool( bTo, "m_bTimeX2", root, m_bTimeX2, nCHANNELS );
    JsonDataInt ( bTo, "m_ChannelMultSelect", root, m_ChannelMultSelect, nCHANNELS );
}

//-----------------------------------------------------
// Procedure:   
//
//-----------------------------------------------------
json_t *MasterClockx4::toJson() 
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
void MasterClockx4::fromJson(json_t *root) 
{
    JsonParams( FROMJSON, root );

    m_pButtonGlobalStop->Set( m_bGlobalStopState );

    for( int ch = 0; ch < nCHANNELS; ch++ )
    {
        m_pButtonStop[ ch ]->Set( m_bStopState[ ch ] );
        m_pButtonTimeX2[ ch ]->Set( m_bTimeX2[ ch ] );
        //lg.f( "value = %d\n", (int)params[ PARAM_MULT + ch ].value );
        SetDisplayLED( ch, m_ChannelMultSelect[ ch ] );
    }

    m_fMainClockCount = 0;
    BPMChange( params[ PARAM_BPM ].value, true );

    if( m_pDigitDisplayBPM )
        m_pDigitDisplayBPM->SetFloat( m_fBPM );
}

//-----------------------------------------------------
// Procedure:   reset
//
//-----------------------------------------------------
void MasterClockx4::onReset()
{
    if( !m_bInitialized )
        return;

    m_fBPM = 120;

    if( m_pDigitDisplayBPM )
        m_pDigitDisplayBPM->SetFloat( m_fBPM );

    m_bGlobalStopState = false;
    m_pButtonGlobalStop->Set( m_bGlobalStopState );

    for( int ch = 0; ch < nCHANNELS; ch++ )
    {
        m_bTimeX2[ ch ] = false;
        m_bStopState[ ch ] = false;
        m_pButtonStop[ ch ]->Set( false );
        m_pButtonTimeX2[ ch ]->Set( false );
        SetDisplayLED( ch, MID_INDEX );
    }

    BPMChange( m_fBPM, true );
}

//-----------------------------------------------------
// Procedure:   SetDisplayLED
//
//-----------------------------------------------------
void MasterClockx4::SetDisplayLED( int ch, int val )
{
    int col, mult = 1;

    if( !m_bInitialized )
        return;

    if( m_bTimeX2[ ch ] )
        mult = 2;

    if( val < MID_INDEX )
    {
        col = DWRGB( 0xFF, 0, 0 );
    }
    else if( val > MID_INDEX )
    {
        col = DWRGB( 0, 0xFF, 0xFF );
    }
    else
    {
        mult = 1;
        col = DWRGB( 0xFF, 0xFF, 0xFF );
    }

    if( m_pDigitDisplayMult[ ch ] )
    {
        m_ChannelMultSelect[ ch ] = val;
        m_pDigitDisplayMult[ ch ]->SetLEDCol( col );


        m_pDigitDisplayMult[ ch ]->SetInt( multdisplayval[ val ] * mult );
    }

    CalcChannelClockRate( ch );
}

//-----------------------------------------------------
// Procedure:   GetNewHumanizeVal
//
//-----------------------------------------------------
void MasterClockx4::GetNewHumanizeVal( void )
{
    m_fHumanize = randomUniform() * engineGetSampleRate() * 0.1 * params[ PARAM_HUMANIZE ].value;

    if( randomUniform() > 0.5 )
        m_fHumanize *= -1;
}

//-----------------------------------------------------
// Procedure:   BMPChange
//
//-----------------------------------------------------
void MasterClockx4::BPMChange( float fbpm, bool bforce )
{
    // don't change if it is already the same
    if( !bforce && ( (int)(fbpm * 1000.0f ) == (int)(m_fBPM * 1000.0f ) ) )
        return;

    m_fBPM = fbpm;
    m_fBeatsPers = fbpm / 60.0;

    if( m_pDigitDisplayBPM )
       m_pDigitDisplayBPM->SetFloat( m_fBPM );

    for( int i = 0; i < nCHANNELS; i++ )
        CalcChannelClockRate( i );
}

//-----------------------------------------------------
// Procedure:   CalcChannelClockRate
//
//-----------------------------------------------------
void MasterClockx4::CalcChannelClockRate( int ch )
{
    int mult = 1;

    if( m_bTimeX2[ ch ] )
        mult = 2;

    // for beat division just keep a count of beats
    if( m_ChannelMultSelect[ ch ] == MID_INDEX )
        m_ChannelDivBeatCount[ ch ] = 1;
    else if( m_ChannelMultSelect[ ch ] < MID_INDEX )
        m_ChannelDivBeatCount[ ch ] = multdisplayval[ m_ChannelMultSelect[ ch ] ] * mult;
    else
        m_fChannelBeatsPers[ ch ] = m_fBeatsPers * (float)( multdisplayval[ m_ChannelMultSelect[ ch ] ] * mult );
}

//-----------------------------------------------------
// Procedure:   step
//
//-----------------------------------------------------
void MasterClockx4::step() 
{
    int ch, mult = 1;
    unsigned int ival;
    float fSyncPulseOut, fClkPulseOut;
    bool bMainClockTrig = false, bChannelClockTrig;

    if( !m_bInitialized )
        return;

    // use the input chain trigger for our clock
    if( inputs[ INPUT_CHAIN ].active )
    {
        if( !m_bWasChained )
        {
            m_pHumanKnob->visible = false;
            m_pBpmKnob->visible = false;
        }

        m_bWasChained = true;

        // value of less than zero is a trig
        if( inputs[ INPUT_CHAIN ].value < 8.0 )
        {
			ival = (unsigned int)inputs[ INPUT_CHAIN ].value;

			bMainClockTrig = ( ival & 1 );

			m_bGlobalSync = ( ival & 2 );

			m_bGlobalStopState = ( ival & 4 );

			m_pButtonGlobalStop->Set( m_bGlobalStopState );
        }
        // values greater than zero are the bpm
        else
        {
            BPMChange( inputs[ INPUT_CHAIN ].value, false );
        }
    }
    else
    {
        // go back to our bpm if chain removed
        if( m_bWasChained )
        {
            m_pHumanKnob->visible = true;
            m_pBpmKnob->visible = true;
            m_bWasChained = false;
            BPMChange( params[ PARAM_BPM ].value, false );
            m_pButtonGlobalStop->Set( m_bGlobalStopState );
        }

        // keep track of main bpm
        m_fMainClockCount += m_fBeatsPers;
        //if( ( m_fMainClockCount + m_fHumanize ) >= engineGetSampleRate() )
        if( m_fMainClockCount >= (engineGetSampleRate()-1) )
        {
            //m_fMainClockCount = ( m_fMainClockCount + m_fHumanize ) - engineGetSampleRate();
        	m_fMainClockCount = m_fMainClockCount - (engineGetSampleRate()-1);

            GetNewHumanizeVal();

            bMainClockTrig = true;
        }
    }

    // send chain
    if( outputs[ OUTPUT_CHAIN ].active )
    {
    	ival = 0;

        if( bMainClockTrig )
        	ival |= 1;

        if( m_bGlobalSync )
        	ival |= 2;

        if( m_bGlobalStopState )
        	ival |= 4;

        if( ival )
            outputs[ OUTPUT_CHAIN ].value = (float)ival;
        else
            outputs[ OUTPUT_CHAIN ].value = m_fBPM;
    }

    for( ch = 0; ch < nCHANNELS; ch++ )
    {
        bChannelClockTrig = false;

        // sync triggers
        if( m_bGlobalSync || m_SchmittSyncIn[ ch ].process( inputs[ INPUT_EXT_SYNC + ch ].value ) )
        {
            if( m_pButtonTrig[ ch ] )
                m_pButtonTrig[ ch ]->Set( true );

            m_bChannelSyncPending[ ch ] = true;
        }

        // sync all triggers to main clock pulse
        if( bMainClockTrig )
        {
            if( m_bChannelSyncPending[ ch ] )
            {
                if( m_pButtonTrig[ ch ] )
                    m_pButtonTrig[ ch ]->Set( true );

                m_bChannelSyncPending[ ch ] = false;
                m_PulseSync[ ch ].trigger(1e-3);

                m_ChannelDivBeatCount[ ch ] = 0;
                m_fChannelClockCount[ ch ] = 0;
                bChannelClockTrig = true;
            }
            else
            {
                // divisions of clock will count beats
                if( m_ChannelMultSelect[ ch ] < MID_INDEX )
                {
                    if( m_bTimeX2[ ch ] )
                        mult = 2;

                    if( ++m_ChannelDivBeatCount[ ch ] >= ( multdisplayval[ m_ChannelMultSelect[ ch ] ] * mult ) )
                    {
                        m_ChannelDivBeatCount[ ch ] = 0;
                        bChannelClockTrig = true;
                    }
                }
                // multiples of clock will sync with every beat
                else
                {
                    m_fChannelClockCount[ ch ] = 0;
                    bChannelClockTrig = true;
                }
            }
        }
        // do multiple clocks
        else if( m_ChannelMultSelect[ ch ] > MID_INDEX )
        {
            m_fChannelClockCount[ ch ] += m_fChannelBeatsPers[ ch ];
            if( m_fChannelClockCount[ ch ] >= engineGetSampleRate() )
            {
                m_fChannelClockCount[ ch ] = m_fChannelClockCount[ ch ] - engineGetSampleRate();
                bChannelClockTrig = true;
            }
        }

        if( bChannelClockTrig )
            m_PulseClock[ ch ].trigger( 0.050f );

        // syncs
        fSyncPulseOut = m_PulseSync[ ch ].process( 1.0 / engineGetSampleRate() ) ? CV_MAX : 0.0;
        outputs[ OUTPUT_TRIG + (ch * 4) + 0 ].value = fSyncPulseOut;
        outputs[ OUTPUT_TRIG + (ch * 4) + 1 ].value = fSyncPulseOut;
        outputs[ OUTPUT_TRIG + (ch * 4) + 2 ].value = fSyncPulseOut;
        outputs[ OUTPUT_TRIG + (ch * 4) + 3 ].value = fSyncPulseOut;

        // clocks
        fClkPulseOut = m_PulseClock[ ch ].process( 1.0 / engineGetSampleRate() ) ? CV_MAX : 0.0;

        if( !m_bGlobalStopState && !m_bStopState[ ch ] )
        {
            outputs[ OUTPUT_CLK + (ch * 4) + 0 ].value = fClkPulseOut;
            outputs[ OUTPUT_CLK + (ch * 4) + 1 ].value = fClkPulseOut;
            outputs[ OUTPUT_CLK + (ch * 4) + 2 ].value = fClkPulseOut;
            outputs[ OUTPUT_CLK + (ch * 4) + 3 ].value = fClkPulseOut;
        }
    }

    m_bGlobalSync = false;
}

} // namespace rack_plugin_mscHack

using namespace rack_plugin_mscHack;

RACK_PLUGIN_MODEL_INIT(mscHack, MasterClockx4) {
   Model *modelMasterClockx4 = Model::create<MasterClockx4, MasterClockx4_Widget>( "mscHack", "MasterClockx4", "Master CLOCK x 4", CLOCK_TAG, QUAD_TAG );
   return modelMasterClockx4;
}
