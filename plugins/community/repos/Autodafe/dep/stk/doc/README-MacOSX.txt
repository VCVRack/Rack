The Synthesis ToolKit in C++ (STK)

By Perry R. Cook and Gary P. Scavone, 1995--2017.

Please read the file README and INSTALL for more general STK information.

The default realtime support for Macintosh OS X uses the CoreAudio HAL API and is specified during compilation using the __MACOSX_CORE__ preprocessor definition.  There is also support for the JACK audio server using the __UNIX_JACK__ preprocessor definition.

It is necessary to install the OS X developer kit (or the command line tools) in order to compile STK.  STK was successfully tested on the latest version of OS X.

Tcl/Tk on OS X:

I think that tcl/tk interpreter is now included in the XCode package, since I haven't had to download it for several years now.

It appears that socket support in Tcl/Tk on OS X uses the Nagle algorithm, which produces poor response between changes made in the tcl/tk script and the resulting audio updates.  Note that this is only a problem when using a socket connection from a Tcl/Tk script.

