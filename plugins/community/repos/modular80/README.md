# A collection of modules for VCV Rack

This repository contains a collection of modules for [VCV Rack](https://vcvrack.com/),
the open-source virtual modular synthesizer.

The minimum supported VCV Rack version is **0.6.x**.

# Overview of modules

![modular80](/modular80.png)

## Logistiker

The `Logistiker` module is based on the [Logistic Map](https://en.wikipedia.org/wiki/Logistic_map),
a non-linear dynamic equation, which for certain input parameters exhibits [chaotic behavior](https://en.wikipedia.org/wiki/Chaos_theory).

The **RATE** knob controls the update rate of the internal clock. It has no function, if an
external clock signal is connected to the **EXT CLOCK** input.

The **R** knob, and the corresponding input, controls the **R** variable of the equation.
The [Wikipedia page](https://en.wikipedia.org/wiki/Logistic_map) has a good overview of the effect
of different **R** values. The default value corresponds to the onset of *Chaos* in the system.

The **X0** knob sets the initial starting value for the **X** variable of the equation.

If the **RESET** button is pressed, or a positive edge arrives at the **RESET** input,
the model starts over from the value set by the **X0** knob. The reset takes effect at the
next rising edge of the (internal or external) clock signal.

[YouTube Module Demo](https://youtu.be/xGSvLBChjzk)

## Nosering

The `Nosering` module is inspired by Grant Richter's [Noisering](https://malekkoheavyindustry.com/product/richter-noisering/) module. It does not implement all of
the Noisering functionality, but enough to be useful and fun.

See [this page](https://www.infinitesimal.eu/modules/index.php?title=Malekko_Noisering) for more
information on theory of operation of the original *Noisering* module.

The **RATE** knob controls the update rate of the internal clock. It has no function, if an
external clock signal is connected to the **EXT CLOCK** input.

The **CHANGE** knob controls the probability that **new data** is introduced into the system.
All the way CCW means only new data is feed into the shift register. All the way CW means only
old data is recycled in the shift register, i.e. the shift register content is looping.
The corresponding input provides CV contol of the **CHANGE** parameter.

The **CHANCE** knob controls the probability of introducing either a **0** (all the way CCW) or
a **1** (all the way CW) into the system. The corresponding input provides CV contol of the **CHANCE** parameter.

The **INV OLD** switch will cause the last bit pushed out of the shift register to be **inverted**
before feeding it back into the shift register input. The corresponding input provides CV control over
the **INV OLD** parameter.

The **EXT CHANCE** input switches the internal signal used with the Chance and Change comparators
from internally generated White Noise to the external signal connected to the **EXT CHANCE** input.

The **NOISE OUT** output carries the internal White Noise signal of the module.

The **n+1** output produces **n+1** or 9 levels, the **2^n** output produces **2^n** or 256 levels.
This is comparable the functionality of the [Buchla 266 Source Of Uncertainty](https://modularsynthesis.com/roman/buchla_266/266sou.htm).

## Radio Music

The `Radio Music` module is an **official port** of the hardware module by [Music Thing Modular](http://musicthing.co.uk/).

Tom Whitwell of `Music Thing Modular` has blessed this port and graciously provided the panel artwork.

Be sure to support Tom and `Music Thing Modular` by buying their excellent kits and/or modules!

The Rack version of the module does not use any of the hardware module's firmware code, but instead implements the module's
fundamental functionality in the context of Rack.

Not all advanced options and modes of the original module are currently implemented, but may be added in future versions.

For more information on the module see the [official documentation](https://github.com/TomWhitwell/RadioMusic/wiki/How-to-use-the-Radio-Music-module).

A collection of sample packs to load can be found on Tom Whitwell's [GitHub page](https://github.com/TomWhitwell/RadioMusic/wiki/Audio-packs-for-the-Radio-Music-module).

[YouTube Module Demo](https://youtu.be/cdk8DFG7_-U)

### Rack module features

- Playback of `.raw` (44.1 kHz, 16 bit, headerless PCM) and `.wav` files (all formats)
- Supports up to 16 banks (subfolders) with a maximum bank size of 2GB per bank

### Notable differences to hardware version

- `Root folder` is selected via the context menu (instead of the settings file).
- `Bank Selection Mode` is accessed via the context menu (instead of pressing and holding the reset button).
- Implemented options are available via the context menu (instead of a settings file).

### Additional information and resources

All hardware and software design in the original `Radio Music` project is Creative Commons licensed by Tom Whitwell:
[CC-BY-SA: Attribution / ShareAlike](https://creativecommons.org/licenses/by-sa/3.0/).

The source code for the `Radio Music` Rack module is also licensed under the same Creative Commons license by myself.

# Licenses

`Logistiker` and `Nosering` module source code licensed under BSD-3-Clause by Christoph Scholtes.

`Radio Music` Rack module source code licensed under [CC-BY-SA: Attribution / ShareAlike](https://creativecommons.org/licenses/by-sa/3.0/) by Christoph Scholtes.

[`dr_wav`](https://mackron.github.io/dr_wav) source code is placed into public domain by the author.