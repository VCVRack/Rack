// grains.cpp
//
// A simple test program for the STK Granulate class.

#include "Granulate.h"
#include "RtAudio.h"
#include <cstdlib>

using namespace stk;

// This tick() function handles sample computation only.  It will be
// called automatically when the system needs a new buffer of audio
// samples.
int tick( void *outputBuffer, void *inputBuffer, unsigned int nBufferFrames,
         double streamTime, RtAudioStreamStatus status, void *dataPointer )
{
  Granulate *grani = (Granulate *) dataPointer;
  register StkFloat *samples = (StkFloat *) outputBuffer;
  const StkFrames& lastframe = grani->lastFrame();
  unsigned int nChannels = lastframe.channels();

  unsigned int j;
  for ( unsigned int i=0; i<nBufferFrames; i++ ) {
    grani->tick();
    for ( j=0; j<nChannels; j++ )
      *samples++ = lastframe[j];
  }

  return 0;
}

void usage( void ) {
  // Error function in case of incorrect command-line
  // argument specifications.
  std::cout << "\nuseage: grains file N dur ramp offset delay stretch ramdomness\n";
  std::cout << "    where file = a soundfile to granulate,\n";
  std::cout << "    N = the number of grain voices to use,\n";
  std::cout << "    dur = the grain duration (ms),\n";
  std::cout << "    ramp = the envelope percent (0-100),\n";
  std::cout << "    offset = hop time between grains (ms),\n";
  std::cout << "    delay = pause time between grains (ms),\n";
  std::cout << "    stretch = stetch factor (1-1000),\n";
  std::cout << "    and randomness = factor between 0 - 1.0 to control grain parameter randomness.\n\n";
  exit( 0 );
}

int main( int argc, char *argv[] )
{
  // Minimal command-line checking.
  if (argc != 9) usage();
  unsigned int N = (unsigned int) atoi(argv[2]);
  unsigned int duration = (unsigned int) atoi(argv[3]);
  unsigned int ramp = (unsigned int) atoi(argv[4]);
  unsigned int offset = (unsigned int) atoi(argv[5]);
  unsigned int delay = (unsigned int) atoi(argv[6]);
  unsigned int stretch = (unsigned int) atoi(argv[7]);
  StkFloat random = (StkFloat) atof(argv[8]);

  // Set the global sample rate before creating class instances.
  Stk::setSampleRate( 44100.0 );

  RtAudio dac;
  Granulate grani;
  grani.setRandomFactor( random );
  grani.setStretch( stretch );
  grani.setGrainParameters( duration, ramp, offset, delay );

  try {
    grani.openFile( argv[1] );
  }
  catch ( StkError& ) {
    exit( 1 );
  }
  grani.setVoices( N );

  // Figure out how many bytes in an StkFloat and setup the RtAudio stream.
  RtAudio::StreamParameters parameters;
  parameters.deviceId = dac.getDefaultOutputDevice();
  parameters.nChannels = grani.channelsOut();
  RtAudioFormat format = ( sizeof(StkFloat) == 8 ) ? RTAUDIO_FLOAT64 : RTAUDIO_FLOAT32;
  unsigned int bufferFrames = RT_BUFFER_SIZE;
  try {
    dac.openStream( &parameters, NULL, format, (unsigned int)Stk::sampleRate(), &bufferFrames, &tick, (void *)&grani );
  }
  catch ( RtAudioError &error ) {
    error.printMessage();
    goto cleanup;
  }

  try {
    dac.startStream();
  }
  catch ( RtAudioError &error ) {
    error.printMessage();
    goto cleanup;
  }


  // Block waiting here.
  char keyhit;
  std::cout << "\nPlaying ... press <enter> to quit.\n";
  std::cin.get( keyhit );

  // Shut down the callback and output stream.
  try {
    dac.closeStream();
  }
  catch ( RtAudioError &error ) {
    error.printMessage();
  }

 cleanup:

  return 0;
}
