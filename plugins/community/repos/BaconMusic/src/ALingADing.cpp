#include "BaconPlugs.hpp"

/*
** Based heavily on http://recherche.ircam.fr/pub/dafx11/Papers/66_e.pdf 
*/

namespace rack_plugin_BaconMusic {

struct ALingADing : Module {
  enum ParamIds {
    WET_DRY_MIX, // TODO: Implement this
    
    NUM_PARAMS
  };

  enum InputIds {
    SIGNAL_INPUT,
    CARRIER_INPUT,

    NUM_INPUTS
  };

  enum OutputIds {
    MODULATED_OUTPUT,
    NUM_OUTPUTS
  };

  enum LightIds {
    NUM_LIGHTS
  };

  ALingADing() : Module( NUM_PARAMS, NUM_INPUTS, NUM_OUTPUTS, NUM_LIGHTS ) {
    params[ WET_DRY_MIX ].value = 1.0;
  }


  inline float diode_sim( float in )
  {
    if( in < 0 ) return 0;
    else return 0.2 * log( 1.0 + exp( 10 * ( in - 1 ) ) );
  }
  
  void step() override
  {
    float vin = inputs[ SIGNAL_INPUT ].value;
    float vc  = inputs[ CARRIER_INPUT ].value;
    float wd  = params[ WET_DRY_MIX ].value;

    float A = 0.5 * vin + vc;
    float B = vc - 0.5 * vin;

    float dPA = diode_sim( A );
    float dMA = diode_sim( -A );
    float dPB = diode_sim( B );
    float dMB = diode_sim( -B );

    float res = dPA + dMA - dPB - dMB;
    outputs[ MODULATED_OUTPUT ].value = wd * res + ( 1.0 - wd ) * vin;
  }
};

struct ALingADingWidget : ModuleWidget {
  ALingADingWidget(ALingADing *module);
};


ALingADingWidget::ALingADingWidget(ALingADing *module) : ModuleWidget( module )
{
  box.size = Vec( SCREW_WIDTH * 5, RACK_HEIGHT );

  BaconBackground *bg = new BaconBackground( box.size, "ALingADing" );
  
  addChild( bg->wrappedInFramebuffer() );
  
  bg->addPlugLabel( Vec( 7, 70 ), BaconBackground::SIG_IN, "sig" );
  addInput( Port::create< PJ301MPort >( Vec( 7, 70 ),
                                        Port::INPUT,
                                        module,
                                        ALingADing::SIGNAL_INPUT ) );
  
  bg->addPlugLabel( Vec( box.size.x-24-7, 70 ), BaconBackground::SIG_IN, "car" );
  
  addInput( Port::create< PJ301MPort >( Vec( box.size.x-24 - 7, 70 ), // That 24 makes no sense but hey
                                        Port::INPUT,
                                        module,
                                        ALingADing::CARRIER_INPUT ) );
  
  bg->addLabel( Vec( bg->cx(), 140 ), "Mix", 14, NVG_ALIGN_CENTER | NVG_ALIGN_MIDDLE );
  
  bg->addLabel( Vec( bg->cx() + 10, 140 + 72 ),
                "Wet", 13, NVG_ALIGN_LEFT | NVG_ALIGN_TOP );
  bg->addLabel( Vec( bg->cx() - 10, 140 + 72 ),
                "Dry", 13, NVG_ALIGN_RIGHT | NVG_ALIGN_TOP );

  addParam( ParamWidget::create< RoundHugeBlackKnob >( Vec( bg->cx( 56 ), 150 ),
                                                module,
                                                ALingADing::WET_DRY_MIX,
                                                0, 1, 1 ));

  Vec outP = Vec( bg->cx( 24 ), RACK_HEIGHT - 15 - 43 );
  bg->addPlugLabel( outP, BaconBackground::SIG_OUT, "out" );
  addOutput( Port::create< PJ301MPort >( outP,
                                         Port::OUTPUT,
                                         module,
                                         ALingADing::MODULATED_OUTPUT ) );
}

} // namespace rack_plugin_BaconMusic

using namespace rack_plugin_BaconMusic;

RACK_PLUGIN_MODEL_INIT(BaconMusic, ALingADing) {
   Model *modelALingADing = Model::create<ALingADing,ALingADingWidget>("Bacon Music", "ALingADing", "ALingADing", RING_MODULATOR_TAG); 
   return modelALingADing;
}
