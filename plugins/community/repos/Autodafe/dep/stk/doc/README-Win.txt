The Synthesis ToolKit in C++ (STK)

By Perry R. Cook and Gary P. Scavone, 1995--2017.

Please read the file README for more general STK information.

The configure script supports MinGW.  As well, STK is distributed with Visual C++ .NET project and workspace files (though these may no longer work with current versions of Visual Studio).  It no longer compiles with Visual C++ 6.0.

With Windows XP/7, piping works as under unix.  Simply fire up the script files (ex. StkDemo.bat) by either double-clicking on them or from within a shell.

IMPORTANT VC++ NOTE: When compiling "release" versions of STK programs, link to the release multithreaded library.  When compiling "debug" versions, link to the debug multithreaded library.  Compiler errors will result otherwise.

The DirectSound, WASAPI and Steinberg ASIO audio APIs are supported for realtime audio input/output.  The Visual C++ project files included with this distribution are configured to use all supported APIs.  In order to use the ASIO API, it is necessary to use the preprocessor definition __WINDOWS_ASIO__, as well as include most of the files in the /src/include/ directory (i.e. asio.h, asio.cpp, ...).  If you have a good quality soundcard and a native ASIO driver (not emulated), you are likely to get much better input/output response using that.

When using the DirectSound API for audio input, latency can be high.  If you experience realtime audio "stuttering", you should experiment with different "buffer size" and "number of buffers" values.

Realtime MIDI input/output is supported by RtMidi using the winmm.lib API and requires the __WINDOWS_MM__ preprocessor definition.

Visual C++ workspaces have been created for the various STK projects.  Everything has already been configured for you.  The intermediate .obj files will be written to either the "Release" or "Debug" directories, but the executable files will be written to the main project directories (where they need to be for proper execution).  If you should somehow lose or hose the VC++ workspace file for a project, then you will have to do a LOT of configuring to recreate it ... it's probably easier just to download the distribution again from our WWW sites.  Anyway, for your benefit and mine, here is a list of things that need to be added to the various "Project Settings" (this was for VC 6.0 ... things have changed with the newer versions of the VC compiler):

1. Under General: Set "Output files:" to <blank> (this will put the executable in the main project directory.

2. Under C/C++ > Code Generation: Set "Use run-time library:" to Multithreaded (use "debug" versions for the debug configuration).

3. Under Link > General:  Add winmm.lib, dsound.lib, and Wsock32.lib to the end of the Object/library modules list.

4. Under C/C++ > Preprocessor: Add "../../include" directory to the "extra include" field.

5. Under C/C++ > Preprocessor: Add "__WINDOWS_DS__", "__WINDOWS_MM__", and "__LITTLE_ENDIAN__ to the definitions field.

6. Add all the necessary files to the project.

Remember that items 1-5 above need to be done for each project and for each configuration.  There might be an easy way to make global changes, but I couldn't figure it out.

To use the Tcl/Tk GUIs, you will have to install Tcl/Tk.
