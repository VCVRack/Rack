#include "mscHack.hpp"
#include "dsp/digital.hpp"

namespace rack_plugin_mscHack {

//-----------------------------------------------------
// General Definition
//-----------------------------------------------------
#define nCHANNELS 3
#define SEMI ( 1.0f / 12.0f )

//-----------------------------------------------------
// Reverb Definition
//-----------------------------------------------------
#define REV_TAPS 16
#define REV_BUF_SIZE 0x4000
#define REV_BUF_MAX  0x3FFF

typedef struct
{
	float 			buf[ REV_BUF_SIZE ];
	unsigned int   	in;
	unsigned int   	out[ REV_TAPS ];
}REVERB_STRUCT;

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
	MOD_MORPH,
    MOD_LEVEL,
	MOD_DET1,
	MOD_DET2,
	nMODS
};

typedef struct
{
	float        inc;
	float        semi;
	float        fval[ nMODS ];

	//FILTER_STRUCT filter;

}MORPH_OSC_STRUCT;

enum GLOBAL_OSCS
{
	GOSC_FILTER,
	GOSC_NOISE,
	nGLOBALOSCS
};

typedef struct
{
	float        finc;
	float        fval;

}GLOBAL_OSC_STRUCT;

//-----------------------------------------------------
// Module Definition
//
//-----------------------------------------------------
struct Dronez : Module
{
	enum ParamIds 
    {
		PARAM_SPEED,
        nPARAMS
    };

	enum InputIds 
    {
		IN_VOCT,
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
	Dronez() : Module(nPARAMS, nINPUTS, nOUTPUTS, nLIGHTS){}

	Label				*m_pTextLabel = NULL;
	Label				*m_pTextLabel2 = NULL;

	// oscillators
	MORPH_OSC_STRUCT 	m_osc[ nCHANNELS ] = {};
	EnvelopeData 		m_wav[ nCHANNELS ][ nMORPH_WAVES ] = {};
	EnvelopeData 		m_mod[ nCHANNELS ][ nMODS ] = {};
	EnvelopeData 		m_global[ nGLOBALOSCS ];

	GLOBAL_OSC_STRUCT   m_GlobalOsc[ nGLOBALOSCS ] = {};

	float               m_cuttoff = 0.5, m_rez = 0.5;
	float               m_finc[ nCHANNELS ][ nMODS ] = {};

	FILTER_STRUCT 		m_filter={};

	// reverb
	REVERB_STRUCT 		m_reverb = {};

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
        Dronez *mymodule;
        char strVal[ 10 ] = {};

        void onChange( EventChange &e ) override
        {
            mymodule = (Dronez*)module;

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
	void ChangeFilterCutoff( FILTER_STRUCT *pf, float cutfreq );
	void processFilter( FILTER_STRUCT *pfilter, float *pIn );
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
// Dronez_SeedButton
//-----------------------------------------------------
void Dronez_SeedButton( void *pClass, int id, bool bOn )
{
	Dronez *mymodule;
    mymodule = (Dronez*)pClass;

    mymodule->ChangeSeedPending( mymodule->getseed() );
}

//-----------------------------------------------------
// Dronez_RandButton
//-----------------------------------------------------
void Dronez_RandButton( void *pClass, int id, bool bOn )
{
	Dronez *mymodule;
    mymodule = (Dronez*)pClass;

    mymodule->ChangeSeedPending( (int)randomu32() );
}

//-----------------------------------------------------
// Procedure:   Widget
//
//-----------------------------------------------------

struct Dronez_Widget : ModuleWidget {
	Dronez_Widget( Dronez *module );
};

Dronez_Widget::Dronez_Widget( Dronez *module ) : ModuleWidget(module)
{
	int i, x, y;

	box.size = Vec( 15*5, 380 );

	{
		SVGPanel *panel = new SVGPanel();
		panel->box.size = box.size;
		panel->setBackground(SVG::load(assetPlugin(plugin, "res/Dronez.svg")));
		addChild(panel);
	}

	addInput(Port::create<MyPortInSmall>( Vec( 10, 20 ), Port::INPUT, module, Dronez::IN_VOCT ) );
	addInput(Port::create<MyPortInSmall>( Vec( 10, 241 ), Port::INPUT, module, Dronez::IN_RANDTRIG ) );

    // random button
    module->m_pButtonRand = new MyLEDButton( 40, 238, 25, 25, 20.0, DWRGB( 180, 180, 180 ), DWRGB( 255, 0, 0 ), MyLEDButton::TYPE_MOMENTARY, 0, module, Dronez_RandButton );
	addChild( module->m_pButtonRand );

	addOutput(Port::create<MyPortOutSmall>( Vec( 48, 20 ), Port::OUTPUT, module, Dronez::OUT_L ) );
	addOutput(Port::create<MyPortOutSmall>( Vec( 48, 45 ), Port::OUTPUT, module, Dronez::OUT_R ) );

	y = 95;
	x = 9;

    //module->lg.Open("c://users//mark//documents//rack//Dronez.txt");

	for( i = 31; i >=0; i-- )
	{
	    module->m_pButtonSeed[ i ] = new MyLEDButton( x, y, 11, 11, 8.0, DWRGB( 180, 180, 180 ), DWRGB( 255, 255, 0 ), MyLEDButton::TYPE_SWITCH, i, module, Dronez_SeedButton );
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

	addParam(ParamWidget::create<Dronez::MySpeed_Knob>( Vec( 10, 280 ), module, Dronez::PARAM_SPEED, 0.0, 8.0, 4.0 ) );

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
void Dronez::JsonParams( bool bTo, json_t *root)
{
    JsonDataInt( bTo, "m_Seed", root, &m_Seed, 1 );
}

//-----------------------------------------------------
// Procedure: toJson  
//
//-----------------------------------------------------
json_t *Dronez::toJson()
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
void Dronez::fromJson( json_t *root )
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
void Dronez::onCreate()
{
}

//-----------------------------------------------------
// Procedure:   onDelete
//
//-----------------------------------------------------
void Dronez::onDelete()
{
}

//-----------------------------------------------------
// Procedure:   onReset
//
//-----------------------------------------------------
void Dronez::onReset()
{
}

//-----------------------------------------------------
// Procedure:   onRandomize
//
//-----------------------------------------------------
void Dronez::onRandomize()
{
	ChangeSeedPending( (int)randomu32() );
}

//-----------------------------------------------------
// Procedure:   getseed
//
//-----------------------------------------------------
int Dronez::getseed( void )
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
void Dronez::putseed( int seed )
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
void Dronez::ChangeSeedPending( int seed )
{
	m_FadeState = FADE_OUT;
	putseed( seed );
}

//-----------------------------------------------------
// Procedure:   RandWave
//-----------------------------------------------------
void Dronez::RandWave( EnvelopeData *pEnv, float min, float max )
{
	int i;

    for( i = 0; i < ENVELOPE_HANDLES - 1; i++ )
        pEnv->setVal( i, frand_mm( min, max ) );

    pEnv->setVal( i, pEnv->m_HandleVal[ 0 ] );
}

//-----------------------------------------------------
// Procedure:   RandPresetWaveAdjust
//-----------------------------------------------------
void Dronez::RandPresetWaveAdjust( EnvelopeData *pEnv )
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
float semis[ 8 ] = { (SEMI * 5.0f), (SEMI * 6.0f), (SEMI * 9.0f), (SEMI * 11.0f), (SEMI * 21.0f), (SEMI * 12.0f), (SEMI * 24.0f), (SEMI * 29.0f) };
void Dronez::BuildWave( int ch )
{
	m_osc[ ch ].semi = semis[ srand() & 0x7 ];

	// audio waves
	m_wav[ ch ][ 0 ].Init( EnvelopeData::MODE_LOOP, EnvelopeData::RANGE_Audio, false, 1.0f );
	RandPresetWaveAdjust( &m_wav[ ch ][ 0 ] );
	//RandWave( &m_wav[ ch ][ 0 ], 0.0f, 1.0f );

	m_wav[ ch ][ 1 ].Init( EnvelopeData::MODE_LOOP, EnvelopeData::RANGE_Audio, false, 1.0f );
	//RandPresetWaveAdjust( &m_wav[ ch ][ 1 ], -0.1f, 0.1f );
	m_wav[ ch ][ 1 ].Preset( (int)frand_mm( 2.0f, 7.2f) );

	m_wav[ ch ][ 2 ].Init( EnvelopeData::MODE_LOOP, EnvelopeData::RANGE_Audio, false, 1.0f );
	RandPresetWaveAdjust( &m_wav[ ch ][ 2 ] );
	//RandWave( &m_wav[ ch ][ 2 ], 0.0f, 1.0f );

	// modulation waveforms
	m_mod[ ch ][ MOD_MORPH ].Init( EnvelopeData::MODE_LOOP, EnvelopeData::RANGE_n1to1, false, 1.0f );
	m_finc[ ch ][ MOD_MORPH ] = 1.0f / frand_mm( 14.5f, 38.0f );
	RandWave( &m_mod[ ch ][ MOD_MORPH ], 0.3f, 0.7f );

	m_mod[ ch ][ MOD_LEVEL ].Init( EnvelopeData::MODE_LOOP, EnvelopeData::RANGE_0to1, false, 1.0f );
	m_finc[ ch ][ MOD_LEVEL ] = 1.0f / frand_mm( 14.5f, 38.0f );
	RandWave( &m_mod[ ch ][ MOD_LEVEL ], 0.1f, 0.4f );

	m_mod[ ch ][ MOD_DET1 ].Init( EnvelopeData::MODE_LOOP, EnvelopeData::RANGE_0to1, false, 1.0f );
	m_finc[ ch ][ MOD_DET1 ] = 1.0f / frand_mm( 14.5f, 38.0f );
	RandWave( &m_mod[ ch ][ MOD_DET1 ], 0.01f, 0.1f );

	m_mod[ ch ][ MOD_DET2 ].Init( EnvelopeData::MODE_LOOP, EnvelopeData::RANGE_0to1, false, 1.0f );
	m_finc[ ch ][ MOD_DET2 ] = 1.0f / frand_mm( 14.5f, 38.0f );
	RandWave( &m_mod[ ch ][ MOD_DET2 ], 0.01f, 0.1f );
}

//-----------------------------------------------------
// Procedure:   BuildDrone
//
//-----------------------------------------------------
void Dronez::BuildDrone( void )
{
	int i, ch;

    init_rand( m_Seed );

	for( ch = 0; ch < nCHANNELS; ch++ )
	{
		BuildWave( ch );
	}


	m_global[ GOSC_FILTER ].Init( EnvelopeData::MODE_LOOP, EnvelopeData::RANGE_0to1, false, 1.0f );
	m_GlobalOsc[ GOSC_FILTER ].finc = 1.0f / frand_mm( 14.5f, 38.0f );
	RandWave( &m_global[ GOSC_FILTER ], 0.01f, 1.0f );
	//m_GlobalOsc[GOSC_FILTERi ].Env.Preset( EnvelopeData::PRESET_SIN );
	//RandPresetWaveAdjust( &m_GlobalOsc[GOSC_FILTER ].Env );

	m_global[ GOSC_NOISE ].Init( EnvelopeData::MODE_LOOP, EnvelopeData::RANGE_0to1, false, 1.0f );
	m_GlobalOsc[ GOSC_NOISE ].finc = 1.0f / frand_mm( 14.5f, 38.0f );
	RandWave( &m_global[ GOSC_NOISE ], 0.01f, 0.3f );
	//m_GlobalOsc[GOSC_NOISE ].Env.Preset( EnvelopeData::PRESET_SIN );
	//RandPresetWaveAdjust( &m_GlobalOsc[ GOSC_NOISE ].Env );

	m_cuttoff = frand_mm( 0.05f, 0.4f );
	m_rez = frand_mm( 0.1f, 0.8f );

    //-----------------------------------------------------
    // Reverb
	for( i = 0; i < REV_TAPS; i++ )
		m_reverb.out[ i ] = ( m_reverb.in - (int)( engineGetSampleRate() * frand_mm( 0.01f, .1f ) ) ) & REV_BUF_MAX;

	m_bInitialized = true;
}

//-----------------------------------------------------
// Procedure:   putf
//
//-----------------------------------------------------
void Dronez::putf( float fval )
{
    char strVal[ 10 ] = {};

    sprintf( strVal, "%.3f", fval );
    m_pTextLabel->text = strVal;
}

//-----------------------------------------------------
// Procedure:   putf
//
//-----------------------------------------------------
void Dronez::putx( int x )
{
    char strVal[ 10 ] = {};

    sprintf( strVal, "%.8X", x );
    m_pTextLabel->text = strVal;
}

//-----------------------------------------------------
// Procedure:   ChangeFilterCutoff
//
//-----------------------------------------------------
void Dronez::ChangeFilterCutoff( FILTER_STRUCT *pf, float cutfreq )
{
    float fx, fx2, fx3, fx5, fx7;

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
void Dronez::processFilter( FILTER_STRUCT *pf, float *pIn )
{
    float rez, hp1;
    float input, lowpass, bandpass, highpass;

    rez = 1.0 - m_rez;

    input = *pIn / AUDIO_MAX;

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

    *pIn = lowpass * AUDIO_MAX;
}

//-----------------------------------------------------
// Procedure:   processReverb
//
//-----------------------------------------------------
void Dronez::processReverb( float In, float *pL, float *pR )
{
	float fl = 0, fr = 0, rin;

	for( int i = 0; i < REV_TAPS; i++ )
	{
		rin = m_reverb.buf[ m_reverb.out[ i ]++ ] * 0.2f;
		m_reverb.out[ i ] &= REV_BUF_MAX;

		if( i < (REV_TAPS / 2) )
			fl += rin;
		else
			fr += rin;
	}

	m_reverb.buf[ m_reverb.in++ ] = In;
	m_reverb.in &= REV_BUF_MAX;

	*pL = (In * .3) + fl;
	*pR = (In * .3) + fr;
}

//-----------------------------------------------------
// Procedure:   step
//
//-----------------------------------------------------
void Dronez::step()
{
	float out = 0.0f, In =0.0f, outL = 0.0f, outR = 0.0f, fmorph[ nCHANNELS ], fcv=0.0f;
	int ch, i, wv;

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

	for( i = 0; i < nGLOBALOSCS; i++ )
	{
		m_global[ i ].m_Clock.syncInc = m_GlobalOsc[ i ].finc * speeds[ (int)params[ PARAM_SPEED ].value ];
		m_GlobalOsc[ i ].fval = m_global[ i ].procStep( false, false );
	}

	// process oscillators
	for( ch = 0; ch < nCHANNELS; ch++ )
	{
		// process modulation waves
		for( i = 0; i < nMODS; i++ )
		{
			m_mod[ ch ][ i ].m_Clock.syncInc = m_finc[ ch ][ i ] * speeds[ (int)params[ PARAM_SPEED ].value ];
			m_osc[ ch ].fval[ i ] = m_mod[ ch ][ i ].procStep( false, false );
		}

		// wav morph modulation
		memset( fmorph, 0, sizeof(fmorph));

	    fcv = m_osc[ ch ].fval[ MOD_MORPH ];
	    fmorph[ 1 ] = 1.0 - fabs( fcv );

	    // left wave
	    if( fcv <= 0.0f )
	    {
	        fmorph[ 0 ] = -fcv;
	    }
	    else
	    {
	        fmorph[ 2 ] = fcv;
	    }

		In= 0.0f;

		// get wave audio
		for( wv = 0; wv < nMORPH_WAVES; wv++ )
		{
			m_wav[ ch ][ wv ].m_Clock.syncInc = 32.7032f * clamp( powf( 2.0f, clamp( inputs[ IN_VOCT ].normalize( 3.0f ) + m_osc[ ch ].semi, 0.0f, VOCT_MAX ) ), 0.0f, 4186.01f );

			if( wv == 0 )
			{
				m_wav[ ch ][ wv ].m_Clock.syncInc += m_osc[ ch ].fval[ MOD_DET1 ];
			}
			else if( wv == 2 )
			{
				m_wav[ ch ][ wv ].m_Clock.syncInc += m_osc[ ch ].fval[ MOD_DET2 ];
			}

			In += m_wav[ ch ][ wv ].procStep( false, false ) * fmorph[ wv ];
		}

		if( wv != 1 )
			out += In * m_osc[ ch ].fval[ MOD_LEVEL ];
		else
			out += In;
	}

	if( frand_perc( 75.0f ) )
		out += frand_mm( -1.0f, 1.0f ) * m_GlobalOsc[ GOSC_NOISE ].fval;

	// filter
	ChangeFilterCutoff( &m_filter, m_cuttoff * m_GlobalOsc[ GOSC_FILTER ].fval );
	processFilter( &m_filter, &out );

	out *= m_fFade;

	processReverb( out, &outL, &outR );

	outputs[ OUT_L ].value = outL;
	outputs[ OUT_R ].value = outR;
}

} // namespace rack_plugin_mscHack

using namespace rack_plugin_mscHack;

RACK_PLUGIN_MODEL_INIT(mscHack, Dronez) {
   Model *modelDronez = Model::create<Dronez, Dronez_Widget>( "mscHack", "Dronez", "Dronez module", OSCILLATOR_TAG, MULTIPLE_TAG );
   return modelDronez;
}
