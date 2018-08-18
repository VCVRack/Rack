% The Synthesis ToolKit in C++ (STK)
% Perry R. Cook and Gary P. Scavone
% 1995--2017

# The Synthesis ToolKit in C++ (STK)
By Perry R. Cook and Gary P. Scavone, 1995--2017.

The Synthesis ToolKit in C++ can be used in a variety of ways, depending on your particular needs.  Some people simply choose the classes they need for a particular project and copy those to their project directory.  Others like to compile and link to a library of object files.  STK was not designed with one particular style of use in mind.

## Unix systems and MinGW

1. If you downloaded the software from the git repository, first run autoconf,

        autoconf

otherwise, if you downloaded the software from the STK source distribution, unpack the tar file,

        tar -xzf stk-4.x.x.tar.gz

2. From within the directory containing this file, run configure:

        ./configure

3. From within each project directory, type `make`.

4. To compile a library of objects, type `make` from within the `src` directory.

Several options can be passed to configure, including:

    --disable-realtime = only compile generic non-realtime classes
    --enable-debug = enable various debug output
    --with-alsa = choose native ALSA API support (default, linux only)
    --with-oss = choose native OSS API support (unixes only)
    --with-jack = choose native JACK server API support (linux and macintosh OS-X)
    --with-core = choose OS-X Core Audio API (macintosh OS-X only)
    --with-asio = choose ASIO API support (windows only)
    --with-ds = choose DirectSound API support (windows only)
    --with-wasapi = choose Windows Audio Session API support (windows only)

It is now possible to specify more than one audio and MIDI API where supported.  Note, however, that the ALSA library is required in order to compile the RtMidi class in Linux if the `--with-oss` option is provided (only the OSS audio API will be used, not the OSS MIDI API).  Typing `./configure --help` will display all the available options.  In addition, it is possible to specify the RAWWAVES and INCLUDE paths to configure as (ex. to set to /home/me/rawwaves and /home/me/include):

    ./configure RAWWAVE_PATH='$(HOME)/rawwaves/'
    ./configure INCLUDE_PATH='$(HOME)/include/'

The ending "/" is required for the RAWWAVES path.  The default behavior will set a relative path that works for the project files included with the distribution (assuming they are not moved).  You can also change the RAWWAVE_PATH dynamically via the static Stk::setRawwavePath() function.

If you wish to use a different compiler than that selected by configure, specify that compiler in the command line (ex. to use CC):

    ./configure CXX=CC


## Windows

MinGW support is provided in the configure script.  In addition, Visual C++ 6.0 project files are included for each of the example STK projects, though these may not work with more recent versions of Visual Studio.

##iOS

You can integrate the STK in iOS projects either by using its iOS static library or Cocoapods. See the [iOS README file](iOS/README-iOS.md) for instructions. 
