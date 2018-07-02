# MPE to CV modules

These are VCVRack modules designed for using "[MIDI Polyphonic Expression](https://www.midi.org/articles/midi-polyphonic-expression-mpe)" (MPE) controllers such as the Haken Continuum, LinnStrument, Madrona Labs Soundplane and ROLI devices with VCV Rack.

We currently have a monophonic and Quad MPE to CV modules with several outputs and configuration options designed to adapt to different MPE controllers. Over time we'll add some modules exploring the digital'ness and advantages of running a virtual modular :)

### New Quad module

I have just added a new Quad module. It is a bit more robust than the previous monophonic and now retains the settings between sessions (will fix that on the mono).

![SCREENSHOT](doc/Quad_faceplate.png)
![SCREENSHOT](doc/Quad_menu.png)


## 1. Installation 

You can find the latest release here: https://github.com/bafonso/Erratic/releases
Just download for you architecture and unzip in your _**Rack**_ folder located in your documents.


## 2. Outputs and details

1V/oct: Note you play combined with 14bit pitchwheel information. Adjust the pitch bend range (in semitones) to match your controller. Note that you can "slide" over 96 semitones using the current standard!

Gate: Note On/Off 

Velocity: 0-10V representation of MIDI Note On velocity. (Could potentially implement Note Off velocity from ROLI in the future)

Pressure: 0-10V representation of pressure or channel aftertouch. 

Y axis: 0-10V representation of CC 74 (7 bit resolution) on the selected channel. 

Pedal: 0-10V representation of CC 12 on global channel (1 or 16). 

## MPE and MPE+
The Haken Continuum supports 14bit MPE for both pressure and Y control by using two 7bit CC (MSB and LSB). Let me know if your other controllers have this and we need to support different combinations of CC #s.

See also http://www.rogerlinndesign.com/mpe.html

# Donations

[Donate through Paypal](https://paypal.me/ErraticModules)
