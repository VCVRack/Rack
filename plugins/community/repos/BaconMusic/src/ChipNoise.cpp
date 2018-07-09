#include "BaconPlugs.hpp"
#include "ChipSym.hpp"

namespace rack_plugin_BaconMusic {

struct ChipNoise : virtual Module {
  enum ParamIds {
    NOISE_LENGTH,
    LONG_MODE,
    SHORT_LEN,
    PERIOD_93,
    NUM_PARAMS
  };

  enum InputIds {
    NOISE_LENGTH_INPUT,
    NUM_INPUTS
  };

  enum OutputIds {
    NOISE_OUTPUT,
    NUM_OUTPUTS
  };

  enum LightIds {
    NOISE_FROM_INPUT,
    NOISE_FROM_KNOB,

    NOISE_LENGTH_LIGHT,

    PERIOD_93_LIGHT,

    USING_93,
    
    NUM_LIGHTS
  };

  ChipSym::NESNoise noise;
  int prior_shortlen;
  bool prior_longmode;
  
  ChipNoise() : Module( NUM_PARAMS, NUM_INPUTS, NUM_OUTPUTS, NUM_LIGHTS ),
                noise( -5.0, 5.0, engineGetSampleRate() )
                
  {
    params[ LONG_MODE ].value = 1;
    params[ NOISE_LENGTH ].value = 9;
    params[ SHORT_LEN ].value = 1;
    params[ PERIOD_93 ].value = 1;
    prior_shortlen = 1;
    prior_longmode = false;
  }

  void step() override
  {
    lights[ NOISE_FROM_KNOB ].value  = !inputs[ NOISE_LENGTH_INPUT ].active;
    lights[ NOISE_FROM_INPUT ].value =  inputs[ NOISE_LENGTH_INPUT ].active;

    unsigned int nl = (unsigned int)clamp( params[ NOISE_LENGTH ].value, 0.0f, 15.0f );
    if( inputs[ NOISE_LENGTH_INPUT ].active )
      nl = (unsigned int)clamp( inputs[ NOISE_LENGTH_INPUT ].value * 1.5, 0.0f, 15.0f );
    
    lights[ NOISE_LENGTH_LIGHT ].value = nl;
    noise.setPeriod(nl);

    int p93 = (int)params[ PERIOD_93 ].value;
    lights[ PERIOD_93_LIGHT ].value = p93;
    if( params[ LONG_MODE ].value == 0 && params[ SHORT_LEN ].value == 1 )
      {
        noise.set93Key( p93 );
        lights[ USING_93 ].value = 1;
      }
    else
      {
        lights[ USING_93 ].value = 0;
      }
    

    bool tmpNoiseFlag = ( params[ LONG_MODE ].value == 0 );
    if ( tmpNoiseFlag != prior_longmode )
      {
        prior_longmode = tmpNoiseFlag;
        noise.setModeFlag( prior_longmode );
      }

    if( params[ SHORT_LEN ].value != prior_shortlen )
      {
        prior_shortlen = params[SHORT_LEN].value;
        if( prior_shortlen == 1 )
          {
            noise.setShortLength( ChipSym::NESNoise::SHORT_93 );
          }
        else
          {
            noise.setShortLength( ChipSym::NESNoise::SHORT_31 );
          }
      }
    
    outputs[ NOISE_OUTPUT ].value = noise.step();
  }
};

struct ChipNoiseWidget : ModuleWidget {
  ChipNoiseWidget( ChipNoise *module);
};

ChipNoiseWidget::ChipNoiseWidget( ChipNoise *module ) : ModuleWidget( module )
{
  box.size = Vec( SCREW_WIDTH * 6, RACK_HEIGHT );

  BaconBackground *bg = new BaconBackground( box.size, "ChipNoise" );
  addChild( bg->wrappedInFramebuffer());

  // Control the noise length
  bg->addRoundedBorder( Vec( 8, 45 ), Vec( SCREW_WIDTH * 6 - 16, 75 ) );
  bg->addLabel( Vec( bg->cx() + 7, 55 ), "wave", 11, NVG_ALIGN_LEFT | NVG_ALIGN_TOP );
  bg->addLabel( Vec( bg->cx() + 5, 66 ), "length", 11, NVG_ALIGN_LEFT | NVG_ALIGN_TOP );
  Vec inP = Vec( 16, 53 );
  addInput( Port::create< PJ301MPort >( inP,
                                        Port::INPUT,
                                        module,
                                        ChipNoise::NOISE_LENGTH_INPUT ) );
  addChild( ModuleLightWidget::create< SmallLight< BlueLight > >( inP.minus( Vec( 4, 4 ) ), module, ChipNoise::NOISE_FROM_INPUT ) );

  int ybot = 120;
  addParam( ParamWidget::create< RoundSmallBlackKnob >( Vec( 16, ybot - 3 - 28 ),
                                                        module,
                                                        ChipNoise::NOISE_LENGTH,
                                                        0, 15, 7 ) );
  addChild( ModuleLightWidget::create< SmallLight< BlueLight > >( Vec( 16-4, ybot - 3 - 28 -4 ), module, ChipNoise::NOISE_FROM_KNOB ) );
  addChild( MultiDigitSevenSegmentLight< BlueLight, 2, 2 >::create( Vec( 47, ybot - 5 - 24 ),
                                                                    module,
                                                                    ChipNoise::NOISE_LENGTH_LIGHT ) );


  bg->addRoundedBorder( Vec( 8, 135 ), Vec( SCREW_WIDTH * 6 - 16, 160 ) );
  bg->addLabel( Vec( bg->cx(), 155 ), "Sequence", 13, NVG_ALIGN_CENTER | NVG_ALIGN_BOTTOM );
  addParam( ParamWidget::create< NKK >( Vec( bg->cx() - 32, 175 ), module, ChipNoise::LONG_MODE, 0, 1, 1 ) );
  addParam( ParamWidget::create< NKK >( Vec( bg->cx() + 2, 175 ), module, ChipNoise::SHORT_LEN, 0, 1, 1 ) );
  bg->addLabel( Vec( bg->cx() + 16 - 32, 160 ), "long", 11, NVG_ALIGN_CENTER | NVG_ALIGN_TOP );
  bg->addLabel( Vec( bg->cx() + 16 - 32, 223 ), "short", 11, NVG_ALIGN_CENTER| NVG_ALIGN_TOP );

  bg->addLabel( Vec( bg->cx() + 16 + 2, 160 ), "93", 11, NVG_ALIGN_CENTER | NVG_ALIGN_TOP );
  bg->addLabel( Vec( bg->cx() + 16 + 2, 223 ), "31", 11, NVG_ALIGN_CENTER| NVG_ALIGN_TOP );


  bg->addLabel( Vec( bg->cx(), 258 ), "Which 93 seq", 11, NVG_ALIGN_CENTER | NVG_ALIGN_BOTTOM );

  addChild( MultiDigitSevenSegmentLight< BlueLight, 2, 3 >::create( Vec( 50 - 14, 262 ),
                                                                    module,
                                                                    ChipNoise::PERIOD_93_LIGHT ) );
  
  addParam( ParamWidget::create< RoundSmallBlackKnob >( Vec( 11, 262 ),
                                                        module,
                                                        ChipNoise::PERIOD_93,
                                                        0, 351, 17 ) );
  addChild( ModuleLightWidget::create< SmallLight< BlueLight > >( Vec( 12, 249 ), module, ChipNoise::USING_93 ) );
  

  // Output port
  Vec outP = Vec( bg->cx( 24 ), RACK_HEIGHT - 15 - 43 );
  bg->addPlugLabel( outP, BaconBackground::SIG_OUT, "out" );
  addOutput( Port::create< PJ301MPort >( outP,
                                         Port::OUTPUT,
                                         module,
                                         ChipNoise::NOISE_OUTPUT ) );

}

} // namespace rack_plugin_BaconMusic

using namespace rack_plugin_BaconMusic;

RACK_PLUGIN_MODEL_INIT(BaconMusic, ChipNoise) {
   Model *modelChipNoise = Model::create<ChipNoise, ChipNoiseWidget>("Bacon Music", "ChipNoise", "ChipNoise", NOISE_TAG );
   return modelChipNoise;
}
