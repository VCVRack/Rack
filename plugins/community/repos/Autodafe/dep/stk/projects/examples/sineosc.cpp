// sineosc.cpp STK tutorial program

#include "FileLoop.h"
#include "FileWvOut.h"
#include <cstdlib>

using namespace stk;

int main()
{
  // Set the global sample rate before creating class instances.
  Stk::setSampleRate( 44100.0 );

  int nFrames = 100000;
  FileLoop input;
  FileWvOut output;

  try {
    // Load the sine wave file.
    input.openFile( "rawwaves/sinewave.raw", true );

    // Open a 16-bit, one-channel WAV formatted output file
    output.openFile( "hellosine.wav", 1, FileWrite::FILE_WAV, Stk::STK_SINT16 );
  }
  catch ( StkError & ) {
    exit( 1 );
  }

  input.setFrequency( 440.0 );

  // Option 1: Use StkFrames
  /*
  StkFrames frames( nFrames, 1 );
  try {
    output.tick( input.tick( frames ) );
  }
  catch ( StkError & ) {
    exit( 1 );
  }
  */

  // Option 2: Single-sample computations
  for ( int i=0; i<nFrames; i++ ) {
    try {
      output.tick( input.tick() );
    }
    catch ( StkError & ) {
      exit( 1 );
    }
  }

  return 0;
}
