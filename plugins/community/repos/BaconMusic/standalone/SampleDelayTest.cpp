#include "standalone_helpers.hpp"
#include "../src/SampleDelay.hpp"
#include <iostream>

int main( int argc, char **argv )
{
  typedef SampleDelay< StandaloneModule > SD;

  {
    SD sd;
    int dk = 37;
    sd.params[ SD::DELAY_KNOB ].value = dk;
    sd.inputs[ SD::SIGNAL_IN ].active = 1;

    SD::results_t ov;
    int ns = 1000;
    for( int i=0; i<ns; ++i )
      {
        sd.inputs[ SD::SIGNAL_IN ].value = i * 1.0f / ns;
        sd.step();
        ov.push_back( sd.outputs );
      }
    for( int i=dk; i<ns; ++i )
      std::cout << i << " " << ov[ i-1 ][ SD::SIGNAL_OUT ].value << " " << 1.0f * (i-dk)/ ns << "\n";
  }
  
}
