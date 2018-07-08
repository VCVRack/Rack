#include "mscHack.hpp"
//#include "mscHack_Controls.hpp"
#include "dsp/digital.hpp"
//#include "CLog.h"

namespace rack_plugin_mscHack {

#define PATTERNS 16
#define STEPS    16
#define CHANNELS 3
#define LIGHT_LAMBDA ( 0.065f )
#define CHANNEL_OFF_H 91

//-----------------------------------------------------
// Module Definition
//
//-----------------------------------------------------
struct Seq_3x16x16 : Module 
{
	enum ParamIds 
    {
        PARAM_RAND,
        PARAM_CPY_NEXT          = PARAM_RAND + CHANNELS,
		PARAM_RUN               = PARAM_CPY_NEXT + CHANNELS,
		PARAM_RESET,
		PARAM_SLIDERS,
		PARAM_STEPS             = PARAM_SLIDERS  + ( CHANNELS * STEPS ),
		PARAM_PATTERNS          = PARAM_STEPS    + ( CHANNELS * STEPS ),
        PARAM_STEP_NUM          = PARAM_PATTERNS + ( CHANNELS * PATTERNS ),
        PARAM_GLOBAL_PAT        = PARAM_STEP_NUM + ( STEPS ),
        nPARAMS                 = PARAM_GLOBAL_PAT + ( PATTERNS )
    };

	enum InputIds 
    {
		INPUT_EXT_CLOCK,
		INPUT_RESET,
        INPUT_GLOBAL_PAT_CHANGE,
        INPUT_PAT_CHANGE,
        nINPUTS = INPUT_PAT_CHANGE + CHANNELS
	};

	enum OutputIds 
    {
		OUT_GATES,
        OUT_CV      = OUT_GATES + CHANNELS,
        nOUTPUTS    = OUT_CV + CHANNELS
	};

	enum LightIds 
    {
		LIGHT_RUN,
        LIGHT_RESET,
        LIGHT_STEP_NUM,
        LIGHT_STEP          = LIGHT_STEP_NUM + ( STEPS ),
        LIGHT_PAT           = LIGHT_STEP + ( CHANNELS * STEPS ),
        LIGHT_COPY          = LIGHT_PAT + ( CHANNELS * PATTERNS ),
        LIGHT_RAND          = LIGHT_COPY + CHANNELS,
        LIGHT_GLOBAL_PAT    = LIGHT_RAND + CHANNELS,
        nLIGHTS             = LIGHT_GLOBAL_PAT + PATTERNS
	};

	enum GateMode 
    {
		TRIGGER,
		RETRIGGER,
		CONTINUOUS,
	};

    CLog            lg;
    bool            m_bInitJson = false;
	GateMode        m_GateMode = TRIGGER;
	PulseGenerator  m_PulseStep;
	bool            m_bRunning = true;
	float           m_fPhase = 0.0;

    // Inputs
	SchmittTrigger  m_SchTrigClock;
	SchmittTrigger  m_SchTrigRun;
	SchmittTrigger  m_SchTrigReset;
    SchmittTrigger  m_SchTrigPatChange[ CHANNELS ];

    // random pattern button
    SchmittTrigger  m_SchTrigRandPat;

    // copy next buttons
    SchmittTrigger  m_SchTrigCopyNext;

    // number of steps
    int             m_nSteps = STEPS;    
    SchmittTrigger  m_SchTrigStepNumbers[ STEPS ];
    
    // Level settings
    ParamWidget     *m_pLevelToggleParam2[ CHANNELS ][ STEPS ] = {};
    SchmittTrigger  m_SchTrigLevels[ CHANNELS ][ STEPS ];
    float           m_fLevels[ PATTERNS ][ CHANNELS ][ STEPS ] = {};

    // Global Pattern Select
    SchmittTrigger  m_SchTrigGlobalPatternSelect;
    int             m_GlobalSelect = 0;
    bool            m_bGlobalPatternChangePending = false;
    int             m_GlobalPendingLight = 0;

    // Pattern Select
    int             m_PatternSelect[ CHANNELS ] = {0};    
    SchmittTrigger  m_SchTrigPatternSelects[ CHANNELS ][ PATTERNS ];
    bool            m_bPatternPending = false;
    bool            m_bPatternChangePending[ CHANNELS ] = {0};
    int             m_PendingLight[ CHANNELS ] = {0};

    // Steps
    int             m_CurrentStep = 0;
    SchmittTrigger  m_SchTrigSteps[ CHANNELS ][ STEPS ];
    bool            m_bStepStates[ PATTERNS ][ CHANNELS ][ STEPS ] = {};    
    float           m_fLightStepLevels[ CHANNELS ][ STEPS ] = {};

    // Contructor
	Seq_3x16x16() : Module( nPARAMS, nINPUTS, nOUTPUTS, nLIGHTS ){} 

    // Overrides 
	void    step() override;
    json_t* toJson() override;
    void    fromJson(json_t *rootJ) override;
    void    onRandomize() override;
    void    onReset() override;

    void Randomize_Channel( int ch );
    void CopyNext( int ch );
    void ChangePattern( int ch, int index, bool bForceChange );
    void SetSteps( int nSteps );
    void SetGlobalPattern( int step );
};

//-----------------------------------------------------
// Procedure:   MySquareButton_CopyNext
//
//-----------------------------------------------------
struct MySquareButton_CopyNext : MySquareButton
{
    Seq_3x16x16 *mymodule;
    int param;

    void onChange( EventChange &e ) override 
    {
        mymodule = (Seq_3x16x16*)module;

        if( mymodule && value == 1.0 )
        {
            param = paramId - Seq_3x16x16::PARAM_CPY_NEXT;
            mymodule->CopyNext( param );
        }

		MomentarySwitch::onChange( e );
	}
};

//-----------------------------------------------------
// Procedure:   MySquareButton_Rand
//
//-----------------------------------------------------
struct MySquareButton_Rand : MySquareButton
{
    Seq_3x16x16 *mymodule;
    int param;

    void onChange( EventChange &e ) override 
    {
        mymodule = (Seq_3x16x16*)module;

        if( mymodule && value == 1.0 )
        {
            param = paramId - Seq_3x16x16::PARAM_RAND;
            mymodule->Randomize_Channel( param );
        }

		MomentarySwitch::onChange( e );
	}
};

//-----------------------------------------------------
// Procedure:   MySquareButton_GlobalPattern
//
//-----------------------------------------------------
struct MySquareButton_GlobalPattern : MySquareButton //SVGSwitch, MomentarySwitch
{
    Seq_3x16x16 *mymodule;
    int param;

    void onChange( EventChange &e ) override 
    {
        mymodule = (Seq_3x16x16*)module;

        if( mymodule && value == 1.0 )
        {
            param = paramId - Seq_3x16x16::PARAM_GLOBAL_PAT;

            if( mymodule->m_bRunning )
            {
                if( param != mymodule->m_GlobalSelect && !mymodule->m_bGlobalPatternChangePending )
                {
                    mymodule->m_bGlobalPatternChangePending = true;
                    mymodule->m_GlobalPendingLight = param;
                }
            }
            else
            {
                mymodule->SetGlobalPattern( param );
            }
        }

		MomentarySwitch::onChange( e );
	}
};

//-----------------------------------------------------
// Procedure:   MySquareButton_Pattern
//
//-----------------------------------------------------
struct MySquareButton_Pattern : MySquareButton //SVGSwitch, MomentarySwitch
{
    Seq_3x16x16 *mymodule;
    int ch, stp, param;

    void onChange( EventChange &e ) override 
    {
        mymodule = (Seq_3x16x16*)module;

        if( mymodule && value == 1.0 )
        {
            param = paramId - Seq_3x16x16::PARAM_PATTERNS;
            ch = param / STEPS;
            stp = param - (ch * STEPS);

            //mymodule->lg.f( (char*)"pat = %d, ch = %d, stp = %d\n", param, ch, stp );

            if( mymodule->m_bRunning )
            {
                if( stp != mymodule->m_PatternSelect[ ch ] && !mymodule->m_bPatternChangePending[ ch ] )
                {
                    mymodule->m_bPatternPending = true;
                    mymodule->m_bPatternChangePending[ ch ] = true;
                    mymodule->m_PendingLight[ ch ] = stp;
                }
            }
            else
            {
                mymodule->ChangePattern( ch, stp, false );
            }
        }

		MomentarySwitch::onChange( e );
	}
};

//-----------------------------------------------------
// Procedure:   MySquareButton_Step
//
//-----------------------------------------------------
struct MySquareButton_Step : MySquareButton //SVGSwitch, MomentarySwitch 
{
    Seq_3x16x16 *mymodule;
    int ch, stp, param;

    void onChange( EventChange &e ) override 
    {
        mymodule = (Seq_3x16x16*)module;

        if( mymodule && value == 1.0 )
        {
            param = paramId - Seq_3x16x16::PARAM_STEPS;
            ch = param / STEPS;
            stp = param - (ch * STEPS);

            //lg.f( (char*)"step = %d, ch = %d, stp = %d\n", param, ch, stp );
            mymodule->m_bStepStates[ mymodule->m_PatternSelect[ ch ] ][ ch ][ stp ] = !mymodule->m_bStepStates[ mymodule->m_PatternSelect[ ch ] ][ ch ][ stp ];
            mymodule->lights[ Seq_3x16x16::LIGHT_STEP + ( ch * STEPS ) + stp ].value = mymodule->m_bStepStates[ mymodule->m_PatternSelect[ ch ] ][ ch ][ stp ] ? 1.0f : 0.0f;
        }

		MomentarySwitch::onChange( e );
	}
};

//-----------------------------------------------------
// Procedure:   MySquareButton_Step
//
//-----------------------------------------------------
struct MySquareButton_StepNum : MySquareButton 
{
    Seq_3x16x16 *mymodule;
    int stp, i;

    void onChange( EventChange &e ) override 
    {
        mymodule = (Seq_3x16x16*)module;

        if( mymodule && value == 1.0 )
        {
            stp = paramId - Seq_3x16x16::PARAM_STEP_NUM;
            mymodule->SetSteps( stp + 1 );
        }

		MomentarySwitch::onChange( e );
	}
};

//-----------------------------------------------------
// Procedure:   MySlider_Levels
//
//-----------------------------------------------------
struct MySlider_Levels : MySlider_01
{
    Seq_3x16x16 *mymodule;
    int ch, stp, param;

    void onChange( EventChange &e ) override 
    {
        mymodule = (Seq_3x16x16*)module;

        if( mymodule )
        {
            param = paramId - Seq_3x16x16::PARAM_SLIDERS;
            ch = param / STEPS;
            stp = param - (ch * STEPS);

            //lg.f( "slider = %d, ch = %d, stp = %d, value = %.3f\n", param, ch, stp, value );

            mymodule->m_fLevels[ mymodule->m_PatternSelect[ ch ] ][ ch ][ stp ] = value;
        }

        SVGFader::onChange( e );
	}
};

//-----------------------------------------------------
// Procedure:   Widget
//
//-----------------------------------------------------

struct Seq_3x16x16_Widget : ModuleWidget {
	Seq_3x16x16_Widget( Seq_3x16x16 *module );
};

Seq_3x16x16_Widget::Seq_3x16x16_Widget( Seq_3x16x16 *module ) : ModuleWidget(module) 
{
    int i, ch, stp, x, y;

	box.size = Vec(15*23, 380);

	{
		SVGPanel *panel = new SVGPanel();
		panel->box.size = box.size;
		panel->setBackground(SVG::load(assetPlugin(plugin, "res/Seq_3x16x16.svg")));
		addChild(panel);
	}

    //module->lg.Open("Seq_3x16x16.txt");

	addChild(Widget::create<ScrewSilver>(Vec(15, 0)));
	addChild(Widget::create<ScrewSilver>(Vec(box.size.x-30, 0)));
	addChild(Widget::create<ScrewSilver>(Vec(15, 365))); 
	addChild(Widget::create<ScrewSilver>(Vec(box.size.x-30, 365)));

    // clk/run button
	addParam(ParamWidget::create<LEDButton>(Vec( 78, 17 ), module, Seq_3x16x16::PARAM_RUN, 0.0, 1.0, 0.0 ) );
    addChild(ModuleLightWidget::create<SmallLight<GreenLight>>( Vec( 78 + 5, 17 + 5 ), module, Seq_3x16x16::LIGHT_RUN ) );
    addInput(Port::create<MyPortInSmall>( Vec( 47, 17 ), Port::INPUT, module, Seq_3x16x16::INPUT_EXT_CLOCK ) );

    // reset button
	addParam(ParamWidget::create<LEDButton>( Vec( 143, 17 ), module, Seq_3x16x16::PARAM_RESET, 0.0, 1.0, 0.0 ) );
    addChild(ModuleLightWidget::create<SmallLight<GreenLight>>( Vec( 143 + 5, 17 + 5 ), module, Seq_3x16x16::LIGHT_RESET ) );
    addInput(Port::create<MyPortInSmall>( Vec( 117, 17 ), Port::INPUT, module, Seq_3x16x16::INPUT_RESET ) );

    //----------------------------------------------------
    // Step Select buttons 

    y = 41;
    x = 50;

	for ( stp = 0; stp < STEPS; stp++ ) 
    {
        // step button
		addParam(ParamWidget::create<MySquareButton_StepNum>( Vec( x, y ), module, Seq_3x16x16::PARAM_STEP_NUM + stp, 0.0, 1.0, 0.0 ) );
        addChild(ModuleLightWidget::create<SmallLight<DarkRedValueLight>>( Vec( x + 2, y + 3 ), module, Seq_3x16x16::LIGHT_STEP_NUM + stp ) );

        if( ( stp & 0x3 ) == 0x3 )
            x += 20;
        else
            x += 15;

        module->lights[ Seq_3x16x16::LIGHT_STEP_NUM + stp ].value = 1.0;
    }

    //----------------------------------------------------
    // All the Channel buttons    

    // channel
	for ( ch = 0; ch < CHANNELS; ch++ ) 
    {
        x = 50;

        // step
	    for ( stp = 0; stp < STEPS; stp++ ) 
        {
            y = 64 + (ch * CHANNEL_OFF_H);

            // level button
		    module->m_pLevelToggleParam2[ ch ][ stp ] = ParamWidget::create<MySlider_Levels>( Vec( x, y ), module, Seq_3x16x16::PARAM_SLIDERS + ( ch * STEPS ) + stp , 0.0, 6.0, 0.0);
            addParam( module->m_pLevelToggleParam2[ ch ][ stp ] );

            y += 46;

            // step button
		    addParam(ParamWidget::create<MySquareButton_Step>( Vec( x, y ), module, Seq_3x16x16::PARAM_STEPS + ( ch * STEPS ) + stp, 0.0, 1.0, 0.0 ) );
            addChild(ModuleLightWidget::create<SmallLight<DarkGreenValueLight>>( Vec( x + 2, y + 3 ), module, Seq_3x16x16::LIGHT_STEP + ( ch * STEPS ) + stp ) );

            if( ( stp & 0x3 ) == 0x3 )
                x += 20;
            else
                x += 15;
        }

        x = 45;
        y = 130 + (ch * CHANNEL_OFF_H);

	    for ( i = 0; i < PATTERNS; i++ ) 
        {
            // pattern button
		    addParam(ParamWidget::create<MySquareButton_Pattern>( Vec( x, y ), module, Seq_3x16x16::PARAM_PATTERNS + ( ch * PATTERNS ) + i, 0.0, 1.0, 0.0 ) );
            addChild(ModuleLightWidget::create<SmallLight<OrangeValueLight>>( Vec( x + 2, y + 3 ), module, Seq_3x16x16::LIGHT_PAT + ( ch * PATTERNS ) + i ) );

            x += 13;
        }

        x = 260;
        y = 126 + (ch * CHANNEL_OFF_H);

        // copy next button
		addParam(ParamWidget::create<MySquareButton_CopyNext>( Vec( x, y ), module, Seq_3x16x16::PARAM_CPY_NEXT + ch, 0.0, 1.0, 0.0 ) );
        addChild(ModuleLightWidget::create<SmallLight<CyanValueLight>>( Vec( x + 2, y + 3 ), module, Seq_3x16x16::LIGHT_COPY + ch ) );

        x = 290;

        // random button
		addParam(ParamWidget::create<MySquareButton_Rand>( Vec( x, y ), module, Seq_3x16x16::PARAM_RAND + ch, 0.0, 1.0, 0.0 ) );
        addChild(ModuleLightWidget::create<SmallLight<CyanValueLight>>( Vec( x + 2, y + 3 ), module, Seq_3x16x16::LIGHT_RAND + ch ) );

        // pattern change input trigger
        addInput(Port::create<MyPortInSmall>( Vec( 17, 127 + (ch * CHANNEL_OFF_H) ), Port::INPUT, module, Seq_3x16x16::INPUT_PAT_CHANGE + ch ) );

        x = 310;
        y = 65 + (ch * CHANNEL_OFF_H);

        // outputs
        addOutput(Port::create<MyPortOutSmall>( Vec( x, y ), Port::OUTPUT, module, Seq_3x16x16::OUT_CV + ch ) );
        addOutput(Port::create<MyPortOutSmall>( Vec( x, y + 42 ), Port::OUTPUT, module, Seq_3x16x16::OUT_GATES + ch ) );
	}

    //----------------------------------------------------
    // Global patter select buttons  
    x = 45;
    y = 340;

	for ( stp = 0; stp < PATTERNS; stp++ ) 
    {
        // pattern button
		addParam(ParamWidget::create<MySquareButton_GlobalPattern>( Vec( x, y ), module, Seq_3x16x16::PARAM_GLOBAL_PAT + stp, 0.0, 1.0, 0.0 ) );
        addChild(ModuleLightWidget::create<SmallLight<OrangeValueLight>>( Vec( x + 2, y + 3 ), module, Seq_3x16x16::LIGHT_GLOBAL_PAT + stp ) );

        x += 13;
    }

    // pattern change input trigger
    addInput(Port::create<MyPortInSmall>( Vec( 17, 338 ), Port::INPUT, module, Seq_3x16x16::INPUT_GLOBAL_PAT_CHANGE ) );

    module->lights[ Seq_3x16x16::LIGHT_GLOBAL_PAT ].value = 1.0;
    module->m_GlobalSelect = 0;

    module->ChangePattern( 0, PATTERNS, false );
    module->ChangePattern( 1, PATTERNS, false );
    module->ChangePattern( 2, PATTERNS, false );
}

//-----------------------------------------------------
// Procedure:   
//
//-----------------------------------------------------
json_t *Seq_3x16x16::toJson() 
{
    bool *pgateState;
    float *pfloat;
	json_t *rootJ = json_object();

	// running
	json_object_set_new(rootJ, "running", json_boolean (m_bRunning));

	// steps
    pgateState = &m_bStepStates[ 0 ][ 0 ][ 0 ];

	json_t *gatesJ = json_array();

	for (int i = 0; i < ( PATTERNS * CHANNELS * STEPS ); i++)
    {
		json_t *gateJ = json_integer( (int) pgateState[ i ] );
		json_array_append_new( gatesJ, gateJ );
	}

	json_object_set_new( rootJ, "steps", gatesJ );

    // level settings
    pfloat = &m_fLevels[ 0 ][ 0 ][ 0 ];

	json_t *gatesJ_3 = json_array();

	for (int i = 0; i < ( PATTERNS * CHANNELS * STEPS ); i++) 
    {
		json_t *gateJ = json_real( pfloat[ i ] );
		json_array_append_new(gatesJ_3, gateJ);
	}

	json_object_set_new(rootJ, "levelsettings", gatesJ_3);

    // number of steps
	json_t *stepsJ = json_integer( m_nSteps );
	json_object_set_new(rootJ, "numberofsteps", stepsJ);

	// gateMode
	json_t *gateModeJ = json_integer((int) m_GateMode );
	json_object_set_new(rootJ, "gateMode", gateModeJ);

	return rootJ;
}

//-----------------------------------------------------
// Procedure:   fromJson
//
//-----------------------------------------------------
void Seq_3x16x16::fromJson(json_t *rootJ) 
{
    bool *pgateState;
    float *pfloat;

	// running
	json_t *runningJ = json_object_get(rootJ, "running");

	if (runningJ)
		m_bRunning = json_is_true(runningJ);

	// steps
    pgateState = &m_bStepStates[ 0 ][ 0 ][ 0 ];

	json_t *StepsJ = json_object_get(rootJ, "steps");

	if (StepsJ) 
    {
		for (int i = 0; i < ( PATTERNS * CHANNELS * STEPS ); i++)
        {
			json_t *gateJ = json_array_get(StepsJ, i);

			if (gateJ)
				pgateState[ i ] = json_integer_value( gateJ );
		}
	}

	// level settings
    pfloat = &m_fLevels[ 0 ][ 0 ][ 0 ];

	json_t *LvlSettingsJ = json_object_get(rootJ, "levelsettings");

    if (LvlSettingsJ) 
    {
		for (int i = 0; i < ( PATTERNS * CHANNELS * STEPS ); i++) 
        {
			json_t *gateJ = json_array_get(LvlSettingsJ, i);

			if (gateJ)
				pfloat[ i ] = json_real_value(gateJ);

            //lg.f("loading %.3f\n", pfloat[ i ] );
		}
	}

	// number of steps
	json_t *nStepsJ = json_object_get(rootJ, "numberofsteps");
	if (nStepsJ)
		m_nSteps = (GateMode)json_integer_value( nStepsJ );

	// gateMode
	json_t *gateModeJ = json_object_get(rootJ, "gateMode");
	if (gateModeJ)
		m_GateMode = (GateMode)json_integer_value(gateModeJ);

    SetSteps( m_nSteps );

    //lg.f("json read complete\n");
    m_bInitJson = true;
}

//-----------------------------------------------------
// Procedure:   reset
//
//-----------------------------------------------------
void Seq_3x16x16::onReset()
{
    //lg.f("init\n");
    memset( m_fLevels, 0, sizeof(m_fLevels) );
    memset( m_bStepStates, 0, sizeof(m_bStepStates) );

    for( int ch = 0; ch < CHANNELS; ch ++ )
    {
        for( int i = 0; i < STEPS; i ++ )
        {
            lights[ LIGHT_STEP + ( ch * STEPS ) + i ].value = 0.0;
            m_fLightStepLevels[ ch ][ i ] = 0.0;
        }
    }

    ChangePattern( 0, PATTERNS, false );
    ChangePattern( 1, PATTERNS, false );
    ChangePattern( 2, PATTERNS, false );

    SetSteps( 16 );
}

//-----------------------------------------------------
// Procedure:   randomize
//
//-----------------------------------------------------
float stepchance[ 16 ] = { 0.9, 0.5, 0.8, 0.4, 0.9, 0.5, 0.8, 0.4, 0.9, 0.5, 0.8, 0.4, 0.9, 0.5, 0.8, 0.4 }; 
void Seq_3x16x16::Randomize_Channel( int ch )
{
    int stp, pat;
    int octave;

    pat = m_PatternSelect[ ch ];

    octave = (int)( randomUniform() * 4.0 );

    for( stp = 0; stp < STEPS; stp++ )
    {
        m_fLevels[ pat ][ ch ][ stp ] = (float)octave + ( randomUniform() * 2.0 );
        m_bStepStates[ pat ][ ch ][ stp ] = ( randomUniform() < stepchance[ stp ] ) ? true : false;
    }

    ChangePattern( ch, pat, true );
}

//-----------------------------------------------------
// Procedure:   randomize
//
//-----------------------------------------------------
void Seq_3x16x16::onRandomize() 
{
    int ch, stp, pat;
    int octave;

    for( ch = 0; ch < CHANNELS; ch++ )
    {
        for( pat = 0; pat < PATTERNS; pat++ )
        {
            octave = (int)( randomUniform() * 4.0 );

            for( stp = 0; stp < STEPS; stp++ )
            {
                m_fLevels[ pat ][ ch ][ stp ] = (float)octave + ( randomUniform() * 2.0 );
                m_bStepStates[ pat ][ ch ][ stp ] = ( randomUniform() < stepchance[ stp ] ) ? true : false;
            }
        }
    }

    ChangePattern( 0, 0, true );
    ChangePattern( 1, 0, true );
    ChangePattern( 2, 0, true );
}

//-----------------------------------------------------
// Procedure:   CopyNext
//
//-----------------------------------------------------
void Seq_3x16x16::CopyNext( int ch )
{
    int stp, pat;

    pat = m_PatternSelect[ ch ];

    // don't copy from the last pattern
    if( pat == (PATTERNS - 1) )
        return;

    for( stp = 0; stp < STEPS; stp++ )
    {
        m_fLevels[ pat + 1 ][ ch ][ stp ] = m_fLevels[ pat ][ ch ][ stp ];
        m_bStepStates[ pat + 1 ][ ch ][ stp ] = m_bStepStates[ pat ][ ch ][ stp ];
    }

    ChangePattern( ch, pat + 1, true );
}

//-----------------------------------------------------
// Procedure:   ChangePattern
//
//-----------------------------------------------------
void Seq_3x16x16::ChangePattern( int ch, int index, bool bForceChange )
{
    int stp, i;

    if( !bForceChange && index == m_PatternSelect[ ch ] )
        return;

    if( index < 0 )
        index = PATTERNS - 1;
    else if( index >= PATTERNS )
        index = 0;

    m_PatternSelect[ ch ] = index;

    // update lights
    for( stp = 0; stp < STEPS; stp++ ) 
        lights[ LIGHT_STEP + ( ch * STEPS ) + stp ].value = m_bStepStates[ m_PatternSelect[ ch ] ][ ch ][ stp ] ? 1.0 : 0.0;

    for( i = 0; i < PATTERNS; i++ )
    {
        if( m_PatternSelect[ ch ] == i )
            lights[ LIGHT_PAT + ( ch * PATTERNS ) + i ].value = 1.0f; 
        else
            lights[ LIGHT_PAT + ( ch * PATTERNS ) + i ].value = 0.0f;
    }

    // step
	for( stp = 0; stp < STEPS; stp++ ) 
    {
        // level button
		m_pLevelToggleParam2[ ch ][ stp ]->setValue( m_fLevels[ m_PatternSelect[ ch ] ][ ch ][ stp ] );
    }
}

//-----------------------------------------------------
// Procedure:   SetSteps
//
//-----------------------------------------------------
void Seq_3x16x16::SetSteps( int nSteps )
{
    int i;

    if( nSteps < 1 || nSteps > STEPS )
        nSteps = STEPS;

    m_nSteps = nSteps;

	for ( i = 0; i < STEPS; i++ ) 
    {
        // level button
		if( i < nSteps )
            lights[ LIGHT_STEP_NUM + i ].value = 1.0f; 
        else
            lights[ LIGHT_STEP_NUM + i ].value = 0.0f;
    }
}

//-----------------------------------------------------
// Procedure:   SetGlobalPattern
//
//-----------------------------------------------------
void Seq_3x16x16::SetGlobalPattern( int step )
{
	for ( int i = 0; i < PATTERNS; i++ ) 
        lights[ LIGHT_GLOBAL_PAT + i ].value = 0.0f;

    lights[ LIGHT_GLOBAL_PAT + step ].value = 1.0;

    m_GlobalSelect = step;

    ChangePattern( 0, step, false );
    ChangePattern( 1, step, false );
    ChangePattern( 2, step, false );
}

//-----------------------------------------------------
// Procedure:   step
//
//-----------------------------------------------------
void Seq_3x16x16::step() 
{
    bool bNextStep = false, bPulse, bGatesOn, bChTrig;
    int ch, stp;
    float fout, fnote;
    int octave;
    static float fpendinglight = 0.0f;
    static bool bUp = true;

    // pending light value
    if( bUp )
    {
        fpendinglight += ( 1.0 / engineGetSampleRate() ) * 1.5 ;

        if( fpendinglight >= 0.5f )
        {
            fpendinglight = 0.5f;
            bUp = false;
        }
    }
    else
    {
        fpendinglight -= ( 1.0 / engineGetSampleRate() ) * 2;

        if( fpendinglight <= 0.0f )
        {
            fpendinglight = 0.0f;
            bUp = true;
        }
    }

	// Run
	if( m_SchTrigRun.process( params[ PARAM_RUN ].value ) )
		m_bRunning = !m_bRunning;

	lights[ LIGHT_RUN ].value = m_bRunning ? 1.0f : 0.0f;

	// Random and copy buttons
	for( ch = 0; ch < CHANNELS; ch++ ) 
    {
	    if( m_SchTrigRandPat.process( params[ PARAM_RAND + ch ].value ) ) 
            lights[ LIGHT_RAND + ch ].value = 1.0f;

	    if( m_SchTrigCopyNext.process( params[ PARAM_CPY_NEXT + ch ].value ) )
            lights[ LIGHT_COPY + ch ].value = 1.0f;

        lights[ LIGHT_RAND + ch ].value -= lights[ LIGHT_RAND + ch ].value / LIGHT_LAMBDA / engineGetSampleRate();

        lights[ LIGHT_COPY + ch ].value -= lights[ LIGHT_COPY + ch ].value / LIGHT_LAMBDA / engineGetSampleRate();
    }

	if( m_bRunning ) 
    {
		if( inputs[ INPUT_EXT_CLOCK ].active ) 
        {
			// External clock
			if( m_SchTrigClock.process( inputs[ INPUT_EXT_CLOCK ].value ) ) 
            {
				m_fPhase = 0.0f;
				bNextStep = true;
    		}
		}
	}
    else
    {
        // cancel any pending pattern changes
        if( m_bGlobalPatternChangePending )
        {
            m_bGlobalPatternChangePending = false;
            SetGlobalPattern( m_GlobalPendingLight );
        }

        m_bPatternPending = false;

        for( ch = 0; ch < CHANNELS; ch++ )
        {
	        // pat change trigger
	        if( m_bPatternChangePending[ ch ] )
            {
                m_bPatternChangePending[ ch ] = false;
                ChangePattern( ch, m_PendingLight[ ch ], false );
            }
        }
    }

	// Reset
	if( m_SchTrigReset.process( params[ PARAM_RESET ].value + inputs[ INPUT_RESET ].value ) ) 
    {
		m_fPhase = 0.0f;
		m_CurrentStep = m_nSteps;
		bNextStep = true;
        lights[ LIGHT_RESET ].value = 1.0f;
	}

	if( bNextStep ) 
    {
		m_CurrentStep += 1;

		if( m_CurrentStep >= m_nSteps ) 
			m_CurrentStep = 0;

        for( ch = 0; ch < CHANNELS; ch++ )
		    m_fLightStepLevels[ ch ][ m_CurrentStep ] = 1.0f;

		m_PulseStep.trigger( 1e-3 );

        // resolve any pending pattern changes
        if( m_CurrentStep == 0 )
        {
            if( m_bGlobalPatternChangePending )
            {
                m_bGlobalPatternChangePending = false;

                SetGlobalPattern( m_GlobalPendingLight );

                // cancel other changes on a global change
                m_bPatternChangePending[ 0 ] = false;
                m_bPatternChangePending[ 1 ] = false;
                m_bPatternChangePending[ 2 ] = false;
                m_bPatternPending = false;
            }

            for( ch = 0; ch < CHANNELS; ch++ )
            {
	            // pat change trigger
	            if( m_bPatternChangePending[ ch ] )
                {
                    m_bPatternChangePending[ ch ] = false;
                    ChangePattern( ch, m_PendingLight[ ch ], false );
                }
            }
        }
	}

    // global pattern pending change light blinky
    if( m_bGlobalPatternChangePending )
    {
        lights[ LIGHT_GLOBAL_PAT + m_GlobalPendingLight ].value = fpendinglight;
    }

    // channel pattern pending change light blinky
    if( m_bPatternPending )
    {
        m_bPatternPending = false;

        for( ch = 0; ch < CHANNELS; ch++ )
        {
	        // pat change trigger
	        if( m_bPatternChangePending[ ch ] )
            {
                m_bPatternPending = true;
                lights[ LIGHT_PAT + (ch * PATTERNS) + m_PendingLight[ ch ] ].value = fpendinglight;
            }
        }
    }

   	lights[ LIGHT_RESET ].value -= lights[ LIGHT_RESET ].value / LIGHT_LAMBDA / engineGetSampleRate();

	bPulse = m_PulseStep.process( 1.0 / engineGetSampleRate() );

	// Step buttons
	for( ch = 0; ch < CHANNELS; ch++ ) 
    {
        bChTrig = false;

	    for( stp = 0; stp < STEPS; stp++ ) 
        {
		    bGatesOn = ( m_bRunning && stp == m_CurrentStep && m_bStepStates[ m_PatternSelect[ ch ] ][ ch ][ stp ] );

		    if( m_GateMode == TRIGGER )
			    bGatesOn = bGatesOn && bPulse;
		    else if( m_GateMode == RETRIGGER )
			    bGatesOn = bGatesOn && !bPulse;

            if( bGatesOn )
                bChTrig = true;
		    
		    m_fLightStepLevels[ ch ][ stp ] -= m_fLightStepLevels[ ch ][ stp ] / LIGHT_LAMBDA / engineGetSampleRate();

            lights[ LIGHT_STEP + (ch * STEPS) + stp ].value = m_bStepStates[ m_PatternSelect[ ch ] ][ ch ][ stp ] ? 1.0 - m_fLightStepLevels[ ch ][ stp ] : m_fLightStepLevels[ ch ][ stp ];
	    }

        // trigger
        outputs[ OUT_GATES + ch ].value = bChTrig ? CV_MAX : 0.0;
    }

    // outputs
	for( ch = 0; ch < CHANNELS; ch++ ) 
    {
        // set CV
        if( m_bRunning && m_bStepStates[ m_PatternSelect[ ch ] ][ ch ][ m_CurrentStep ]  )
        {
            // quantizie note
            fout = params[ PARAM_SLIDERS + ( ch * STEPS ) + m_CurrentStep ].value;
            octave = round( fout );
            fnote   = round( ( fout - (float)octave ) * 12.0f );
            outputs[ OUT_CV + ch ].value = (float)octave + fnote/12.0f;
        }
    }

    if( m_bRunning )
    {
        // external pattern select change
        for( ch = 0; ch < CHANNELS; ch++ )
        {
	        // pat change trigger - ignore if already pending
	        if( m_SchTrigPatChange[ ch ].process( inputs[ INPUT_PAT_CHANGE + ch ].value ) && !m_bPatternChangePending[ ch ] ) 
            {
                m_bPatternPending = true;
                m_bPatternChangePending[ ch ] = true;
                m_PendingLight[ ch ] = ( m_PatternSelect[ ch ] + 1 ) & 0xF;
	        }
        }

	    // global pat change trigger - ignore if already pending
	    if( m_SchTrigGlobalPatternSelect.process( inputs[ INPUT_GLOBAL_PAT_CHANGE ].value ) && !m_bGlobalPatternChangePending ) 
        {
            m_bGlobalPatternChangePending = true;
            m_GlobalPendingLight = ( m_GlobalSelect + 1 ) & 0xF;
	    }
    }
}

} // namespace rack_plugin_mscHack

using namespace rack_plugin_mscHack;

RACK_PLUGIN_MODEL_INIT(mscHack, Seq_3x16x16) {
   Model *modelSeq_3x16x16 = Model::create<Seq_3x16x16, Seq_3x16x16_Widget>( "mscHack", "Seq_3ch_16step", "SEQ 3 x 16", SEQUENCER_TAG, MULTIPLE_TAG );
   return modelSeq_3x16x16;
}
