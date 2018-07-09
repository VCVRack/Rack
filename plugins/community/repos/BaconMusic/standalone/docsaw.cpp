/*
** Just showing how to use the standalone helpers to make a mono generator which gets 
** punped to default output quickly and easily
*/

#include "standalone_helpers.hpp"
#include <iostream>
#include <cstdlib>

struct SawGen : StepHandler
{
  double lastValue;
  SawGen() : lastValue( 0 ) {}

  virtual int dostep( void *outputBuffer, void *inputBuffer, unsigned int nBufferFrames,
                      double streamTime, RtAudioStreamStatus status ) override
  {
    unsigned int i, j;
    double *buffer = (double *) outputBuffer;

    for ( i=0; i<nBufferFrames; i++ ) {
      *buffer++ = lastValue;
      lastValue += 0.015;
      if ( lastValue >= 1.0 ) lastValue -= 2.0;
    }
    return 0;
  }
};

int main()
{
  SawGen sg;
  sg.playAudioUntilEnterPressed();
}
