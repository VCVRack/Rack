/******************************************/
/*
  Example program to output N channels of audio
  data over a network socket connection.

  by Gary P. Scavone, 2000

  This program will load a specified WAV, SND, AIFF, STK RAW, or
  MAT-file formatted file.  The output data format is set for signed
  16-bit integers.  However, it is easy to change the InetWvOut
  setting to any of the other defined StkFormats.  If using tcpIn, it
  will be necessary to change the expected data format there as well.

  The class InetWvOut first attempts to establish a socket connection
  to a socket server running on port 2006.  Therefore, this program
  needs to be started AFTER the streaming server.
*/
/******************************************/

#include "FileWvIn.h"
#include "InetWvOut.h"
#include <cstdlib>

using namespace stk;

void usage(void) {
  // Error function in case of incorrect command-line
  // argument specifications.
  std::cout << "\nuseage: inetOut file host <rate>\n";
  std::cout << "    where file = the file to load,\n";
  std::cout << "    host = the hostname where the receiving\n";
  std::cout << "           application is running.\n";
  std::cout << "    and rate = an optional playback rate for the file.\n";
  std::cout << "               (default = 1.0, can be negative)\n\n";
  exit( 0 );
}

int main( int argc, char *argv[] )
{
  // Minimal command-line checking.
  if ( argc < 3 || argc > 4 ) usage();

  FileWvIn input;
  InetWvOut output;

  // Load the file.
  try {
    input.openFile( (char *)argv[1] );
  }
  catch ( StkError & ) {
    exit( 1 );
  }

  // Set the global STK sample rate to the file rate.
  Stk::setSampleRate( input.getFileRate() );

  // Set input read rate.
  double rate = 1.0;
  if ( argc == 4 ) rate = atof( argv[3] );
  input.setRate( rate );

  // Find out how many channels we have.
  int channels = input.channelsOut();
  StkFrames frames( 4096, channels );

  // Attempt to connect to the socket server.
  try {
    //output.connect( 2006, Socket::PROTO_UDP, (char *)argv[2], channels, Stk::STK_SINT16 );
    output.connect( 2006, Socket::PROTO_TCP, (char *)argv[2], channels, Stk::STK_SINT16 );
  }
  catch ( StkError & ) {
    exit( 1 );
  }

  // Here's the runtime loop
  while ( !input.isFinished() )
    output.tick( input.tick( frames ) );

  return 0;
}
