# The Bacon Music VCVRack Modules

"Bacon Music" is my set of EuroRack style plugins for 
[VCVRack](http://www.vcvrack.com). The modules are mostly inspired by me noodling around, and 
they sort of fall into a few groups: 

* Control voltage manipulation on 1v/oct signals to do things like glissando and musical quantization and a polyrhytmic clock;
* Classic synth algorithms, including an implementation of the NES oscillators and a Karplus Strong implementation.
* and finally, not very useful modulations and  distortions and stuff.


All the source is here, released under an Apache 2.0 license. You are free to use
these modules as you see fit. If you happen to use them to make music you want to share, please
do let me know, either by raising an issue on this github or by tagging me on twitter (@baconpaul) or
soundcloud (@baconpaul).

As of the release of Rack 0.6, these plugins will be built as part of the 
[v2 community distribution](https://github.com/VCVRack/community/tree/v2) so they should just be
available for you to try. A massive thanks to the [Rack community team](https://github.com/VCVRack/community/issues/248)
for maintaining these builds. 

I'm happy to hear any feedback and bug reports. The best way
to reach me is to just open a github issue [right here on github](https://github.com/baconpaul/BaconPlugs/issues). 

Finally, all the [sample patches I used to make the screen-shots](https://github.com/baconpaul/BaconPlugs/tree/master/patches) are
on the github here. 

I hope you enjoy the plugins! 

## Control Voltage Manipulation and Clocks

### HarMoNee

HarMoNee is a plugin which takes a 1v/oct CV signal and outputs two signals,
one which is the original, and the second which is modified by a musical amount,
like a minor 3rd. It spans plus or minus one octave, and is controlled by toggles.

The toggles are additive. So if you want a fourth, choose a major third and a half step 
both. You get the idea. 

<a href="https://baconpaul.github.io/audio/HarMoNee.mp3">
<img src="docs/HarMoNee.png" alt="ExampleQuanteyes Patch">
<br>
<img src="docs/SpeakerIconSmall.png" alt="Hear HarMoNee Sample">
</a>


### Glissinator

Glissinator takes a control voltage which is undergoing change and smooths out that
change with a linear glissando. It is not triggered by a gate, just by differences
in the input CV. It never jumps discontinuously, so if the CV changes "target" value
mid-gliss, the whole thing turns around. The slider will give you between 0 and 1 seconds
of gliss time. There is a gate output which is +10v when the module is glissing and
0 when it isn't. Note that the Glissinator is a constant time gliss, not a constant
slope gliss (which is what a slew limiter would do). A future version may have a switch
to pick between the modes.

Here's a sample patch.

<a href="https://baconpaul.github.io/audio/Glissinator.mp3">
<img src="docs/Glissinator.png" alt="ExampleQuanteyes Patch">
<br>
<img src="docs/SpeakerIconSmall.png" alt="Hear Glissinator Sample">
</a>


### QuantEyes

QuantEyes takes a CV signal and clamps it to certain values 1/12 of a volt apart.
Functionally this means that CV signals which are changing on input will be clamped to
a chromatic scale on output if all the notes are activated. But you can also deactivate
certain notes to allow you to pick scales to which you quantize.

Since quantizing to scales could be useful for multiple things driving oscillators, 
you can apply this quantization to up to 3 inputs using the same scale.

Finally, you can choose where the "root" note is in CV space. The default is that
1 volt is the "R" note, but if you set root to 3, then 1 3/12 volts would be R. If you don't
understand this, send in a changing signal, select only the R note in the set of LED buttons, 
and then twiddle the root note.

Here's a (pretty cool sounding, I think) patch which combines QuantEyes with the 
Glissinator and HarMoNee modules.

<a href="https://baconpaul.github.io/audio/QuantEyes.mp3">
<img src="docs/QuantEyes.png" alt="ExampleQuanteyes Patch">
<br>
<img src="docs/SpeakerIconSmall.png" alt="Hear QuantEyes Sample">
</a>


### PolyGnome

PolyGnome is a polyrhytmic clock generator. It can output up to 5 clock signals with strict 
fractional relationships between them. There is one clock which is the "1/1" clock controlled by
the speed settings implemented exactly like the clock rate controls in SEQ3. Then there are 4
other clocks which are fractionally adjusted clocks. This way you can do a 1/1 vs a 5/3 vs a 4/5
polyrhythm easily in your rack.

Here's an example which uses this to drive 3 independent oscillator / envelope sets all of which have
pitch set through the QuantEyes module.

<a href="https://baconpaul.github.io/audio/PolyGnome.mp3">
<img src="docs/PolyGnome.png" alt="Example PolyGnome Patch">
<br>
<img src="docs/SpeakerIconSmall.png" alt="Hear PolyGnome Sample">
</a>


## Classic and 8-bit algorithms

### ChipWaves

ChipWaves implements the NES triangle and pulse wave generator. It would have been
impossible to implement without the careful description of the algorithms at
[the NES Dev Wiki](http://wiki.nesdev.com/w/index.php/APU_Triangle).

The NES has two tonal oscillators, a triangle wave and a pulse. The pulse has 4
different duty cycles. The triangle is a fixed wave. 

They are tuned by wavelength with values up to 2^11 clock cycles. Rather than
expose this very digital interface, though, I've set up the inputs to be tuned
to CV in exactly the same way as VCO-1. So the conversion from 1v/oct signal
to the 2^11 different wavelengths based on the simulated clock frequency (I chose NTSC)
is all done for you.

Basically, it just works like an oscillator. Drop it in and go chip crazy. The sample
patch runs it mixed along with a VCO-1 so I could check tuning. Here's how I did it.

<a href="https://baconpaul.github.io/audio/ChipWaves.mp3">
<img src="docs/ChipWaves.png" alt="Example ChipWaves Patch">
<br>
<img src="docs/SpeakerIconSmall.png" alt="Hear ChipWaves Sample">
</a>



### ChipNoise

ChipNoise implements the NES noise generator without the NES envelope. It would have been
impossible to implement without the careful description of the noise algorithm at
[the NES Dev WIKI](http://wiki.nesdev.com/w/index.php/APU_Noise). I also appreciate the
[lengthy conversation with @alto77](https://github.com/baconpaul/BaconPlugs/issues/6) which helped
identify a bug in the 0.6.1 release and add a new feature.

The NES noise system has 16 different frequencies; and two modes. The two modes generate either
a long pseudo-random pattern or a set of short pseudo-random patterns. That long pattern is just 
long, but the short patterns are either 93 or 31 bits long. There are 351 distinct 93 bit patterns
and a single 31 bit pattern. 

The sequence controls allow you to pick these patterns. If set at "long" then you choose the longest
pattern. If set at short, then either you have the 31 long pattern or one of the 93 patterns. Which of
the 93 patterns you pick is chosen by the "which 93 seq" knob. 

This is a lot of information. If you just play with it you'll get the idea.

Here's a simple patch.

<a href="https://baconpaul.github.io/audio/ChipNoise.mp3">
<img src="docs/ChipNoise.png" alt="Example ChipNoise Patch">
<br>
<img src="docs/SpeakerIconSmall.png" alt="Hear ChipNoise Sample">
</a>

### ChipYourWaves

ChipYourWaves is a chip-like oscillator which doesn't actually exist in the old NES hardware.
The way the NES triangle wave generator works is it oscillates across a 32-sample-wide waveform of values
between 0 and 15, with 7/8 being the "zero point". This is set up as a ramp 0 1 2 ... 15 15 14 ... 0 
and then tuned and oscillated. If you want this waveform, just use the ChipWaves module up above.

But I got to thinking. That waveform is something we could change and make even buzzier grunky chip-like 
sounds. Perhaps. So I wrote a module which is the same internal implementation as the NES triangle wave
generator in ChipWaves but which has an editor which lets you pick each of the 32 values. Then you can
hear it. And see if you like it. 

To set the values just click or drag on the LED vertical. It's pretty intuitive when you use it, I think. But
so you get an idea, here's the sample patch which sets a fixed frequency and hooks it up to the scope so you can
see the generated waveform is, indeed, the bits you draw in the LED-like controls.

![Example ChipYourWaves Patch](docs/ChipYourWaves.png)

Oh if you have any idea what to put in all that blank space at the top of the module, by the way, please do
just raise a github issue and let me know!

### KarplusStrongPoly

The [Karplus-Strong algorithm](https://en.m.wikipedia.org/wiki/Karplus–Strong_string_synthesis) is one of the
earlier methods to simulate plucked string instruments. The KarplusStrongPoly module implements a polyphonic
voides implementation of this. The module maps to the algorithm fairly cleanly. It's probably easier that you 
just play with it, using the sample patch shown below.

<a href="https://baconpaul.github.io/audio/KarplusStrongPoly.mp3">
<img src="docs/KarplusStrongPoly.png" alt="Example ChipNoise Patch">
<br>
<img src="docs/SpeakerIconSmall.png" alt="Hear ChipNoise Sample">
</a>

There's one really important thing to know about this module. Unlike more traditional voltage controlled oscillators 
which always produce output and are then fed into envelopes and stuff, the KarplusStrongPoly module needs to be
triggered with a gate signal to produce any sound. When it is triggered it will snap all the parameters set on
the front panel and play that voice until it fades. The system is configured to play upto 32 voices and will
voice steal beyond that. But since Rack adds a 1 sample delay to all its signals as they go through each module,
if you trigger from SEQ-3 and use a frequency you have modified, the trigger will "beat" the modified signal.
So adding a few sample delay to your trigger may be approrpriate. There's a really simple SampleDelay module
which ships with this plugin set if you want to do that.

I've only implemented one filter so far, so the only control which does anything in the filter space is the "A" 
knob and CV input. If/as I add more that will get way more rich, kind of like initial packet is now.

Finally I think the algorithm is stable under all possible front panel configurations. There's certainly
regimes of parameters in the C++ which can break the synthesis, though. So if you get an
odd or growing sound, let me know the configuration which did it in a github issue and I'll put a check
in the widget to synth snap appropriately.

## Distortions and Modulations and so on
### ALingADing 

ALingADing is a simulation of a Ring Modulator based on [this paper by Julian Parker](http://recherche.ircam.fr/pub/dafx11/Papers/66_e.pdf) and
then taking some shortcuts.
Rather than following Parker's use of a few polynomials to simulate his diode, 
I basically use an implementation of a softmax, eye-balling the parameters to roughly meet the figure in his
paper. The only control is a wet/dry mix (where wet is the signal modulated by carrier
and dry is just the signal). Sloppy, sure, but it sounds kinda cool. Here's a sample patch.

<a href="https://baconpaul.github.io/audio/ALingADing.mp3">
<img src="docs/ALingADing.png" alt="ExampleQuanteyes Patch">
<br>
<img src="docs/SpeakerIconSmall.png" alt="Here QuantEyes">
</a>



### Bitulator

Bitulator is really just me screwing around with some math on the input. It has two
functions. Firstly, it "quantizes" to a smaller number of "bits", but does it in a
weird and sloppy way of basically making sure there are only N values possible in the 
output. Apply this to a sine wave with a low value of N and you get sort of stacked squares. 
Secondly it has a gross digital clipping amplifier. Basically signal is the clamp of input times
param. Apply this to a sine wave and turn it up and you get pretty much a perfect square.
Combine them for grunky grunk noise. Dumb, but fun. Here's a sample patch.

![Example Bitulator Patch](docs/Bitulator.png)

### SampleDelay

This is an incredibly simple module. All it does is add an n-sample digital delay at the 
engine clock speed. Radically non-analog, I know. But it's useful since Rack adds a 1 sample
delay in each module hop to do things like triggering the Karplus Strong poly.

![Example SampleDelay](docs/SampleDelay.png)

## Credits and Comments

The Keypunch font used in the textual display LED widget comes from 
[Stewart C. Russell's blog](http://scruss.com/blog/2017/03/21/keypunch029-for-all-your-punched-card-font-needs/). 
The font is Copyright 2017 Stewart C. Russell and is released under 
the [SIL Open Font License 1.1](http://scripts.sil.org/cms/scripts/page.php?site_id=nrsi&id=OFL).

The slug name ('BaconMusic') is different than the repo name ('BaconPlugs'), somewhat confusingly.
When I made my git repo I had no idea really how anything worked or if I'd write anything. 
I was thinking "Hey I'm writing a collection of plugins for this software right". When I went
with my first release, Andrew Rust pointed out that "BaconPlugs" wasn't a very good name for my 
plugin and it's collected modules. He did it very politely, of course, and so I changed it to "Bacon Music" 
for the slug name. The repo is still called BaconPlugs though, because that's more trouble to change than 
I can handle.

## License

Copyright © 2017-2018  Paul Walker

Licensed under the Apache License, Version 2.0 (the "License");
you may not use this file except in compliance with the License.
You may obtain a copy of the License at

http://www.apache.org/licenses/LICENSE-2.0

Unless required by applicable law or agreed to in writing, software
distributed under the License is distributed on an "AS IS" BASIS,
WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
See the License for the specific language governing permissions and
limitations under the License.


