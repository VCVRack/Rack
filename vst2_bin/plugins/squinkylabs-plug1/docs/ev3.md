# EV3 triple VCO

![ev3 image](./ev3-panel.png)

## About EV3

EV3 is made from the sound generating part of Befaco’s EvenVCO, replicated three times. dWe added sync, which was never implemented in the EvenVCO. Then we did our usual overhaul to reduce CPU usage.

The result is a module containing three VCOs that together use about the same amount of CPU as a single instance of EvenVCO.

While the waveform generation and alias suppression is lifted directly from EvenVCO, the control selection and response of EV3 is our own design. While the three VCOs are easiest to use together, they may be used completely independently.

EvenVCO remains an excellent choice for a VCO, but there are definitely some cases where you might choose EV3 instead:

* It is very easy to make giant stacked oscillator patches, like we did in the old days.

* One EV3  uses less panel space and CPU than three separate instances of EvenVCO.

* The sync feature is a welcome addition.

* The semitone pitch offset can inspire patches and harmonies that you might not find with the standard controls on most VCOs.

* It is easy to patch up chords with EV3.

That said, EV3 only offers one waveform output at a time per VCO, whereas EvenVCO makes them all available at the same time.
Using EV3

The Initial pitch is controlled by a stepped octave control, a stepped semitone control, and a fine tune. The octave and semitone are displayed as an octave number and a musical interval.

VCO 2 and 3 have an option for hard sync. The sync input is always the saw output of VCO 1.

There are independent outputs for each VCO, as well as a 3-to-1 mixer driving a mixed output.

The CV connections are a bit unusual. If you patch one of the top inputs (VCO 1 inputs) it will drive all three. Each VCO will pick up its input from the first patched input. So, for example, VCO 2 will get its input from the second row, but if nothing is patched to this input it will pick up input from the first row (VCO 1).

This makes for much less patching when stacking two or three VCO sections in a single voice.

The controls in depth
The octave knob is at the top left. It is unlabeled, but does have the octave number displayed on top of it. It has a 10 octave range, just like EvenVCO.

The semitone knob just to the right will add or subtract up to 12 semitones.  The label above the knob displays the semitone offset as an interval in diatonic harmony. For example, 7 semitones up is labeled “5th”. Note that the intervals displayed are always an octave plus a transposition up. So lowering 5:0 by two semitones will give you 4:m7th – so it’s displayed as one octave down and a minor 7th up. Note that some of these intervals have more than one spelling. In these cases we made some arbitrary decisions:

* One semitone up is called minor second, although some would call it a flat second.
* Six semitones up is called Diminished fifth, although it could be called an augmented fourth or a tritone.
* No transposition is displayed as zero, although it would be more correct to call it P1, or perhaps unison.

The fine control transposes the VCO pitch up and down up to a semitone.

The mod knob controls the amount of pitch modulation applied to the Fm input CV. This is of course exponential pitch modulation, suitable for pitch bending and vibrato, but not so much for FM synthesis which works linear FM.

Below the blue knobs are two small black knobs to control the pulse width and pulse width modulation depth. These only have an effect when the square wave output is selected. Although they are only labeled on VCO1, they function just the same on VCO 2 and 3.

The switches below the knobs are radio buttons that select the waveform for each VCO. The waveforms are sine, triangle, saw-tooth, square, even, and off. The even waveform is what gave the original EvenVCO its name. It is an unusual waveform that has only even harmonics, and not odd ones (not counting the fundamental, which is there). Selecting “no waveform” can be useful when you are patching and want to hear each VCO by itself – like a mute button on a mixing console.

The CV inputs are at the bottom. The top row is for VCO1, the next for VCO2, and the last row is for VCO3.

V/Oct is where the main pitch CV is patched, and sets the overall pitch of the VCO. Fm is a less sensitive pitch input used to modulate the pitch. As noted above, the sensitivity of this input is controlled by the Mod knob. The last column of CV inputs is for pulse width modulation. This only has an effect when the square wave is selected. It works in conjunction with the PW and PWM knobs.

The output section has a column of three output level controls, one for each VCO. Then there are the three jack, one for each VCO output, and a mixed output.

We have an informational article that talks more about aliasing. It shows you how to compare different modules using a spectrum analyzer. [Aliasing Story](./aliasing.md).

If you would like some information on how we reduced the CPU usage of EvenVCO, you can [find it here](../docs/vco-optimization.md).