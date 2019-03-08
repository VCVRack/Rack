# ValleyRack Free

3rd party plugins for VCV Rack version 0.6.2c

### Version

0.6.15

    • [Update] Added input gain sensitivity and output saturation modes to Plateau.
    • [Fix] Fixed DC offset switch save state bug in Amalgam.

### Compatibility

The modules are Mac, Windows and Linux compatible. Some source is inherently open source, so you are free to download / clone and build on your own computer. See the appropriate license information for more details.

You must have 0.6.1 to run these modules.

### Modules

* Topograph - A port of the Mutable Instruments "Grids", the rhythm sequencer module that was missing from the VCV Audible Instruments plugin bundle. Covered by GPL-3.0 license.
* µGraph - Same as Topograph except more compact. It does, however, default to the Olivier pattern modes rather than Henri.
* Dexter - An FM wavetable oscillator with tonnes of modulation inputs, phase shaping and synchronisation options, as well multiple outputs from 2 separate voices and each operator.
* Plateau - A plate reverb with a twist. Can add large expansive textures to your
sounds, as well as be able to be tuned and excited at very short reverb times. It is based on the
well known Dattorro (1997) plate reverb algorithm.
* Amalgam - A signal masher and multipler
* Interzone - A complete, monophonic, virtual analogue synth voice

![Valley](./ValleyImg.png)

### Installation
The latest release will be always available from the official Rack [Plugin Manager](https://vcvrack.com/plugins.html). However if you prefer, you can download and extract the .zip file from this git repository under releases, then place the extracted folder in:

    * Mac - ~/Documents/Rack/plugins
    * Windows - /My Documents/Rack/plugins
    * Linux - ~/.Rack/plugins

## Usage

### Topograph & µGraph

The behaviour of this module is nearly identical to the hardware version of "Grids", and therefore it is worth familiarising yourself with it by visiting the Mutable Instruments' [website](https://mutable-instruments.net/modules/grids/).

This module is a rhythm sequencer module. Yet, unlike the usual x0x style drum machine this module contains a vast "map" of drum rhythms that can be cross-faded and explored in a continuous fashion. There are 3 drum lines (Kick, Snare and Hats) each with their own Fill knob, Trigger and Accent outputs. The fill knobs control the density of the given drum line pattern. The higher the setting, the denser the pattern. The fill amounts can be randomly varied using the Chaos control.

The drum map is explored using the Map X and Y knobs. Finally, the Tempo knob controls the speed of the sequencer.

To control the sequencer from an external clock, turn the Tempo knob fully counter-clockwise and patch a clock source to the clock input. The clock input does not upscale a low frequency clock so you must select the appropriate PPQN (pulses per quarter note) resolution for your clock to get the correct tempo. 4 and 8 PPQN modes can distort certain patterns.

In Euclidean mode, the Map X, Y and Chaos controls set the sequence length for each drum channel. The fill controls then set the density of the sequence, of course Euclidean style. Each sequence runs independently so they will drift out of sync if the length controls are changed on the fly (maybe an alternate knob layout mode could be implemented here in the future). So to bring them back into sync, simply reset the sequencer and they will resynchronise.

All controls have a CV input to control them. These are the silver jacks.

### Dexter

In brief, Dexter is an FM wavetable oscillator. See the Dexter Manual pdf for more information available [here](https://github.com/ValleyAudio/ValleyRackFree/files/1887925/DexterManual.pdf)

### Plateau

Plateau is an implementation of the Dattorro (1997) plate reverb algorithm, with CV control over practically every section of the reverberator.

The reverberator is split into 4 sections:
    Input Equaliser -> Input Diffuser -> Reverb Equaliser -> Reverb "Tank"

The white knobs control the dry and wet ratio of the effect, whilst the predelay delays the signal by upto 500ms before it enters the reverberator.

Both equaliser sections (represented by the green knobs) sculpt the tone of the reverb, removing either low or high end from the signal. The reverb equaliser in particular allows the frequency response of the reverberant signal to evolve over time, either slowly damping the high end to create a natural decay, or by quickly removing the low end to create interesting "dub reggae" effects.

The blue knobs control the reverb tank:
* "Size"" manipulates the overall length of the reverb, and determines how much delay there is between a signal entering and leaving the tank.
* "Diffusion" controls how smeared and reverberant the signal becomes over time. 0% diffusion means the signal will flutter rather than reverberate.
* "Decay" sets how much the signal will decay over time. Fully clockwise means the signal will take a long to disappear.

The red knobs control the LFOs that modulate the delay lines in the reverb tank:
* "Rate" sets the speed that the delay lines will be modulated at.
* "Shape" controls the shape of the LFOs. In the middle position the shape is a symmetrical triangle, whilst either extreme will be a falling or rising ramp.
* "Depth" determines how far the delay lines in the tank will be modulated. Slight modulation creates a pleasant diffusion effect that prevents ringing from building up, whilst deep modulation creates a chaotic, pitch shifting effect.

Finally, there are five buttons dotted around the panel:
* "Tuned Mode" sets the all-pass filters to have a short, tuneable delay time, allowing you to 'play' the reverb like an instrument. The size input becomes 1V/Oct, although by default the tuning inverted. The size attenuverter can be set to fully anti-clockwise to restore normal tuning behaviour.
* "Diffuse Input" enables / disables the input diffuser section. This is particularly useful in tuned mode where the diffuser normally blurs the inputs signal. Bypassing it results in a sharper, more precise sound.
* "Hold" causes the reverb to be held indefinitely, where decay is set to 100% and the equalisers are bypassed.
* "Tog." sets if the hold switch is momentary or toggle.
* "Clear" purges the entire reverb, which is very handy if things get out of hand, although it can be used to great musical effect.

## Todo
* Develop more modules ;)

## Future

I really want to focus on modules that are deep and versatile rather than "bread and butter stuff".

I hope to produce more modules based on some of my existing research work.

## Bibliography

Dattorro, J. (1997). Effect design part 1: Reverberator and other filters, J. Audio Eng. Soc, 45(9), 660-684.

## License

Topograph & µGraph are covered by GPL-3.0

Topograph's and µGraph's dependencies "Oneshot" and "Metronome" are covered by BSD-3-Clause

Dexter and Plateau are covered by BSD-3-Clause
