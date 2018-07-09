
#include "RtAudio.h"
#include <unistd.h>

int engineGetSampleRate() { return 44100; }

struct StepHandler
{
  virtual ~StepHandler() { };
  virtual int dostep( void *outputBuffer, void *inputBuffer, unsigned int nBufferFrames,
                      double streamTime, RtAudioStreamStatus status ) = 0;

  static int step( void *outputBuffer, void *inputBuffer, unsigned int nBufferFrames,
                   double streamTime, RtAudioStreamStatus status, void* userData )
  {
    StepHandler *sh = (StepHandler *)userData;
    return sh->dostep( outputBuffer, inputBuffer, nBufferFrames, streamTime, status );
  }

  RtAudio startDac()
  {
    RtAudio dac;
    if ( dac.getDeviceCount() < 1 ) {
      std::cout << "\nNo audio devices found!\n";
      exit( 0 );
    }
    
    RtAudio::StreamParameters parameters;
    parameters.deviceId = dac.getDefaultOutputDevice();
    parameters.nChannels = 1;
    parameters.firstChannel = 0;
    
    unsigned int sampleRate = 44100;
    unsigned int bufferFrames = 256; // 256 sample frames

    try {
      dac.openStream( &parameters, NULL, RTAUDIO_FLOAT64,
                      sampleRate, &bufferFrames, &StepHandler::step, (void *)this );
      dac.startStream();
    }
    catch ( RtAudioError& e ) {
      e.printMessage();
      exit( 0 );
    }
    return dac;
  }

  void stopDac( RtAudio dac )
  {
    try {
      // Stop the stream
      dac.stopStream();
    }
    catch (RtAudioError& e) {
      e.printMessage();
    }
    if ( dac.isStreamOpen() ) dac.closeStream();
  }

  int playAudioUntilStepsDone()
  {
    RtAudio dac = startDac();
    
    while( dac.isStreamRunning() )
      {
        usleep( 100 );
      }

    if ( dac.isStreamOpen() ) dac.closeStream();

    return 0;
  }
  
  int playAudioUntilEnterPressed()
  {
    RtAudio dac = startDac();
    
    char input;
    std::cout << "\nPlaying ... press <enter> to quit.\n";
    std::cin.get( input );

    stopDac( dac );
    
    return 0;
  }
};

struct StandaloneModule
{
  struct thing
  {
    float value;
    bool active;
  };

  typedef std::vector< thing > values_t;
  typedef std::vector< values_t > results_t;

  void multiStep( size_t stepCount, results_t &into )
  {
    for( size_t i=0; i<stepCount; ++i )
      {
        step();
        into.push_back( outputs );
      }
  }
  
  values_t params;
  values_t lights;
  values_t inputs;
  values_t outputs;

  StandaloneModule( int nparam, int ninp, int nout, int nlight )
  {
    params.resize( nparam );
    lights.resize( nlight );
    inputs.resize( ninp );
    outputs.resize( nout );
  }

  
  virtual void step() { };
};
