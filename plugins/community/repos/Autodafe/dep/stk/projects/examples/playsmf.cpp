// playsmf.cpp
//
// Simple program to test the MidiFileIn class by reading and playing
// a single track from a given Standard MIDI file.
//
// by Gary Scavone, 2003.

#include "MidiFileIn.h"
#include "RtMidi.h"
#include <signal.h>
#include <cstdlib>

bool done = false;
static void finish(int ignore){ done = true; }

using namespace stk;

void usage(void) {
  // Error function in case of incorrect command-line
  // argument specifications.
  std::cout << "\nusage: playsmf file track <port>\n";
  std::cout << "   where file = a standard MIDI file,\n";
  std::cout << "   track = the track to play (0 = 1st track),\n";
  std::cout << "   and an optional port integer identifier can be specified\n";
  std::cout << "   (default = 0) or a value of -1 to use a virtual MIDI output port.\n\n";
  exit( 0 );
}

int main( int argc, char *argv[] )
{
  RtMidiOut *midiout = 0;

  if ( argc < 3 || argc > 4 ) usage();

  // Attempt to instantiate MIDI output class.
  try {
    midiout = new RtMidiOut();
  }
  catch ( RtMidiError& error ) {
    error.printMessage();
    exit(0);
  }

  // Check command-line arguments.
  int port = 0;
  if ( argc == 4 ) port = atoi( argv[3] );
  if ( port == -1 ) {
    try {
      midiout->openVirtualPort();
    }
    catch ( RtMidiError& error ) {
      error.printMessage();
      goto cleanup;
    }
    std::cout << "\nVirtual port open.\n\n";
  }
  else {
    if ( midiout->getPortCount() < 1 ) {
      std::cout << "\nThere are no MIDI output destinations available!\n\n";
      goto cleanup;
    }
    try {
      midiout->openPort( port );
    }
    catch ( RtMidiError& error ) {
      error.printMessage();
      goto cleanup;
    }
  }

  // Install an interrupt handler function.  Type "ctrl-c" to quit the
  // program.
  (void) signal( SIGINT, finish );
  
  try {
    MidiFileIn midiFile( argv[1] );

    // Print a little information about the file.
    std::cout << "\nThe MIDI file (" << argv[1] << ") information:\n";
    std::cout << "  - format = " << midiFile.getFileFormat() << "\n";
    std::cout << "  - tracks = " << midiFile.getNumberOfTracks() << "\n";
    std::cout << "  - seconds / ticks = " << midiFile.getTickSeconds() << "\n";

    unsigned int track = (unsigned int) atoi( argv[2] );
    if ( midiFile.getNumberOfTracks() <= track ) {
      std::cout << "\nInvalid track number ... playing track 0.\n";
      track = 0;
    }

    std::cout << "\nPress <enter> to start reading/playing.\n";
    char input;
    std::cin.get(input);
    
    std::vector<unsigned char> event;
    unsigned long ticks = midiFile.getNextMidiEvent( &event, track );
    while ( !done && event.size() ) {

      // Pause for the MIDI event delta time.
      Stk::sleep( (unsigned long) (ticks * midiFile.getTickSeconds() * 1000 ) );

      midiout->sendMessage( &event );

      // Get a new event.
      ticks = midiFile.getNextMidiEvent( &event, track );
    }

    // Send a "all notes off" to the synthesizer.
    event.clear();
    event.push_back( 0xb0 );
    event.push_back( 0x7b );
    event.push_back( 0x0 );
    midiout->sendMessage( &event );
  }
  catch ( StkError & ) {
    // You might want to do something more useful here.
    std::cout << "\nAborting program!\n";
    goto cleanup;
  }

 cleanup:
  delete midiout;

  return 0;
}
