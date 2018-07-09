#include "BaconPlugs.hpp"

namespace rack_plugin_BaconMusic {

struct HarMoNee : Module {
  enum ParamIds {
    UP_OR_DOWN,
    HALF_STEP,
    WHOLE_STEP,
    MINOR_THIRD,
    MAJOR_THIRD,
    FIFTH,
    OCTAVE,
    NUM_PARAMS
  };
  enum InputIds {
    SOURCE_INPUT,
    NUM_INPUTS
  };
  enum OutputIds {
    ECHO_OUTPUT,
    INCREASED_OUTPUT,
    NUM_OUTPUTS
  };
  enum LightIds {
    UP_LIGHT,
    DOWN_LIGHT,
    HALF_STEP_LIGHT,
    WHOLE_STEP_LIGHT,
    MINOR_THIRD_LIGHT,
    MAJOR_THIRD_LIGHT,
    FIFTH_LIGHT,
    OCTAVE_LIGHT,

    DIGIT_LIGHT,
    
    NUM_LIGHTS
  };
  
  std::vector< float > offsets;
  float priorOffset;
  float targetOffset;
  int offsetCount;
  
  HarMoNee() : Module( NUM_PARAMS, NUM_INPUTS, NUM_OUTPUTS, NUM_LIGHTS ) {
    for( int i=0; i<OCTAVE; ++i ) offsets.push_back( 0 );

    offsets[ HALF_STEP ] = 1;
    offsets[ WHOLE_STEP ] = 2;
    offsets[ MINOR_THIRD ] = 3; 
    offsets[ MAJOR_THIRD ] = 4;
    offsets[ FIFTH ] = 7;
    offsets[ OCTAVE ] = 12;
    priorOffset = 0;
    targetOffset = 0;
    offsetCount = 0;
  }

  void step() override;
};

void HarMoNee::step() {
  /* TODO
     
     Display the shift 
     Tests
  */

  float in = inputs[ SOURCE_INPUT ].value;
  float echo = in;

  float offsetI = 0;
  float uod = ( params[ UP_OR_DOWN ].value > 0 ) ? 1.0 : -1.0;
  if( uod > 0 )
    {
      lights[ UP_LIGHT ].value = 1; lights[ DOWN_LIGHT ].value = 0;
    }
  else
    {
      lights[ DOWN_LIGHT ].value = 1; lights[ UP_LIGHT ].value = 0;
    }

  int ld = HALF_STEP_LIGHT - HALF_STEP;
  for( int i=HALF_STEP; i <= OCTAVE; ++i )
    {
      if( params[ i ].value > 0 )
        {
          lights[ i + ld ].value = 1.0;
          offsetI += offsets[ i ];
        }
      else
        {
          lights[ i + ld ].value = 0.0;
        }
    }
  lights[ DIGIT_LIGHT ].value = offsetI;
  
  offsetI = uod * offsetI / 12.0;

  int shift_time = 44000 / 5;
  /* Glissando state management
     - priorOffset is the place we are starting the glide from
     - targetOffset is where we are headed
     - offsetI is where the switches are set
     - offsetCount is how far we are in.

     when we aren't in a glissando offsetCount will be 0 and
     all three will be the same. offsetCount being
     non-zero is the same as in-gliss.
   */
  bool inGliss = offsetCount != 0;
  if( ! inGliss )
    {
      // We are not sliding. Should we be?
      if( offsetI != priorOffset )
        {
          targetOffset = offsetI;
          offsetCount = 1;
          inGliss = true;
        }
    }

  if( inGliss )
    {
      // If the target == the offset we haven't changed anything so
      // just march along linear time
      if( offsetI != targetOffset )
        {
          float lastKnown = ( ( shift_time - offsetCount ) * priorOffset +
                              offsetCount * targetOffset ) / shift_time;
          targetOffset = offsetI;
          priorOffset = lastKnown;
          offsetCount = 0;
        }
      
      offsetI = ( ( shift_time - offsetCount ) * priorOffset +
                  offsetCount * offsetI ) / shift_time;
      
      offsetCount ++;
    }

  // Finally if we are done, reset it all to zero
  if( offsetCount == shift_time )
    {
      offsetCount = 0;
      priorOffset = offsetI;
      targetOffset = offsetI;
    }
  
  float increased = in + offsetI;

  outputs[ ECHO_OUTPUT ].value = echo;
  outputs[ INCREASED_OUTPUT ].value = increased;
}

struct HarMoNeeWidget : ModuleWidget {
  HarMoNeeWidget(HarMoNee *model);
};


HarMoNeeWidget::HarMoNeeWidget( HarMoNee *model ) : ModuleWidget( model )
{
  box.size = Vec( SCREW_WIDTH*8 , RACK_HEIGHT );

  BaconBackground *bg = new BaconBackground( box.size, "HarMoNee" );

  addChild( bg->wrappedInFramebuffer() );
  
  Vec iPos( 12, 100 );
  bg->addPlugLabel( iPos, BaconBackground::SIG_IN, "in" );
  addInput( Port::create< PJ301MPort >( iPos, Port::INPUT, module, HarMoNee::SOURCE_INPUT ) );

  iPos.y += 60;
  bg->addPlugLabel( iPos, BaconBackground::SIG_OUT, "root" );
  addOutput( Port::create<PJ301MPort>(iPos, Port::OUTPUT, module, HarMoNee::ECHO_OUTPUT ) );

  iPos.y += 60;
  bg->addPlugLabel( iPos, BaconBackground::SIG_OUT, "harm" );
  addOutput( Port::create<PJ301MPort>(iPos, Port::OUTPUT, module, HarMoNee::INCREASED_OUTPUT ) );

  // NKK is 32 x 44
  addParam( ParamWidget::create< NKK >( Vec( 80, 26 ), module, HarMoNee::UP_OR_DOWN, 0, 1, 1 ) );
  bg->addLabel( Vec( 74, 26+22-4-5-5 ), "up", 12, NVG_ALIGN_CENTER | NVG_ALIGN_BOTTOM );
  addChild( ModuleLightWidget::create< MediumLight< GreenLight >>( Vec( 70, 26 + 22 - 4 - 5 ), module, HarMoNee::UP_LIGHT ) );

  bg->addLabel( Vec( 74, 26+22-4+5+8+7 ), "dn", 12, NVG_ALIGN_CENTER | NVG_ALIGN_TOP );
  addChild( ModuleLightWidget::create< MediumLight< RedLight >>( Vec( 70, 26 + 22 - 4 + 5 ), module, HarMoNee::DOWN_LIGHT ) );


  addChild( MultiDigitSevenSegmentLight< BlueLight, 4, 2 >::create( Vec( 10, 30 ),
                                                                    module,
                                                                    HarMoNee::DIGIT_LIGHT ) );


  int x = 80; int y = 26 + 45; float v = -1;
  int ld = HarMoNee::HALF_STEP_LIGHT - HarMoNee::HALF_STEP;

  const char* labels[] = { "1/2", "W", "m3", "III", "V", "O" };
  for( int i = HarMoNee::HALF_STEP; i <= HarMoNee::OCTAVE; ++i )
    {
      if( i == HarMoNee::OCTAVE ) { v = 1; } { v = -1; }
      addParam( ParamWidget::create<NKK>( Vec( x, y ), module, i, 0, 1, v ) );
      bg->addLabel( Vec( 66, y+22 ), labels[ i - HarMoNee::HALF_STEP ],
                    14, NVG_ALIGN_RIGHT | NVG_ALIGN_MIDDLE );
      addChild( ModuleLightWidget::create< MediumLight< BlueLight > >( Vec( 70, y + 22 - 5 ), module, i + ld ) );
      y += 45;
    }
}

} // namespace rack_plugin_BaconMusic

using namespace rack_plugin_BaconMusic;

RACK_PLUGIN_MODEL_INIT(BaconMusic, HarMoNee) {
   Model *modelHarMoNee = Model::create<HarMoNee,HarMoNeeWidget>("Bacon Music", "HarMoNee", "HarMoNee", TUNER_TAG);
   return modelHarMoNee;
}
