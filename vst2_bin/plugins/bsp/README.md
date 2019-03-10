
# AttenuMixer

A compact attenuator and mixer (/merger), mainly designed for control voltages.

The scale knobs (0..1 in unipolar mode) have a bias towards small values (subtle modulation).

The default scaling factor of input 2 is calibrated to +-24 semitones (e.g. MPE pitchbend).

Inputs 3 and 4 use default scaling factors of 0.5 and 0.25, respectively.

The right switch at the top is used to enabled bipolar scaling (-1..1).

The left switch enables fine offset scaling (e.g. for precise detuning).


Suggested applications:
  - Mix pitch voltages, e.g. base pitch + pitchbend + vibrato.
  - Mix filter cutoff voltages, e.g. ADSR + LFO + modwheel
  - Mix audio signals



# Bias

Rescales incoming signals.

Values below the "CTR" point are scaled by the "NEG" param, values above the "CTR" point are scaled by the "POS" param.

Suggested application: Filter keyboard tracking.

Example:
- Connect MIDI-1 CV output to the input
- Connect the output to a filter's frequency input
- Adjust center and scale values to taste

NOTE: This can also be used as a simple asymmetric waveshaper for audio signals

NOTE: For use as an amplifier, set "CTR" to -10, then use "POS" to set the amplification (-4..4) (negative values flip the phase)



# DownSampler

Records audio into a ringbuffer (~10 seconds) and plays it back at a lower sample rate (/1../8).

The trigger input resets the record and play heads.

NOTE: this used to be a sampler trick that was especially popular in 90ies dub techno music (e.g. downsampled chord stabs).

NOTE: the module makes absolutely no attempt to hide / filter out the aliased frequencies. feel free to pre-process the input with a steep lowpass filter.

NOTE: here's an example video: https://vimeo.com/288968750



# Legato

Meant for legato-slides, this module applies a slew filter to the incoming (V/oct) signal.

Two parameter sets are used to configure the slide speed
1) when a new note is triggered ("min")
2) when the next note is played while the previous note key is still held down (i.e. no new trigger) ("max")

- Connect the original V/oct signal to the "I" input
- Connect the trigger (gate) to the "T" input. 
- The "R" knob (rate) determines the interpolation speed between the min/max parameter sets. The speed can be modulated via the "M" input. Whe a new note is triggered, the interpolation is reset to 0.
- The "min" and "max" knobs are used to adjust the rise and fall rates



# Obxd_VCF

An adaption of Filatov Vadim's excellent Ob-Xd filter. Distributed under terms of the GNU General Public License V3.



# Rescaler

Clips the input signal to the "IN" min/max range (min=upper knob, max=lower knob), normalizes it, and scales it to the "OUT" min/max range.

If the lower scale input jack is connected, the output value is scaled by the scale input's current value.

Suggest application: Finetune velocity responses.

Example:
- Connect the MIDI-1 velocity output to the main input
- Connect the output of an envelope generator to the scale input
- Connect the module's output to the frequency modulation input of a filter
- Adjust the min/max knobs to taste

NOTE: This module can also be used as a clipper


# RMS

A Root-Mean-Square based envelope follower, coupled with a slew limiter.

The rise and fall rates can be configured separately.

The module can be used to derive envelopes from audio signals, e.g. to implement compressor effects.



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
