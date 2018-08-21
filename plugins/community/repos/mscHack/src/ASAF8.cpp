#include "mscHack.hpp"
#include "dsp/digital.hpp"

namespace rack_plugin_mscHack {

#define nCHANNELS 8
#define nENVELOPE_SEGS 5

typedef struct
{
	float m, b;
}ENV_SEG;

//-----------------------------------------------------
// Module Definition
//
//-----------------------------------------------------
struct ASAF8 : Module
{
	enum ParamIds 
    {
		PARAM_SPEED_IN,
		PARAM_SPEED_OUT	= PARAM_SPEED_IN + nCHANNELS,
        nPARAMS			= PARAM_SPEED_OUT + nCHANNELS
    };

	enum InputIds 
    {
		IN_TRIGS,
		IN_AUDIOL	= IN_TRIGS + nCHANNELS,
		IN_AUDIOR	= IN_AUDIOL + nCHANNELS,
        nINPUTS 	= IN_AUDIOR + nCHANNELS
	};

	enum OutputIds 
    {
		OUT_AUDIOL,
		OUT_AUDIOR	= OUT_AUDIOL + nCHANNELS,
        nOUTPUTS	= OUT_AUDIOR + nCHANNELS,
	};

	enum LightIds 
    {
        nLIGHTS
	};

	enum FadeStates
    {
        STATE_OFF,
		STATE_FIN,
		STATE_ON,
		STATE_FOUT
	};

    CLog            lg;
    bool            m_bInitialized = false;

    // triggers
    MyLEDButton     *m_pTrigButton[ nCHANNELS ] = {};

    int             m_State[ nCHANNELS ] = {STATE_OFF};

    float           m_fFade[ nCHANNELS ] = {};
    float           m_fPos[ nCHANNELS ] = {};

    ENV_SEG         m_EnvSeg[ nENVELOPE_SEGS ] = {};

    Label           *m_pTextLabel = NULL;

    // Contructor
	ASAF8() : Module(nPARAMS, nINPUTS, nOUTPUTS, nLIGHTS){}

    //-----------------------------------------------------
    // spd_Knob
    //-----------------------------------------------------
    struct spd_Knob : Knob_Green1_15
    {
    	ASAF8 *mymodule;
        int param;
        char strVal[ 10 ] = {};

        void onChange( EventChange &e ) override
        {
            mymodule = (ASAF8*)module;

            sprintf( strVal, "[%.2fs]", value );

            mymodule->m_pTextLabel->text = strVal;

		    RoundKnob::onChange( e );
	    }
    };

	void    envSeg_from_points( float x1, float y1, float x2, float y2, ENV_SEG *L );

	bool    processFade( int ch, bool bfin );
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
// ASAF8_TrigButton
//-----------------------------------------------------
void ASAF8_TrigButton( void *pClass, int id, bool bOn )
{
	//ASAF8 *mymodule;
    //mymodule = (ASAF8*)pClass;

    /*if( bOn )
    	mymodule->m_State[ id ] = ASAF8::STATE_FIN;
    else
    	mymodule->m_State[ id ] = ASAF8::STATE_FOUT;*/
}

//-----------------------------------------------------
// Procedure:   Widget
//
//-----------------------------------------------------

struct ASAF8_Widget : ModuleWidget {
	ASAF8_Widget( ASAF8 *module );
};

ASAF8_Widget::ASAF8_Widget( ASAF8 *module ) : ModuleWidget(module)
{
	int x, y;

	box.size = Vec( 15*12, 380);

	{
		SVGPanel *panel = new SVGPanel();
		panel->box.size = box.size;
		panel->setBackground(SVG::load(assetPlugin(plugin, "res/ASAF8.svg")));
		addChild(panel);
	}

    //module->lg.Open("ASAF8.txt");

	module->m_pTextLabel = new Label();
	module->m_pTextLabel->box.pos = Vec( 90, 28 );
	module->m_pTextLabel->text = "----";
	addChild( module->m_pTextLabel );

	x = 3;
	y = 77;

	for( int ch = 0; ch < nCHANNELS; ch++ )
	{
		// inputs
		addInput(Port::create<MyPortInSmall>( Vec( x + 1, y ), Port::INPUT, module, ASAF8::IN_AUDIOL + ch ) );
		addInput(Port::create<MyPortInSmall>( Vec( x + 22, y ), Port::INPUT, module, ASAF8::IN_AUDIOR + ch ) );

		// trigger input
		addInput(Port::create<MyPortInSmall>( Vec( x + 47, y ), Port::INPUT, module, ASAF8::IN_TRIGS + ch ) );

        // trigger button
        module->m_pTrigButton[ ch ] = new MyLEDButton( x + 68, y - 1, 19, 19, 15.0, DWRGB( 180, 180, 180 ), DWRGB( 0, 255, 0 ), MyLEDButton::TYPE_SWITCH, ch, module, ASAF8_TrigButton );
	    addChild( module->m_pTrigButton[ ch ] );

        // speed knobs
        addParam(ParamWidget::create<ASAF8::spd_Knob>( Vec( x + 94, y ), module, ASAF8::PARAM_SPEED_IN + ch, 0.05f, 20.0f, 5.0f ) );
        addParam(ParamWidget::create<ASAF8::spd_Knob>( Vec( x + 115, y ), module, ASAF8::PARAM_SPEED_OUT + ch, 0.05f, 20.0f, 5.0f ) );

        // outputs
        addOutput(Port::create<MyPortOutSmall>( Vec( x + 137, y ), Port::OUTPUT, module, ASAF8::OUT_AUDIOL + ch ) );
        addOutput(Port::create<MyPortOutSmall>( Vec( x + 158, y ), Port::OUTPUT, module, ASAF8::OUT_AUDIOR + ch ) );

        y += 33;
	}

	addChild(Widget::create<ScrewSilver>(Vec(15, 0)));
	addChild(Widget::create<ScrewSilver>(Vec(box.size.x-30, 0)));
	addChild(Widget::create<ScrewSilver>(Vec(15, 365))); 
	addChild(Widget::create<ScrewSilver>(Vec(box.size.x-30, 365)));

	module->envSeg_from_points( 0.0f, 1.0f, 0.2f, 0.975f, &module->m_EnvSeg[ 0 ] );
	module->envSeg_from_points( 0.2f, 0.975f, 0.3f, 0.9f, &module->m_EnvSeg[ 1 ] );
	module->envSeg_from_points( 0.3f, 0.9f, 0.7f, 0.1f, &module->m_EnvSeg[ 2 ] );
	module->envSeg_from_points( 0.7f, 0.1f, 0.8f, 0.075f, &module->m_EnvSeg[ 3 ] );
	module->envSeg_from_points( 0.8f, 0.075f, 1.0f, 0.0f, &module->m_EnvSeg[ 4 ] );

	module->m_bInitialized = true;
}

//-----------------------------------------------------
// Procedure: JsonParams  
//
//-----------------------------------------------------
void ASAF8::JsonParams( bool bTo, json_t *root)
{
    JsonDataInt( bTo, "m_State", root, m_State, nCHANNELS );
}

//-----------------------------------------------------
// Procedure: toJson  
//
//-----------------------------------------------------
json_t *ASAF8::toJson()
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
void ASAF8::fromJson( json_t *root )
{
    JsonParams( FROMJSON, root );

    for( int ch = 0; ch < nCHANNELS; ch++ )
    {
    	if( m_State[ ch ] == STATE_OFF || m_State[ ch ] == STATE_FOUT )
    	{
    		m_pTrigButton[ ch ]->Set( false );
    		m_State[ ch ] = STATE_OFF;
    		m_fFade[ ch ] = 0.0f;
    	}
    	else
    	{
    		m_pTrigButton[ ch ]->Set( true );
    		m_State[ ch ] = STATE_ON;
    		m_fFade[ ch ] = 1.0f;
    	}
    }
}

//-----------------------------------------------------
// Procedure:   onCreate
//
//-----------------------------------------------------
void ASAF8::onCreate()
{
}

//-----------------------------------------------------
// Procedure:   onDelete
//
//-----------------------------------------------------
void ASAF8::onDelete()
{
}

//-----------------------------------------------------
// Procedure:   onReset
//
//-----------------------------------------------------
void ASAF8::onReset()
{
}

//-----------------------------------------------------
// Procedure:   onRandomize
//
//-----------------------------------------------------
void ASAF8::onRandomize()
{
}

//-----------------------------------------------------
// Function:    envSeg_from_points
//
//-----------------------------------------------------
void ASAF8::envSeg_from_points( float x1, float y1, float x2, float y2, ENV_SEG *L )
{
    L->m = (y2 - y1) / (x2 - x1);
    L->b  = y1 - (L->m * x1);
}

//-----------------------------------------------------
// Procedure:   processFade
//
//-----------------------------------------------------
bool ASAF8::processFade( int ch, bool bfin )
{
	int i = 0;
	float inc;

	if( bfin )
		inc = 1.0 / ( engineGetSampleRate() * params[ PARAM_SPEED_IN + ch ].value );
	else
		inc = 1.0 / ( engineGetSampleRate() * params[ PARAM_SPEED_OUT + ch ].value );

	if( m_fPos[ ch ] < 0.2f )
		i = 0;
	else if( m_fPos[ ch ] < 0.3f )
		i = 1;
	else if( m_fPos[ ch ] < 0.7f )
		i = 2;
	else if( m_fPos[ ch ] < 0.8f )
		i = 3;
	else
		i = 4;

	if( bfin )
		m_fFade[ ch ] = 1.0 - ( ( m_fPos[ ch ] * m_EnvSeg[ i ].m ) + m_EnvSeg[ i ].b );
	else
		m_fFade[ ch ] = ( m_fPos[ ch ] * m_EnvSeg[ i ].m ) + m_EnvSeg[ i ].b;

	m_fPos[ ch ] += inc;

	// return true means we are finished fade
	if( m_fPos[ ch ] >= 1.0f )
		return true;

	return false;
}

//-----------------------------------------------------
// Procedure:   step
//
//-----------------------------------------------------
#define FADE_GATE_LVL (0.01f)

void ASAF8::step()
{
	if( !m_bInitialized )
		return;

	for( int ch = 0; ch < nCHANNELS; ch++ )
	{
		switch( m_State[ ch ] )
		{
		case STATE_FOUT:

			if( inputs[ IN_TRIGS + ch ].normalize( 0.0f ) >= FADE_GATE_LVL || m_pTrigButton[ ch ]->m_bOn )
			{
				m_pTrigButton[ ch ]->Set( true );
				m_fPos[ ch ] = 1.0f - m_fPos[ ch ];
				m_State[ ch ] = STATE_FIN;
				break;
			}

			if( processFade( ch, false ) )
			{
				m_fFade[ ch ] = 0.0f;
				m_State[ ch ] = STATE_OFF;
			}
			break;

		case STATE_OFF:

			if( inputs[ IN_TRIGS + ch ].normalize( 0.0f ) >= FADE_GATE_LVL || m_pTrigButton[ ch ]->m_bOn )
			{
				m_pTrigButton[ ch ]->Set( true );
				m_State[ ch ] = STATE_FIN;
				m_fPos[ ch ] = 0.0f;
				break;
			}

			m_fFade[ ch ] = 0.0f;
			break;

		case STATE_FIN:

			if( inputs[ IN_TRIGS + ch ].normalize( 1.0f ) < FADE_GATE_LVL || !m_pTrigButton[ ch ]->m_bOn )
			{
				m_pTrigButton[ ch ]->Set( false );
				m_fPos[ ch ] = 1.0f - m_fPos[ ch ];
				m_State[ ch ] = STATE_FOUT;
				break;
			}

			if( processFade( ch, true )  )
			{
				m_fFade[ ch ] = 1.0f;
				m_State[ ch ] = STATE_ON;
			}
			break;

		case STATE_ON:

			if( inputs[ IN_TRIGS + ch ].normalize( 1.0f ) < FADE_GATE_LVL || !m_pTrigButton[ ch ]->m_bOn )
			{
				m_pTrigButton[ ch ]->Set( false );
				m_State[ ch ] = STATE_FOUT;
				m_fPos[ ch ] = 0.0f;
				break;
			}

			m_fFade[ ch ] = 1.0f;
			break;
		}

		if( inputs[ IN_AUDIOL + ch ].active )
			outputs[ OUT_AUDIOL + ch ].value = inputs[ IN_AUDIOL + ch ].value * m_fFade[ ch ];
		else
			outputs[ OUT_AUDIOL + ch ].value = CV_MAX * m_fFade[ ch ];

		if( inputs[ IN_AUDIOR + ch ].active )
			outputs[ OUT_AUDIOR + ch ].value = inputs[ IN_AUDIOR + ch ].value * m_fFade[ ch ];
		else
			outputs[ OUT_AUDIOR + ch ].value = CV_MAX * m_fFade[ ch ];
	}
}

} // namespace rack_plugin_mscHack

using namespace rack_plugin_mscHack;

RACK_PLUGIN_MODEL_INIT(mscHack, ASAF8) {
   Model *modelASAF8 = Model::create<ASAF8, ASAF8_Widget>( "mscHack", "ASAF8", "ASAF-8 Channel Auto Stereo Audio Fader", ATTENUATOR_TAG, CONTROLLER_TAG, UTILITY_TAG, MULTIPLE_TAG );
   return modelASAF8;
}
