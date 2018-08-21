#include "mscHack.hpp"
#include "dsp/digital.hpp"

namespace rack_plugin_mscHack {

//-----------------------------------------------------
// General Definition
//-----------------------------------------------------
#define nCHANNELS 3

//-----------------------------------------------------
// filter
//-----------------------------------------------------
enum FILTER_TYPES
{
    FILTER_OFF,
    FILTER_LP,
    FILTER_HP,
    FILTER_BP,
    FILTER_NT
};

typedef struct
{
	int type;
    float basef, q, f, qmod, fmod;
    float lp1, bp1;

}FILTER_STRUCT;

//-----------------------------------------------------
// Morph oscillator
//-----------------------------------------------------
#define nMORPH_WAVES 3

enum MOD_TYPES
{
    MOD_LEVEL,
	MOD_REZ,
	MOD_FILTER,
	nMODS
};

//-----------------------------------------------------
// Module Definition
//
//-----------------------------------------------------
struct Windz : Module
{
	enum ParamIds 
    {
		PARAM_SPEED,
        nPARAMS
    };

	enum InputIds 
    {
		IN_RANDTRIG,
        nINPUTS 
	};

	enum OutputIds 
    {
		OUT_L,
		OUT_R,
        nOUTPUTS
	};

	enum LightIds 
    {
        nLIGHTS
	};

	enum FADE_STATE
	{
	    FADE_IDLE,
		FADE_OUT,
		FADE_IN,
	};

	bool            m_bInitialized = false;
    CLog            lg;

    // Contructor
	Windz() : Module(nPARAMS, nINPUTS, nOUTPUTS, nLIGHTS){}

	Label				*m_pTextLabel = NULL;
	Label				*m_pTextLabel2 = NULL;

	// modulation envelopes
	EnvelopeData 		m_mod[ nCHANNELS ][ nMODS ] = {};
	float               m_fval[ nCHANNELS ][ nMODS ] = {};
	float               m_finc[ nCHANNELS ][ nMODS ] = {};

	FILTER_STRUCT 		m_filter[ nCHANNELS ]={};

	// random seed
	SchmittTrigger 		m_SchmitTrigRand;

	MyLEDButton    		*m_pButtonSeed[ 32 ] = {};
	MyLEDButton    		*m_pButtonRand = NULL;
	int   				m_Seed = 0;
	int             	m_FadeState = FADE_IN;
	float           	m_fFade = 0.0f;
	float speeds[ 9 ] = { 0.1f, 0.25f, 0.50f, 0.75f, 1.0f, 1.5f, 2.0f, 4.0f, 8.0f };

    //-----------------------------------------------------
    // MySpeed_Knob
    //-----------------------------------------------------
    struct MySpeed_Knob : Knob_Yellow3_20_Snap
    {
        Windz *mymodule;
        char strVal[ 10 ] = {};

        void onChange( EventChange &e ) override
        {
            mymodule = (Windz*)module;

            if( mymodule )
            {
                if( !mymodule->m_bInitialized )
                	return;

                sprintf( strVal, "x%.2f", mymodule->speeds[ (int)value ] );
                mymodule->m_pTextLabel2->text = strVal;
            }

		    RoundKnob::onChange( e );
	    }
    };


	void putx( int x );
	void putf( float fval );

	int  getseed( void );
	void putseed( int seed );

	void ChangeSeedPending( int seed );
	void BuildWave( int ch );
	void BuildDrone( void );

	void RandWave( EnvelopeData *pEnv, float min=0.0f, float max= 1.0f );
	void RandPresetWaveAdjust( EnvelopeData *pEnv );

	// audio
	void ChangeFilterCutoff( int ch );
	void processFilter( int ch, float *pIn );
	void processReverb( float In, float *pL, float *pR );

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
// Windz_SeedButton
//-----------------------------------------------------
void Windz_SeedButton( void *pClass, int id, bool bOn )
{
	Windz *mymodule;
    mymodule = (Windz*)pClass;

    mymodule->ChangeSeedPending( mymodule->getseed() );
}

//-----------------------------------------------------
// Windz_RandButton
//-----------------------------------------------------
void Windz_RandButton( void *pClass, int id, bool bOn )
{
	Windz *mymodule;
    mymodule = (Windz*)pClass;

    mymodule->ChangeSeedPending( (int)randomu32() );
}

//-----------------------------------------------------
// Procedure:   Widget
//
//-----------------------------------------------------

struct Windz_Widget : ModuleWidget {
	Windz_Widget( Windz *module );
};

Windz_Widget::Windz_Widget( Windz *module ) : ModuleWidget(module)
{
	int i, x, y;

	box.size = Vec( 15*5, 380 );

	{
		SVGPanel *panel = new SVGPanel();
		panel->box.size = box.size;
		panel->setBackground(SVG::load(assetPlugin(plugin, "res/Windz.svg")));
		addChild(panel);
	}

	//addInput(Port::create<MyPortInSmall>( Vec( 10, 20 ), Port::INPUT, module, Windz::IN_VOCT ) );
	addInput(Port::create<MyPortInSmall>( Vec( 10, 241 ), Port::INPUT, module, Windz::IN_RANDTRIG ) );

    // random button
    module->m_pButtonRand = new MyLEDButton( 40, 238, 25, 25, 20.0, DWRGB( 180, 180, 180 ), DWRGB( 255, 0, 0 ), MyLEDButton::TYPE_MOMENTARY, 0, module, Windz_RandButton );
	addChild( module->m_pButtonRand );

	addOutput(Port::create<MyPortOutSmall>( Vec( 48, 20 ), Port::OUTPUT, module, Windz::OUT_L ) );
	addOutput(Port::create<MyPortOutSmall>( Vec( 48, 45 ), Port::OUTPUT, module, Windz::OUT_R ) );

	y = 95;
	x = 9;

    //module->lg.Open("c://users//mark//documents//rack//Windz.txt");

	for( i = 31; i >=0; i-- )
	{
	    module->m_pButtonSeed[ i ] = new MyLEDButton( x, y, 11, 11, 8.0, DWRGB( 180, 180, 180 ), DWRGB( 255, 255, 0 ), MyLEDButton::TYPE_SWITCH, i, module, Windz_SeedButton );
		addChild( module->m_pButtonSeed[ i ] );

		if( i % 4 == 0 )
		{
			y += 15;
			x = 9;
		}
		else
		{
			x += 15;
		}
	}

	addParam(ParamWidget::create<Windz::MySpeed_Knob>( Vec( 10, 280 ), module, Windz::PARAM_SPEED, 0.0, 8.0, 4.0 ) );

	module->m_pTextLabel2 = new Label();
	module->m_pTextLabel2->box.pos = Vec( 30, 280 );
	module->m_pTextLabel2->text = "x1.00";
	addChild( module->m_pTextLabel2 );

	module->m_pTextLabel = new Label();
	module->m_pTextLabel->box.pos = Vec( 0, 213 );
	module->m_pTextLabel->text = "----";
	addChild( module->m_pTextLabel );

	addChild(Widget::create<ScrewSilver>(Vec(30, 0)));
	addChild(Widget::create<ScrewSilver>(Vec(30, 365)));

	module->putseed( (int)randomu32() );
	module->BuildDrone();
}

//-----------------------------------------------------
// Procedure: JsonParams  
//
//-----------------------------------------------------
void Windz::JsonParams( bool bTo, json_t *root)
{
    JsonDataInt( bTo, "m_Seed", root, &m_Seed, 1 );
}

//-----------------------------------------------------
// Procedure: toJson  
//
//-----------------------------------------------------
json_t *Windz::toJson()
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
void Windz::fromJson( json_t *root )
{
	//char strVal[ 10 ] = {};

    JsonParams( FROMJSON, root );

    ChangeSeedPending( m_Seed );

    //sprintf( strVal, "x%.2f", speeds[ (int)params[ PARAM_SPEED ].value ] );
    //m_pTextLabel2->text = strVal;
}

//-----------------------------------------------------
// Procedure:   onCreate
//
//-----------------------------------------------------
void Windz::onCreate()
{
}

//-----------------------------------------------------
// Procedure:   onDelete
//
//-----------------------------------------------------
void Windz::onDelete()
{
}

//-----------------------------------------------------
// Procedure:   onReset
//
//-----------------------------------------------------
void Windz::onReset()
{
}

//-----------------------------------------------------
// Procedure:   onRandomize
//
//-----------------------------------------------------
void Windz::onRandomize()
{
	ChangeSeedPending( (int)randomu32() );
}

//-----------------------------------------------------
// Procedure:   getseed
//
//-----------------------------------------------------
int Windz::getseed( void )
{
	int seed = 0, shift= 1;;

	for( int i = 0; i < 32; i++ )
	{
	    if( m_pButtonSeed[ i ]->m_bOn )
	    	seed |= shift;

	    shift<<=1;
	}

	return seed;
}

//-----------------------------------------------------
// Procedure:   putseed
//
//-----------------------------------------------------
void Windz::putseed( int seed )
{
	m_Seed = seed;

	init_rand( seed );
	putx( seed );

	for( int i = 0; i < 32; i++ )
	{
	    m_pButtonSeed[ i ]->Set( (bool)(seed & 1) );
	    seed>>=1;
	}
}

//-----------------------------------------------------
// Procedure:   ChangeSeedPending
//
//-----------------------------------------------------
void Windz::ChangeSeedPending( int seed )
{
	m_FadeState = FADE_OUT;
	putseed( seed );
}

//-----------------------------------------------------
// Procedure:   RandWave
//-----------------------------------------------------
void Windz::RandWave( EnvelopeData *pEnv, float min, float max )
{
	int i;

    for( i = 0; i < ENVELOPE_HANDLES - 1; i++ )
        pEnv->setVal( i, frand_mm( min, max ) );

    pEnv->setVal( i, pEnv->m_HandleVal[ 0 ] );
}

//-----------------------------------------------------
// Procedure:   RandPresetWaveAdjust
//-----------------------------------------------------
void Windz::RandPresetWaveAdjust( EnvelopeData *pEnv )
{
	int i;
	float fval;

	if( frand_perc( 25.0f ) )
	{
		RandWave( pEnv, 0.0f, 1.0f );
	}
	else
	{
		//pEnv->Preset( (int)frand_mm( 2.0f, 7.25f) );
		pEnv->Preset( EnvelopeData::PRESET_SIN );

		for( i = 0; i < ENVELOPE_HANDLES - 1; i++ )
		{
			fval = clamp( pEnv->m_HandleVal[ i ] + frand_mm( -0.01f,  0.01f ), -1.0f, 1.0f );
			pEnv->setVal( i, fval );
		}
	}
}

//-----------------------------------------------------
// Procedure:   BuildWave
//
//-----------------------------------------------------
void Windz::BuildWave( int ch )
{
	// modulation waveforms
	m_mod[ ch ][ MOD_LEVEL ].Init( EnvelopeData::MODE_LOOP, EnvelopeData::RANGE_n1to1, false, 1.0f );
	m_finc[ ch ][ MOD_LEVEL ] = 1.0f / frand_mm( 14.5f, 38.0f );
	RandWave( &m_mod[ ch ][ MOD_LEVEL ], 0.2f, 0.9f );

	m_mod[ ch ][ MOD_FILTER ].Init( EnvelopeData::MODE_LOOP, EnvelopeData::RANGE_0to1, false, 1.0f );
	m_finc[ ch ][ MOD_FILTER ] = 1.0f / frand_mm( 14.5f, 38.0f );
	RandWave( &m_mod[ ch ][ MOD_FILTER ], 0.02f, 0.8f );

	m_mod[ ch ][ MOD_REZ ].Init( EnvelopeData::MODE_LOOP, EnvelopeData::RANGE_0to1, false, 1.0f );
	m_finc[ ch ][ MOD_REZ ] = 1.0f / frand_mm( 14.5f, 38.0f );
	RandWave( &m_mod[ ch ][ MOD_REZ ], 0.5f, 0.99f );
}

//-----------------------------------------------------
// Procedure:   BuildDrone
//
//-----------------------------------------------------
void Windz::BuildDrone( void )
{
	int ch;

    init_rand( m_Seed );

	for( ch = 0; ch < nCHANNELS; ch++ )
	{
		BuildWave( ch );
	}

	m_bInitialized = true;
}

//-----------------------------------------------------
// Procedure:   putf
//
//-----------------------------------------------------
void Windz::putf( float fval )
{
    char strVal[ 10 ] = {};

    sprintf( strVal, "%.3f", fval );
    m_pTextLabel->text = strVal;
}

//-----------------------------------------------------
// Procedure:   putf
//
//-----------------------------------------------------
void Windz::putx( int x )
{
    char strVal[ 10 ] = {};

    sprintf( strVal, "%.8X", x );
    m_pTextLabel->text = strVal;
}

//-----------------------------------------------------
// Procedure:   ChangeFilterCutoff
//
//-----------------------------------------------------
void Windz::ChangeFilterCutoff( int ch )
{
    float fx, fx2, fx3, fx5, fx7, cutfreq;
    FILTER_STRUCT *pf;

    pf = &m_filter[ ch ];

    cutfreq = m_fval[ ch ][ MOD_FILTER ];

    // clamp at 1.0 and 20/samplerate
    cutfreq = fmax(cutfreq, 20 / engineGetSampleRate());
    cutfreq = fmin(cutfreq, 1.0);

    // calculate eq rez freq
    fx = 3.141592 * (cutfreq * 0.026315789473684210526315789473684) * 2 * 3.141592;
    fx2 = fx*fx;
    fx3 = fx2*fx;
    fx5 = fx3*fx2;
    fx7 = fx5*fx2;

    pf->f = 2.0 * (fx
	    - (fx3 * 0.16666666666666666666666666666667)
	    + (fx5 * 0.0083333333333333333333333333333333)
	    - (fx7 * 0.0001984126984126984126984126984127));
}

//-----------------------------------------------------
// Procedure:   Filter
//
//-----------------------------------------------------
#define MULTI (0.33333333333333333333333333333333f)
void Windz::processFilter( int ch, float *pIn )
{
    float rez, hp1;
    float input, lowpass, bandpass, highpass;
    FILTER_STRUCT *pf;

    rez = 1.0 - m_fval[ ch ][ MOD_REZ ];

    pf = &m_filter[ ch ];

    input = *pIn;

    input  = input + 0.000000001;

    pf->lp1     = pf->lp1 + pf->f * pf->bp1;
    hp1         = input - pf->lp1 - rez * pf->bp1;
    pf->bp1     = pf->f * hp1 + pf->bp1;
    lowpass     = pf->lp1;
    highpass    = hp1;
    bandpass    = pf->bp1;

    pf->lp1     = pf->lp1 + pf->f * pf->bp1;
    hp1         = input - pf->lp1 - rez * pf->bp1;
    pf->bp1     = pf->f * hp1 + pf->bp1;
    lowpass     = lowpass  + pf->lp1;
    highpass    = highpass + hp1;
    bandpass    = bandpass + pf->bp1;

    input  = input - 0.000000001;

    pf->lp1     = pf->lp1 + pf->f * pf->bp1;
    hp1         = input - pf->lp1 - rez * pf->bp1;
    pf->bp1     = pf->f * hp1 + pf->bp1;

    lowpass  = (lowpass  + pf->lp1) * MULTI;
    highpass = (highpass + hp1) * MULTI;
    bandpass = (bandpass + pf->bp1) * MULTI;

    /*switch( pf->type )
    {
    case FILTER_LP:
        out = lowpass;
        break;
    case FILTER_HP:
        out  = highpass;
        break;
    case FILTER_BP:
        out  = bandpass;
        break;
    case FILTER_NT:
        out  = lowpass + highpass;
        break;
    default:
        return;
    }*/

    *pIn = lowpass;
}



//-----------------------------------------------------
// Procedure:   step
//
//-----------------------------------------------------
void Windz::step()
{
	float In =0.0f, fout[ nCHANNELS ] = {};
	int ch, i;

	if( !m_bInitialized )
		return;

	// randomize trigger
	if( m_SchmitTrigRand.process( inputs[ IN_RANDTRIG ].normalize( 0.0f ) ) )
	{
		m_pButtonRand->Set( true );
		ChangeSeedPending( (int)randomu32() );
	}

    switch( m_FadeState )
    {
    case FADE_OUT:
    	m_fFade -= 0.00005f;
    	if( m_fFade <= 0.0f )
    	{
    		m_fFade = 0.0f;
    		BuildDrone();
    		m_FadeState = FADE_IN;
    	}
    	break;
    case FADE_IN:
    	m_fFade += 0.00005f;
    	if( m_fFade >= 1.0f )
    	{
    		m_fFade = 1.0f;
    		m_FadeState = FADE_IDLE;
    	}
    	break;
    case FADE_IDLE:
    default:
    	break;
    }

	// process oscillators
	for( ch = 0; ch < nCHANNELS; ch++ )
	{
		// process modulation waves
		for( i = 0; i < nMODS; i++ )
		{
			m_mod[ ch ][ i ].m_Clock.syncInc = m_finc[ ch ][ i ] * speeds[ (int)params[ PARAM_SPEED ].value ];
			m_fval[ ch ][ i ] = m_mod[ ch ][ i ].procStep( false, false );
		}

		In = frand_mm( -m_fval[ ch ][ MOD_LEVEL ], m_fval[ ch ][ MOD_LEVEL ] );

		// filter
		ChangeFilterCutoff( ch );
		processFilter( ch, &In );

		fout[ ch ] = In * AUDIO_MAX;
	}

	outputs[ OUT_L ].value = (fout[ 0 ] + fout[ 1 ]) * m_fFade;
	outputs[ OUT_R ].value = (fout[ 0 ] + fout[ 2 ]) * m_fFade;
}

} // namespace rack_plugin_mscHack

using namespace rack_plugin_mscHack;

RACK_PLUGIN_MODEL_INIT(mscHack, Windz) {
   Model *modelWindz = Model::create<Windz, Windz_Widget>( "mscHack", "Windz", "Windz module", OSCILLATOR_TAG, MULTIPLE_TAG );
   return modelWindz;
}
