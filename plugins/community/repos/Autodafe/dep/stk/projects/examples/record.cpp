/******************************************/
/*
  Example program to record N channels of data
  by Gary P. Scavone, 2000

  NOTE: This program makes use of blocking audio
  input/output routines.  On systems where the
  underlying audio API is based on a callback scheme
  (Macintosh OS-X, Windows ASIO, and Linux JACK), these
  routines are not fully robust (over/underruns can
  happen with some frequency).  See the STK tutorial
  for example programs using callback schemes and/or
  visit the RtAudio tutorial page
  (http://music.mcgill.ca/~gary/rtaudio/) for more
  information.

  This program is currently written to read
  from a realtime audio input device and to
  write to a WAV output file.  However, it
  is simple to replace the FILE_TYPE specifier
  to WvOut with another file type.
*/
/******************************************/

#include "RtWvIn.h"
#include "FileWvOut.h"
#include <cstdlib>

using namespace stk;

void usage( void ) {
  // Error function in case of incorrect command-line
  // argument specifications.
  std::cout << "\nuseage: record N file time fs \n";
  std::cout << "    where N = number of channels,\n";
  std::cout << "    file = the .wav file to create,\n";
  std::cout << "    time = the amount of time to record (in seconds),\n";
  std::cout << "    and fs = the sample rate.\n\n";
  exit( 0 );
}

int main( int argc, char *argv[] )
{
  // minimal command-line checking
  if ( argc != 5 ) usage();

  Stk::showWarnings( true );

  unsigned int channels = (unsigned int) atoi( argv[1] );
  double sampleRate = atof( argv[4] );
  double time = atof( argv[3] );
  long samples, i;
  StkFrames frame( 1, channels );

  // Set the global sample rate.
  Stk::setSampleRate( sampleRate );

  // Initialize our WvIn/WvOut pointers.
  RtWvIn *input = 0;
  FileWvOut *output = 0;

  // Open the realtime input device
  try {
    input = new RtWvIn( channels );
  }
  catch ( StkError & ) {
    exit( 1 );
  }
  
  // Open the soundfile for output.
  try {
    output = new FileWvOut( argv[2], channels, FileWrite::FILE_WAV );
  }
  catch ( StkError & ) {
    goto cleanup;
  }

  // Here's the runtime loop
  samples = (long) ( time * Stk::sampleRate() );
  for ( i=0; i<samples; i++ ) {
    output->tick( input->tick( frame ) );
  }

 cleanup:
  delete input;
  delete output;
  return 0;
}
