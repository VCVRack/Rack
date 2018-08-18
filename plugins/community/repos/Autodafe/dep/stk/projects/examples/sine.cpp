/******************************************/
/*
  Example program to write N sine tones to
  an N channel soundfile.

  By default, the program will write an
  N channel WAV file.  However, it is
  simple to change the file type argument
  in the FileWvOut constructor.

  By Gary P. Scavone, 2000 - 2002.
*/
/******************************************/

#include "SineWave.h"
#include "FileWvOut.h"
#include <cstdlib>

using namespace stk;

void usage(void) {
  // Error function in case of incorrect command-line
  // argument specifications.
  std::cout << "\nuseage: sine N file time fs\n";
  std::cout << "    where N = number of channels (sines),\n";
  std::cout << "    file = the .wav file to create,\n";
  std::cout << "    time = the amount of time to record (in seconds),\n";
  std::cout << "    and fs = the sample rate (in Hz).\n\n";
  exit( 0 );
}

int main( int argc, char *argv[] )
{
  float base_freq = 220.0;
  int i;

  // Minimal command-line checking.
  if ( argc != 5 ) usage();

  int channels = (int) atoi( argv[1] );
  double time = atof( argv[3] );
  double srate = atof( argv[4] );

  // Create our object instances.
  FileWvOut output;
  SineWave **oscs = (SineWave **) malloc( channels * sizeof(SineWave *) );
  for ( i=0; i<channels; i++ ) oscs[i] = 0;

  // If you want to change the default sample rate (set in Stk.h), do
  // it before instantiating any objects!!
  Stk::setSampleRate( srate );

  // Define the sinewaves.
  for ( i=0; i<channels; i++ )
    oscs[i] = new SineWave;

  // Set oscillator frequency(ies) here ... somewhat random.
  for ( i=0; i<channels; i++ )
    oscs[i]->setFrequency( base_freq + i*(45.0) );

  long nFrames = (long) ( time * Stk::sampleRate() );
  StkFrames frames( nFrames, channels );

  // Open the soundfile for output.  Other file format options
  // include: FILE_SND, FILE_AIF, FILE_MAT, and FILE_RAW.  Other data
  // type options include: STK_SINT8, STK_INT24, STK_SINT32,
  // STK_FLOAT32, and STK_FLOAT64.
  try {
    output.openFile( argv[2], channels, FileWrite::FILE_WAV, Stk::STK_SINT16 );
  }
  catch ( StkError & ) {
    goto cleanup;
  }

  // Here's the runtime code ... no loop
  for ( i=0; i<channels; i++ )
    oscs[i]->tick( frames, i );

  output.tick( frames );

 cleanup:
  for ( i=0; i<channels; i++ )
    delete oscs[i];
  free( oscs );

  return 0;
}
