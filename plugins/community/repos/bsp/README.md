
# AttenuMixer

A compact attenuator and mixer (/merger), mainly designed for control voltages.

The scale knobs (0..1 in unipolar mode) have a bias towards small values (subtle modulation).

The default scaling factor of input 2 is calibrated to +-24 semitones (e.g. MPE pitchbend).

Inputs 3 and 4 use default scaling factors of 0.5 and 0.25, respectively.

The switch at the top is used to enabled bipolar scaling (-1..1).


Suggested applications:
  - Mix pitch voltages, e.g. base pitch + pitchbend + vibrato.
  - Mix filter cutoff voltages, e.g. ADSR + LFO + modwheel



# Obxd_VCF

An adaption of Filatov Vadim's excellent Ob-Xd filter. Distributed under terms of the GNU General Public License V3.


# Scanner

A mixer that can seamlessly blend up to 16 input channels.

"pos" sets the center position

"pmod" sets the position modulation amount

"shape" selects the input window (when multiple inputs are mixed together). It goes from narrow sine to wide sine, to triangle, then to various pulse shapes.

"width" selects the number of neighbouring inputs (around the center position).

The post section is activated by pluggin a cable into the post output.
It's basically an experiment where I wanted to hear how the shape window sounds when applied to audio data.
I left it in since it turned out to be useful for synthesizing cymbal and hihat sounds (among other things).
The knob selects the window shape (same as the main shape parameter), and the switch toggles a window offset
(this used to be a bug in earlier versions but it sounded nice with some sounds).

The "RND" section (right above the output port) can be used to shuffle / randomize the inputs.
The switch enables the randomizer, and the button next to it is used to generate a new random seed.

NOTE: try modulating the position with the post output (feedback).


# Sway

A kind of slew-filtered noise generator, mainly designed for randomizing control voltages.

The "time" knobs select the minimum and maximum time (up to 60sec).

The "amp" knobs select the minimum and maximum amplification (-1..1).

The "s+o" knobs are used to apply a final scaling/amplification (-5..5) and offset (-5..5) to the output signal.

NOTE: when the min/max time is set to very small values, the module can be used to generate audio-rate noise.


# Tuned Delay Line

This module was designed for Karplus-Strong synthesis.
If you don't know what this is: The basic idea is to feed short noise bursts into a feedback delay (to "excite the string").

The frequency (V/Oct) input at the top controls the delay length. The knob below can be used for finetuning (+- 1 semitone).

The next two ports are the feedback send and return. They are usually hooked up to a filter module.
The knob controls the feedback amount (usually set to very high values to create sustained sounds).

If the return jack is left unconnected, a simple builtin filter is used instead.

The knob at the bottom controls the dry/wet amount (usually set to 100% wet).

Last but not least, the last two ports are for the audio input, and the audio output.


NOTE: make sure to only input very short noise bursts or the output signal will become far too loud very quickly (because of the high feedback amount). One way to do that is to feed the oscillator/noise signal into an AS.KillGate module which is triggered by a pulse oscillator (~C-4).

NOTE: getting usable sounds out of this module requires a lot of finetuning. The AttenuMixer can be very handy for this.

NOTE: Here's a [video](https://vimeo.com/287875320) with some example sounds / patches.


# Known Issues

The graphics, especially the texts, look really bad. I currently have no idea to fix that.
It seems to be an issue with Inkscape, the nanosvg/nanovg SVG loader/renderer, my lack of experience with Inkscape, or all of the above.
