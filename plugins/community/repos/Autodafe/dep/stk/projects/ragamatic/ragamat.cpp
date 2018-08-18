/**************  Test Main Program Individual Voice *********************/

#include "SKINImsg.h"
#include "Instrmnt.h"
#include "JCRev.h"
#include "Drone.h"
#include "Sitar.h"
#include "Tabla.h"
#include "VoicDrum.h"
#include "Messager.h"
#include "RtAudio.h"

#include <signal.h>
#include <cstring>
#include <iostream>
#include <algorithm>
#include <cstdlib>

using std::min;
using namespace stk;

StkFloat float_random(StkFloat max) // Return random float between 0.0 and max
{	
  StkFloat temp = (StkFloat) (max * rand() / (RAND_MAX + 1.0) );
  return temp;	
}

void usage(void) {
  // Error function in case of incorrect command-line argument specifications.
  std::cout << "\nuseage: ragamat flags \n";
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
  JCRev    reverbs[2];
  Drone    drones[3];
  Sitar    sitar;
  VoicDrum voicDrums;
  Tabla    tabla;
  Messager messager;
  Skini::Message message;
  StkFloat lastSample;
  StkFloat t60;
  int counter;
  bool settling;
  bool haveMessage;
  StkFloat droneChance, noteChance;
  StkFloat drumChance, voiceChance;
  int tempo;
  int chanceCounter;
  int key;
  int ragaStep;
  int ragaPoint;
  int endPhase;
  StkFloat rateScaler;

  // Default constructor.
  TickData()
    : t60(4.0), counter(0),
      settling( false ), haveMessage( false ), droneChance(0.01), noteChance(0.01),
      drumChance(0.0), voiceChance(0.0), tempo(3000), chanceCounter(3000), key(0), ragaPoint(6), endPhase(0) {}
};

// Raga key numbers and drone frequencies.
const int ragaUp[2][13] = {{57, 60, 62, 64, 65, 68, 69, 71, 72, 76, 77, 81},
                           {52, 54, 55, 57, 59, 60, 63, 64, 66, 67, 71, 72}};

const int ragaDown[2][13] = {{57, 60, 62, 64, 65, 67, 69, 71, 72, 76, 79, 81},
                             {48, 52, 53, 55, 57, 59, 60, 64, 66, 68, 70, 72}};

StkFloat droneFreqs[3] = { 55.0, 82.5, 220.0 };

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
    if ( data->endPhase < 5 ) return;
    done = true;
    return;

  case __SK_ControlChange_:

    switch ( value1 ) {

    case 1:
      data->droneChance = temp;
      break;

    case 2:
      data->noteChance = temp;
      break;

    case 4:
      data->voiceChance = temp;
      break;

    case 7:
      data->tempo = (int) (11025 - value2 * 70.0 );
      break;

    case 11:
      data->drumChance = temp;
      break;

    case 64:
      if ( value2 == 0.0 ) {
        data->key = 1;
        droneFreqs[0] = 55.0;
        droneFreqs[1] = 82.5;
        droneFreqs[2] = 220.0;
      }
      else 	{
        data->key = 0;
        droneFreqs[0] = 82.5;
        droneFreqs[1] = 123.5;
        droneFreqs[2] = 330.0;
      }
      break;

    default:
      break;
    }

  } // end of type switch

  data->haveMessage = false;
  return;

 settle:
  // Exit and program change messages are preceeded with a short settling period.
  data->counter = (int) (data->t60 * Stk::sampleRate());
  data->drones[1].noteOn( droneFreqs[1], 0.1 );
  data->settling = true;
  std::cout << "What Need Have I for This?" << std::endl;
}

// The tick() function handles sample computation and scheduling of
// control updates.  It will be called automatically by RtAudio when
// the system needs a new buffer of audio samples.
int tick( void *outputBuffer, void *inputBuffer, unsigned int nBufferFrames,
         double streamTime, RtAudioStreamStatus status, void *dataPointer )
{
  TickData *data = (TickData *) dataPointer;
  register StkFloat temp, outs[2], *samples = (StkFloat *) outputBuffer;
  int i, voiceNote, counter, nTicks = (int) nBufferFrames;

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
      outs[0] = data->reverbs[0].tick( data->drones[0].tick() + data->drones[2].tick()
                                       + data->sitar.tick() );
      outs[1] = data->reverbs[1].tick( 1.5 * data->drones[1].tick() + 0.5 * data->voicDrums.tick()
                                       + 0.5 * data->tabla.tick() );
      // Mix a little left to right and back.
      *samples++ = outs[0] + 0.3 * outs[1];
      *samples++ = outs[1] + 0.3 * outs[0];
      nTicks--;

      // Do a bunch of random controls unless settling down to end.
      if ( data->settling ) {
        if ( data->counter == 0 ) {
          data->counter = (int) (data->t60 * Stk::sampleRate());
          if ( data->endPhase == 0 ) {
            data->drones[2].noteOn( droneFreqs[2], 0.1 );
            std::cout << "What Need Have I for This?" << std::endl;
          }
          else if ( data->endPhase == 1 ) {
            data->drones[0].noteOn( droneFreqs[0], 0.1 );
            std::cout << "RagaMatic finished ... " << std::endl;
          }
          else if ( data->endPhase == 2 ) {
            std::cout << "All is Bliss ... " << std::endl;
          }
          else if ( data->endPhase == 3 ) {
            std::cout << "All is Bliss ..." << std::endl;
          }
          data->endPhase++;
        }
      }
      else {
        data->chanceCounter--;
        if (data->chanceCounter == 0)	{
          data->chanceCounter = (int) ( data->tempo / data->rateScaler );
          if ( float_random(1.0) < data->droneChance )
            data->drones[0].noteOn( droneFreqs[0], 0.1 );
          if ( float_random(1.0) < data->droneChance )
            data->drones[1].noteOn( droneFreqs[1], 0.1 );
          if ( float_random(1.0) < data->droneChance )
            data->drones[2].noteOn( droneFreqs[2], 0.1 );
          if ( float_random(1.0) < data->noteChance ) {
            temp = float_random(1.0);
            if ( temp < 0.1) data->ragaStep = 0;
            else if (temp < 0.5) data->ragaStep = 1;
            else data->ragaStep = -1;
            data->ragaPoint += data->ragaStep;
            if ( data->ragaPoint < 0 ) 
              data->ragaPoint -= ( 2 * data->ragaStep );
            if ( data->ragaPoint > 11 ) data->ragaPoint = 11;
            if ( data->ragaStep > 0 )
              data->sitar.noteOn( Midi2Pitch[ragaUp[data->key][data->ragaPoint]],
                                  0.05 + float_random(0.3) );
            else
              data->sitar.noteOn( Midi2Pitch[ragaDown[data->key][data->ragaPoint]],
                                  0.05 + float_random(0.3) );
          }
          if ( float_random(1.0) < data->voiceChance ) {
            voiceNote = (int) float_random(11);
            data->voicDrums.noteOn( voiceNote, 0.3 + (0.4 * data->drumChance) +
                                    float_random(0.3 * data->voiceChance));
          }
          if ( float_random(1.0) < data->drumChance ) {
            voiceNote = (int) float_random(TABLA_NUMWAVES);
            data->tabla.noteOn( voiceNote, 0.2 + (0.2 * data->drumChance) + 
                                float_random(0.6 * data->drumChance));
          }
        }
      }
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
  RtAudio dac;
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

  // Allocate the dac here.
  RtAudioFormat format = ( sizeof(StkFloat) == 8 ) ? RTAUDIO_FLOAT64 : RTAUDIO_FLOAT32;
  RtAudio::StreamParameters parameters;
  parameters.deviceId = dac.getDefaultOutputDevice();
  parameters.nChannels = 2;
  unsigned int bufferFrames = RT_BUFFER_SIZE;
  try {
    dac.openStream( &parameters, NULL, format, (unsigned int)Stk::sampleRate(), &bufferFrames, &tick, (void *)&data );
  }
  catch ( RtAudioError& error ) {
    error.printMessage();
    goto cleanup;
  }

  data.reverbs[0].setT60( data.t60 );
  data.reverbs[0].setEffectMix( 0.5 );
  data.reverbs[1].setT60( 2.0 );
  data.reverbs[1].setEffectMix( 0.2 );

  data.drones[0].noteOn( droneFreqs[0], 0.1 );
  data.drones[1].noteOn( droneFreqs[1], 0.1 );
  data.drones[2].noteOn( droneFreqs[2], 0.1 );

  data.rateScaler = 22050.0 / Stk::sampleRate();

  // Install an interrupt handler function.
	(void) signal( SIGINT, finish );

  // If realtime output, set our callback function and start the dac.
  try {
    dac.startStream();
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
    dac.closeStream();
  }
  catch ( RtAudioError& error ) {
    error.printMessage();
  }

 cleanup:

  return 0;

}
