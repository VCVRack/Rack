
#ifdef BUILD_SORTACHORUS

#include "BaconPlugs.hpp"
#include <algorithm>
#include <cmath>
#include <iostream>

namespace rack_plugin_BaconMusic {

struct SortaChorus : virtual Module {
private:
  // Borrowing from RingBuffer which this sort of is, S must be a power of 2
  template <typename T, size_t S>
  struct ChorusBuffer
  {
    T data[ S ];
    size_t SZ = S;
    
    size_t currp = 0;
    ChorusBuffer()
    {
      std::fill( data, data + S, 0 );
    }

    size_t mask(size_t i) const {
      return i & (S - 1);
    }
    
    void push( T t )
    {
      size_t i = mask( currp++ );
      data[ i ] = t;
    }

    // todo interpolate since this will sound like poop
    T readBack( size_t off )
    {
      size_t pos = ( currp + ( S - 1 ) - off ) & ( S - 1 );
      return data[ pos ];
    }
    
  };

  ChorusBuffer< float, 16384 > buf;
  float tNow;
  
public:
  enum ParamIds {
    DEPTH,
    SPEED,
    SHAPE,
    NOISE,
    NUM_PARAMS
  };

  enum InputIds {
    SIGNAL_INPUT,
    NUM_INPUTS
  };

  enum OutputIds {
    CHORUSED_OUTPUT,
    NUM_OUTPUTS
  };

  enum LightIds {
    NUM_LIGHTS
  };

  SortaChorus() : Module( NUM_PARAMS, NUM_INPUTS, NUM_OUTPUTS, NUM_LIGHTS )
  {
    tNow = 0;
  }

  void step() override
  {
    buf.push( inputs[ SIGNAL_INPUT ].value );

    // crude for now - basically a saw.
    tNow += engineGetSampleTime();
    float intpart;
    float fractpart = modf( tNow, &intpart );
    if( (int)intpart % 2 == 0 ) fractpart = 1.0 - fractpart;
    
    float chSig = buf.readBack( (size_t)( fractpart * buf.SZ / 20 ) );

    float depth = params[ DEPTH ].value;
    
    outputs[ CHORUSED_OUTPUT ].value = ( inputs[ SIGNAL_INPUT ].value + depth * chSig ) / ( 1.0 + depth );
  }
};

struct SortaChorusWidget : ModuleWidget {
  SortaChorusWidget( SortaChorus *module);
};

SortaChorusWidget::SortaChorusWidget( SortaChorus *module ) : ModuleWidget( module )
{
  box.size = Vec( SCREW_WIDTH * 10, RACK_HEIGHT );

  BaconBackground *bg = new BaconBackground( box.size, "SortaChorus" );
  addChild( bg->wrappedInFramebuffer());

  Vec wdPos( 40, 40 ), knobPos;
  bg->addLabelsForLargeKnob( wdPos, "Depth", "None", "Lots", knobPos );
  addParam( ParamWidget::create< RoundLargeBlackKnob >( knobPos, module,
                                                        SortaChorus::DEPTH,
                                                        0, 2.0, 0.5 ) );

  wdPos.x = box.size.x - 40;
  bg->addLabelsForLargeKnob( wdPos, "Speed", "Slow", "Fast", knobPos );
  addParam( ParamWidget::create< RoundLargeBlackKnob >( knobPos, module,
                                                        SortaChorus::SPEED,
                                                        0, 1.0, 0.5 ) );

  Vec inP = Vec( bg->cx( 24 ) - 30, RACK_HEIGHT - 15 - 43 );
  bg->addPlugLabel( inP, BaconBackground::SIG_IN, "in" );
  addInput( Port::create< PJ301MPort >( inP,
                                        Port::INPUT,
                                        module,
                                        SortaChorus::SIGNAL_INPUT ) );

  Vec outP = Vec( bg->cx( 24 ) + 30, RACK_HEIGHT - 15 - 43 );
  bg->addPlugLabel( outP, BaconBackground::SIG_OUT, "out" );
  addOutput( Port::create< PJ301MPort >( outP,
                                         Port::OUTPUT,
                                         module,
                                         SortaChorus::CHORUSED_OUTPUT ) );
}

} // namespace rack_plugin_BaconMusic

using namespace rack_plugin_BaconMusic;

RACK_PLUGIN_MODEL_INIT(BaconMusic, SortaChorus) {
   Model *modelSortaChorus = Model::create<SortaChorus, SortaChorusWidget>("Bacon Music", "SortaChorus", "SortaChorus", CHORUS_TAG );
   return modelSortaChorus;
}

#endif
