The Synthesis ToolKit in C++ (STK)

By Perry R. Cook and Gary P. Scavone, 1995--2017.

EFFECTS PROJECT:

This directory contains a program that demonstrates realtime duplex
mode (simultaneous audio input and output) operation, as well as
several simple delay-line based effects algorithms.  Duplex mode
operation is very hardware dependent.  If you have trouble with this
application, make sure your soundcard supports the desired sample rate
and sample size (16-bit).

NOTES: 

1. This project will not run under WindowsNT or NeXTStep, due to lack
   of realtime audio input support.  However, it should run under
   other flavors of Windows.

2. Audio input from either a microphone or line-input device MUST be
   available to the audio input port when the program is started.

3. Latency can be controlled using the nBufferFrames argument to the
   RtAudio openStream() function.  The default settings in effects.cpp
   are relatively high because some Windows soundcard drivers crash if
   the settings are too low.
