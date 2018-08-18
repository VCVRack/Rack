/***************************************************/
/*
  Simple realtime MIDI to SKINI parser.

  This object takes MIDI from the input stream
  (via the RtMidi class), parses it, and turns it
  into SKINI messages.

  by Perry R. Cook and Gary P. Scavone, 1995 - 2004.
*/
/***************************************************/

#include "RtMidi.h"
#include "SKINImsg.h"
#include <iostream>
#include <stdlib.h>
#include <stdio.h>

void usage(void) {
  std::cout << "\nuseage: Md2Skini <flag(s)>\n\n";
  std::cout << "   With no arguments, Md2Skini converts MIDI input to SKINI\n";
  std::cout << "   format and sends the output directly to stdout.\n";
  std::cout << "   With flag = -f <filename>, the output stream is simultaneously\n";
  std::cout << "   written to the file specified by the optional <filename>\n";
  std::cout << "   (default = test.ski).\n";
  std::cout << "   With flag = -c, MIDI control change messages will not be\n";
  std::cout << "   converted to SKINI-specific named controls.\n";
  std::cout << "   A MIDI input port can be specified with flag = -p portNumber.\n" << std::endl;
  exit(0);
}

#include <signal.h>
static void finish( int ignore ){ std::cout << "Type 'Exit' to quit." << std::endl; }
bool parseSkiniControl = true;

void midiCallback( double deltatime, std::vector< unsigned char > *bytes, void *userData )
{
  if ( bytes->size() < 2 ) return;

  // Parse the MIDI bytes ... only keep MIDI channel messages.
  if ( bytes->at(0) > 239 ) return;

  register long type = bytes->at(0) & 0xF0;
  register int channel = bytes->at(0) & 0x0F;
  register long databyte1 = bytes->at(1);
  register long databyte2 = 0;
  if ( ( type != 0xC0 ) && ( type != 0xD0 ) ) {
    if ( bytes->size() < 3 ) return;
    databyte2 = bytes->at(2);
  }

  std::string typeName;
  switch( type ) {
  case __SK_NoteOn_:
    if ( databyte2 == 0 ) {
      typeName = "NoteOff\t\t";
      databyte2 = 64;
    }
    else typeName = "NoteOn\t\t";
    break;

  case __SK_NoteOff_:
    typeName = "NoteOff\t\t";
    break;

  case __SK_PolyPressure_:
    typeName = "PolyPressure\t";
    break;

  case __SK_ProgramChange_:
    typeName = "ProgramChange\t";
    break;

  case __SK_ChannelPressure_:
    typeName = "ChannelPressure\t";
    break;

  case __SK_PitchBend_:
    typeName = "PitchBend\t";
    break;

  case __SK_ControlChange_:

    if ( parseSkiniControl != true ) {
      typeName = "ControlChange\t";
      goto output;
    }

    switch( databyte1 ) {
    case __SK_PitchChange_:
      typeName = "PitchChange\t";
      goto output;

    case __SK_Volume_:
      typeName = "Volume\t";
      goto output;

    case __SK_ModWheel_:
      typeName = "ModWheel\t";
      goto output;

    case __SK_Breath_:
      typeName = "Breath\t\t";
      goto output;

    case __SK_FootControl_:
      typeName = "FootControl\t";
      goto output;

    case __SK_Portamento_:
      typeName = "Portamento\t";
      goto output;

    case __SK_Balance_:
      typeName = "Balance\t";
      goto output;

    case __SK_Pan_:
      typeName = "Pan\t\t";
      goto output;

    case __SK_Sustain_:
      typeName = "Sustain\t";
      goto output;

    case __SK_Expression_:
      typeName = "Expression\t";
      goto output;

    default:
      typeName = "ControlChange\t";
      goto output;
    }

  default:
    typeName = "Unknown\t";
  }

 output:

  FILE *file = (FILE *) userData;
  if ( type == 0xC0 || type == 0xD0 || type == 0xE0 ) { // program change, channel pressure, or pitchbend
    fprintf( stdout, "%s  %.3f  %d  %.1f\n", typeName.c_str(), 0.0, channel, (float)databyte1 );
    if ( file != NULL )
      fprintf( file, "%s  %.3f  %d  %.1f\n", typeName.c_str(), deltatime, channel, (float)databyte1 );
  }
  else if ( type == 0xB0 ) { // control change
    if ( typeName == "ControlChange\t" ) {
      fprintf( stdout, "%s  %.3f  %d  %.1f %.1f\n", typeName.c_str(), 0.0, channel, (float)databyte1, (float)databyte2 );
      if ( file != NULL )
        fprintf( file, "%s  %.3f  %d  %.1f %.1f\n", typeName.c_str(), deltatime, channel, (float)databyte1, (float)databyte2 );
    }
    else {
    fprintf( stdout, "%s  %.3f  %d  %.1f\n", typeName.c_str(), 0.0, channel, (float)databyte2 );
    if ( file != NULL )
      fprintf( file, "%s  %.3f  %d  %.1f\n", typeName.c_str(), deltatime, channel, (float)databyte2 );
    }
  }
  else { // noteon, noteoff, aftertouch, and unknown
    fprintf( stdout, "%s  %.3f  %d  %.1f  %.1f\n", typeName.c_str(), 0.0, channel, (float)databyte1, (float)databyte2 );
    if ( file != NULL )
      fprintf( file, "%s  %.3f  %d  %.1f  %.1f\n", typeName.c_str(), deltatime, channel, (float)databyte1, (float)databyte2 );
  }

  fflush( stdout );
}

int main( int argc,char *argv[] )
{
  FILE *file = NULL;
  std::string fileName;
  RtMidiIn *midiin = 0;
  unsigned int port = 0;
  std::string input;

  if ( argc > 5 ) usage();

  // Parse the command-line arguments.
  int i = 1;
  while ( i < argc ) {
    if (argv[i][0] == '-') {
      switch(argv[i][1]) {

      case 'f':
        if ( (i+1 < argc) && argv[i+1][0] != '-' ) {
          i++;
          fileName = argv[i];
          if ( fileName.find( ".ski" ) == std::string::npos ) fileName.append( ".ski" );
        }
        else fileName = "test.ski";
        file = fopen( fileName.c_str(), "wb" );
        break;

      case 'p':
        if ( i++ >= argc) usage();
        port = (unsigned int) atoi( argv[i] );
        break;

      case 'c':
        parseSkiniControl = false;
        break;
          
      default:
        usage();
        break;
      }
    }
    else usage();
    i++;
  }

  try {
    midiin = new RtMidiIn();
  }
  catch (RtMidiError &error) {
    error.printMessage();
    if ( file != NULL ) fclose( file );
    exit(EXIT_FAILURE);
  }

  // Check available ports vs. specified.
  unsigned int nPorts = midiin->getPortCount();
  if ( nPorts == 0 ) {
    std::cout << "No MIDI ports available!\n";
    goto cleanup;
  }
  else if ( port >= nPorts ) {
    std::cout << "Invalid port specifier!\n";
    goto cleanup;
  }

  // Open the port.
  try {
    midiin->openPort( port );
  }
  catch (RtMidiError &error) {
    error.printMessage();
    goto cleanup;
  }

  // Set our callback function.  This should be done immediately after
  // opening the port to avoid having incoming messages written to the
  // queue instead of sent to the callback function.
  midiin->setCallback( &midiCallback, file );

  // We'll ignore sysex, timing, and active sensing messages.
  midiin->ignoreTypes( true, true, true );

  // Install an interrupt handler function.
  (void) signal(SIGINT, finish);

  std::cout << "\nReading MIDI input ... type 'Exit' to quit.\n";
  while ( input != "Exit" && input != "exit" ) {
    input.erase();
    std::cin >> input;
    std::cout << input << std::endl;
  }

 cleanup:
  delete midiin;
  if ( file != NULL ) fclose( file );

  std::cout << "Md2Skini finished ... bye!" << std::endl;
  return 0;
}
