/**************  Effects Program  *********************/

#include "Skini.h"
#include "SKINImsg.h"
#include "Envelope.h"
#include "PRCRev.h"
#include "JCRev.h"
#include "NRev.h"
#include "FreeVerb.h"
#include "Echo.h"
#include "PitShift.h"
#include "LentPitShift.h"
#include "Chorus.h"
#include "Messager.h"
#include "RtAudio.h"

#include <signal.h>
#include <cstring>
#include <iostream>
#include <algorithm>
using std::min;

using namespace stk;

void usage(void) {
  // Error function in case of incorrect command-line argument specifications
  std::cout << "\nuseage: effects flags \n";
  std::cout << "    where flag = -s RATE to specify a sample rate,\n";
  std::cout << "    flag = -ip for realtime SKINI input by pipe\n";
  std::cout << "           (won't work under Win95/98),\n";
  std::cout << "    and flag = -is <port> for realtime SKINI input by socket.\n";
  exit(0);
}

bool done;
static void finish(int ignore){ done = true; }

// The TickData structure holds all the class instances and data that
// are shared by the various processing functions.
struct TickData {
  unsigned int effectId;
  PRCRev   prcrev;
  JCRev    jcrev;
  NRev     nrev;
  FreeVerb frev;
  Echo     echo;
  PitShift shifter;
  LentPitShift lshifter;
  Chorus   chorus;
  Envelope envelope;
  Messager messager;
  Skini::Message message;
  StkFloat lastSample;
  StkFloat t60;
  int counter;
  bool settling;
  bool haveMessage;

  // Default constructor.
  TickData()
    : effectId(0), t60(1.0), counter(0),
      settling( false ), haveMessage( false ) {}
};

#define DELTA_CONTROL_TICKS 64 // default sample frames between control input checks

// The processMessage() function encapsulates the handling of control
// messages.  It can be easily relocated within a program structure
// depending on the desired scheduling scheme.
void processMessage( TickData* data )
{
  register unsigned int value1 = data->message.intValues[0];
  register StkFloat value2 = data->message.floatValues[1];
  register StkFloat temp = value2 * ONE_OVER_128;

  switch( data->message.type ) {

  case __SK_Exit_:
    if ( data->settling == false ) goto settle;
    done = true;
    return;

  case __SK_NoteOn_:
    if ( value2 == 0.0 ) // velocity is zero ... really a NoteOff
      data->envelope.setTarget( 0.0 );
    else // a NoteOn
      data->envelope.setTarget( 1.0 );
    break;

  case __SK_NoteOff_:
    data->envelope.setTarget( 0.0 );
    break;

  case __SK_ControlChange_:
    // Change all effect values so they are "synched" to the interface.
    switch ( value1 ) {

    case 20: { // effect type change
      int type = data->message.intValues[1];
      data->effectId = (unsigned int) type;
      break;
    }

    case 22: // effect parameter change 1
      data->echo.setDelay( (unsigned long) (temp * Stk::sampleRate() * 0.95) );
      data->lshifter.setShift( 1.4 * temp + 0.3 );
      data->shifter.setShift( 1.4 * temp + 0.3 );
      data->chorus.setModFrequency( temp );
      data->prcrev.setT60( temp * 10.0 );
      data->jcrev.setT60( temp * 10.0 );
      data->nrev.setT60( temp * 10.0 );
      data->frev.setDamping( temp );
      break;

    case 23: // effect parameter change 2
      data->chorus.setModDepth( temp * 0.2 );
      data->frev.setRoomSize( temp );
      break;

    case 44: // effect mix
      data->echo.setEffectMix( temp );
      data->shifter.setEffectMix( temp );
      data->lshifter.setEffectMix( temp );
      data->chorus.setEffectMix( temp );
      data->prcrev.setEffectMix( temp );
      data->jcrev.setEffectMix( temp );
      data->nrev.setEffectMix( temp );
      data->frev.setEffectMix( temp );
      break;

    default:
      break;
    }

  } // end of type switch

  data->haveMessage = false;
  return;

 settle:
  // Exit and program change messages are preceeded with a short settling period.
  data->envelope.setTarget( 0.0 );
  data->counter = (int) (0.3 * data->t60 * Stk::sampleRate());
  data->settling = true;
}

// The tick() function handles sample computation and scheduling of
// control updates.  It will be called automatically by RtAudio when
// the system needs a new buffer of audio samples.
int tick( void *outputBuffer, void *inputBuffer, unsigned int nBufferFrames,
         double streamTime, RtAudioStreamStatus status, void *dataPointer )
{
  TickData *data = (TickData *) dataPointer;
  register StkFloat *oSamples = (StkFloat *) outputBuffer, *iSamples = (StkFloat *) inputBuffer;
  register StkFloat sample;
  Effect *effect;
  int i, counter, nTicks = (int) nBufferFrames;

  while ( nTicks > 0 && !done ) {

    if ( !data->haveMessage ) {
      data->messager.popMessage( data->message );
      if ( data->message.type > 0 ) {
        data->counter = (long) (data->message.time * Stk::sampleRate());
        data->haveMessage = true;
      }
      else
        data->counter = DELTA_CONTROL_TICKS;
    }

    counter = min( nTicks, data->counter );
    data->counter -= counter;
    for ( i=0; i<counter; i++ ) {
      if ( data->effectId < 3 ) { // Echo, PitShift and LentPitShift ... mono output
        if ( data->effectId == 0 )
          sample = data->envelope.tick() * data->echo.tick( *iSamples++ );
        else if ( data->effectId == 1 )
          sample = data->envelope.tick() * data->shifter.tick( *iSamples++ );
        else
          sample = data->envelope.tick() * data->lshifter.tick( *iSamples++ );
        *oSamples++ = sample; // two channels interleaved
        *oSamples++ = sample;
      }
      else { // Chorus or a reverb ... stereo output
        if ( data->effectId == 3 ) {
          data->chorus.tick( *iSamples++ );
          effect = (Effect *) &(data->chorus);
        }
        else if ( data->effectId == 4 ) {
          data->prcrev.tick( *iSamples++ );
          effect = (Effect *) &(data->prcrev);
        }
        else if ( data->effectId == 5 ) {
          data->jcrev.tick( *iSamples++ );
          effect = (Effect *) &(data->jcrev);
        }
        else if ( data->effectId == 6 ) {
          data->nrev.tick( *iSamples++ );
          effect = (Effect *) &(data->nrev);
        }
        else {
          data->frev.tick( *iSamples++ );
          effect = (Effect *) &(data->frev);
        }
        const StkFrames& samples = effect->lastFrame();
        *oSamples++ = data->envelope.tick() * samples[0];
        *oSamples++ = data->envelope.lastOut() * samples[1];
      }
      nTicks--;
    }
    if ( nTicks == 0 ) break;

    // Process control messages.
    if ( data->haveMessage ) processMessage( data );
  }

  return 0;
}

int main( int argc, char *argv[] )
{
  TickData data;
  RtAudio adac;
  int i;

  if ( argc < 2 || argc > 6 ) usage();

  // If you want to change the default sample rate (set in Stk.h), do
  // it before instantiating any objects!  If the sample rate is
  // specified in the command line, it will override this setting.
  Stk::setSampleRate( 44100.0 );

  // Parse the command-line arguments.
  unsigned int port = 2001;
  for ( i=1; i<argc; i++ ) {
    if ( !strcmp( argv[i], "-is" ) ) {
      if ( i+1 < argc && argv[i+1][0] != '-' ) port = atoi(argv[++i]);
      data.messager.startSocketInput( port );
    }
    else if (!strcmp( argv[i], "-ip" ) )
      data.messager.startStdInput();
    else if ( !strcmp( argv[i], "-s" ) && ( i+1 < argc ) && argv[i+1][0] != '-')
      Stk::setSampleRate( atoi(argv[++i]) );
    else
      usage();
  }

  // Allocate the adac here.
  RtAudioFormat format = ( sizeof(StkFloat) == 8 ) ? RTAUDIO_FLOAT64 : RTAUDIO_FLOAT32;
  RtAudio::StreamParameters oparameters, iparameters;
  oparameters.deviceId = adac.getDefaultOutputDevice();
  oparameters.nChannels = 2;
  iparameters.deviceId = adac.getDefaultInputDevice();
  iparameters.nChannels = 1;
  unsigned int bufferFrames = RT_BUFFER_SIZE;
  try {
    adac.openStream( &oparameters, &iparameters, format, (unsigned int)Stk::sampleRate(), &bufferFrames, &tick, (void *)&data );
  }
  catch ( RtAudioError& error ) {
    error.printMessage();
    goto cleanup;
  }

  data.envelope.setRate( 0.001 );

  // Install an interrupt handler function.
	(void) signal( SIGINT, finish );

  // If realtime output, set our callback function and start the dac.
  try {
    adac.startStream();
  }
  catch ( RtAudioError &error ) {
    error.printMessage();
    goto cleanup;
  }

  // Setup finished.
  while ( !done ) {
    // Periodically check "done" status.
    Stk::sleep( 50 );
  }

  // Shut down the output stream.
  try {
    adac.closeStream();
  }
  catch ( RtAudioError& error ) {
    error.printMessage();
  }

 cleanup:

	std::cout << "\neffects finished ... goodbye.\n\n";
  return 0;
}
