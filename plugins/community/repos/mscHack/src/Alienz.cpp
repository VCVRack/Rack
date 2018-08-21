#include "mscHack.hpp"
#include "dsp/digital.hpp"

namespace rack_plugin_mscHack {

//-----------------------------------------------------
// General Definition
//-----------------------------------------------------
#define nCHANNELS 2

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
#define nMORPH_WAVES 2

enum MOD_TYPES
{
    MOD_LEVEL,
	MOD_REZ,
	nMODS
};


#define nBITS 2048

typedef struct
{
	int     state;
	int     baud;
	float   finc;
	float   fcount;
	float   fout;
	int     bits[ nBITS ];

}DIGITAL_OSC_STRUCT;

#define OSC2_NOTES 5

//-----------------------------------------------------
// Module Definition
//
//-----------------------------------------------------
struct Alienz : Module
{
	enum ParamIds 
    {
		PARAM_SPEED,
        nPARAMS
    };

	enum InputIds 
    {
		IN_RANDTRIG,
		IN_GATE,
        nINPUTS 
	};

	enum OutputIds 
    {
		OUT,
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
	Alienz() : Module(nPARAMS, nINPUTS, nOUTPUTS, nLIGHTS){}

	Label				*m_pTextLabel = NULL;
	Label				*m_pTextLabel2 = NULL;

	// osc
	DIGITAL_OSC_STRUCT  m_bitosc = {};

	EnvelopeData        m_osc2;
	float               m_osc2notes[ OSC2_NOTES ];
	float               m_osc2freq;

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
        Alienz *mymodule;
        char strVal[ 10 ] = {};

        void onChange( EventChange &e ) override
        {
            mymodule = (Alienz*)module;

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
	void ChangeFilterCutoff( int ch, float f );
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
// Alienz_SeedButton
//-----------------------------------------------------
void Alienz_SeedButton( void *pClass, int id, bool bOn )
{
	Alienz *mymodule;
    mymodule = (Alienz*)pClass;

    mymodule->ChangeSeedPending( mymodule->getseed() );
}

//-----------------------------------------------------
// Alienz_RandButton
//-----------------------------------------------------
void Alienz_RandButton( void *pClass, int id, bool bOn )
{
	Alienz *mymodule;
    mymodule = (Alienz*)pClass;

    mymodule->ChangeSeedPending( (int)randomu32() );
}

//-----------------------------------------------------
// Procedure:   Widget
//
//-----------------------------------------------------

struct Alienz_Widget : ModuleWidget {
	Alienz_Widget( Alienz *module );
};

Alienz_Widget::Alienz_Widget( Alienz *module ) : ModuleWidget(module)
{
	int i, x, y;

	box.size = Vec( 15*5, 380 );

	{
		SVGPanel *panel = new SVGPanel();
		panel->box.size = box.size;
		panel->setBackground(SVG::load(assetPlugin(plugin, "res/Alienz.svg")));
		addChild(panel);
	}

	addInput(Port::create<MyPortInSmall>( Vec( 10, 20 ), Port::INPUT, module, Alienz::IN_GATE ) );
	addInput(Port::create<MyPortInSmall>( Vec( 10, 241 ), Port::INPUT, module, Alienz::IN_RANDTRIG ) );

    // random button
    module->m_pButtonRand = new MyLEDButton( 40, 238, 25, 25, 20.0, DWRGB( 180, 180, 180 ), DWRGB( 255, 0, 0 ), MyLEDButton::TYPE_MOMENTARY, 0, module, Alienz_RandButton );
	addChild( module->m_pButtonRand );

	addOutput(Port::create<MyPortOutSmall>( Vec( 48, 20 ), Port::OUTPUT, module, Alienz::OUT ) );

	y = 95;
	x = 9;

    //module->lg.Open("c://users//mark//documents//rack//Alienz.txt");

	for( i = 31; i >=0; i-- )
	{
	    module->m_pButtonSeed[ i ] = new MyLEDButton( x, y, 11, 11, 8.0, DWRGB( 180, 180, 180 ), DWRGB( 255, 255, 0 ), MyLEDButton::TYPE_SWITCH, i, module, Alienz_SeedButton );
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

	addParam(ParamWidget::create<Alienz::MySpeed_Knob>( Vec( 10, 280 ), module, Alienz::PARAM_SPEED, 0.0, 8.0, 4.0 ) );

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
void Alienz::JsonParams( bool bTo, json_t *root)
{
    JsonDataInt( bTo, "m_Seed", root, &m_Seed, 1 );
}

//-----------------------------------------------------
// Procedure: toJson  
//
//-----------------------------------------------------
json_t *Alienz::toJson()
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
void Alienz::fromJson( json_t *root )
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
void Alienz::onCreate()
{
}

//-----------------------------------------------------
// Procedure:   onDelete
//
//-----------------------------------------------------
void Alienz::onDelete()
{
}

//-----------------------------------------------------
// Procedure:   onReset
//
//-----------------------------------------------------
void Alienz::onReset()
{
}

//-----------------------------------------------------
// Procedure:   onRandomize
//
//-----------------------------------------------------
void Alienz::onRandomize()
{
	ChangeSeedPending( (int)randomu32() );
}

//-----------------------------------------------------
// Procedure:   getseed
//
//-----------------------------------------------------
int Alienz::getseed( void )
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
void Alienz::putseed( int seed )
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
void Alienz::ChangeSeedPending( int seed )
{
	m_FadeState = FADE_OUT;
	putseed( seed );
}

//-----------------------------------------------------
// Procedure:   RandWave
//-----------------------------------------------------
void Alienz::RandWave( EnvelopeData *pEnv, float min, float max )
{
	int i;

    for( i = 0; i < ENVELOPE_HANDLES - 1; i++ )
        pEnv->setVal( i, frand_mm( min, max ) );

    pEnv->setVal( i, pEnv->m_HandleVal[ 0 ] );
}

//-----------------------------------------------------
// Procedure:   RandPresetWaveAdjust
//-----------------------------------------------------
void Alienz::RandPresetWaveAdjust( EnvelopeData *pEnv )
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
void Alienz::BuildWave( int ch )
{
	// modulation waveforms
	m_mod[ ch ][ MOD_LEVEL ].Init( EnvelopeData::MODE_LOOP, EnvelopeData::RANGE_0to1, false, 1.0f );
	m_finc[ ch ][ MOD_LEVEL ] = 1.0f / frand_mm( 14.5f, 38.0f );

	if( ch == 0 )
		RandWave( &m_mod[ ch ][ MOD_LEVEL ], 0.8f, 0.9f );
	else
		RandWave( &m_mod[ ch ][ MOD_LEVEL ], 0.1f, 0.4f );

	m_mod[ ch ][ MOD_REZ ].Init( EnvelopeData::MODE_LOOP, EnvelopeData::RANGE_0to1, false, 1.0f );
	m_finc[ ch ][ MOD_REZ ] = 1.0f / frand_mm( 14.5f, 38.0f );
	RandWave( &m_mod[ ch ][ MOD_REZ ], 0.9f, 1.0f );
}

//-----------------------------------------------------
// Procedure:   BuildDrone
//
//-----------------------------------------------------
int bauds[ 4 ] = { 2400, 4800, 9600, 19200 };
void Alienz::BuildDrone( void )
{
	int ch, i;
	int byte;
	int bit;

    init_rand( m_Seed );

	for( ch = 0; ch < nCHANNELS; ch++ )
	{
		BuildWave( ch );

		ChangeFilterCutoff( ch, frand_mm( 0.1, 0.5 ) );
	}

	//set up osc
	m_bitosc.baud = bauds[ srand() & 3 ];

	m_bitosc.finc = engineGetSampleRate() / frand_mm( 100.0f, 400.0f );

	m_bitosc.fcount = 0.0f;

	m_bitosc.fout = 0.0f;

	for( i= 0; i < nBITS; i+=8 )
	{
		byte = (int)frand_mm( (float)0x20, (float)0x80 );

		for( bit = 0; bit < 8; bit++ )
		{
			m_bitosc.bits[ i + bit ] = (byte >> bit) & 1;
		}
	}

	// osc 2
	m_osc2.Init( EnvelopeData::MODE_LOOP, EnvelopeData::RANGE_n1to1, false, 1.0f );
	m_osc2.Preset( EnvelopeData::PRESET_TRI_FULL );

	for( i = 0; i < OSC2_NOTES; i++ )
		m_osc2notes[ i ] = frand_mm( 3.0f, 6.0f );

	m_osc2freq = engineGetSampleRate() / frand_mm( 60.0f, 90.0f );

	m_bInitialized = true;
}

//-----------------------------------------------------
// Procedure:   putf
//
//-----------------------------------------------------
void Alienz::putf( float fval )
{
    char strVal[ 10 ] = {};

    sprintf( strVal, "%.3f", fval );
    m_pTextLabel->text = strVal;
}

//-----------------------------------------------------
// Procedure:   putf
//
//-----------------------------------------------------
void Alienz::putx( int x )
{
    char strVal[ 10 ] = {};

    sprintf( strVal, "%.8X", x );
    m_pTextLabel->text = strVal;
}

//-----------------------------------------------------
// Procedure:   ChangeFilterCutoff
//
//-----------------------------------------------------
void Alienz::ChangeFilterCutoff( int ch, float f )
{
    float fx, fx2, fx3, fx5, fx7, cutfreq;
    FILTER_STRUCT *pf;

    pf = &m_filter[ ch ];

    cutfreq = f;

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
void Alienz::processFilter( int ch, float *pIn )
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
void Alienz::step()
{
	float In = 0.0f, fout;
	int i, ch;
	static int bcount = 0, note = 0, fcount = 0;

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
    	m_fFade -= 0.0005f;
    	if( m_fFade <= 0.0f )
    	{
    		m_fFade = 0.0f;
    		BuildDrone();
    		m_FadeState = FADE_IN;
    	}
    	break;
    case FADE_IN:
    	m_fFade += 0.0005f;
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

    if( inputs[ IN_GATE ].active )
    {
    	if( inputs[ IN_GATE ].value < 0.000001 )
    	{
    		outputs[ OUT ].value= 0.0f;
    		return;
    	}
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
	}

	m_bitosc.fcount -= 1.0f;

	// change
	if( m_bitosc.fcount <= 0.0f )
	{
		m_bitosc.fcount += m_bitosc.finc;

		if( m_bitosc.state == 2 )
			m_bitosc.state = m_bitosc.bits[ ( bcount++ ) & (nBITS - 1) ];
		else
			m_bitosc.state = 2;

		switch( m_bitosc.state )
		{
		case 0:
			m_bitosc.fout = -0.7f;
			break;
		case 1:
			m_bitosc.fout = 0.7f;
			break;
		case 2:
			m_bitosc.fout = 0.0f;
			break;
		}
	}

	In = m_bitosc.fout;
	processFilter( 0, &In );

	// osc2
	fcount -= 1.0f;

	// change
	if( fcount <= 0.0f )
	{
		fcount += m_osc2freq;

		if( ++note >= OSC2_NOTES )
			note = 0;

		m_osc2.m_Clock.syncInc = 32.7032f * clamp( powf( 2.0f, m_osc2notes[ note ] ), 0.0f, 4186.01f );
	}

	fout = m_osc2.procStep( false, false );

	processFilter( 1, &fout );

	fout *= AUDIO_MAX * m_fval[ 1 ][ MOD_LEVEL ];

	fout += In * m_fval[ 0 ][ MOD_LEVEL ] * AUDIO_MAX;

	outputs[ OUT ].value = fout * m_fFade;
}

} // namespace rack_plugin_mscHack

using namespace rack_plugin_mscHack;

RACK_PLUGIN_MODEL_INIT(mscHack, Alienz) {
   Model *modelAlienz = Model::create<Alienz, Alienz_Widget>( "mscHack", "Alienz", "Alienz module", OSCILLATOR_TAG, MULTIPLE_TAG );
   return modelAlienz;
}
