#include "../src/KSSynth.hpp"
#include "standalone_helpers.hpp"

#include <iostream>

struct KSGen : StepHandler
{
  KSSynth s;
  KSGen() : s( -0.9, 0.9, 44100 ) { }

  virtual int dostep( void *outputBuffer, void *inputBuffer, unsigned int nBufferFrames,
                      double streamTime, RtAudioStreamStatus status ) override
  {
    unsigned int i, j;
    double *buffer = (double *) outputBuffer;

    for ( i=0; i<nBufferFrames; i++ ) {
      *buffer++ = s.step();
    }
    if( ! s.active )
      return 1;
    return 0;
    
  }
};

int main( int argc, char **argv )
{
  KSGen gen;
  
  std::cout << "packets " << gen.s.numInitPackets() << "\n";

  gen.s.filtAtten = 3.0;
  gen.s.filtParamA = 0.3;
  gen.s.packet = KSSynth::RANDOM;
  float freq = 440;
  for( int i=0; i<=12; ++i )
    {
      float mul = pow( 2.0, i / 12.0f );
      gen.s.trigger( freq * mul );
      
      gen.playAudioUntilStepsDone();
    }
}
