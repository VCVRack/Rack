/******************************************/
/*
  Example program to play an N channel
  soundfile.

  This program will load WAV, SND, AIF, and
  MAT-file formatted files of various data
  types.  If the audio system does not support
  the number of channels or sample rate of
  the soundfile, the program will stop.

  By Gary P. Scavone, 2000 - 2004.
*/
/******************************************/

#include "FileWvIn.h"
#include "RtAudio.h"

#include <signal.h>
#include <iostream>
#include <cstdlib>

using namespace stk;

// Eewww ... global variables! :-)
bool done = false;
StkFrames frames;
static void finish(int ignore){ done = true; }

void usage(void) {
  // Error function in case of incorrect command-line
  // argument specifications.
  std::cout << "\nuseage: play file sr <rate>\n";
  std::cout << "    where file = the file to play,\n";
  std::cout << "    where sr = sample rate,\n";
  std::cout << "    and rate = an optional playback rate.\n";
  std::cout << "               (default = 1.0, can be negative)\n\n";
  exit( 0 );
}

// This tick() function handles sample computation only.  It will be
// called automatically when the system needs a new buffer of audio
// samples.
int tick( void *outputBuffer, void *inputBuffer, unsigned int nBufferFrames,
         double streamTime, RtAudioStreamStatus status, void *userData )
{
  FileWvIn *input = (FileWvIn *) userData;
  StkFloat *samples = (StkFloat *) outputBuffer;

  input->tick( frames );

  for ( unsigned int i=0; i<frames.size(); i++ ) {
    *samples++ = frames[i];
    if ( input->channelsOut() == 1 ) *samples++ = frames[i]; // play mono files in stereo
  }
  
  if ( input->isFinished() ) {
    done = true;
    return 1;
  }
  else
    return 0;
}

int main(int argc, char *argv[])
{
  // Minimal command-line checking.
  if ( argc < 3 || argc > 4 ) usage();

  // Set the global sample rate before creating class instances.
  Stk::setSampleRate( (StkFloat) atof( argv[2] ) );

  // Initialize our WvIn and RtAudio pointers.
  RtAudio dac;
  FileWvIn input;

  // Try to load the soundfile.
  try {
    input.openFile( argv[1] );
  }
  catch ( StkError & ) {
    exit( 1 );
  }

  // Set input read rate based on the default STK sample rate.
  double rate = 1.0;
  rate = input.getFileRate() / Stk::sampleRate();
  if ( argc == 4 ) rate *= atof( argv[3] );
  input.setRate( rate );

  input.ignoreSampleRateChange();

  // Find out how many channels we have.
  int channels = input.channelsOut();

  // Figure out how many bytes in an StkFloat and setup the RtAudio stream.
  RtAudio::StreamParameters parameters;
  parameters.deviceId = dac.getDefaultOutputDevice();
  parameters.nChannels = ( channels == 1 ) ? 2 : channels; //  Play mono files as stereo.
  RtAudioFormat format = ( sizeof(StkFloat) == 8 ) ? RTAUDIO_FLOAT64 : RTAUDIO_FLOAT32;
  unsigned int bufferFrames = RT_BUFFER_SIZE;
  try {
    dac.openStream( &parameters, NULL, format, (unsigned int)Stk::sampleRate(), &bufferFrames, &tick, (void *)&input );
  }
  catch ( RtAudioError &error ) {
    error.printMessage();
    goto cleanup;
  }

  // Install an interrupt handler function.
	(void) signal(SIGINT, finish);

  // Resize the StkFrames object appropriately.
  frames.resize( bufferFrames, channels );

  try {
    dac.startStream();
  }
  catch ( RtAudioError &error ) {
    error.printMessage();
    goto cleanup;
  }

  // Block waiting until callback signals done.
  while ( !done )
    Stk::sleep( 100 );
  
  // By returning a non-zero value in the callback above, the stream
  // is automatically stopped.  But we should still close it.
  try {
    dac.closeStream();
  }
  catch ( RtAudioError &error ) {
    error.printMessage();
  }

 cleanup:
  return 0;
}
