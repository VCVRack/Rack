The Synthesis ToolKit in C++ (STK)

By Perry R. Cook and Gary P. Scavone, 1995--2017.

Please read the file README and INSTALL for more general STK information.

Realtime audio support for Linux currently includes the Advanced Linux Sound Architecture (ALSA), the JACK low-latency audio server, and/or Open Sound System (OSS version 4.0 and higher only) APIs.  That said, the OSS API support has not been tested in several years and is not considered a high priority.  One or more APIs are selected during compilation using the __LINUX_ALSA__, __UNIX_JACK__, and/or __LINUX_OSS__ definitions.  Because the ALSA library is now integrated into the standard Linux kernel, it is the default audio/MIDI API with STK versions 4.2 and higher.

Realtime MIDI support Linux currently includes the Jack and ALSA sequencer support.  Native OSS MIDI support no longer exists in RtMidi.  If the __LINUX_OSS__ preprocessor definition is specified, only OSS audio support will be compiled and RtMidi will still be compiled using the ALSA API.  For this reason, STK now requires the asound library for realtime support (unless only using the Jack API).  Realtime programs must also link with the pthread library.

STK should compile without much trouble under Linux.  Since all Linux distributions typically include the GNU makefile utilities, you should be able to use the default Makefiles.  Typing "make" in a project directory will initiate the compilation process (after initially running the configure script in the top-level directory).


