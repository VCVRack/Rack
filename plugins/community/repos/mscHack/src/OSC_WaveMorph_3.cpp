#include "mscHack.hpp"
#include "dsp/digital.hpp"

namespace rack_plugin_mscHack {

//-----------------------------------------------------
// Module Definition
//
//-----------------------------------------------------
struct OSC_WaveMorph_3 : Module 
{
#define nCHANNELS 3

	enum ParamIds 
    {
        PARAM_BAND,
        PARAM_LEVEL,
        PARAM_CUTOFF,
        PARAM_RES,
        PARAM_FILTER_MODE,
        nPARAMS
    };

	enum InputIds 
    {
        INPUT_VOCT,
        INPUT_MORPHCV,
        IN_FILTER,
        IN_REZ,
        IN_LEVEL,
        IN_WAVE_CHANGE,
        nINPUTS
	};

	enum OutputIds 
    {
        OUTPUT_AUDIO,
        nOUTPUTS
	};

	enum LightIds 
    {
        nLIGHTS
	};

    bool                m_bInitialized = false;

    CLog            lg;

    // Contructor
	OSC_WaveMorph_3() : Module(nPARAMS, nINPUTS, nOUTPUTS, nLIGHTS){}

    int                 m_CurrentChannel = 0;
    int                 m_GraphData[ MAX_ENVELOPE_CHANNELS ][ ENVELOPE_HANDLES ] = {};

    int                 m_waveSet = 0;
    bool                m_bCpy = false;

    bool                m_bSolo = false;

    SchmittTrigger      m_SchmittChangeWave;

    Widget_EnvelopeEdit *m_pEnvelope = NULL;
    MyLEDButtonStrip    *m_pButtonChSelect = NULL;

    MyLEDButton         *m_pButtonWaveSetBck = NULL;
    MyLEDButton         *m_pButtonWaveSetFwd = NULL;

    MyLEDButton         *m_pButtonDraw = NULL;
    MyLEDButton         *m_pButtonCopy = NULL;
    MyLEDButton         *m_pButtonRand = NULL;
    MyLEDButton         *m_pButtonInvert = NULL;
    MyLEDButton         *m_pButtonSolo = NULL;

    Label               *m_pTextLabel = NULL;

    // filter
    enum FILTER_TYPES
    {
        FILTER_OFF,
        FILTER_LP,
        FILTER_HP,
        FILTER_BP,
        FILTER_NT
     };

    int   filtertype;
    float q, f;
    float lp1 = 0, bp1 = 0;

    //-----------------------------------------------------
    // Band_Knob
    //-----------------------------------------------------
    struct Band_Knob : Knob_Yellow2_26
    {
        OSC_WaveMorph_3 *mymodule;
        int param;

        void onChange( EventChange &e ) override 
        {
            mymodule = (OSC_WaveMorph_3*)module;

            if( mymodule )
                mymodule->m_pEnvelope->m_fband = value; 

		    RoundKnob::onChange( e );
	    }
    };

    void    ChangeChannel( int ch );
    void    ChangeFilterCutoff( float cutfreq );
    void    Filter( float *In );

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
void OSC_WaveMorph_3_EnvelopeEditCALLBACK ( void *pClass, float val )
{
    char strVal[ 10 ] = {};

    OSC_WaveMorph_3 *mymodule;
    mymodule = (OSC_WaveMorph_3*)pClass;

    sprintf( strVal, "[%.3fV]", val );

    mymodule->m_pTextLabel->text = strVal;
}

//-----------------------------------------------------
// OSC_WaveMorph_3_DrawMode
//-----------------------------------------------------
void OSC_WaveMorph_3_DrawMode( void *pClass, int id, bool bOn ) 
{
    OSC_WaveMorph_3 *mymodule;
    mymodule = (OSC_WaveMorph_3*)pClass;
    mymodule->m_pEnvelope->m_bDraw = bOn;
}

//-----------------------------------------------------
// OSC_WaveMorph_3_Solo
//-----------------------------------------------------
void OSC_WaveMorph_3_Solo( void *pClass, int id, bool bOn ) 
{
    OSC_WaveMorph_3 *mymodule;
    mymodule = (OSC_WaveMorph_3*)pClass;
    mymodule->m_bSolo = bOn;
}

//-----------------------------------------------------
// Procedure:   OSC_WaveMorph_3_ChSelect
//-----------------------------------------------------
void OSC_WaveMorph_3_ChSelect( void *pClass, int id, int nbutton, bool bOn )
{
    OSC_WaveMorph_3 *mymodule;
    mymodule = (OSC_WaveMorph_3*)pClass;

    mymodule->ChangeChannel( nbutton );
}

//-----------------------------------------------------
// Procedure:   OSC_WaveMorph_3_WaveSet
//-----------------------------------------------------
void OSC_WaveMorph_3_WaveSet( void *pClass, int id, bool bOn )
{
    OSC_WaveMorph_3 *mymodule;
    mymodule = (OSC_WaveMorph_3*)pClass;

    if( id == 0 )
    {
        if( ++mymodule->m_waveSet >= EnvelopeData::nPRESETS )
            mymodule->m_waveSet = 0;
    }
    else
    {
        if( --mymodule->m_waveSet < 0 )
            mymodule->m_waveSet = EnvelopeData::nPRESETS - 1;
    }

    mymodule->m_pEnvelope->m_EnvData[ mymodule->m_CurrentChannel ].Preset( mymodule->m_waveSet );
}

//-----------------------------------------------------
// Procedure:   OSC_WaveMorph_3_WaveInvert
//-----------------------------------------------------
void OSC_WaveMorph_3_WaveInvert( void *pClass, int id, bool bOn )
{
    int i;
    OSC_WaveMorph_3 *mymodule;
    mymodule = (OSC_WaveMorph_3*)pClass;

    for( i = 0; i < ENVELOPE_HANDLES; i++ )
        mymodule->m_pEnvelope->setVal( mymodule->m_CurrentChannel, i, 1.0f - mymodule->m_pEnvelope->m_EnvData[ mymodule->m_CurrentChannel ].m_HandleVal[ i ] );
}

//-----------------------------------------------------
// Procedure:   OSC_WaveMorph_3_WaveRand
//-----------------------------------------------------
void OSC_WaveMorph_3_WaveRand( void *pClass, int id, bool bOn )
{
    int i;
    OSC_WaveMorph_3 *mymodule;
    mymodule = (OSC_WaveMorph_3*)pClass;

    for( i = 0; i < ENVELOPE_HANDLES; i++ )
        mymodule->m_pEnvelope->setVal( mymodule->m_CurrentChannel, i, randomUniform() );
}

//-----------------------------------------------------
// Procedure:   OSC_WaveMorph_3_WaveCopy
//-----------------------------------------------------
void OSC_WaveMorph_3_WaveCopy( void *pClass, int id, bool bOn )
{
    OSC_WaveMorph_3 *mymodule;
    mymodule = (OSC_WaveMorph_3*)pClass;

    mymodule->m_bCpy = bOn;
}

//-----------------------------------------------------
// Procedure:   Widget
//
//-----------------------------------------------------

struct OSC_WaveMorph_3_Widget : ModuleWidget {
	OSC_WaveMorph_3_Widget( OSC_WaveMorph_3 *module );
};

OSC_WaveMorph_3_Widget::OSC_WaveMorph_3_Widget( OSC_WaveMorph_3 *module ) : ModuleWidget(module) 
{
	box.size = Vec( 15*16, 380);

	{
		SVGPanel *panel = new SVGPanel();
		panel->box.size = box.size;
		panel->setBackground(SVG::load(assetPlugin(plugin, "res/OSC_WaveMorph_3.svg")));
		addChild(panel);
	}

    //module->lg.Open("OSC_WaveMorph_3.txt");

    // input V/OCT
    addInput(Port::create<MyPortInSmall>( Vec( 14, 20 ), Port::INPUT, module, OSC_WaveMorph_3::INPUT_VOCT ) );

    // input morph cv
    addInput(Port::create<MyPortInSmall>( Vec( 14, 311 ), Port::INPUT, module, OSC_WaveMorph_3::INPUT_MORPHCV ) );

    // invert
    module->m_pButtonInvert = new MyLEDButton( 88, 31, 11, 11, 8.0, DWRGB( 180, 180, 180 ), DWRGB( 0, 255, 255 ), MyLEDButton::TYPE_MOMENTARY, 0, module, OSC_WaveMorph_3_WaveInvert );
	addChild( module->m_pButtonInvert );

    // envelope editor
    module->m_pEnvelope = new Widget_EnvelopeEdit( 16, 47, 208, 96, 5, module, OSC_WaveMorph_3_EnvelopeEditCALLBACK, nCHANNELS );
	addChild( module->m_pEnvelope );

	module->m_pEnvelope->m_EnvData[ 0 ].m_Range = EnvelopeData::RANGE_Audio;
	module->m_pEnvelope->m_EnvData[ 1 ].m_Range = EnvelopeData::RANGE_Audio;
	module->m_pEnvelope->m_EnvData[ 2 ].m_Range = EnvelopeData::RANGE_Audio;

    // solo button
    module->m_pButtonSolo = new MyLEDButton( 158, 146, 11, 11, 8.0, DWRGB( 180, 180, 180 ), DWRGB( 0, 255, 0 ), MyLEDButton::TYPE_SWITCH, 0, module, OSC_WaveMorph_3_Solo );
    addChild( module->m_pButtonSolo );

    // wave change (when soloing) cv input
    addInput(Port::create<MyPortInSmall>( Vec( 131, 144 ), Port::INPUT, module, OSC_WaveMorph_3::IN_WAVE_CHANGE ) );

    // envelope select buttons
    module->m_pButtonChSelect = new MyLEDButtonStrip( 183, 146, 11, 11, 3, 8.0, nCHANNELS, false, DWRGB( 180, 180, 180 ), DWRGB( 0, 255, 255 ), MyLEDButtonStrip::TYPE_EXCLUSIVE, 0, module, OSC_WaveMorph_3_ChSelect );
    addChild( module->m_pButtonChSelect );

	module->m_pTextLabel = new Label();
	module->m_pTextLabel->box.pos = Vec( 150, 4 );
	module->m_pTextLabel->text = "----";
	addChild( module->m_pTextLabel );

    // wave set buttons
    module->m_pButtonWaveSetBck = new MyLEDButton( 122, 31, 11, 11, 8.0, DWRGB( 180, 180, 180 ), DWRGB( 0, 255, 255 ), MyLEDButton::TYPE_MOMENTARY, 0, module, OSC_WaveMorph_3_WaveSet );
	addChild( module->m_pButtonWaveSetBck );

    module->m_pButtonWaveSetFwd = new MyLEDButton( 134, 31, 11, 11, 8.0, DWRGB( 180, 180, 180 ), DWRGB( 0, 255, 255 ), MyLEDButton::TYPE_MOMENTARY, 1, module, OSC_WaveMorph_3_WaveSet );
	addChild( module->m_pButtonWaveSetFwd );

    // random
    module->m_pButtonRand = new MyLEDButton( 163, 31, 11, 11, 8.0, DWRGB( 180, 180, 180 ), DWRGB( 0, 255, 255 ), MyLEDButton::TYPE_MOMENTARY, 0, module, OSC_WaveMorph_3_WaveRand );
	addChild( module->m_pButtonRand );

    // copy
    module->m_pButtonCopy = new MyLEDButton( 188, 31, 11, 11, 8.0, DWRGB( 180, 180, 180 ), DWRGB( 0, 255, 255 ), MyLEDButton::TYPE_SWITCH, 0, module, OSC_WaveMorph_3_WaveCopy );
	addChild( module->m_pButtonCopy );

    // draw mode
    module->m_pButtonDraw = new MyLEDButton( 17, 145, 11, 11, 8.0, DWRGB( 180, 180, 180 ), DWRGB( 255, 128, 0 ), MyLEDButton::TYPE_SWITCH, 0, module, OSC_WaveMorph_3_DrawMode );
	addChild( module->m_pButtonDraw );

    // band knob
    addParam(ParamWidget::create<OSC_WaveMorph_3::Band_Knob>( Vec( 60 , 145 ), module, OSC_WaveMorph_3::PARAM_BAND, 0.0, 0.8, 0.333 ) );

    // filter
    addParam(ParamWidget::create<Knob_Green1_40>( Vec( 30, 200 ), module, OSC_WaveMorph_3::PARAM_CUTOFF, 0.0, 0.1, 0.0 ) );
    addParam(ParamWidget::create<FilterSelectToggle>( Vec( 73, 200 ), module, OSC_WaveMorph_3::PARAM_FILTER_MODE, 0.0, 4.0, 0.0 ) );
    addParam(ParamWidget::create<Knob_Purp1_20>( Vec( 76, 219 ), module, OSC_WaveMorph_3::PARAM_RES, 0.0, 1.0, 0.0 ) );

    // in cvs
    addInput(Port::create<MyPortInSmall>( Vec( 41, 244 ), Port::INPUT, module, OSC_WaveMorph_3::IN_FILTER ) );
    addInput(Port::create<MyPortInSmall>( Vec( 77, 244 ), Port::INPUT, module, OSC_WaveMorph_3::IN_REZ ) );
    addInput(Port::create<MyPortInSmall>( Vec( 162, 265 ), Port::INPUT, module, OSC_WaveMorph_3::IN_LEVEL ) );

    // level knob
    addParam(ParamWidget::create<Knob_Blue2_56>( Vec( 143 , 200 ), module, OSC_WaveMorph_3::PARAM_LEVEL, 0.0, 1.0, 0.0 ) );

    // audio out
    addOutput(Port::create<MyPortOutSmall>( Vec( 203, 218 ), Port::OUTPUT, module, OSC_WaveMorph_3::OUTPUT_AUDIO ) );

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
void OSC_WaveMorph_3::JsonParams( bool bTo, json_t *root) 
{
    JsonDataInt( bTo, "m_GraphData", root, (int*)m_GraphData, nCHANNELS * ENVELOPE_HANDLES );
    JsonDataBool( bTo, "m_bSolo", root, &m_bSolo, 1 );
}

//-----------------------------------------------------
// Procedure: toJson  
//
//-----------------------------------------------------
json_t *OSC_WaveMorph_3::toJson() 
{
	json_t *root = json_object();

    if( !root )
        return NULL;

    m_pEnvelope->getDataAll( (int*)m_GraphData );

    JsonParams( TOJSON, root );
    
	return root;
}

//-----------------------------------------------------
// Procedure:   fromJson
//
//-----------------------------------------------------
void OSC_WaveMorph_3::fromJson( json_t *root ) 
{
    JsonParams( FROMJSON, root );

    m_pEnvelope->setDataAll( (int*)m_GraphData );

    m_pButtonSolo->Set( m_bSolo );

    ChangeChannel( 0 );
}

//-----------------------------------------------------
// Procedure:   onCreate
//
//-----------------------------------------------------
void OSC_WaveMorph_3::onCreate()
{
}

//-----------------------------------------------------
// Procedure:   onDelete
//
//-----------------------------------------------------
void OSC_WaveMorph_3::onDelete()
{
}

//-----------------------------------------------------
// Procedure:   onReset
//
//-----------------------------------------------------
void OSC_WaveMorph_3::onReset()
{
    memset( m_GraphData, 0, sizeof( m_GraphData ) );

    m_pEnvelope->setDataAll( (int*)m_GraphData );

    ChangeChannel( 0 );
}

//-----------------------------------------------------
// Procedure:   onRandomize
//
//-----------------------------------------------------
void OSC_WaveMorph_3::onRandomize()
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
void OSC_WaveMorph_3::ChangeChannel( int ch )
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
            m_pEnvelope->setVal( ch, i, m_pEnvelope->m_EnvData[ m_CurrentChannel ].m_HandleVal[ i ] );
        }
    }

    m_CurrentChannel = ch;
    m_pButtonChSelect->Set( ch, true );
    m_pEnvelope->setView( ch );
}

//-----------------------------------------------------
// Procedure:   ChangeFilterCutoff
//
//-----------------------------------------------------
void OSC_WaveMorph_3::ChangeFilterCutoff( float cutfreq )
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

    f = 2.0 * (fx 
	    - (fx3 * 0.16666666666666666666666666666667) 
	    + (fx5 * 0.0083333333333333333333333333333333) 
	    - (fx7 * 0.0001984126984126984126984126984127));
}

//-----------------------------------------------------
// Procedure:   Filter
//
//-----------------------------------------------------
#define MULTI (0.33333333333333333333333333333333f)
void OSC_WaveMorph_3::Filter( float *In )
{
    float rez, hp1; 
    float input, out = 0.0f, lowpass, bandpass, highpass;

    if( (int)params[ PARAM_FILTER_MODE ].value == 0 )
        return;

    rez = 1.0 - clamp( params[ PARAM_RES ].value * ( inputs[ IN_REZ ].normalize( CV_MAX ) / CV_MAX ), 0.0f, 1.0f );

    input = *In / AUDIO_MAX;

    input  = input + 0.000000001;

    lp1         = lp1 + f * bp1; 
    hp1         = input - lp1 - rez * bp1; 
    bp1         = f * hp1 + bp1; 
    lowpass     = lp1; 
    highpass    = hp1; 
    bandpass    = bp1; 

    lp1         = lp1 + f * bp1; 
    hp1         = input - lp1 - rez * bp1; 
    bp1         = f * hp1 + bp1; 
    lowpass     = lowpass  + lp1; 
    highpass    = highpass + hp1; 
    bandpass    = bandpass + bp1; 

    input  = input - 0.000000001;

    lp1         = lp1 + f * bp1; 
    hp1         = input - lp1 - rez * bp1; 
    bp1         = f * hp1 + bp1; 

    lowpass  = (lowpass  + lp1) * MULTI; 
    highpass = (highpass + hp1) * MULTI; 
    bandpass = (bandpass + bp1) * MULTI;

    switch( (int)params[ PARAM_FILTER_MODE ].value )
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
    }

    *In = out * AUDIO_MAX;
}

//-----------------------------------------------------
// Procedure:   step
//
//-----------------------------------------------------
void OSC_WaveMorph_3::step() 
{
    int ch;
    float fout = 0.0f, fmorph[ nCHANNELS ] = {}, fcv, cutoff, flevel;
    bool bChangeWave = false;

    if( !m_bInitialized )
        return;

    fcv = clamp( inputs[ INPUT_MORPHCV ].normalize( 0.0f ) / CV_MAX, -1.0f, 1.0f );

    fmorph[ 1 ] = 1.0 - fabs( fcv );

    // left wave
    if( fcv <= 0.0f )
        fmorph[ 0 ] = -fcv;
    else if( fcv > 0.0f )
        fmorph[ 2 ] = fcv;

    bChangeWave = ( m_SchmittChangeWave.process( inputs[ IN_WAVE_CHANGE ].normalize( 0.0f ) ) );

    if( bChangeWave && m_bSolo )
    {
        ch = m_CurrentChannel + 1;

        if( ch >= nCHANNELS )
            ch = 0;

        ChangeChannel( ch );
    }

    // process each channel
    for( ch = 0; ch < nCHANNELS; ch++ )
    {
        m_pEnvelope->m_EnvData[ ch ].m_Clock.syncInc = 32.7032f * clamp( powf( 2.0f, clamp( inputs[ INPUT_VOCT ].normalize( 3.0f ), 0.0f, VOCT_MAX ) ), 0.0f, 4186.01f );

        if( m_bSolo && ch == m_CurrentChannel )
            fout += m_pEnvelope->procStep( ch, false, false );
        else if( !m_bSolo )
            fout += m_pEnvelope->procStep( ch, false, false ) * fmorph[ ch ];
    }

    cutoff = clamp( params[ PARAM_CUTOFF ].value * ( inputs[ IN_FILTER ].normalize( CV_MAX ) / CV_MAX ), 0.0f, 1.0f );
    ChangeFilterCutoff( cutoff );

    flevel = clamp( params[ PARAM_LEVEL ].value * ( inputs[ IN_LEVEL ].normalize( CV_MAX ) / CV_MAX ), 0.0f, 1.0f );
    fout *= flevel;
    Filter( &fout );

    outputs[ OUTPUT_AUDIO ].value = clamp( fout, -AUDIO_MAX, AUDIO_MAX );
}

} // namespace rack_plugin_mscHack

using namespace rack_plugin_mscHack;

RACK_PLUGIN_MODEL_INIT(mscHack, OSC_WaveMorph_3) {
   Model *modelOSC_WaveMorph_3 = Model::create<OSC_WaveMorph_3, OSC_WaveMorph_3_Widget>( "mscHack", "OSC_WaveMorph_3", "OSC Wavemorph3", OSCILLATOR_TAG, MULTIPLE_TAG );
   return modelOSC_WaveMorph_3;
}
