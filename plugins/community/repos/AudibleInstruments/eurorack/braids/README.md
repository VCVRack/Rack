Braids
==
> Braids is a voltage-controlled monophonic digital sound source.
> 
> [more info](http://mutable-instruments.net/modules/braids)


**/bootloader** contains the bootloader. This is a short program describing the first things the module does when it is powered on. It either jumps to the main code, or puts the module in firmware update mode. Think about it: the firmware update code cannot be part of the main firmware – otherwise it would need to erase and rewrite itself during a firmware update!

**/data** contains raw binary data (waveforms and layout of the wavetables). This data is loaded and processed by the python scripts in /resources (more about that later).

**/drivers** are C++ classes that describe how to “talk to” Braids’ hardware – this provides routines for writing a character on the display, send a sample to the DAC, etc. At the exception of this code and the code in braids.cc, all the other .cc / .h files are designed so that they can be compiled and run on both the actual hardware and a desktop computer. This allows me to test/design bits of code without having to send it to the actual hardware (which is slow and doesn’t always allow me to observe and reproduce bugs).

**/hardware_design** are the hardware description files (Eagle board layout and panel drawings).

**/resources** are python scripts that build lookup tables. Some DSP algorithms make use of functions that are expensive to compute on the STM32F (like trigonometric functions, or exponential functions, or even divisions). To cut CPU use, I try to precompute as much stuff as possible on the computer building the code, and just store the results in lookup tables. All the scripts in /resources are run during compilation, and they generate a big file, resources.cc with the pre-computed results. This file also stores data like wavetables.

**/test** is a simple command line program allowing you to run Braids’ synthesis code on your computer. It generates a .wav file with the resulting audio signal, and you can modify the program to change the TIMBRE / COLOR and pitch parameters.

**init.py** is a file required by the python interpreter declaring that the directory or subdirectories contain python modules.

**makefile** describes the steps to compile the files into a binary file for the STM32F (it provides configuration data for the real meaty stuff in stmlib/makefile.inc)

The build process is the following:

- The python scripts are run to generate resources.cc and resources.h.
- All the .cc files in /bootloader are compiled to make a .hex files with the bootloader.
- All the .cc files in / are compiled to make a .hex file with the main code.
- The two hex files are merged together, and that’s what is written to the MCU.


**analog_oscillator** contains DSP code for basic “analog” waveforms (sawtooth, square…)

**braids** is the “top-level” code – what the module does when it starts (hardware initialization), what the module does every 1ms (UI handling), what the module does for every sample (calls into DSP code and DAC writes).

**digital_oscillator** contains DSP code for all the fancy digital oscillator algorithms.

**envelope** is the internal AD envelope generator.

**excitation** comes from Peaks’ code and is used for the 808 style drum synthesis.

**macro_oscillator** contains the oscillator algorithms that are built by gluing together several basic analog type oscillators.

**parameter_interpolation.h** are C macros for smoothing TIMBRE/COLOR/pitch parameters over the duration of a block of 24 samples.

**quantizer** is the built-in quantizer.

**quantizer_scales** are lookup tables with the scales for the quantizer.

**resources** are pre-computed lookup tables.

**settings** contains the range and name of all the settings, along with code for making them persist when the module is powered off/on.

**signature_waveshaper** contains the code for the SIGN feature.

**svf** is a digital state variable filter used in the 808 emulations.

**ui** contains all the menus and encoder-related stuff.

**vco_jitter_source** is the random generator generating VCO-style “sloppiness”.