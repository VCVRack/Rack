// foursine.cpp STK tutorial program

#include "SineWave.h"
#include "FileWvOut.h"
#include <cstdlib>

using namespace stk;

int main()
{
  // Set the global sample rate before creating class instances.
  Stk::setSampleRate( 44100.0 );

  int i;
  FileWvOut output;
  SineWave inputs[4];

  // Set the sine wave frequencies.
  for ( i=0; i<4; i++ )
    inputs[i].setFrequency( 220.0 * (i+1) );

  // Define and open a 16-bit, four-channel AIFF formatted output file
  try {
    output.openFile( "foursine.aif", 4, FileWrite::FILE_AIF, Stk::STK_SINT16 );
  }
  catch (StkError &) {
    exit( 1 );
  }

  // Write two seconds of four sines to the output file
  StkFrames frames( 88200, 4 );
  for ( i=0; i<4; i++ )
    inputs[i].tick( frames, i );

  output.tick( frames );

  // Now write the first sine to all four channels for two seconds
  for ( i=0; i<88200; i++ ) {
    output.tick( inputs[0].tick() );
  }

  return 0;
}
