#include "mscHack.hpp"
//#include "mscHack_Controls.hpp"
#include "dsp/digital.hpp"
//#include "CLog.h"

namespace rack_plugin_mscHack {

#define nCHANNELS 3
#define CHANNEL_H 90
#define CHANNEL_Y 65
#define CHANNEL_X 10

#define freqMAX 300.0f
#define ADS_MAX_TIME_SECONDS 0.5f

#define WAVE_BUFFER_LEN ( 192000 / 20 ) // (9600) based on quality for 20Hz at max sample rate 192000

typedef struct
{
    int   state;
    int   a, s, r;
    int   acount, scount, rcount, fadecount;
    float fainc, fsinc, frinc, fadeinc;
    float out;
    bool  bTrig;
}ASR_STRUCT;

typedef struct
{
    int   wavetype;
    

    // wave
    float phase;

    //filter
    float q, f;
    bool  bHighpass;

    float lp1, bp1;
    float hpIn;
    float lpIn;
    float mpIn;

    // ads
    ASR_STRUCT adsr_wave;
    ASR_STRUCT adsr_freq;
    
}OSC_PARAM_STRUCT;

//-----------------------------------------------------
// Module Definition
//
//-----------------------------------------------------
struct SynthDrums : Module 
{
    enum WaveTypes
    {
        WAVE_SIN,
        WAVE_TRI,
        WAVE_SQR,
        WAVE_SAW,
        WAVE_NOISE,
        nWAVEFORMS
    };

	enum ParamIds 
    {
        PARAM_FREQ,
        PARAM_ATT       = PARAM_FREQ + nCHANNELS,
        PARAM_REL       = PARAM_ATT + nCHANNELS,
        PARAM_REZ       = PARAM_REL + nCHANNELS,
        PARAM_WAVES     = PARAM_REZ + nCHANNELS,
        nPARAMS         = PARAM_WAVES + (nWAVEFORMS * nCHANNELS)
    };

	enum InputIds 
    {
        IN_LEVEL,
        IN_TRIG         = IN_LEVEL + nCHANNELS,
        IN_FREQ_MOD     = IN_TRIG + nCHANNELS,
        nINPUTS         = IN_FREQ_MOD + nCHANNELS
	};

	enum OutputIds 
    {
        OUTPUT_AUDIO,
        nOUTPUTS        = OUTPUT_AUDIO + nCHANNELS
	};

    enum ADSRSTATES
    {
        ADSR_OFF,
        ADSR_FADE,
        ADSR_ON,
        ADSR_ATTACK,
        ADSR_SUSTAIN,
        ADSR_RELEASE
    };

    CLog            lg;

    SchmittTrigger   m_SchTrig[ nCHANNELS ];

    OSC_PARAM_STRUCT m_Wave[ nCHANNELS ] = {};

    MyLEDButtonStrip *m_pButtonWaveSelect[ nCHANNELS ] = {};

    // waveforms
    float           m_BufferWave[ nWAVEFORMS ][ WAVE_BUFFER_LEN ] = {};

    // Contructor
	SynthDrums() : Module( nPARAMS, nINPUTS, nOUTPUTS, 0 ){}

    //-----------------------------------------------------
    // MyParamKnob
    //-----------------------------------------------------
    struct MyParamFreq : Knob_Yellow2_26
    {
        SynthDrums *mymodule;
        int param;

        void onChange( EventChange &e ) override 
        {
            mymodule = (SynthDrums*)module;

            if( mymodule )
            {
                param = paramId - SynthDrums::PARAM_FREQ;

                if( mymodule->m_Wave[ param ].wavetype == WAVE_NOISE )
                    mymodule->ChangeFilterCutoff( param, value );
                else
                    mymodule->ChangeFilterCutoff( param, 0.6 );
            }

		    RoundKnob::onChange( e );
	    }
    };

    // Overrides 
	void    step() override;
    json_t* toJson() override;
    void    fromJson(json_t *rootJ) override;
    void    onRandomize() override;
    void    onReset() override;

    void    SetWaveLights( void );
    void    BuildWaves( void );
    void    ChangeFilterCutoff( int ch, float cutfreq );
    float   Filter( int ch, float in, bool bHighPass );
    float   GetWave( int type, float phase );
    float   ProcessADS( int ch, bool bWave );
    float   GetAudio( int ch );
};

//-----------------------------------------------------
// SynthDrums_WaveSelect
//-----------------------------------------------------
void SynthDrums_WaveSelect( void *pClass, int id, int nbutton, bool bOn )
{
    SynthDrums *mymodule;
    mymodule = (SynthDrums*)pClass;
    mymodule->m_Wave[ id ].wavetype = nbutton;
}

//-----------------------------------------------------
// Procedure:   
//
//-----------------------------------------------------
json_t *SynthDrums::toJson() 
{
    json_t *gatesJ;
	json_t *rootJ = json_object();

    // wavetypes
	gatesJ = json_array();

	for (int i = 0; i < nCHANNELS; i++)
    {
		json_t *gateJ = json_integer( m_Wave[ i ].wavetype );
		json_array_append_new( gatesJ, gateJ );
	}

	json_object_set_new( rootJ, "wavetypes", gatesJ );

	return rootJ;
}

//-----------------------------------------------------
// Procedure:   fromJson
//
//-----------------------------------------------------
void SynthDrums::fromJson(json_t *rootJ) 
{
    int i;
    json_t *StepsJ;

    // wave select
	StepsJ = json_object_get( rootJ, "wavetypes" );

	if (StepsJ) 
    {
		for ( i = 0; i < nCHANNELS; i++)
        {
			json_t *gateJ = json_array_get(StepsJ, i);

			if (gateJ)
				m_Wave[ i ].wavetype = json_integer_value( gateJ );
		}
	}

    SetWaveLights();
}

//-----------------------------------------------------
// Procedure:   Widget
//
//-----------------------------------------------------

struct SynthDrums_Widget : ModuleWidget {
	SynthDrums_Widget( SynthDrums *module );
};

SynthDrums_Widget::SynthDrums_Widget( SynthDrums *module ) : ModuleWidget(module) 
{
    int ch, x, y;

	box.size = Vec( 15*11, 380);

	{
		SVGPanel *panel = new SVGPanel();
		panel->box.size = box.size;
		panel->setBackground(SVG::load(assetPlugin(plugin, "res/SynthDrums.svg")));
		addChild(panel);
	}

    //module->lg.Open("SynthDrums.txt");

	addChild(Widget::create<ScrewSilver>(Vec(15, 0)));
	addChild(Widget::create<ScrewSilver>(Vec(box.size.x-30, 0)));
	addChild(Widget::create<ScrewSilver>(Vec(15, 365))); 
	addChild(Widget::create<ScrewSilver>(Vec(box.size.x-30, 365)));

    y = CHANNEL_Y;
    x = CHANNEL_X;

    for( ch = 0; ch < nCHANNELS; ch++ )
    {
        x = CHANNEL_X;

        // IN_FREQ_MOD
        addInput(Port::create<MyPortInSmall>( Vec( x + 48, y ), Port::INPUT, module, SynthDrums::IN_FREQ_MOD + ch ) );

        // inputs
        addInput(Port::create<MyPortInSmall>( Vec( x, y ), Port::INPUT, module, SynthDrums::IN_LEVEL + ch ) );

        y += 43;

        addInput(Port::create<MyPortInSmall>( Vec( x, y ), Port::INPUT, module, SynthDrums::IN_TRIG + ch ) );

        x += 32;
        y += 7;

        // wave select buttons
        module->m_pButtonWaveSelect[ ch ] = new MyLEDButtonStrip( x, y, 11, 11, 5, 8.0, 5, false, DWRGB( 180, 180, 180 ), DWRGB( 255, 255, 0 ), MyLEDButtonStrip::TYPE_EXCLUSIVE, ch, module, SynthDrums_WaveSelect );
	    addChild( module->m_pButtonWaveSelect[ ch ] );

        x = CHANNEL_X + 25;
        y -= 34;

        // params
        addParam(ParamWidget::create<SynthDrums::MyParamFreq>( Vec( x, y ), module, SynthDrums::PARAM_FREQ + ch, 0.0, 1.0, 0.0 ) );

        x += 34;

        addParam(ParamWidget::create<Knob_Yellow2_26>( Vec( x, y ), module, SynthDrums::PARAM_ATT + ch, 0.0, 0.1, 0.0 ) );

        x += 34;

        addParam(ParamWidget::create<Knob_Yellow2_26>( Vec( x, y ), module, SynthDrums::PARAM_REL + ch, 0.0, 1.0, 0.0 ) );

        y -= 15;
        x += 34;

        // outputs
        addOutput(Port::create<MyPortOutSmall>( Vec( x, y ), Port::OUTPUT, module, SynthDrums::OUTPUT_AUDIO + ch ) );

        y += CHANNEL_H;
    }

    module->BuildWaves();

    module->ChangeFilterCutoff( 0, 0.6 );
    module->ChangeFilterCutoff( 1, 0.6 );
    module->ChangeFilterCutoff( 2, 0.6 );

    module->SetWaveLights();
}

//-----------------------------------------------------
// Procedure:   reset
//
//-----------------------------------------------------
void SynthDrums::onReset()
{
}

//-----------------------------------------------------
// Procedure:   randomize
//
//-----------------------------------------------------
void SynthDrums::onRandomize()
{
    int ch;

    for( ch = 0; ch < nCHANNELS; ch++ )
    {
        m_Wave[ ch ].wavetype = (int)( randomUniform() * (nWAVEFORMS-1) );
    }

    SetWaveLights();
}

//-----------------------------------------------------
// Procedure:   SetWaveLights
//
//-----------------------------------------------------
void SynthDrums::SetWaveLights( void )
{
    int ch;

    for( ch = 0; ch < nCHANNELS; ch++ )
        m_pButtonWaveSelect[ ch ]->Set( m_Wave[ ch ].wavetype, true );
}

//-----------------------------------------------------
// Procedure:   initialize
//
//-----------------------------------------------------
//#define DEG2RAD( x ) ( ( x ) * ( 3.14159f / 180.0f ) )
void SynthDrums::BuildWaves( void )
{
    int i;
    float finc, pos, val;

    finc = 360.0 / WAVE_BUFFER_LEN;
    pos = 0;

    // create sin wave
    for( i = 0; i < WAVE_BUFFER_LEN; i++ )
    {
        m_BufferWave[ WAVE_SIN ][ i ] = sin( DEG2RAD( pos ) );
        pos += finc;
    }

    // create sqr wave
    for( i = 0; i < WAVE_BUFFER_LEN; i++ )
    {
        if( i < WAVE_BUFFER_LEN / 2 )
            m_BufferWave[ WAVE_SQR ][ i ] = 1.0;
        else
            m_BufferWave[ WAVE_SQR ][ i ] = -1.0;
    }

    finc = 2.0 / (float)WAVE_BUFFER_LEN;
    val = 1.0;

    // create saw wave
    for( i = 0; i < WAVE_BUFFER_LEN; i++ )
    {
        m_BufferWave[ WAVE_SAW ][ i ] = val;

        val -= finc;
    }

    finc = 4 / (float)WAVE_BUFFER_LEN;
    val = 0;

    // create tri wave
    for( i = 0; i < WAVE_BUFFER_LEN; i++ )
    {
        m_BufferWave[ WAVE_TRI ][ i ] = val;

        if( i < WAVE_BUFFER_LEN / 4 )
            val += finc;
        else if( i < (WAVE_BUFFER_LEN / 4) * 3 )
            val -= finc;
        else if( i < WAVE_BUFFER_LEN  )
            val += finc;
    }
}

//-----------------------------------------------------
// Procedure:   GetAudio
//
//-----------------------------------------------------
float SynthDrums::GetWave( int type, float phase )
{
    float fval = 0.0;
    float ratio = (float)(WAVE_BUFFER_LEN-1) / engineGetSampleRate();

    switch( type )
    {
    case WAVE_SIN:
    case WAVE_TRI:
    case WAVE_SQR:
    case WAVE_SAW:
        fval = m_BufferWave[ type ][ int( ( phase * ratio ) + 0.5 ) ];
        break;

    case WAVE_NOISE:
        fval = ( randomUniform() > 0.5 ) ? (randomUniform() * -1.0) : randomUniform();
        break;

    default:
        break;
    }

    return fval;
}

//-----------------------------------------------------
// Procedure:   ProcessADS
//
//-----------------------------------------------------
float SynthDrums::ProcessADS( int ch, bool bWave )
{
    ASR_STRUCT *pasr;

    if( bWave )
        pasr = &m_Wave[ ch ].adsr_wave;
    else
        pasr = &m_Wave[ ch ].adsr_freq;

    // rettrig the adsr
    if( pasr->bTrig )
    {
        pasr->state = ADSR_FADE;

        pasr->fadecount = 900;
        pasr->fadeinc  = pasr->out / (float)pasr->fadecount;

        pasr->acount = 20;
        pasr->fainc  = 1.0 / pasr->acount;

        pasr->scount = 0;//(int)(.1 * engineGetSampleRate());
        pasr->fsinc  = 0;//1.0 / pasr->scount;
        
        if( bWave )
        {
            // for the wave asr the release is the waveform release
            pasr->rcount = (int)( params[ PARAM_REL + ch ].value * ADS_MAX_TIME_SECONDS * engineGetSampleRate() );
        }
        else
        {
            // for the wave asr the release is the hit
            pasr->rcount = (int)( params[ PARAM_ATT + ch ].value * ADS_MAX_TIME_SECONDS * engineGetSampleRate() );
        }

        if( pasr->rcount )
            pasr->frinc = 1.0 / pasr->rcount;

        pasr->bTrig = false;
    }

    // process
    switch( pasr->state )
    {
    case ADSR_FADE:
        if( --pasr->fadecount <= 0 )
        {
            pasr->state = ADSR_ATTACK;
            pasr->out = 0.0f;
            m_Wave[ ch ].phase = 0.0;
        }
        else
        {
            pasr->out -= pasr->fadeinc;
        }

        break;

    case ADSR_OFF:
        pasr->out = 0.0;
        break;

    case ADSR_ATTACK:
        if( --pasr->acount <= 0 )
        {
            pasr->state = ADSR_SUSTAIN;
        }
        else
        {
            pasr->out += pasr->fainc;
        }
        break;

    case ADSR_SUSTAIN:
        pasr->out = 1.0;
        if( --pasr->scount <= 0 )
        {
            pasr->state = ADSR_RELEASE;
        }

        break;

    case ADSR_RELEASE:
        
        if( --pasr->rcount <= 0 )
        {
            pasr->out = 0.0f;
            pasr->state = ADSR_OFF;
        }
        else
        {
            pasr->out -= pasr->frinc;
        }
        break;
    }

    return clamp( pasr->out, 0.0f, 1.0f );
}

//-----------------------------------------------------
// Procedure:   ChangeFilterCutoff
//
//-----------------------------------------------------
void SynthDrums::ChangeFilterCutoff( int ch, float cutfreq )
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

    m_Wave[ ch ].f = 2.0 * (fx 
	    - (fx3 * 0.16666666666666666666666666666667) 
	    + (fx5 * 0.0083333333333333333333333333333333) 
	    - (fx7 * 0.0001984126984126984126984126984127));
}

//-----------------------------------------------------
// Procedure:   Filter
//
//-----------------------------------------------------
#define MULTI (0.33333333333333333333333333333333f)
float SynthDrums::Filter( int ch, float in, bool bHighPass )
{
    OSC_PARAM_STRUCT *p;
    float rez, hp1; 
    float lowpass, highpass;

    p = &m_Wave[ ch ];

    rez = 1.00;//1.00 - p->q;

    in = in + 0.000000001;

    p->lp1   = p->lp1 + p->f * p->bp1; 
    hp1      = in - p->lp1 - rez * p->bp1; 
    p->bp1   = p->f * hp1 + p->bp1; 
    lowpass  = p->lp1; 
    highpass = hp1; 
    //bandpass = p->bp1; 

    p->lp1   = p->lp1 + p->f * p->bp1; 
    hp1      = in - p->lp1 - rez * p->bp1; 
    p->bp1   = p->f * hp1 + p->bp1; 
    lowpass  = lowpass  + p->lp1; 
    highpass = highpass + hp1; 
    //bandpass = bandpass + p->bp1; 

    in = in - 0.000000001;

    p->lp1   = p->lp1 + p->f * p->bp1; 
    hp1      = in - p->lp1 - rez * p->bp1; 
    p->bp1   = p->f * hp1 + p->bp1; 

    lowpass  = (lowpass  + p->lp1) * MULTI; 
    highpass = (highpass + hp1) * MULTI; 
    //bandpass = (bandpass + p->bp1) * MULTI;

    if( bHighPass )
        return highpass;

    return lowpass;
}

//-----------------------------------------------------
// Procedure:   GetAudio
//
//-----------------------------------------------------
float SynthDrums::GetAudio( int ch )
{
    float fout = 0, fenv = 0.0, freq, freqmod;

    if( outputs[ OUTPUT_AUDIO + ch ].active )
    {
        freqmod = clamp( inputs[ IN_FREQ_MOD + ch ].value / CV_MAX, 0.0f, 1.0f );

        // process our second envelope for hit
        fenv = ProcessADS( ch, false );

        // if noise then frequency affects the filter cutoff and not the wave frequency
        if( m_Wave[ ch ].wavetype == WAVE_NOISE )
        {
            freq = clamp( ( ( freqmod + params[ PARAM_FREQ + ch ].value ) + (fenv*2) ), 0.0f, 1.0f );

            ChangeFilterCutoff( ch, freq );
        }
        // other signals the second ADS affects the frequency for the hit
        else
        {
            m_Wave[ ch ].phase += 35 + ( ( freqmod + params[ PARAM_FREQ + ch ].value ) * freqMAX ) + ( fenv * 400.0f );

            if( m_Wave[ ch ].phase >= engineGetSampleRate() )
                m_Wave[ ch ].phase = m_Wave[ ch ].phase - engineGetSampleRate();
        }

        fout = ProcessADS( ch, true ) * GetWave( m_Wave[ ch ].wavetype, m_Wave[ ch ].phase );
        fout = Filter( ch, fout, ( m_Wave[ ch ].wavetype == WAVE_NOISE ) );
    }

    return fout;
}

//-----------------------------------------------------
// Procedure:   step
//
//-----------------------------------------------------
void SynthDrums::step() 
{
    int ch;

    // check for triggers
    for( ch = 0; ch < nCHANNELS; ch++ )
    {
	    if( inputs[ IN_TRIG + ch ].active ) 
        {
		    if( m_SchTrig[ ch ].process( inputs[ IN_TRIG + ch ].value ) )
            {
    		    m_Wave[ ch ].adsr_freq.bTrig = true;
                m_Wave[ ch ].adsr_wave.bTrig = true;
            }
	    }
    }

    // process sounds
    outputs[ OUTPUT_AUDIO + 0 ].value = GetAudio( 0 ) * AUDIO_MAX * clamp( (inputs[ IN_LEVEL + 0 ].value / CV_MAX), 0.0f, 1.0f );
    outputs[ OUTPUT_AUDIO + 1 ].value = GetAudio( 1 ) * AUDIO_MAX * clamp( (inputs[ IN_LEVEL + 1 ].value / CV_MAX), 0.0f, 1.0f );
    outputs[ OUTPUT_AUDIO + 2 ].value = GetAudio( 2 ) * AUDIO_MAX * clamp( (inputs[ IN_LEVEL + 2 ].value / CV_MAX), 0.0f, 1.0f );
}

} // namespace rack_plugin_mscHack

using namespace rack_plugin_mscHack;

RACK_PLUGIN_MODEL_INIT(mscHack, SynthDrums) {
   Model *modelSynthDrums = Model::create<SynthDrums, SynthDrums_Widget>( "mscHack", "SynthDrums", "SYNTH Drums", DRUM_TAG, MULTIPLE_TAG );
   return modelSynthDrums;
}
