#include "mscHack.hpp"
//#include "mscHack_Controls.hpp"
#include "dsp/digital.hpp"
//#include "CLog.h"

namespace rack_plugin_mscHack {

typedef struct
{
    int             state;
    float           finc;
    int             count;
    float           fade;
}COMP_STATE;

//-----------------------------------------------------
// Module Definition
//
//-----------------------------------------------------
struct Compressor : Module 
{
	enum ParamIds 
    {
        PARAM_INGAIN, 
        PARAM_OUTGAIN,
        PARAM_THRESHOLD,
        PARAM_RATIO,
        PARAM_ATTACK,
        PARAM_RELEASE,
        PARAM_BYPASS,
        PARAM_SIDE_CHAIN,
        nPARAMS
    };

	enum InputIds 
    {
        IN_AUDIOL,
        IN_AUDIOR,
        IN_SIDE_CHAIN,
        nINPUTS 
	};

	enum OutputIds 
    {
        OUT_AUDIOL,
        OUT_AUDIOR,
        nOUTPUTS
	};

    enum CompState
    {
        COMP_DONE,
        COMP_START,
        COMP_ATTACK,
        COMP_RELEASE,
        COMP_IDLE
    };

    bool            m_bInitialized = false;
    CLog            lg;

    LEDMeterWidget  *m_pLEDMeterIn[ 2 ] = {0};
    CompressorLEDMeterWidget  *m_pLEDMeterThreshold = NULL;
    CompressorLEDMeterWidget  *m_pLEDMeterComp[ 2 ] = {0};
    LEDMeterWidget  *m_pLEDMeterOut[ 2 ] = {0};

    bool            m_bBypass = false;
    MyLEDButton     *m_pButtonBypass = NULL;

    COMP_STATE      m_CompL = {};
    COMP_STATE      m_CompR = {};
    float           m_fThreshold;

    // Contructor
	Compressor() : Module(nPARAMS, nINPUTS, nOUTPUTS, 0 ){}

    // Overrides 
	void    step() override;
    json_t* toJson() override;
    void    fromJson(json_t *rootJ) override;
    void    onReset() override;
    void    onRandomize() override;

    bool    ProcessCompState( COMP_STATE *pComp, bool bAboveThreshold );
    float   Compress( float *pDetectInL, float *pDetectInR );
};

//-----------------------------------------------------
// Compressor_Bypass
//-----------------------------------------------------
void Compressor_Bypass( void *pClass, int id, bool bOn ) 
{
    Compressor *mymodule;
    mymodule = (Compressor*)pClass;
    mymodule->m_bBypass = bOn;
}

//-----------------------------------------------------
// Procedure:   Widget
//
//-----------------------------------------------------

struct Compressor_Widget : ModuleWidget {
	Compressor_Widget( Compressor *module );
};

Compressor_Widget::Compressor_Widget( Compressor *module ) : ModuleWidget(module) 
{
    int x, y, y2;

	box.size = Vec( 15*8, 380);

	{
		SVGPanel *panel = new SVGPanel();
		panel->box.size = box.size;
		panel->setBackground(SVG::load(assetPlugin(plugin, "res/Compressor.svg")));
		addChild(panel);
	}

    //module->lg.Open("Compressor.txt");
    x = 10;
    y = 34;

	addChild(Widget::create<ScrewSilver>(Vec(15, 0)));
	addChild(Widget::create<ScrewSilver>(Vec(box.size.x-30, 0)));
	addChild(Widget::create<ScrewSilver>(Vec(15, 365))); 
	addChild(Widget::create<ScrewSilver>(Vec(box.size.x-30, 365)));

    // bypass switch
    module->m_pButtonBypass = new MyLEDButton( x, y, 11, 11, 8.0, DWRGB( 180, 180, 180 ), DWRGB( 255, 0, 0 ), MyLEDButton::TYPE_SWITCH, 0, module, Compressor_Bypass );
	addChild( module->m_pButtonBypass );

    // audio inputs
    addInput(Port::create<MyPortInSmall>( Vec( x, y + 32 ), Port::INPUT, module, Compressor::IN_AUDIOL ) );
    addInput(Port::create<MyPortInSmall>( Vec( x, y + 78 ), Port::INPUT, module, Compressor::IN_AUDIOR ) );
    addInput(Port::create<MyPortInSmall>( Vec( x - 1, y + 210 ), Port::INPUT, module, Compressor::IN_SIDE_CHAIN ) );

    // LED meters
    module->m_pLEDMeterIn[ 0 ] = new LEDMeterWidget( x + 22, y + 25, 5, 3, 2, true );
    addChild( module->m_pLEDMeterIn[ 0 ] );
    module->m_pLEDMeterIn[ 1 ] = new LEDMeterWidget( x + 28, y + 25, 5, 3, 2, true );
    addChild( module->m_pLEDMeterIn[ 1 ] );

    module->m_pLEDMeterThreshold = new CompressorLEDMeterWidget( true, x + 39, y + 25, 5, 3, DWRGB( 245, 10, 174 ), DWRGB( 96, 4, 68 ) );
    addChild( module->m_pLEDMeterThreshold );

    module->m_pLEDMeterComp[ 0 ] = new CompressorLEDMeterWidget( true, x + 48, y + 25, 5, 3, DWRGB( 0, 128, 255 ), DWRGB( 0, 64, 128 ) );
    addChild( module->m_pLEDMeterComp[ 0 ] );
    module->m_pLEDMeterComp[ 1 ] = new CompressorLEDMeterWidget( true, x + 55, y + 25, 5, 3, DWRGB( 0, 128, 255 ), DWRGB( 0, 64, 128 ) );
    addChild( module->m_pLEDMeterComp[ 1 ] );

    module->m_pLEDMeterOut[ 0 ] = new LEDMeterWidget( x + 65, y + 25, 5, 3, 2, true );
    addChild( module->m_pLEDMeterOut[ 0 ] );
    module->m_pLEDMeterOut[ 1 ] = new LEDMeterWidget( x + 72, y + 25, 5, 3, 2, true );
    addChild( module->m_pLEDMeterOut[ 1 ] );

    // audio  outputs
    addOutput(Port::create<MyPortOutSmall>( Vec( x + 83, y + 32 ), Port::OUTPUT, module, Compressor::OUT_AUDIOL ) );
    addOutput(Port::create<MyPortOutSmall>( Vec( x + 83, y + 78 ), Port::OUTPUT, module, Compressor::OUT_AUDIOR ) );

    // add param knobs
    y2 = y + 149;
    addParam(ParamWidget::create<Knob_Yellow1_26>( Vec( x + 11, y + 113 ), module, Compressor::PARAM_INGAIN, 0.0, 4.0, 1.0 ) );
    addParam(ParamWidget::create<Knob_Yellow1_26>( Vec( x + 62, y + 113 ), module, Compressor::PARAM_OUTGAIN, 0.0, 8.0, 1.0 ) );
    addParam(ParamWidget::create<Knob_Blue2_26>( Vec( x - 5, y2 + 20 ), module, Compressor::PARAM_SIDE_CHAIN, 0.0, 1.0, 0.0 ) );
    addParam(ParamWidget::create<Knob_Yellow1_26>( Vec( x + 39, y2 ), module, Compressor::PARAM_THRESHOLD, 0.0, 0.99, 0.0 ) ); y2 += 40;
    addParam(ParamWidget::create<Knob_Yellow1_26>( Vec( x + 39, y2 ), module, Compressor::PARAM_RATIO, 0.0, 2.0, 0.0 ) ); y2 += 40;
    addParam(ParamWidget::create<Knob_Yellow1_26>( Vec( x + 39, y2 ), module, Compressor::PARAM_ATTACK, 0.0, 1.0, 0.0 ) ); y2 += 40;
    addParam(ParamWidget::create<Knob_Yellow1_26>( Vec( x + 39, y2 ), module, Compressor::PARAM_RELEASE, 0.0, 1.0, 0.0 ) );

    //for( int i = 0; i < 15; i++ )
        //module->lg.f("level %d = %.3f\n", i, module->m_pLEDMeterThreshold->flevels[ i ] );

    module->m_bInitialized = true;
}

//-----------------------------------------------------
// Procedure:   
//
//-----------------------------------------------------
json_t *Compressor::toJson() 
{
	json_t *rootJ = json_object();

    // reverse state
    json_object_set_new(rootJ, "m_bBypass", json_boolean (m_bBypass));

	return rootJ;
}

//-----------------------------------------------------
// Procedure:   fromJson
//
//-----------------------------------------------------
void Compressor::fromJson(json_t *rootJ) 
{
	// reverse state
	json_t *revJ = json_object_get(rootJ, "m_bBypass");

	if (revJ)
		m_bBypass = json_is_true( revJ );

    m_pButtonBypass->Set( m_bBypass );
}

//-----------------------------------------------------
// Procedure:   reset
//
//-----------------------------------------------------
void Compressor::onReset()
{
}

//-----------------------------------------------------
// Procedure:   randomize
//
//-----------------------------------------------------
void Compressor::onRandomize()
{
}

//-----------------------------------------------------
// Procedure:   ProcessCompStatus
//
//-----------------------------------------------------
#define MAX_ATT_TIME (0.5f) // 500ms
#define MAX_REL_TIME (2.0f) // 2s
bool Compressor::ProcessCompState( COMP_STATE *pComp, bool bAboveThreshold )
{
    bool bCompressing = true;

    // restart compressor if it has finished
    if( bAboveThreshold && ( pComp->state == COMP_IDLE ) )
    {
        pComp->state = COMP_START;
    }
    // ready compressor for restart
    else if( !bAboveThreshold && ( pComp->state == COMP_DONE ) )
    {
        pComp->state = COMP_IDLE;
    }

    switch( pComp->state )
    {
    case COMP_START:
        pComp->count = 10 + (int)( MAX_ATT_TIME * engineGetSampleRate() * params[ PARAM_ATTACK ].value );

        pComp->state = COMP_ATTACK;
        pComp->finc = (1.0f - pComp->fade) / (float)pComp->count;
            
        break;

    case COMP_ATTACK:
        if( --pComp->count > 0 )
        {
            pComp->fade += pComp->finc;

            if( pComp->fade > 1.0f )
                pComp->fade = 1.0f;
        }
        else
        {
            pComp->count = 10 + (int)( MAX_REL_TIME * engineGetSampleRate() * params[ PARAM_RELEASE ].value );
            pComp->fade = 1.0f;
            pComp->finc = 1.0f / (float)pComp->count;
            pComp->state = COMP_RELEASE;
        }

        break;

    case COMP_RELEASE:
        if( --pComp->count > 0 )
        {
            pComp->fade -= pComp->finc;

            if( pComp->fade < 0.0f )
                pComp->fade = 0.0f;
        }
        else
        {
            pComp->fade = 0.0f;
            pComp->state = COMP_DONE;
            bCompressing = false;
        }
        break;

    case COMP_DONE:
        pComp->fade = 0.0f;
        bCompressing = false;
        break;

    case COMP_IDLE:
        pComp->fade = 0.0f;
        bCompressing = false;
        break;
    }

    return bCompressing;
}

//-----------------------------------------------------
// Procedure:   Compress
//
//-----------------------------------------------------
float Compressor::Compress( float *pDetectInL, float *pDetectInR )
{
    float rat, th, finL, finR, compL = 1.0f, compR = 1.0f;

    m_fThreshold = params[ PARAM_THRESHOLD ].value;
    th = 1.0f - m_fThreshold;
    rat = params[ PARAM_RATIO ].value;

    finL = fabs( *pDetectInL );

    if( ProcessCompState( &m_CompL, ( finL > th ) ) )
        compL = 1.0f - ( rat * m_CompL.fade );

    if( pDetectInR )
    {
        finR = fabs( *pDetectInR );

        if( ProcessCompState( &m_CompR, ( finR > th ) ) )
            compR = 1.0f - ( rat * m_CompR.fade );
    }
    else
    {
        m_CompR.state = COMP_IDLE;
        m_CompR.fade = 0.0;
    }

    return fmin( compL, compR );
}

//-----------------------------------------------------
// Procedure:   step
//
//-----------------------------------------------------
void Compressor::step() 
{
    float outL, outR, diffL, diffR, fcomp, fside;

    if( !m_bInitialized )
        return;

    outL = inputs[ IN_AUDIOL ].normalize( 0.0 ) / AUDIO_MAX;
    outR = inputs[ IN_AUDIOR ].normalize( 0.0 ) / AUDIO_MAX;

    if( !m_bBypass )
    {
        outL = clamp( outL * params[ PARAM_INGAIN ].value, -1.0f, 1.0f );
        outR = clamp( outR * params[ PARAM_INGAIN ].value, -1.0f, 1.0f );
    }

    if( m_pLEDMeterIn[ 0 ] )
        m_pLEDMeterIn[ 0 ]->Process( outL );

    if( m_pLEDMeterIn[ 1 ] )
       m_pLEDMeterIn[ 1 ]->Process( outR );

    diffL = fabs( outL );
    diffR = fabs( outR );

    if( !m_bBypass )
    {
        // compress
        if( inputs[ IN_SIDE_CHAIN ].active )
        {
            fside = clamp( ( inputs[ IN_SIDE_CHAIN ].normalize( 0.0 ) / AUDIO_MAX ) * params[ PARAM_SIDE_CHAIN ].value, -1.0f, 1.0f );
            fcomp = Compress( &fside, NULL );
        }
        else
        {
            fcomp = Compress( &outL, &outR );
        }

        outL *= fcomp;
        outR *= fcomp;

        diffL -= fabs( outL );
        diffR -= fabs( outR );

        if( m_pLEDMeterComp[ 0 ] )
            m_pLEDMeterComp[ 0 ]->Process( diffL );

        if( m_pLEDMeterComp[ 1 ] )
            m_pLEDMeterComp[ 1 ]->Process( diffR );

        if( m_pLEDMeterThreshold )
            m_pLEDMeterThreshold->Process( m_fThreshold );

        outL = clamp( outL * params[ PARAM_OUTGAIN ].value, -1.0f, 1.0f );
        outR = clamp( outR * params[ PARAM_OUTGAIN ].value, -1.0f, 1.0f );
    }
    else
    {
        if( m_pLEDMeterComp[ 0 ] )
            m_pLEDMeterComp[ 0 ]->Process( 0 );

        if( m_pLEDMeterComp[ 1 ] )
            m_pLEDMeterComp[ 1 ]->Process( 0 );

        if( m_pLEDMeterThreshold )
           m_pLEDMeterThreshold->Process( 0 );
    }

    if( m_pLEDMeterOut[ 0 ] )
        m_pLEDMeterOut[ 0 ]->Process( outL );

    if( m_pLEDMeterOut[ 1 ] )
        m_pLEDMeterOut[ 1 ]->Process( outR );

    outputs[ OUT_AUDIOL ].value = outL * AUDIO_MAX;
    outputs[ OUT_AUDIOR ].value = outR * AUDIO_MAX;
}

} // namespace rack_plugin_mscHack

using namespace rack_plugin_mscHack;

RACK_PLUGIN_MODEL_INIT(mscHack, Compressor) {
   Model *modelCompressor = Model::create<Compressor, Compressor_Widget>( "mscHack", "Compressor1", "COMP Basic Compressor", DYNAMICS_TAG );
   return modelCompressor;
}
