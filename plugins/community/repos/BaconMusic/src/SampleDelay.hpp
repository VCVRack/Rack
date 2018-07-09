
#include <vector>
#include <algorithm>

namespace rack_plugin_BaconMusic {

template< typename TBase >
struct SampleDelay : virtual TBase {
  enum ParamIds {
    DELAY_KNOB,
    NUM_PARAMS
  };

  enum InputIds {
    SIGNAL_IN,
    NUM_INPUTS
  };

  enum OutputIds {
    SIGNAL_OUT,
    NUM_OUTPUTS
  };

  enum LightIds {
    DELAY_VALUE_LIGHT,
    NUM_LIGHTS
  };

  using TBase::params;
  using TBase::inputs;
  using TBase::outputs;
  using TBase::lights;

  std::vector< float > ring;
  size_t ringSize;
  size_t pos;
  
  SampleDelay() : TBase( NUM_PARAMS, NUM_INPUTS, NUM_OUTPUTS, NUM_LIGHTS )
  {
    params[ DELAY_KNOB ].value = 1;
    ringSize = 100;
    ring.resize( ringSize );
    std::fill( ring.begin(), ring.end(), 0 );
    pos = 0;
  }

  void step() override
  {
    int del = params[ DELAY_KNOB ].value - 1;
    int dpos = ( (int)pos - del );
    if( dpos < 0 ) dpos += ringSize;

    ring[ pos ] = inputs[ SIGNAL_IN ].value;
    outputs[ SIGNAL_OUT ].value = ring[ dpos ];
    lights[ DELAY_VALUE_LIGHT ].value = del + 1;

    pos++;
    if( pos >= ringSize ) pos = 0;
  }
};

} // namespace rack_plugin_BaconMusic
