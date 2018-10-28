# Growler

![vocal formant filter image](./growler.jpg)

**Growler** is a re-creation of the Vocal Animator circuit invented by Bernie Hutchins, and published in Electronotes magazine in the late 70's. It continuously morphs between different vaguely voice like tones.

**To get a good sound:** run any harmonically rich signal into the input, and something good will come out. Low frequency pulse waves and distorted sounds make great input.

The controls do pretty much what you would expect:

* **LFO** controls the speed of the modulation LFOs.
* **Fc** controls the average frequency of the multiple filters.
* **Q** controls the sharpness of the filters.
* **Depth** controls how much of the modulation LFOs are applied to the filters.

## How Growler works
![growler scope](./growler.png)

There are four **bandpass filters**, roughly tuned to some typical vocal formant frequencies: 522, 1340, 2570, and 3700 Hz. The filters are run in parallel, with their outputs summed together.

The first three filter frequencies are modulated by an LFO comprised of **4 triangle wave LFOs** running at different frequencies. They are summed together in various combinations to drive each of the filters.

Each **CV input stage** is the same: a knob that supplies a fixed  offset and a CV input that is processed by an attenuverter. The processed CV is added to the knob voltage. See below for more on [Attenuverters](#atten) and [CV ranges](#cv).

The **LFO** Rate control shifts the speed of all 4 LFOs while maintaining the ratio of their frequencies.

The **Fc** control moves the frequencies of the first three filters, but not by equal amounts. The lowest filter moves at 1V/Oct, but the middle two move less. The top filter is fixed at 3700 Hz.

The **Q** control does just what it says - controls the Q (resonance) of the filters.

The **Modulation Depth** controls how much of the summed LFOs get to each filter. Again, the lower filters move farther, and the top filter is fixed.

The smaller knobs next to the main knobs are **attenuverters**, which scale control voltages. For more on attenuverters, [see below](#atten)

There are three LFO outputs next to the blinking LFOs. These may be used to modulate other modules, or as semi-random voltage sources.

**Bass boost** switch. When it’s in the up position (on) there should be more bass. This is done by switching some or all of the filters from bandpass to lowpass.

LFO **Matrix** switch. This is the unlabeled switch in the LFO section. When it’s down (default position) the LFOs are closely correlated. In the middle we try to make them a little bit more independent. When it’s in the up position the LFOs will often go in different directions.
