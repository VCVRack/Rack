#include "mscHack.hpp"
//#include "mscHack_Controls.hpp"
#include "dsp/digital.hpp"
//#include "CLog.h"

namespace rack_plugin_mscHack {

#define nKEYBOARDS  3
#define nKEYS       37
#define nOCTAVESEL  5
#define nPATTERNS   16
#define nPHRASE_SAVES 8
#define SEMI ( 1.0f / 12.0f )

typedef struct
{
    float        fsemi;

}KEYBOARD_KEY_STRUCT;

typedef struct
{
    int     note;
    bool    bTrigOff;
    int     pad[ 6 ];

}PATTERN_STRUCT;

typedef struct
{
    bool    bPending;
    int     phrase;
}PHRASE_CHANGE_STRUCT;

//-----------------------------------------------------
// Module Definition
//
//-----------------------------------------------------
struct Seq_Triad2 : Module 
{
#define NO_COPY -1

	enum ParamIds 
    {
        PARAM_PAUSE,          
        PARAM_OCTAVES           = PARAM_PAUSE + ( nKEYBOARDS ),
        PARAM_GLIDE             = PARAM_OCTAVES + (nOCTAVESEL * nKEYBOARDS),
        PARAM_TRIGOFF           = PARAM_GLIDE + ( nKEYBOARDS ),
        nPARAMS                 = PARAM_TRIGOFF + ( nKEYBOARDS )
    };

	enum InputIds 
    {
        IN_PATTERN_TRIG,
        IN_VOCT_OFF             = IN_PATTERN_TRIG + ( nKEYBOARDS ),
        IN_PROG_CHANGE          = IN_VOCT_OFF + ( nKEYBOARDS ),
        IN_CLOCK_RESET          = IN_PROG_CHANGE + ( nKEYBOARDS ),
        IN_GLOBAL_PAT_CLK,
        nINPUTS
	};

	enum OutputIds 
    {
        OUT_TRIG,               
        OUT_VOCTS               = OUT_TRIG + nKEYBOARDS,
        nOUTPUTS                = OUT_VOCTS + nKEYBOARDS
	};

    CLog            lg;
    bool            m_bInitialized = false;

    // octaves
    int             m_Octave[ nKEYBOARDS ] = {};
    float           m_fCvStartOut[ nKEYBOARDS ] = {};
    float           m_fCvEndOut[ nKEYBOARDS ] = {};

    // patterns
    int             m_CurrentPattern[ nKEYBOARDS ] = {};
    PATTERN_STRUCT  m_PatternNotes[ nKEYBOARDS ][ nPHRASE_SAVES ][ nPATTERNS ] = {};
    SchmittTrigger  m_SchTrigPatternSelectInput[ nKEYBOARDS ];
    PatternSelectStrip *m_pPatternSelect[ nKEYBOARDS ] = {};

    int             m_CopySrc = NO_COPY;

    // phrase save
    int             m_CurrentPhrase[ nKEYBOARDS ] = {};
    PHRASE_CHANGE_STRUCT m_PhrasePending[ nKEYBOARDS ] = {};
    SchmittTrigger  m_SchTrigPhraseSelect[ nKEYBOARDS ] = {};
    int             m_PhrasesUsed[ nKEYBOARDS ] = {0};
    PatternSelectStrip *m_pPhraseSelect[ nKEYBOARDS ] = {};

    // number of steps
    int             m_nSteps[ nKEYBOARDS ][ nPHRASE_SAVES ] = {};    

    // pause button
    bool            m_bPause[ nKEYBOARDS ] = {};

    // triggers     
    bool            m_bTrig[ nKEYBOARDS ] = {};
    PulseGenerator  m_gatePulse[ nKEYBOARDS ];

    // global triggers
    SchmittTrigger  m_SchTrigGlobalClkReset;
    SchmittTrigger  m_SchTrigGlobalPatChange;
    bool            m_GlobalClkResetPending = false;

    // glide
    float           m_fglideInc[ nKEYBOARDS ] = {};
    int             m_glideCount[ nKEYBOARDS ] = {};
    float           m_fglide[ nKEYBOARDS ] = {};
    float           m_fLastNotePlayed[ nKEYBOARDS ];
    bool            m_bWasLastNotePlayed[ nKEYBOARDS ] = {};

    Keyboard_3Oct_Widget *pKeyboardWidget[ nKEYBOARDS ];
    float           m_fKeyNotes[ 37 ];

    float           m_VoctOffsetIn[ nKEYBOARDS ] = {};

    // buttons
    MyLEDButtonStrip *m_pButtonOctaveSelect[ nKEYBOARDS ] = {};
    MyLEDButton      *m_pButtonPause[ nKEYBOARDS ] = {};
    MyLEDButton      *m_pButtonTrig[ nKEYBOARDS ] = {};
    MyLEDButton      *m_pButtonCopy[ nKEYBOARDS ] = {};

    // Contructor
	Seq_Triad2() : Module( nPARAMS, nINPUTS, nOUTPUTS, 0 )
    {
        int i;

        for( i = 0; i < 37; i++ )
            m_fKeyNotes[ i ] = (float)i * SEMI;
    
    }

    // Overrides 
	void    step() override;
    json_t* toJson() override;
    void    fromJson(json_t *rootJ) override;
    void    onRandomize() override;
    void    onReset() override;

    void    SetPhraseSteps( int kb, int nSteps );
    void    SetSteps( int kb, int steps );
    void    SetKey( int kb );
    void    SetOut( int kb );
    void    ChangePattern( int kb, int index, bool bForce, bool bPlay );
    void    ChangePhrase( int kb, int index, bool bForce );
    void    SetPendingPhrase( int kb, int phrase );
    void    Copy( int kb, bool bOn );
};

//-----------------------------------------------------
// Seq_Triad2_OctSelect
//-----------------------------------------------------
void Seq_Triad2_OctSelect( void *pClass, int id, int nbutton, bool bOn )
{
    Seq_Triad2 *mymodule;
    mymodule = (Seq_Triad2*)pClass;

    mymodule->m_Octave[ id ] = nbutton;
    mymodule->SetOut( id );
}

//-----------------------------------------------------
// Seq_Triad2_Pause
//-----------------------------------------------------
void Seq_Triad2_Pause( void *pClass, int id, bool bOn ) 
{
    Seq_Triad2 *mymodule;
    mymodule = (Seq_Triad2*)pClass;
    mymodule->m_bPause[ id ] = !mymodule->m_bPause[ id ];
}

//-----------------------------------------------------
// Seq_Triad2_Trig
//-----------------------------------------------------
void Seq_Triad2_Trig( void *pClass, int id, bool bOn ) 
{
    Seq_Triad2 *mymodule;
    mymodule = (Seq_Triad2*)pClass;
    mymodule->m_PatternNotes[ id ][ mymodule->m_CurrentPhrase[ id ] ][ mymodule->m_CurrentPattern[ id ] ].bTrigOff = !mymodule->m_PatternNotes[ id ][ mymodule->m_CurrentPhrase[ id ] ][ mymodule->m_CurrentPattern[ id ] ].bTrigOff;
}

//-----------------------------------------------------
// Procedure:   Widget
//
//-----------------------------------------------------
void Seq_Triad2_Widget_NoteChangeCallback ( void *pClass, int kb, int notepressed, int *pnotes, bool bOn, int button )
{
    Seq_Triad2 *mymodule = (Seq_Triad2 *)pClass;

    if( !pClass )
        return;

    // don't allow program unless paused
    if( button == 1 && !mymodule->m_bPause[ kb ] )
        return;

    // right click advance step
    if( button == 1 )
    {
        mymodule->ChangePattern( kb, mymodule->m_CurrentPattern[ kb ] + 1, true, false );
        mymodule->m_PatternNotes[ kb ][ mymodule->m_CurrentPhrase[ kb ] ][ mymodule->m_CurrentPattern[ kb ] ].note = notepressed;
        mymodule->SetKey( kb );
    }
    else
        mymodule->m_PatternNotes[ kb ][ mymodule->m_CurrentPhrase[ kb ] ][ mymodule->m_CurrentPattern[ kb ] ].note = notepressed;

    mymodule->SetOut( kb );
}

//-----------------------------------------------------
// Procedure:   PatternChangeCallback
//
//-----------------------------------------------------
void Seq_Triad2_Widget_PatternChangeCallback ( void *pClass, int kb, int pat, int max )
{
    Seq_Triad2 *mymodule = (Seq_Triad2 *)pClass;

    if( !mymodule || !mymodule->m_bInitialized )
        return;

    if( mymodule->m_CurrentPattern[ kb ] != pat )
        mymodule->ChangePattern( kb, pat, false, true );
    else if( mymodule->m_nSteps[ kb ][ mymodule->m_CurrentPhrase[ kb ] ] != max )
        mymodule->SetSteps( kb, max );
}

//-----------------------------------------------------
// Procedure:   PhraseChangeCallback
//
//-----------------------------------------------------
void Seq_Triad2_Widget_PhraseChangeCallback ( void *pClass, int kb, int pat, int max )
{
    Seq_Triad2 *mymodule = (Seq_Triad2 *)pClass;

    if( !mymodule || !mymodule->m_bInitialized )
        return;

    if( mymodule->m_PhrasesUsed[ kb ] != max )
    {
        mymodule->SetPhraseSteps( kb, max ); 
    }
    else if( mymodule->m_CurrentPhrase[ kb ] == pat && mymodule->m_bPause[ kb ] && mymodule->m_CopySrc != NO_COPY )
    {
        mymodule->ChangePhrase( kb, pat, true );
    }
    else if( mymodule->m_CurrentPhrase[ kb ] != pat )
    {
        if( !mymodule->m_bPause[ kb ] && mymodule->inputs[ Seq_Triad2::IN_PATTERN_TRIG + kb ].active )
            mymodule->SetPendingPhrase( kb, pat );
        else
            mymodule->ChangePhrase( kb, pat, false );
            
    }
}

//-----------------------------------------------------
// MyLEDButton_Copy
//-----------------------------------------------------
void MyLEDButton_Copy( void *pClass, int id, bool bOn ) 
{
    Seq_Triad2 *mymodule;
    mymodule = (Seq_Triad2*)pClass;
    mymodule->Copy( id, bOn );
}

//-----------------------------------------------------
// Procedure:   Widget
//
//-----------------------------------------------------

struct Seq_Triad2_Widget : ModuleWidget {
	Seq_Triad2_Widget( Seq_Triad2 *module );
};

Seq_Triad2_Widget::Seq_Triad2_Widget( Seq_Triad2 *module ) : ModuleWidget(module) 
{
    int kb, x, x2, y, y2;

	box.size = Vec( 15*25, 380);

	{
		SVGPanel *panel = new SVGPanel();
		panel->box.size = box.size;
		panel->setBackground(SVG::load(assetPlugin(plugin, "res/TriadSequencer2.svg")));
		addChild(panel);
	}

    //module->lg.Open("TriadSequencer2.txt");

    //----------------------------------------------------
    // Keyboard Keys 
    y = 21;
    x = 11;

    for( kb = 0; kb < nKEYBOARDS; kb++ )
    {
        // pause button
        module->m_pButtonPause[ kb ] = new MyLEDButton( x + 60, y + 4, 11, 11, 8.0, DWRGB( 180, 180, 180 ), DWRGB( 255, 0, 0 ), MyLEDButton::TYPE_SWITCH, kb, module, Seq_Triad2_Pause );
	    addChild( module->m_pButtonPause[ kb ] );

        // trig button
        module->m_pButtonTrig[ kb ] = new MyLEDButton( x + 314, y + 5, 11, 11, 8.0, DWRGB( 180, 180, 180 ), DWRGB( 255, 0, 0 ), MyLEDButton::TYPE_SWITCH, kb, module, Seq_Triad2_Trig );
	    addChild( module->m_pButtonTrig[ kb ] );

        // glide knob
        addParam( ParamWidget::create<Knob_Yellow1_15>( Vec( x + 235, y + 86 ), module, Seq_Triad2::PARAM_GLIDE + kb, 0.0, 1.0, 0.0 ) );

        module->m_pButtonCopy[ kb ] = new MyLEDButton( x + 194, y + 89, 11, 11, 8.0, DWRGB( 180, 180, 180 ), DWRGB( 0, 244, 244 ), MyLEDButton::TYPE_SWITCH, kb, module, MyLEDButton_Copy );
	    addChild( module->m_pButtonCopy[ kb ] );

        x2 = x + 274;

        // octave select
        module->m_pButtonOctaveSelect[ kb ] = new MyLEDButtonStrip( x2, y + 90, 11, 11, 3, 8.0, nOCTAVESEL, false, DWRGB( 180, 180, 180 ), DWRGB( 0, 255, 255 ), MyLEDButtonStrip::TYPE_EXCLUSIVE, kb, module, Seq_Triad2_OctSelect );
	    addChild( module->m_pButtonOctaveSelect[ kb ] );

        // keyboard widget
        module->pKeyboardWidget[ kb ] = new Keyboard_3Oct_Widget( x + 39, y + 19, 1, kb, DWRGB( 255, 128, 64 ), module, Seq_Triad2_Widget_NoteChangeCallback, &module->lg );
	    addChild( module->pKeyboardWidget[ kb ] );

        // pattern selects
        module->m_pPatternSelect[ kb ] = new PatternSelectStrip( x + 79, y + 1, 9, 7, DWRGB( 255, 128, 64 ), DWRGB( 128, 64, 0 ), DWRGB( 255, 0, 128 ), DWRGB( 128, 0, 64 ), nPATTERNS, kb, module, Seq_Triad2_Widget_PatternChangeCallback );
	    addChild( module->m_pPatternSelect[ kb ] );

        // phrase selects
        module->m_pPhraseSelect[ kb ] = new PatternSelectStrip( x + 79, y + 86, 9, 7, DWRGB( 255, 255, 0 ), DWRGB( 128, 128, 64 ), DWRGB( 0, 255, 255 ), DWRGB( 0, 128, 128 ), nPHRASE_SAVES, kb, module, Seq_Triad2_Widget_PhraseChangeCallback );
	    addChild( module->m_pPhraseSelect[ kb ] );

        x2 = x + 9;
        y2 = y + 4;
        // prog change trigger
        addInput(Port::create<MyPortInSmall>( Vec( x2, y2 ), Port::INPUT, module, Seq_Triad2::IN_PATTERN_TRIG + kb ) ); y2 += 40;

        // VOCT offset input
        addInput(Port::create<MyPortInSmall>( Vec( x2, y2 ), Port::INPUT, module, Seq_Triad2::IN_VOCT_OFF + kb ) ); y2 += 40;

        // prog change trigger
        addInput(Port::create<MyPortInSmall>( Vec( x2, y2 ), Port::INPUT, module, Seq_Triad2::IN_PROG_CHANGE + kb ) );

        // outputs
        x2 = x + 330;
        addOutput(Port::create<MyPortOutSmall>( Vec( x2, y + 27 ), Port::OUTPUT, module, Seq_Triad2::OUT_VOCTS + kb ) );
        addOutput(Port::create<MyPortOutSmall>( Vec( x2, y + 68 ), Port::OUTPUT, module, Seq_Triad2::OUT_TRIG + kb ) );

        y += 111;
    }

    // reset inputs
    y2 = 357; 
    addInput(Port::create<MyPortInSmall>( Vec( x + 89, y2 ), Port::INPUT, module, Seq_Triad2::IN_CLOCK_RESET ) );
    addInput(Port::create<MyPortInSmall>( Vec( x + 166, y2 ), Port::INPUT, module, Seq_Triad2::IN_GLOBAL_PAT_CLK ) );

	addChild(Widget::create<ScrewSilver>(Vec(15, 0)));
	addChild(Widget::create<ScrewSilver>(Vec(box.size.x-30, 0)));
	addChild(Widget::create<ScrewSilver>(Vec(15, 365))); 
	addChild(Widget::create<ScrewSilver>(Vec(box.size.x-30, 365)));

    module->m_bInitialized = true;

    module->onReset();
}

//-----------------------------------------------------
// Procedure:   reset
//
//-----------------------------------------------------
void Seq_Triad2::onReset()
{
    if( !m_bInitialized )
        return;

    for( int kb = 0; kb < nKEYBOARDS; kb++ )
        m_pButtonOctaveSelect[ kb ]->Set( 0, true );

    memset( m_fCvStartOut, 0, sizeof(m_fCvStartOut) );
    memset( m_fCvEndOut, 0, sizeof(m_fCvEndOut) );
    memset( m_PatternNotes, 0, sizeof(m_PatternNotes) );
    
    for( int kb = 0; kb < nKEYBOARDS; kb++ )
    {
        for( int pat = 0; pat < nPHRASE_SAVES; pat++ )
            m_nSteps[ kb ][ pat ] = 3;

        SetPhraseSteps( kb, 3 );
        ChangePattern( kb, 0, true, true );
        ChangePhrase( kb, 0, true );
    }
}

//-----------------------------------------------------
// Procedure:   randomize
//
//-----------------------------------------------------
const int keyscalenotes[ 7 ] = { 0, 2, 4, 5, 7, 9, 11};
const int keyscalenotes_minor[ 7 ] = { 0, 2, 3, 5, 7, 9, 11};
void Seq_Triad2::onRandomize()
{
    int kb, pat, phrase, basekey, note;

    for( int kb = 0; kb < nKEYBOARDS; kb++ )
        m_pButtonOctaveSelect[ kb ]->Set( 0, true );

    memset( m_fCvStartOut, 0, sizeof(m_fCvStartOut) );
    memset( m_fCvEndOut, 0, sizeof(m_fCvEndOut) );
    memset( m_PatternNotes, 0, sizeof(m_PatternNotes) );

    basekey = (int)(randomUniform() * 24.0);

    for( kb = 0; kb < nKEYBOARDS; kb++ )
    {
        m_Octave[ kb ] = (int)( randomUniform() * 4.0 );

        for( pat = 0; pat < nPATTERNS; pat++ )
        {
            for( phrase = 0; phrase < nPHRASE_SAVES; phrase++ )
            {
                if( randomUniform() > 0.7 )
                    note = keyscalenotes_minor[ (int)(randomUniform() * 7.0 ) ];
                else
                    note = keyscalenotes[ (int)(randomUniform() * 7.0 ) ];

                m_PatternNotes[ kb ][ phrase ][ pat ].bTrigOff = ( randomUniform() < 0.10 );
                m_PatternNotes[ kb ][ phrase ][ pat ].note = basekey + note; 
            }
        }

        ChangePattern( kb, 0, true, true );
    }
}

//-----------------------------------------------------
// Procedure:   SetPhraseSteps
//
//-----------------------------------------------------
void Seq_Triad2::SetPhraseSteps( int kb, int nSteps )
{
    if( nSteps < 0 || nSteps >= nPHRASE_SAVES )
        nSteps = 0;

    m_PhrasesUsed[ kb ] = nSteps;
}

//-----------------------------------------------------
// Procedure:   SetSteps
//
//-----------------------------------------------------
void Seq_Triad2::SetSteps( int kb, int nSteps )
{
    if( nSteps < 0 || nSteps >= nPATTERNS )
        nSteps = 0;

    m_nSteps[ kb ][ m_CurrentPhrase[ kb ] ] = nSteps;
}

//-----------------------------------------------------
// Procedure:   Copy
//
//-----------------------------------------------------
void Seq_Triad2::Copy( int kb, bool bOn )
{
    if( kb < 0 || kb >= nKEYBOARDS )
        return;

    if( !m_bPause[ kb ] || !bOn || m_CopySrc != NO_COPY )
    {
        if( m_CopySrc != NO_COPY )
            m_pButtonCopy[ m_CopySrc ]->Set( false );

        m_CopySrc = NO_COPY;
        m_pButtonCopy[ kb ]->Set( false );
    }
    else if( bOn )
    {
        m_CopySrc = kb;
    }
}

//-----------------------------------------------------
// Procedure:   SetOut
//
//-----------------------------------------------------
void Seq_Triad2::SetOut( int kb )
{
    int note;
    float foct;

    if( kb < 0 || kb >= nKEYBOARDS )
        return;

    // end glide note (current pattern note)
    foct = (float)m_Octave[ kb ];

    note = m_PatternNotes[ kb ][ m_CurrentPhrase[ kb ] ][ m_CurrentPattern[ kb ] ].note;

    if( note > 36 || note < 0 )
        note = 0;

    m_fCvEndOut[ kb ] = foct + m_fKeyNotes[ note ] + m_VoctOffsetIn[ kb ];

    // start glide note (last pattern note)
    if( m_bWasLastNotePlayed[ kb ] )
    {
        m_fCvStartOut[ kb ] = m_fLastNotePlayed[ kb ] + m_VoctOffsetIn[ kb ];
    }
    else
    {
        m_bWasLastNotePlayed[ kb ] = true;
        m_fCvStartOut[ kb ] = m_fCvEndOut[ kb ] + m_VoctOffsetIn[ kb ];
    }

    m_fLastNotePlayed[ kb ] = m_fCvEndOut[ kb ] + m_VoctOffsetIn[ kb ];

    // glide time ( max glide = 0.5 seconds )
    m_glideCount[ kb ] = 1 + (int)( ( params[ PARAM_GLIDE + kb ].value * 0.5 ) * engineGetSampleRate());

    m_fglideInc[ kb ] = 1.0 / (float)m_glideCount[ kb ];

    m_fglide[ kb ] = 1.0;

    // always trig on pause
    if( m_bPause[ kb ] )
        m_bTrig[ kb ] = true;
    else if( !m_PatternNotes[ kb ][ m_CurrentPhrase[ kb ] ][ m_CurrentPattern[ kb ] ].bTrigOff )
        m_bTrig[ kb ] = true;
    else
        m_bTrig[ kb ] = false;
}

//-----------------------------------------------------
// Procedure:   SetKey
//
//-----------------------------------------------------
void Seq_Triad2::SetKey( int kb )
{
    pKeyboardWidget[ kb ]->setkey( &m_PatternNotes[ kb ][ m_CurrentPhrase[ kb ] ][ m_CurrentPattern[ kb ] ].note );
}

//-----------------------------------------------------
// Procedure:   SetPendingPhrase
//
//-----------------------------------------------------
void Seq_Triad2::SetPendingPhrase( int kb, int phraseIn )
{
    int phrase;

    if( phraseIn < 0 || phraseIn >= nPHRASE_SAVES )
        phrase = ( m_CurrentPhrase[ kb ] + 1 ) & 0x7;
    else
        phrase = phraseIn;

    if( phrase > m_PhrasesUsed[ kb ] )
        phrase = 0;

    m_PhrasePending[ kb ].bPending = true;
    m_PhrasePending[ kb ].phrase = phrase;
    m_pPhraseSelect[ kb ]->SetPat( m_CurrentPhrase[ kb ], false );
    m_pPhraseSelect[ kb ]->SetPat( phrase, true );
}

//-----------------------------------------------------
// Procedure:   ChangePhrase
//
//-----------------------------------------------------
void Seq_Triad2::ChangePhrase( int kb, int index, bool bForce )
{
    if( kb < 0 || kb >= nKEYBOARDS )
        return;

    if( !bForce && index == m_CurrentPhrase[ kb ] )
        return;

    if( index < 0 )
        index = nPHRASE_SAVES - 1;
    else if( index >= nPHRASE_SAVES )
        index = 0;

    if( m_CopySrc != NO_COPY )
    {
        // do not copy if we are not paused
        if( m_bPause[ kb ] )
        {
            memcpy( m_PatternNotes[ kb ][ index ], m_PatternNotes[ m_CopySrc ][ m_CurrentPhrase[ m_CopySrc ] ], sizeof(PATTERN_STRUCT) * nPATTERNS );
            m_pButtonCopy[ m_CopySrc ]->Set( false );
            m_nSteps[ kb ][ index ] = m_nSteps[ m_CopySrc ][ m_CurrentPhrase[ m_CopySrc ] ];
            m_CopySrc = NO_COPY;
        }
    }

    m_CurrentPhrase[ kb ] = index;

    m_pPhraseSelect[ kb ]->SetPat( index, false );
    m_pPhraseSelect[ kb ]->SetMax( m_PhrasesUsed[ kb ] );
    m_pPatternSelect[ kb ]->SetMax( m_nSteps[ kb ][ index ] );

    // set keyboard key
    SetKey( kb );

    m_pButtonTrig[ kb ]->Set( m_PatternNotes[ kb ][ m_CurrentPhrase[ kb ] ][ m_CurrentPattern[ kb ] ].bTrigOff );

    // change octave light
    m_pButtonOctaveSelect[ kb ]->Set( m_Octave[ kb ], true );

    // set output note
    SetOut( kb );
}

//-----------------------------------------------------
// Procedure:   ChangePattern
//
//-----------------------------------------------------
void Seq_Triad2::ChangePattern( int kb, int index, bool bForce, bool bPlay )
{
    if( kb < 0 || kb >= nKEYBOARDS )
        return;

    if( !bForce && index == m_CurrentPattern[ kb ] )
        return;

    if( index < 0 )
        index = nPATTERNS - 1;
    else if( index >= nPATTERNS )
        index = 0;

    // update octave offset immediately when not running
    m_VoctOffsetIn[ kb ] = inputs[ IN_VOCT_OFF + kb ].normalize( 0.0 );

    m_CurrentPattern[ kb ] = index;

    m_pPatternSelect[ kb ]->SetPat( index, false );

    // set keyboard key
    SetKey( kb );

    m_pButtonTrig[ kb ]->Set( m_PatternNotes[ kb ][ m_CurrentPhrase[ kb ] ][ m_CurrentPattern[ kb ] ].bTrigOff );

    // change octave light
    m_pButtonOctaveSelect[ kb ]->Set( m_Octave[ kb ], true );

    // set outputted note
    if( bPlay )
        SetOut( kb );
}

//-----------------------------------------------------
// Procedure:   
//
//-----------------------------------------------------
json_t *Seq_Triad2::toJson() 
{
    int *pint;
    bool *pbool;
    json_t *gatesJ;
	json_t *rootJ = json_object();

    // pauses
    pbool = &m_bPause[ 0 ];

	gatesJ = json_array();

	for (int i = 0; i < nKEYBOARDS; i++)
    {
		json_t *gateJ = json_integer( (int)pbool[ i ] );
		json_array_append_new( gatesJ, gateJ );
	}

	json_object_set_new( rootJ, "m_bPause", gatesJ );

    // number of steps
    pint = (int*)&m_nSteps[ 0 ][ 0 ];

	gatesJ = json_array();

	for (int i = 0; i < nKEYBOARDS * nPHRASE_SAVES; i++)
    {
		json_t *gateJ = json_integer( pint[ i ] );
		json_array_append_new( gatesJ, gateJ );
	}

	json_object_set_new( rootJ, "m_nSteps", gatesJ );

    // octaves
    pint = (int*)&m_Octave[ 0 ];

	gatesJ = json_array();

	for (int i = 0; i < nKEYBOARDS; i++)
    {
		json_t *gateJ = json_integer( pint[ i ] );
		json_array_append_new( gatesJ, gateJ );
	}

	json_object_set_new( rootJ, "m_Octave", gatesJ );

    // phrase select
    pint = (int*)&m_CurrentPhrase[ 0 ];

	gatesJ = json_array();

	for (int i = 0; i < nKEYBOARDS; i++)
    {
		json_t *gateJ = json_integer( pint[ i ] );
		json_array_append_new( gatesJ, gateJ );
	}

	json_object_set_new( rootJ, "m_CurrentPhrase", gatesJ );

    // patterns
    pint = (int*)&m_PatternNotes[ 0 ][ 0 ][ 0 ];

	gatesJ = json_array();

	for (int i = 0; i < (nPHRASE_SAVES * nPATTERNS * nKEYBOARDS * 8); i++)
    {
		json_t *gateJ = json_integer( pint[ i ] );
		json_array_append_new( gatesJ, gateJ );
	}

	json_object_set_new( rootJ, "m_PatternNotes", gatesJ );


    // phrase used
    pint = (int*)&m_PhrasesUsed[ 0 ];

	gatesJ = json_array();

	for (int i = 0; i < nKEYBOARDS; i++)
    {
		json_t *gateJ = json_integer( pint[ i ] );
		json_array_append_new( gatesJ, gateJ );
	}

	json_object_set_new( rootJ, "m_PhrasesUsed", gatesJ );

    // current pattern
    pint = (int*)&m_CurrentPattern[ 0 ];

	gatesJ = json_array();

	for (int i = 0; i < nKEYBOARDS; i++)
    {
		json_t *gateJ = json_integer( pint[ i ] );
		json_array_append_new( gatesJ, gateJ );
	}

	json_object_set_new( rootJ, "m_CurrentPattern", gatesJ );

	return rootJ;
}

//-----------------------------------------------------
// Procedure:   fromJson
//
//-----------------------------------------------------
void Seq_Triad2::fromJson(json_t *rootJ) 
{
    bool *pbool;
    int *pint;
    int i;
    json_t *StepsJ;

    // pauses
    pbool = &m_bPause[ 0 ];
	StepsJ = json_object_get( rootJ, "m_bPause" );

	if (StepsJ) 
    {
		for ( i = 0; i < nKEYBOARDS; i++)
        {
			json_t *gateJ = json_array_get(StepsJ, i);

			if (gateJ)
				pbool[ i ] = json_integer_value( gateJ );
		}
	}

    // number of steps
    pint = (int*)&m_nSteps[ 0 ][ 0 ];
	StepsJ = json_object_get( rootJ, "m_nSteps" );

	if (StepsJ) 
    {
		for ( i = 0; i < nKEYBOARDS * nPHRASE_SAVES; i++)
        {
			json_t *gateJ = json_array_get(StepsJ, i);

			if (gateJ)
				pint[ i ] = json_integer_value( gateJ );
		}
	}

    // octaves
    pint = (int*)&m_Octave[ 0 ];
	StepsJ = json_object_get( rootJ, "m_Octave" );

	if (StepsJ) 
    {
		for ( i = 0; i < nKEYBOARDS; i++)
        {
			json_t *gateJ = json_array_get(StepsJ, i);

			if (gateJ)
				pint[ i ] = json_integer_value( gateJ );
		}
	}

    // phrase select
    pint = (int*)&m_CurrentPhrase[ 0 ];
	StepsJ = json_object_get( rootJ, "m_CurrentPhrase" );

	if (StepsJ) 
    {
		for ( i = 0; i < nKEYBOARDS; i++)
        {
			json_t *gateJ = json_array_get(StepsJ, i);

			if (gateJ)
				pint[ i ] = json_integer_value( gateJ );
		}
	}
    
    // all patterns and phrases
    pint = (int*)&m_PatternNotes[ 0 ][ 0 ][ 0 ];

	StepsJ = json_object_get( rootJ, "m_PatternNotes" );

	if (StepsJ) 
    {
		for ( i = 0; i < (nPHRASE_SAVES * nPATTERNS * nKEYBOARDS * 8); i++)
        {
			json_t *gateJ = json_array_get(StepsJ, i);

			if (gateJ)
				pint[ i ] = json_integer_value( gateJ );
		}
	}

    // phrase steps
    pint = (int*)&m_PhrasesUsed[ 0 ];

	StepsJ = json_object_get( rootJ, "m_PhrasesUsed" );

	if (StepsJ) 
    {
		for ( i = 0; i < nKEYBOARDS; i++)
        {
			json_t *gateJ = json_array_get(StepsJ, i);

			if (gateJ)
				pint[ i ] = json_integer_value( gateJ );
		}
	}

    // current pattern
    pint = (int*)&m_CurrentPattern[ 0 ];

	StepsJ = json_object_get( rootJ, "m_CurrentPattern" );

	if (StepsJ) 
    {
		for ( i = 0; i < nKEYBOARDS; i++)
        {
			json_t *gateJ = json_array_get(StepsJ, i);

			if (gateJ)
				pint[ i ] = json_integer_value( gateJ );
		}
	}

    for( i = 0; i < nKEYBOARDS; i++ )
    {
        m_pPhraseSelect[ i ]->SetMax( m_PhrasesUsed[ i ] );
        m_pPhraseSelect[ i ]->SetPat( m_CurrentPhrase[ i ], false );

        m_pPatternSelect[ i ]->SetMax( m_nSteps[ i ][ m_CurrentPhrase[ i ] ] );
        m_pPatternSelect[ i ]->SetPat( m_CurrentPattern[ i ], false );

        m_pButtonPause[ i ]->Set( m_bPause[ i ] );

        SetPhraseSteps( i, m_PhrasesUsed[ i ] );
        ChangePhrase( i, m_CurrentPhrase[ i ], true );
        ChangePattern( i, m_CurrentPattern[ i ], true, false );
    }
}

//-----------------------------------------------------
// Procedure:   step
//
//-----------------------------------------------------
#define LIGHT_LAMBDA ( 0.065f )
void Seq_Triad2::step() 
{
    int kb, useclock;
    bool bGlobalPatChange = false, bGlobalClkTriggered = false, PatTrig[ nKEYBOARDS ] = {};

    if( !m_bInitialized )
        return;

    // global phrase change trigger
    if( inputs[ IN_GLOBAL_PAT_CLK ].active )
    {
	    if( m_SchTrigGlobalPatChange.process( inputs[ IN_GLOBAL_PAT_CLK ].value ) )
            bGlobalPatChange = true;
    }

    // global clock reset
    if( inputs[ IN_CLOCK_RESET ].active )
    {
	    if( m_SchTrigGlobalClkReset.process( inputs[ IN_CLOCK_RESET ].value ) )
            m_GlobalClkResetPending = true;
    }

    // get trigs
    for( kb = 0; kb < nKEYBOARDS; kb++ )
        PatTrig[ kb ] = m_SchTrigPatternSelectInput[ kb ].process( inputs[ IN_PATTERN_TRIG + kb ].value );

    // process triggered phrase changes
    for( kb = 0; kb < nKEYBOARDS; kb++ )
    {
        useclock = kb;

        // if no keyboard clock active then use kb 0's clock
        if( !inputs[ IN_PATTERN_TRIG + kb ].active && inputs[ IN_PATTERN_TRIG + 0 ].active )
            useclock = 0;

        // phrase change trigger
        if( bGlobalPatChange && !m_bPause[ kb ] && inputs[ IN_PATTERN_TRIG + useclock ].active )
        {
            SetPendingPhrase( kb, -1 );
        }
        else if( inputs[ IN_PROG_CHANGE + kb ].active && !m_bPause[ kb ] && inputs[ IN_PATTERN_TRIG + useclock ].active )
        {
	        if( m_SchTrigPhraseSelect[ kb ].process( inputs[ IN_PROG_CHANGE + kb ].value ) )
                SetPendingPhrase( kb, -1 );
        }

	    // pat change trigger - ignore if already pending
        if( inputs[ IN_PATTERN_TRIG + useclock ].active && !m_bPause[ kb ] )
        {
            if( PatTrig[ useclock ] ) 
            {
                if( m_GlobalClkResetPending )
                {
                    bGlobalClkTriggered = true;
                    m_CurrentPattern[ kb ] = -1;
                }
                else if( m_CurrentPattern[ kb ] >= ( m_nSteps[ kb ][ m_CurrentPhrase[ kb ] ] ) )
                {
                    m_CurrentPattern[ kb ] = (nPATTERNS - 1);
                }

                ChangePattern( kb, m_CurrentPattern[ kb ] + 1, true, true );

                if( m_CurrentPattern[ kb ] == 0 )
                {
                    if( m_PhrasePending[ kb ].bPending )
                    {
                        m_PhrasePending[ kb ].bPending = false;
                        ChangePhrase( kb, m_PhrasePending[ kb ].phrase, false );
                    }
                }
            }
        }
        else
        {
            // resolve any left over phrase triggers
            if( m_PhrasePending[ kb ].bPending )
            {
                m_PhrasePending[ kb ].bPending = false;
                ChangePhrase( kb, m_PhrasePending[ kb ].phrase, false );
            }
        }

        if( m_bTrig[ kb ] )
        {
            m_bTrig[ kb ] = false;
            m_gatePulse[ kb ].trigger(1e-3);
        }

        outputs[ OUT_TRIG + kb ].value = m_gatePulse[ kb ].process( 1.0 / engineGetSampleRate() ) ? CV_MAX : 0.0;

        if( --m_glideCount[ kb ] > 0 )
            m_fglide[ kb ] -= m_fglideInc[ kb ];
        else
            m_fglide[ kb ] = 0.0;

        outputs[ OUT_VOCTS + kb ].value = ( m_fCvStartOut[ kb ] * m_fglide[ kb ] ) + ( m_fCvEndOut[ kb ] * ( 1.0 - m_fglide[ kb ] ) );
    }

    if( bGlobalClkTriggered )
        m_GlobalClkResetPending = false;
}

} // namespace rack_plugin_mscHack

using namespace rack_plugin_mscHack;

RACK_PLUGIN_MODEL_INIT(mscHack, Seq_Triad2) {
   Model *modelSeq_Triad2 = Model::create<Seq_Triad2, Seq_Triad2_Widget>( "mscHack", "TriadSeq2", "SEQ Triad 2", SEQUENCER_TAG, MULTIPLE_TAG );
   return modelSeq_Triad2;
}
