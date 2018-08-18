// midiprobe.cpp
//
// Simple program to check MIDI inputs and outputs.
//
// by Gary Scavone, 2003-2004.

#include <iostream>
#include <cstdlib>
#include "RtMidi.h"

int main()
{
  RtMidiIn  *midiin = 0;
  RtMidiOut *midiout = 0;

  // RtMidiIn constructor
  try {
    midiin = new RtMidiIn();
  }
  catch ( RtMidiError &error ) {
    error.printMessage();
    exit( EXIT_FAILURE );
  }

  // Check inputs.
  unsigned int nPorts = midiin->getPortCount();
  std::cout << "\nThere are " << nPorts << " MIDI input sources available.\n";
  std::string portName;
  unsigned int i;
  for ( i=0; i<nPorts; i++ ) {
    try {
      portName = midiin->getPortName(i);
    }
    catch ( RtMidiError &error ) {
      error.printMessage();
      goto cleanup;
    }
    std::cout << "  Input Port #" << i+1 << ": " << portName << '\n';
  }

  // RtMidiOut constructor
  try {
    midiout = new RtMidiOut();
  }
  catch ( RtMidiError &error ) {
    error.printMessage();
    exit( EXIT_FAILURE );
  }

  // Check outputs.
  nPorts = midiout->getPortCount();
  std::cout << "\nThere are " << nPorts << " MIDI output ports available.\n";
  for ( i=0; i<nPorts; i++ ) {
    try {
      portName = midiout->getPortName(i);
    }
    catch ( RtMidiError &error ) {
      error.printMessage();
      goto cleanup;
    }
    std::cout << "  Output Port #" << i+1 << ": " << portName << '\n';
  }
  std::cout << '\n';

  // Clean up
 cleanup:
  delete midiin;
  delete midiout;

  return 0;
}
