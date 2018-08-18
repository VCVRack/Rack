# The Synthesis ToolKit in C++ (STK)
By Perry R. Cook and Gary P. Scavone, 1995--2017.

This distribution of the Synthesis ToolKit in C++ (STK) contains the following:

* [`include`](include/):  STK class header files
* [`src`](src/): STK class source files
* [`rawwaves`](rawwaves): STK audio files (1-channel, 16-bit, big-endian)
* [`doc`](doc): STK documentation
* [`projects`](projects): example STK projects and programs

Please read the [Legal and Ethical notes](#legal-and-ethical) near the bottom of this document and the [License](LICENSE).

For compiling and installing STK, see the [INSTALL.md](INSTALL.md) file in this directory.

##Contents

* [Overview](#overview)
* [System Requirements](#system-requirements)
* [What's New (and not so new)](#whats-new-and-not-so-new)
* [Disclaimer](#disclaimer)
* [Legal and Ethical](#legal-and-ethical)
* [Further Reading](#further-reading)
* [Perry's Notes From the Original Distribution](#perrys-notes-from-the-original-distribution)

# OVERVIEW

The Synthesis ToolKit in C++ (STK) is a set of open source audio
signal processing and algorithmic synthesis classes written in the C++
programming language.  STK was designed to facilitate rapid
development of music synthesis and audio processing software, with an
emphasis on cross-platform functionality, realtime control, ease of
use, and educational example code.  The Synthesis ToolKit is extremely
portable (most classes are platform-independent C++ code), and it's
completely user-extensible (all source included, no unusual libraries,
and no hidden drivers).  We like to think that this increases the
chances that our programs will still work in another 5-10 years.  STK
currently runs with "realtime" support (audio and MIDI) on Linux,
Macintosh OS X, and Windows computer platforms.  Generic, non-realtime
support has been tested under NeXTStep, Sun, and other platforms and
should work with any standard C++ compiler.

The only classes of the Synthesis ToolKit that are platform-dependent
concern sockets, threads, mutexes, and real-time audio and MIDI input
and output.  The interface for MIDI input and the simple Tcl/Tk
graphical user interfaces (GUIs) provided is the same, so it's easy to
experiment in real time using either the GUIs or MIDI.  The Synthesis
ToolKit can generate simultaneous SND (AU), WAV, AIFF, and MAT-file
output soundfile formats (as well as realtime sound output), so you
can view your results using one of a large variety of sound/signal
analysis tools already available (e.g. Snd, Cool Edit, Matlab).

The Synthesis Toolkit is not one particular program.  Rather, it is a
set of C++ classes that you can use to create your own programs.  A
few example applications are provided to demonstrate some of the ways
to use the classes.  If you have specific needs, you will probably
have to either modify the example programs or write a new program
altogether. Further, the example programs don't have a fancy GUI
wrapper. If you feel the need to have a "drag and drop" graphical
patching GUI, you probably don't want to use the ToolKit. Spending
hundreds of hours making platform-dependent graphics code would go
against one of the fundamental design goals of the ToolKit - platform
independence.

For those instances where a simple GUI with sliders and buttons is
helpful, we use Tcl/Tk (http://dev.scriptics.com) which is freely
distributed for all the supported ToolKit platforms. A number of
Tcl/Tk GUI scripts are distributed with the ToolKit release.  For
control, the Synthesis Toolkit uses raw MIDI (on supported platforms),
and SKINI (Synthesis ToolKit Instrument Network Interface, a MIDI-like
text message synthesis control format).


# SYSTEM REQUIREMENTS

See the individual README's (eg. README-linux) in the /doc directory
for platform specific information and system requirements.  In
general, you will use the configure script to create Makefiles on unix
platforms (and MinGW) or the VC++ workspace files to compile the
example programs.  To use the Tcl/Tk GUIs, you will need Tcl/Tk
version 8.0 or higher.


# WHAT'S NEW (AND NOT SO NEW)

Despite being available in one form or another since 1996, we still
consider STK to be alpha software.  We attempt to maintain backward
compatability but changes are sometimes made in an effort to improve
the overall design or performance of the software.  Please read the
"Release Notes" in the /doc directory to see what has changed since
the last release.

A new StkFrames class has been created to facilitate the handling and
passing of multichannel, vectorized audio data.  All STK classes have
been updated to include tick() functions that accept StkFrames
arguments.

The control message handling scheme has been simplified greatly
through the use of the Messager class.  It is now possible to have
access to simultaneous piped, socketed, and/or MIDI input control
messages.  In most cases, this should eliminate the use of the
Md2Skini program.

Realtime audio input capabilities were added to STK with release 3.0,
though the behavior of such is very hardware dependent.  Under Linux
and Macintosh OS-X, audio input and output are possible with very low
latency.  Using the Windoze DirectSound API, minimum dependable output
sound latency seems to be around 20 milliseconds or so, while input
sound latency is generally higher.  Performance with the ASIO audio
API on Windoze provides much better performance.

As mentioned above, it is possible to record the audio ouput of an STK
program to .snd, .wav, .raw, .aif, and .mat (Matlab MAT-file) output
file types.  Though somewhat obsolete, the program Md2Skini can be
used to write SKINI scorefiles from realtime MIDI input.  Finally, STK
should compile with non-realtime functionality on any platform with a
generic C++ compiler.

For those who wish to make a library from the core STK classes, the
configure script generates a Makefile in the src directory that will
accomplish that.


# DISCLAIMER

You probably already guessed this, but just to be sure, we don't
guarantee anything works.  :-) It's free ... what do you expect?  If
you find a bug, please let us know and we'll try to correct it.  You
can also make suggestions, but again, no guarantees.  Send email to
the mail list.


# LEGAL AND ETHICAL

This software was designed and created to be made publicly available
for free, primarily for academic purposes, so if you use it, pass it
on with this documentation, and for free.

If you make a million dollars with it, it would be nice if you would
share. If you make compositions with it, put us in the program notes.

Some of the concepts are covered by various patents, some known to us
and likely others which are unknown.  Many of the ones known to us are
administered by the Stanford Office of Technology and Licensing.

The good news is that large hunks of the techniques used here are
public domain.  To avoid subtle legal issues, we'll not state what's
freely useable here, but we'll try to note within the various classes
where certain things are likely to be protected by patents.


# FURTHER READING

For complete documentation on this ToolKit, the classes, etc., see the
doc directory of the distribution or surf to
http://ccrma.stanford.edu/software/stk/.  Also check the platform
specific README's for specific system requirements.


# PERRY'S NOTES FROM THE ORIGINAL DISTRIBUTION

This whole world was created with no particular hardware in mind.
These examples are intended to be tutorial in nature, as a platform
for the continuation of my research, and as a possible starting point
for a software synthesis system.  The basic motivation was to create
the necessary unit generators to do the synthesis, processing, and
control that I want to do and teach about.  Little thought for
optimization was given and therefore improvements, especially speed
enhancements, should be possible with these classes.  It was written
with some basic concepts in mind about how to let compilers optimize.

Your question at this point might be, "But Perry, with CMix, CMusic,
CSound, CShells, CMonkeys, etc. already cluttering the landscape, why
a new set of stupid C functions for music synthesis and processing?"
The answers lie below.

1) I needed to port many of the things I've done into something which is generic enough to port further to different machines.

2) I really plan to document this stuff, so that you don't have to be me to figure out what's going on. (I'll probably be sorry I said this in a couple of years, when even I can't figure out what I was thinking.)

3) The classic difficulties most people have in trying to implement physical models are:

    A) They have trouble understanding the papers, and/or in turning the theory into practice.

    B) The Physical Model instruments are a pain to get to oscillate, and coming up with stable and meaningful parameter values is required to get the models to work at all.

This set of C++ unit generators and instruments might help to diminish the scores of emails I get asking what to do with those block diagrams I put in my papers.

4) I wanted to try some new stuff with modal synthesis, and implement some classic FM patches as well.

5) I wanted to reimplement, and newly implement more of the intelligent and physical performer models I've talked about in some of my papers. But I wanted to do it in a portable way, and in such a way that I can hook up modules quickly.  I also wanted to make these instruments connectable to such player objects, so folks like Brad Garton who really think a lot about the players can connect them to my instruments, a lot about which I think.

6) More rationalizations to follow . . .

