#include "mscHack.hpp"
//#include "mscHack_Controls.hpp"
#include "dsp/digital.hpp"
//#include "CLog.h"

namespace rack_plugin_mscHack {

//-----------------------------------------------------
// Module Definition
//
//-----------------------------------------------------
struct Maude_221 : Module 
{
	enum ParamIds 
    {
        PARAM_LIMIT_INA,
        PARAM_LIMIT_INB,
        PARAM_AMP_OUTC,
        PARAM_MODE,
        PARAM_DCOFF,
        PARAM_CVAMTA,
        PARAM_CVAMTB,
        nPARAMS
    };

	enum InputIds 
    {
        INPUTA,
        INPUTB,
        INPUTCVA,
        INPUTCVB,
        nINPUTS 
	};

	enum OutputIds 
    {
        OUTPUTC,
        nOUTPUTS
	};

    bool            m_bInitialized = false;
    CLog            lg;

    // Contructor
	Maude_221() : Module(nPARAMS, nINPUTS, nOUTPUTS){}

    MyLEDButtonStrip    *m_pInputA_RectMode = NULL;
    MyLEDButtonStrip    *m_pInputB_RectMode = NULL;
    MyLEDButtonStrip    *m_pOutputC_RectMode = NULL;

    int                 m_RectMode[ 3 ] = {0};

    // Overrides 
	void    step() override;
    void    JsonParams( bool bTo, json_t *root);
    json_t* toJson() override;
    void    fromJson(json_t *rootJ) override;
    void    onRandomize() override;
    void    onReset() override;
};

//-----------------------------------------------------
// Procedure:   Maude_221_RectSelect
//-----------------------------------------------------
void Maude_221_RectSelect( void *pClass, int id, int nbutton, bool bOn )
{
    Maude_221 *mymodule;
    mymodule = (Maude_221*)pClass;

    if( !mymodule || !mymodule->m_bInitialized )
        return;

    mymodule->m_RectMode[ id ] = nbutton;
}

//-----------------------------------------------------
// Procedure:   Widget
//
//-----------------------------------------------------

struct Maude_221_Widget : ModuleWidget {
	Maude_221_Widget( Maude_221 *module );
};

Maude_221_Widget::Maude_221_Widget( Maude_221 *module ) : ModuleWidget(module) 
{
	box.size = Vec( 15*8, 380);

	{
		SVGPanel *panel = new SVGPanel();
		panel->box.size = box.size;
		panel->setBackground(SVG::load(assetPlugin(plugin, "res/Maude221.svg")));
		addChild(panel);
	}

    //module->lg.Open("Maude_221.txt");

    addInput(Port::create<MyPortInSmall>( Vec( 22, 48 ), Port::INPUT, module, Maude_221::INPUTA ) );
    addInput(Port::create<MyPortInSmall>( Vec( 79, 48 ), Port::INPUT, module, Maude_221::INPUTB ) );

    // rectify mode
    module->m_pInputA_RectMode = new MyLEDButtonStrip( 15, 81, 12, 12, 0, 11.5, 3, false, DWRGB( 0, 0, 0 ), DWRGB( 180, 180, 180 ), MyLEDButtonStrip::TYPE_EXCLUSIVE, 0, module, Maude_221_RectSelect );
    addChild( module->m_pInputA_RectMode );

    module->m_pInputB_RectMode = new MyLEDButtonStrip( 71, 81, 12, 12, 0, 11.5, 3, false, DWRGB( 0, 0, 0 ), DWRGB( 180, 180, 180 ), MyLEDButtonStrip::TYPE_EXCLUSIVE, 1, module, Maude_221_RectSelect );
    addChild( module->m_pInputB_RectMode );

    // limit knobs
    addParam(ParamWidget::create<Knob_Green1_40>( Vec( 12, 106 ), module, Maude_221::PARAM_LIMIT_INA, 1.0, 10.0, 10.0 ) );
    addParam(ParamWidget::create<Knob_Green1_40>( Vec( 67, 106 ), module, Maude_221::PARAM_LIMIT_INB, 1.0, 10.0, 10.0 ) );

    // CV amount knobs
    addParam(ParamWidget::create<Knob_Green1_15>( Vec( 8, 152 ), module, Maude_221::PARAM_CVAMTA, 0.0, 1.0, 0.0 ) );
    addParam(ParamWidget::create<Knob_Green1_15>( Vec( 94, 152 ), module, Maude_221::PARAM_CVAMTB, 0.0, 1.0, 0.0 ) );

    // limit CV inputs
    addInput(Port::create<MyPortInSmall>( Vec( 6, 173 ), Port::INPUT, module, Maude_221::INPUTCVA ) );
    addInput(Port::create<MyPortInSmall>( Vec( 93, 173 ), Port::INPUT, module, Maude_221::INPUTCVB ) );

    // mode select knob
    addParam(ParamWidget::create<Knob_Blue2_26_Snap>( Vec( 47, 188 ), module, Maude_221::PARAM_MODE, 0.0, 4.0, 0.0 ) );

    // output rectify mode select
    module->m_pOutputC_RectMode = new MyLEDButtonStrip( 32, 248, 12, 12, 0, 11.5, 5, false, DWRGB( 0, 0, 0 ), DWRGB( 180, 180, 180 ), MyLEDButtonStrip::TYPE_EXCLUSIVE, 2, module, Maude_221_RectSelect );
    addChild( module->m_pOutputC_RectMode );

    // output amp
    addParam(ParamWidget::create<Knob_Green1_40>( Vec( 40, 273 ), module, Maude_221::PARAM_AMP_OUTC, 0.0, 2.0, 1.0 ) );

    // DC Offset
    addParam(ParamWidget::create<Knob_Green1_15>( Vec( 80, 315 ), module, Maude_221::PARAM_DCOFF, -5.0, 5.0, 0.0 ) );

    // output
    addOutput(Port::create<MyPortOutSmall>( Vec( 50, 344 ), Port::OUTPUT, module, Maude_221::OUTPUTC ) );

	addChild(Widget::create<ScrewSilver>(Vec(15, 0)));
	addChild(Widget::create<ScrewSilver>(Vec(box.size.x-30, 0)));
	addChild(Widget::create<ScrewSilver>(Vec(15, 365))); 
	addChild(Widget::create<ScrewSilver>(Vec(box.size.x-30, 365)));

    module->m_pInputA_RectMode->Set( 1, true );
    module->m_pInputB_RectMode->Set( 1, true );
    module->m_pOutputC_RectMode->Set( 1, true );
    module->m_RectMode[ 0 ] = 1;
    module->m_RectMode[ 1 ] = 1;
    module->m_RectMode[ 2 ] = 1;

    module->m_bInitialized = true;
}

//-----------------------------------------------------
// Procedure: JsonParams  
//
//-----------------------------------------------------
void Maude_221::JsonParams( bool bTo, json_t *root) 
{
    JsonDataInt( bTo, "RectModes", root, m_RectMode, 3 );
}

//-----------------------------------------------------
// Procedure: toJson  
//
//-----------------------------------------------------
json_t *Maude_221::toJson() 
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
void Maude_221::fromJson( json_t *root ) 
{
    JsonParams( FROMJSON, root );

    m_pInputA_RectMode->Set( m_RectMode[ 0 ], true );
    m_pInputB_RectMode->Set( m_RectMode[ 1 ], true );
    m_pOutputC_RectMode->Set( m_RectMode[ 2 ], true );
}
//-----------------------------------------------------
// Procedure:   reset
//
//-----------------------------------------------------
void Maude_221::onReset()
{
}

//-----------------------------------------------------
// Procedure:   randomize
//
//-----------------------------------------------------
void Maude_221::onRandomize()
{
}

//-----------------------------------------------------
// Procedure:   step
//
//-----------------------------------------------------
void Maude_221::step() 
{
    float inA, inB, out = 0.0f, lima, limb;

    if( !m_bInitialized )
        return;

    if( !inputs[ INPUTA ].active && !inputs[ INPUTB ].active )
    {
        outputs[ OUTPUTC ].value = 0.0f;
        return;
    }

    lima = clamp( params[ PARAM_LIMIT_INA ].value + ( inputs[ INPUTCVA ].normalize( 0.0f ) * params[ PARAM_CVAMTA ].value ), -10.0, 10.0 );
    limb = clamp( params[ PARAM_LIMIT_INB ].value + ( inputs[ INPUTCVB ].normalize( 0.0f ) * params[ PARAM_CVAMTB ].value ), -10.0, 10.0 );

    inA = clamp( inputs[ INPUTA ].normalize( 0.0f ), -lima, lima );
    inB = clamp( inputs[ INPUTB ].normalize( 0.0f ), -limb, limb );

    switch( m_RectMode[ 0 ] )
    {
    case 0: // negative only
        if( inA > 0.0f )
            inA = 0.0f;
        break;
    case 1: // bipolar
        break;
    case 2: // positive only
        if( inA < 0.0f )
            inA = 0.0f;
        break;
    }

    switch( m_RectMode[ 1 ] )
    {
    case 0: // negative only
        if( inB > 0.0f )
            inB = 0.0f;
        break;
    case 1: // bipolar
        break;
    case 2: // positive only
        if( inB < 0.0f )
            inB = 0.0f;
        break;
    }

    // only do math if both inputs are active
    if( inputs[ INPUTA ].active && inputs[ INPUTB ].active )
    {
        switch( (int)( params[ PARAM_MODE ].value ) )
        {
        case 0: // add
            out = inA + inB;
            break;
        case 1: // subtract
            out = inA - inB;
            break;
        case 2: // multiply
            out = sin(inA) * cos(inB);
            break;
        case 3: // divide
            if( fabs( inA ) < fabs( inB ) )
                out = inA;
            else
                out = inB;

            break;

        case 4: // divide
            if( fabs( inA ) > fabs( inB ) )
                out = inA;
            else
                out = inB;

            break;
        }
    }
    else if( inputs[ INPUTA ].active )
    {
        out = inA;
    }
    else
    {
        out = inB;
    }

     switch( m_RectMode[ 2 ] )
    {
    case 0: // negative half wave
        if( out > 0.0f )
            out = 0.0f;
        break;
    case 1: // negative full wave
        if( out > 0.0f )
            out = -out;
        break;
    case 2: // bipolar
        break;
    case 3: // positive full wave
        if( out < 0.0f )
            out = -out;
        break;
    case 4: // positive half wave
        if( out < 0.0f )
            out = 0.0f;
        break;
    }

    out = ( out * params[ PARAM_AMP_OUTC ].value ) + params[ PARAM_DCOFF ].value;

    outputs[ OUTPUTC ].value = out;
}

} // namespace rack_plugin_mscHack

using namespace rack_plugin_mscHack;

RACK_PLUGIN_MODEL_INIT(mscHack, Maude_221) {
   Model *modelMaude_221 = Model::create<Maude_221, Maude_221_Widget>( "mscHack", "Maude221", "Maude 221 Wave Combiner", WAVESHAPER_TAG, UTILITY_TAG, MULTIPLE_TAG );
   return modelMaude_221;
}
