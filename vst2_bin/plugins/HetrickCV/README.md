
# HetrickCV for VCV Rack

HetrickCV is a collection of [VCV Rack](https://vcvrack.com/) modules by Michael Hetrick of [Unfiltered Audio](https://www.unfilteredaudio.com/). Many of these are ported from [Euro Reakt for Reaktor 6](https://www.native-instruments.com/en/reaktor-community/reaktor-user-library/entry/show/9093/).

### Releases

HetrickCV is compatible with VCV Rack 0.5.0. Releases can be found on the [Release Page](https://github.com/mhetrick/hetrickcv/releases)

## Contributing

I welcome Issues and Pull Requests to this repository if you have suggestions for improvement.

![HetrickCV](/HetrickCV.PNG)

# HetrickCV User Manual

### 2-to-4 Mix Matrix
This simple module takes in two inputs (CV or Audio). It produces four outputs that are various combinations of the two inputs. It is based on [Julius O. Smith's description of a mix matrix for Schroeder Reverberators](https://ccrma.stanford.edu/~jos/pasp/Schroeder_Reverberators.html). Despite its original use for spatializing audio, this can be a great module for creating CV permutations.

### ASR
This can be thought of as a quad Sample-and-Hold. It is useful for creating melodic rounds and canons. When it receives a positive Clock input, the current voltage at the main input will be sampled and sent to the first output. On the next positive Clock input, the current voltage of the first output will be moved to the second output, and the first output will sample the main input again.

Patch Ideas:
- Connect a sequencer to the main input. Connect the sequencer's clock to the Clock input. Connect the ASR's outputs to various oscillators that are tuned to the same base frequency. Mix the oscillator outputs together and listen to the complex voicings that are created.

### Analog to Digital
This module takes a signal and turns it into an 8-bit representation. The eight outputs represent the state of the bits (+5V if the bit is 1, 0V if the bit is 0). If a signal is present on the Sync input, the bits will only be updated upon the reception of a positive gate. The signal runs through a rectification stage before being sent to the encoder. There are multiple rectification modes:
- Half: Negative signals are replaced with 0V.
- Full: Negative signals are inverted (the absolute value of the signal is used).
- None: The input signal is not affected.

There are also multiple encoding modes:
- Uni. 8: The input is half-wave rectified (in addition to whatever rectification mode was selected before the encoder stage). Bit 8 is the Most Significant Bit (meaning it will take a fairly loud input signal for the bit to go high). In this mode, the encoder will only respond to positive voltages.
- Bi. Off.: The input is converted from a bi-polar signal to a uni-polar signal through the use of an internal offset followed by scaling. The signal is then encoded in a manner similar to Uni. 8, where Bit 8 is the Most Significant Bit.
- Bi. Sig.: In this mode, Bit 7 becomes the Most Significant Bit, while Bit 8 encodes whether or not the signal at the input is negative.

Patch ideas:
- Use a slow LFO as the input. The various outputs become semi-related gate streams. Use primitive waveforms (sine, triangle, etc.) for more predictable results. Use the Audible Instruments Wavetable Oscillator in LFO mode and morph the waveform for unpredictable patterns.
- Use this in tandem with the Digital to Analog module to create custom waveshaping effects. You can wire the bits up "correctly" and simply change the various encoder, rectification, offset, and scaling parameters on both modules to come up with unusual permutations. You can wire the bits up more randomly to produce harsher effects. For a lot of fun, try placing the Rotator and/or the Gate Junction modules between the A-to-D and D-to-A converters.

### Bitshift
This is a harsh waveshaping effect. It is particularly useful for taking a slow, smooth CV value and creating a lot of rapid discontinuities. The effect is produced by taking the internal floating-point representation of the signal and turning it into a 32-bit integer. The integer's bits are then shifted left (<<, which produces aggressive alterations to the signal) or right (>>, which is mostly just attenuation). Because the algorithm for this module has expected boundaries, you will need to select a range for the input signal. +/- 5V is the standard range for most audio generators in Rack. Some function generators will produce +/- 10V, though. Regardless, since this is a fairly harsh and experimental module, there's no need to select the "correct" range...

### Blank Panel
This highly fashionable module allows you to display your love for HetrickCV. You can right-click on this to select alternative panels.

### Boolean Logic
These modules take in 2 or 3 gate inputs and produce 6 gates that represent the true-or-false states of the inputs. The input is considered true if it is currently above 1V (gates do not need to be used, but they provide the most predictable behavior... still, try throwing in all sorts of signals). The various outputs are as follows:
- OR: This output is true if any input is true.
- AND: This output is true if every input is true.
- XOR (Exclusive OR): This output is true if at least one input is true, but not every input.
- NOR: This output is true if every input is false (the opposite of OR).
- NAND: This output is true unless every input is true (the opposite of AND).
- XNOR (Exclusive NOR): This output is true if every input is the same state (the opposite of XOR).

Patch Ideas:
- These are some of the best modules for creating unusual, generative rhythms. Try connecting various outputs of a clock divider into these inputs. Alternatively, connect two clocks or square wave LFOs with different frequencies (for more fun, modulate the pulse widths of the clocks if possible).
- The AND output can be useful to manually toggle rhythm streams. Connect the gate stream that you want to toggle to one input. Connect a MIDI note gate to the other input. Now, the AND output will be the first rhythm as long as you hold down a MIDI note.

### Comparator
This is a tool for comparing one signal against a specific voltage. It can be used for many purposes, including clock extraction or distortion. The Threshold knob sets the voltage that is used for comparison against the main Input. If the Input is greater than the Threshold voltage, the `>` outputs will fire. If the Input is less than the Threshold voltage, then the `<` outputs will fire.

The G outputs are gates and will stay high for as long as the voltage comparison is true. The T outputs are triggers and will stay high for 1 ms. The output between the comparison symbols is a Crossing Trigger, and will fire whenever the signal crosses the threshold. It is essentially the sum of the other two trigger outputs.

Patch Ideas:
- Do you have an LFO that lacks a dedicated square output? Set the Threshold to 0.0 (12 o'clock) and use the LFO as the main Input (not as a modulator for the Threshold value). The `>` Gate will provide a square wave that is synced with the LFO.
- You can use this to turn an audio signal into a 1-bit representation. For extra fun, modulate the Threshold with another audio signal for a lot of destruction.

### Contrast
This is a type of phase distortion that I found in the [CCRMA Snd wave editor](https://ccrma.stanford.edu/software/snd/snd/sndclm.html#contrast-enhancement). It will add brightness and saturation to a signal. Please note that the effect will still color the signal even if the knob is fully counter-clockwise. Like the Bitshift module, there is a range selector to set the expected range of the input signal.

Patch Ideas:
- Aside from the obvious use as a mix enhancer, try modulating the Amount parameter with another audio signal to use this as a creative distortion.

### Crackle
This is a chaotic system that generates a vinyl-like hiss with occasional pops. This is a direct port of [a UGen from SuperCollider](https://github.com/supercollider/supercollider/blob/master/server/plugins/NoiseUGens.cpp#L452). When I originally ported this to Euro Reakt, I accidentally implemented the internal copy operations in the wrong order, leading to the fun "Broken" mode. The Broken mode produces stutters, grains, and modem noises at high Chaos values.

Patch Ideas:
- Surprising things can happen if you modulate this with an audio signal... 

### Delta
This is a tool that extracts a signal's rate of change. It compares a signal's current value against its previous value. The amount of change is multiplied by the Delta Boost parameter and sent to the Delta output. Slow moving signals will have very small rates of change and will need a lot of Boost. Fast, audio-rate signals will need very little boost.

The `>` outputs will go high whenever the signal experiences a positive rate of change. The `<` outputs will go high whenever the signal experiences a negative rate of change. The G outputs are gates that stay high for as long as the signal is moving in that direction. The T outputs are triggers that last for 1 ms. The jack in between the comparison symbols is a trigger output that fires whenever the signal changes direction.

Patch Ideas:
- You can use this to extract a clock trigger from an LFO. It will generate a trigger whenever the LFO changes direction. A sine wave will produce stable, steady triggers. Use a wavetable LFO (like the alternate Sheep mode on Tides) to generate more complex rhythms.

### Digital to Analog
This module is the inverse of the Analog to Digital encoder. It takes in eight inputs and produces a single voltage based off of the state of the inputs and the selected decoder mode. The decoder modes are the inverse of the encoder modes described above in the Analog to Digital documentation. If you directly connect the two modules and use the same encoding/decoding modes, the output is typically identical to the input aside from accuracy degradation from the 8-bit representation.

Patch Ideas:
- See the Analog to Digital documentation above for creative ways of mangling the bits between the modules.
- Connect various, rhythmic gate streams to the inputs. The output is a stepped voltage based on the state of the inputs. This will be a jumpy voltage that is related to various rhythms happening inside of the patch.

### Dust
Like Crackle, this is a direct port of [a SuperCollider Noise UGen](https://github.com/supercollider/supercollider/blob/master/server/plugins/NoiseUGens.cpp#L376). This module will produce randomly spaced impulses with random amplitudes. At low frequencies, this is useful as a random trigger generator. At high frequencies, this is a white noise source.

Patch Ideas:
- Connect the output of Dust to the input of a highly resonant filter. Use a slower frequency on Dust to ping the filter and create sine grains.
- Using the patch above, also link Dust's output to a sequencer's input. Use the sequencer to change the filter's cutoff frequency.

### Exponent
This is a simple waveshaper that will raise the input voltage to a power specified by the Amount knob. Turning the knob clockwise will make the output signal more exponential, while turning the knob counter-clockwise will make the output signal more logarithmic. This will have a mild effect on audio signals, but it is extremely useful for shaping LFOs and envelopes.

### Flip Flop
"Flip Flop" is an engineering/electronics term to describe what is typically a toggle switch with potential conditional behavior. There are two types of Flip Flops on this module: a Flip Flop T (toggle) and Flip Flop D (data).
The Flip Flop T can be thought of as a gate controlled light switch (or, in more modular thinking, a /2 clock divider). A positive gate on the IN T input will turn the FFT output on, and the next gate will turn it off.
The Flip Flop D can be thought of as a sample-and-hold for gates only. A positive gate on the IN T input will sample the input at IN D. If IN D is above 1V, the FFD output will turn on. If IN D is below 1V, the FFD output will turn off.
For convenience, there are additional outputs that provide the opposite state of the FFT and FFD outputs.

### Flip Pan
This is a useful mixing module inspired by [Segue by Nonlinear Circuits](http://nonlinearcircuits.blogspot.com/2015/08/segue-vactrol-version.html) and [MMVCA by WMD](https://www.wmdevices.com/products/multimode-vca). It can function as a mono-to-stereo panner, a stereo-to-stereo "flip" panner, a dual crossfader, a VCA, and more. Despite its flexibility, it maintains a simple control set for experimentation.
The heart of the circuit is simple: The PAN control will pan the Left input on the outputs in the expected left-to-right fashion. However, the twist is that the Right input will pan in the opposite direction.

Patch Ideas:
- Mono-to-stereo panner: Simply route the signal that you want to pan into the Left input.
- Stereo-to-stereo "flip" panner: Route two different signals into the inputs. Now, when manipulating the PAN control, you will hear the signals alternate positions on the outputs.
- Crossfader: Route two different signals into the inputs. The Left output is now a crossfade output that responds to the PAN control. The Right output is also a crossfader, but it responds to the PAN control in the opposite manner.
- VCA: Route a signal into the Left input and monitor the Right output. The PAN control and input now act as a VCA for that signal.

### Gate Junction
This is an eight-channel gate manipulator that was designed to work quickly with the Analog to Digital, Digital to Analog, and Rotator modules. This takes in up to eight gate signals. Each gate can be muted and/or inverted. The inversion behavior does not turn a positive gate negative. Rather, a positive gate will be changed to 0V, while a 0V signal will be changed to +5V. In more technical terms, it is a logic inverter. As an added convenience, the inputs are normalled together. If a cable is not plugged into an input, it will receive the value of the input above it.

### OR Logic (Gate Combiner)
This module can be used to combine many separate gate streams into one gate stream. The OR output is true if any of the inputs are above 1V, the NOR output is true if (and only if) all of the inputs are below 1V. The TRIGS output fires a 1ms trigger when any of the inputs go above 1V.

Patch Ideas:
- Route at least two outputs from the Boolean Logic module into the OR Logic inputs. The TRIGS output will now provide interesting rhythms.

### Random Gates
This is a very useful module that routes an incoming gate to one of up to eight outputs. The output is randomly selected every time the input goes above 1V. The MIN control determines the minimum position of the output gate, while the MAX control determines the maximum position. For instance, with MAX fully clockwise, you can turn up the MIN control to ensure that only outputs 4 through 8 are selected.

There are three modes:
- Triggers: Whenever a positive gate is detected, the randomly selected output will fire a 1ms trigger.
- Hold: Whenever a positive gate is detected, the randomly selected output will stay positive until a new input gate is received.
- Gate: Whenever a positive gate is detected, the randomly selected output will stay positive until the input drops below 1V.

### Rotator
This module was inspired by the [4ms Rotating Clock Divider](http://www.4mspedals.com/rcd.php). Unlike the RCD, this takes up to eight arbitrary inputs and rotates them around the eight outputs. The STAGES control determines how many inputs are used (and repeated), while the ROTATE control is what determines their output positions.

Patch Ideas:
- This module thrives at jumbling gate signals and creating odd rhythms. Try connecting multiple Boolean Logic outputs to the inputs, along with various other clock sources. Modulate the ROTATE parameter with an LFO. For more predictable results, use an LFO that is running at the same speed as one of the clocks.

### Scanner
This module was inspired by the [Toppobrillo Mixiplexer](http://www.toppobrillo.com/mixiplexer.html) and [Make Noise RxMx](http://www.makenoisemusic.com/modules/rxmx). It can be thought of as a smooth, CV-controlled 8-way switch.
The SCAN control determines which input is active. The STAGES control determines how many inputs the SCAN control can reach. The WIDTH control determines how many stages can be active at a time, while the SLOPE control determines how much smoothing occurs between scanned stages. Although it may sound complicated and esoteric, try out the following:

- 8-channel crossfader/mixer: Use up to eight different inputs. Monitor the Mix Out. Use the SCAN control to smoothly crossfade between the eight inputs.
- 8-channel distributor: Plug a modulation signal into the All In input. Manipulate the SCAN control to send that signal to up to eight destinations.
- 8 VCAs: Use up to eight different inputs. Manipulate the SCAN control and monitor the individual outputs.

### Waveshaper
This is a hyperbolic waveshaper, the exact same one used on [Unfiltered Audio's Dent](https://unfilteredaudio.com/products/dent) (the SHAPE control in the top-row distortion). At 12 o'clock, the input signal is unaffected. As you turn the control clockwise, the signal is turned into a square wave. As you turn the contol counter-clockwise, signals are turned into needles. This is easiest to hear by using a sine wave input (you can see it by using the Fundamental Scope module).

Creatively, you could imagine that the shaper acts like a magnet. When turning the control clockwise, you could imagine that magnets are placed on the top and bottom of the waveform. Increasing the control intensifies the magnets, pulling the signal toward the boundaries and creating a square wave. When turning the control counter-clockwise, the magnet is instead placed at 0V. It pulls all but the strongest signals down to silence.

Patch Ideas:
- Use an oscillator or LFO with multiple shape outputs. Route a non-square output into the waveshaper's input. Use any other waveform to modulate the shape amount. This will "unlock" new waveforms for your oscillator. Alternatively, use a different oscillator at a different frequency to modulate the shape amount. This will create a lot of movement.
