# ValleyRack Free

3rd party plugins for VCV Rack version 0.6.1

### Version

0.6.4

    • [New Module] Plateau reverb! A plate reverb with a twist. Can add large expansive textures to your
    sounds, as well as be able to be tuned and excited at very short reverb times.
    • [Update] Performance optimisations to Topograph and µGraph.
    • [Update] All modules now have a dark jack look.

### Compatibility

The modules are Mac, Windows and Linux compatible. Some source is inherently open source, so you are free to download / clone and build on your own computer. See the appropriate license information for more details.

You must have 0.6.1 to run these modules.

### Modules

* Topograph - A port of the Mutable Instruments "Grids", the rhythm sequencer module that was missing from the VCV Audible Instruments plugin bundle. Covered by GPL-3.0 license.
* µGraph - Same as Topograph except more compact. It does, however, default to the Olivier pattern modes rather than Henri.
* Dexter - An FM wavetable oscillator with tonnes of modulation inputs, phase shaping and synchronisation options, as well multiple outputs from 2 separate voices and each operator.
* Plateu - A plate reverb with a twist. Can add large expansive textures to your
sounds, as well as be able to be tuned and excited at very short reverb times.

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

## Todo
* Develop more modules ;)

## Future

I really want to focus on modules that are deep and versatile rather than "bread and butter stuff".

I hope to produce more modules based on some of my existing research work.

## License

Topograph & µGraph are covered by GPL-3.0

Topograph's and µGraph's dependencies "Oneshot" and "Metronome" are covered by BSD-3-Clause

Dexter and Plateau are covered by BSD-3-Clause
