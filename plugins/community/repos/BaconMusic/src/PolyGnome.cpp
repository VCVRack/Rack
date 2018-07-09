
#include "BaconPlugs.hpp"

#include <vector>
#include <algorithm>

#define NUM_CLOCKS 4

namespace rack_plugin_BaconMusic {

struct PolyGnome : virtual Module {
  enum ParamIds {
    CLOCK_PARAM,
    CLOCK_NUMERATOR_1,
    CLOCK_DENOMINATOR_1 = CLOCK_NUMERATOR_1 + NUM_CLOCKS,
    NUM_PARAMS = CLOCK_DENOMINATOR_1 + NUM_CLOCKS,
  };

  enum InputIds {
    CLOCK_INPUT,
    NUM_INPUTS,
  };

  enum OutputIds {
    CLOCK_GATE_0,
    
    NUM_OUTPUTS = CLOCK_GATE_0 + NUM_CLOCKS + 1 // the "1" is for the 1/4 note clock which isn't parameterized
  };

  enum LightIds {
    LIGHT_NUMERATOR_1,
    LIGHT_DENOMINATOR_1 = LIGHT_NUMERATOR_1 + NUM_CLOCKS,
    NUM_LIGHTS = LIGHT_DENOMINATOR_1 + NUM_CLOCKS
  };

  float phase;
  long  phase_longpart;
  
  PolyGnome() : Module( NUM_PARAMS, NUM_INPUTS, NUM_OUTPUTS, NUM_LIGHTS )
  {
    phase = 0.0f;
    phase_longpart = 274;
  }

  inline int numi( int i ) { return (int)params[ CLOCK_NUMERATOR_1 + i ].value; }
  inline int deni( int i ) { return (int)params[ CLOCK_DENOMINATOR_1 + i ].value; }
  void step() override
  {
    float clockTime = powf(2.0f, params[CLOCK_PARAM].value + inputs[CLOCK_INPUT].value);
    phase += clockTime * engineGetSampleTime();

    while( phase > 1 )
      {
        phase = phase - 1;
        phase_longpart ++;
      }


    /* Alright we have to stop that longpart from getting too big otherwise it will swamp
       the fractional parts but we have to reset it when all the clocks are firing at once
       otherwise one of the clocks will stutter. So figure out the product of my fractions.
       Probably we should use the common prime factors so we can get an earlier step
       but lets leave it for now.
    */
    int sd = 1;
    int sn = 1;
    for( int i=0; i<NUM_CLOCKS; ++i )
      {
        if( outputs[ CLOCK_GATE_0 + i + 1 ].active )
          {
            sd *= deni( i );
            sn *= numi( i );
          }
      }
    int commonp = sd * sn; // so we know at least that the clocks will intersect at this tick.


    while( phase_longpart > commonp )
      {
        phase_longpart -= commonp;
      }
    
    for( int i=0; i<NUM_CLOCKS+1 ; ++i )
      {
        bool gateIn = false;
        float frac;
        if( i == 0 )
          frac = 1;
        else
          frac = deni( i - 1 ) /  ( 1.0f * numi( i - 1 ) );

        // Note that we have two parts which comprise the phase number now, the float and the long.
        // The addition can overflow, though which is why I mod the phase_longpart with a larger number
        float lphase = phase * frac;
        double liphase = phase_longpart * frac;

        double ipart;
        // I still worry a bit this + may overflow if you let this run long enough and blow out the precision in the decimal
        float fractPhase = modf( lphase + liphase, &ipart );
        gateIn = (fractPhase < 0.5f);

        outputs[ CLOCK_GATE_0 + i ].value = gateIn ? 10.0f : 0.0f;
      }
    
    for( int i=0; i<NUM_CLOCKS; ++i )
      {
        lights[ LIGHT_NUMERATOR_1 + i ].value = (int)params[ CLOCK_NUMERATOR_1 + i ].value;
        lights[ LIGHT_DENOMINATOR_1 + i ].value = (int)params[ CLOCK_DENOMINATOR_1 + i ].value;
      }
  }
};

struct PolyGnomeWidget : ModuleWidget {
  PolyGnomeWidget( PolyGnome *module);
};

PolyGnomeWidget::PolyGnomeWidget( PolyGnome *module ) : ModuleWidget( module )
{
  box.size = Vec( SCREW_WIDTH * 14, RACK_HEIGHT );

  BaconBackground *bg = new BaconBackground( box.size, "PolyGnome" );
  addChild( bg->wrappedInFramebuffer());

  bg->addLabel( Vec( 17, 45 ), "Clock", 13, NVG_ALIGN_LEFT | NVG_ALIGN_TOP );
  addParam( ParamWidget::create< RoundSmallBlackKnob >( Vec( 55, 40 ),
                                                        module,
                                                        PolyGnome::CLOCK_PARAM,
                                                        -2.0f, 6.0f, 2.0f ) );
  addInput( Port::create< PJ301MPort >( Vec( 85, 40 ),
                                        Port::INPUT,
                                        module,
                                        PolyGnome::CLOCK_INPUT ) );
  
  for( size_t i=0; i<= NUM_CLOCKS; ++i )
    {
      Vec outP = Vec( box.size.x - 45, 100 + 48 * i );
      if( i == 0 )
        {
          bg->addLabel( Vec( 17, outP.y + 21 ), "Unit (1/1) clock", 13, NVG_ALIGN_LEFT | NVG_ALIGN_BOTTOM );
        }
      else
        {
          int yoff = 2;
          // knob light knob light
          addParam( ParamWidget::create< RoundSmallBlackKnob >( Vec( 17, outP.y + yoff ),
                                                                module,
                                                                PolyGnome::CLOCK_NUMERATOR_1 + (i-1),
                                                                1, 30, 1 ) );
          addChild( MultiDigitSevenSegmentLight< BlueLight, 2, 2 >::create( Vec( 48, outP.y + yoff ),
                                                                            module,
                                                                            PolyGnome::LIGHT_NUMERATOR_1 + (i-1) ) );

          int mv = 47 + 20 + 14 - 16;
          addParam( ParamWidget::create< RoundSmallBlackKnob >( Vec( 16 + mv, outP.y + yoff ),
                                                                module,
                                                                PolyGnome::CLOCK_DENOMINATOR_1 + (i-1),
                                                                1, 16, 1 ) );
          addChild( MultiDigitSevenSegmentLight< BlueLight, 2, 2 >::create( Vec( 47 + mv, outP.y + yoff ),
                                                                            module,
                                                                            PolyGnome::LIGHT_DENOMINATOR_1 + (i-1) ) );
        }
      addOutput( Port::create< PJ301MPort >( outP,
                                             Port::OUTPUT,
                                             module,
                                             PolyGnome::CLOCK_GATE_0 + i ) );
      bg->addRoundedBorder( Vec( 12, outP.y - 4 ), Vec( box.size.x - 24, 36 ) );
    }

}

} // namespace rack_plugin_BaconMusic

using namespace rack_plugin_BaconMusic;

RACK_PLUGIN_MODEL_INIT(BaconMusic, PolyGnome) {
   Model *modelPolyGnome = Model::create<PolyGnome, PolyGnomeWidget>("Bacon Music", "PolyGnome", "PolyGnome", CLOCK_TAG );
   return modelPolyGnome;
}

