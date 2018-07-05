
# Frozen Wasteland VCV plugins

A collection of unusual plugins that will add a certain coolness to your patches.

## BPM LFO
![BPM LFO](./doc/bpmlfo.png)

- Tempo Sync'd LFO.
- CV Control of Time Division
- Pairs well with the Quad Euclidean Rhythm and Quad Golomb Ruler Rhythm
- When Holding is active, the Outputs stay at last value
- Hold input can either be a gate (which switches each time) or momentary (active while signal is positve)
- If set to Free, LFO still runs while being held (even if outputs don't change), Pause causes LFO to pause.

## BPM LFO 2
![BPM LFO](./doc/bpmlfo2.png)

- Variation of original BPM LFO.
- CV Control of Time Division and Wave Shape
- Can either be variable sawtooth/triangle or variable duty cycle square wave

## Damian Lillard
![Damian Lillard](./doc/damianlillard.png)
- Voltage Controlled Quad Crossover
- Use Sends/Returns to apply FX to different frequency domains of input signal
- Basic example, distort only the mid lows and/or mid highs so your distortion doesn't remove bottom end
- or, apply different delays to create interesting resonances and other FX
- Video of a snare drum being fed into four delay lines: https://www.youtube.com/watch?v=EB7A_hzMpNI
- Use your imagination!

## Everlasting Glottal Stopper
![Everlasting Glottal Stopper](./doc/egs.png)
- Based on the Rosenburg model of human's larynx
- Pairs well with the Vox Inhumana
- Can be a bit fussy to create harmonically "rich" waves, but playing with the "Closed" setting can find some sweet spots
- Use the Noise parameter to add a "breathy" quality to wave

## Hair Pick
![Hair Pick](./doc/hp.png)
- Based on the comb filter section of Intellijel/Cylonix Rainmaker
- Best bet is to refer to this: https://intellijel.com/wp-content/uploads/2016/01/rainmaker_manual-v109.pdf
- Comb filter size is either based on Clock & Division or Size Parameter - patching a clock in disables Size knob
- V/Oct works using either method
- The Size CV can change size +/- 10% using both Clock or Size
- There are 16 comb patterns, the last 8 are randomly perturbed versions of the first 8
- The Tap count can vary the number of taps between 1 and 64. They are deactivated in a pattern shown in the rainmaker manual
- The Edge Level, Tent Level and Tent Tap control the overall volume of the taps.
- Feedback Type can add non-linearity and exponential decay. Clarinet mode is same as guitar for now.
- The size out allows the comb's length to control other modules (say the feedback delay time in Portland Weather)

## Lissajou LFO.

![Lissajou LFO](./doc/llfo.png)

- Loosely based on ADDAC Systems now discontinued 502 module https://www.modulargrid.net/e/addac-system-addac502-ultra-lissajous
- Each LFO is actually a pair of LFOs (x and y)
- Adjusting them will show harmonic relationship between the two
- Yellow is LFO 1, Blue is LFO 2
- Output 1: (x1 + x2) / 2
- Output 2: (y1 + y2) / 2
- Output 3: (x1 + y1 + x2 + y2) / 4
- Output 4: x1/x2
- Output 5: y1/y2

## Mr. Blue Sky

![Mr. Blue Sky](./doc/mrbluesky.png)

- Yes, I love ELO
- This is shamelessly based on Sebastien Bouffier (bid°°)'s fantastic zINC vocoder
- Generally you patch something "synthy" to the carrier input
- The voice sample (or I like to think "harmonically interesting" source) gets patched into the Mod input
- Each modulator band is normalled to its respective carrier input, but the patch points allow you to have different bands modulate different carrier bands
- You can patch in effects (a delay, perhaps?) between the mod out and carrier in.
- CV Control of over almost everything. I highly recommend playing with the band offset.
- You can either CV the value of the band offset, or send triggers to the + and - Inputs to increment/decrement the offset

## The One Ring (modulator)

![The One Ring Modulator](./doc/oneringmod.png)

- Ring modulation essentially multiplies the two frequencies together, creating non-harmonic sidebands
- Doing RM digitally is easy - just multiply the two signals. But it was tougher to do with an analog circuit.  The traditional design used 4 diodes.
- This module is based on this paper: http://recherche.ircam.fr/pub/dafx11/Papers/66_e.pdf which describes how to model an analog ring modulator with all its quirks
- Let's you simulate different types of diode response as used in analog ring modulators
- Bias is the input voltage level where the diode responds - below that the diode outputs 0V
- Linear is the input voltage level where the diode outputs a linear response. Between the Bias and Linar levels, the diode acts 'weird' or let's say 'harmonically interesting'
- Slope is the strength of the diode output
- The graph display is useful for seeing the diodes' voltage responses
- CV control over everything
### Ring Modulation
- Ring Modulation has always appealed to the math nerd in me, because while the process is pretty straightfoward, the results are really cool.
- example. If you feed into a RM a 200hz sine wave and a 500hz sine wave, the output is the sum and difference of those two frequencies, so 300hz and 700hz.
- Where it gets crazy is when harmonics are involved since the output is the sum and difference of "all" the frequencies. example, let's say you had the 200 and 500hz sine waves, but they both each had a 2nd harmonic as well, so a 400hz and 1000hz. The output would be: 500+200=700,1000+200=1200,500+400=900,1000+400=1400,500-200=300,500-400=100,1000-300=700(again),1000-400=600. So a whole slew of new frequencies got created.
- Most waveforms have *way* more harmonics than this, so the results are really crazy.
- The Dalek Voices in the original Dr. Who were done using ring modulators. Eliminate!!!

## Phased Locked Loop (PLL)

![Phased Lock Loop](./doc/pll.png)

- Inspired by Doepfer's A-196 PLL module
- This is a very weird module and can be kind of "fussy". Recommend reading http://www.doepfer.de/A196.htm
- Added CV control of the LPF that the 196 did not have
- Added Pulse Width and Pulse Width Modulation of the internal VCO
- Generally you want to "listen" to the **SQR OUT**
- You'll want to feed a Square-ish wave into Signal In
- Low **LPF FREQ** settings create a warbling effect, high = craziness
- **EXTERNAL IN** overrides interal VCO
- The **LPF OUT** is normalled to the **VCO CV**, try patching something in between the two
- Two comparator modes: XOR and D type Flip Flop
- Does not make pretty sounds, but can be a lot of fun.

## Portland Weather
![Portland Weather](./doc/pw.png)
- Based on the rhythmic tap delay section of Intellijel/Cylonix Rainmaker

### This is a crazy deep module and is best just to play with. 
### Since it is a "rhythmic" delay, short duration or percussive sounds generally work best

- Best bet is to refer to this: https://intellijel.com/wp-content/uploads/2016/01/rainmaker_manual-v109.pdf
- Delay time is either based on Clock & Division or Size Parameter - patching a clock in disables Size knob
- There are 16 delay groove patterns. The groove amount balances the delay time between the groove or straight time
- Stacking a tap makes the tap use the delay time of its neighbor to right. If multiple taps are stacked the right most tap controls time
- Stacking is useful for making chords
- Each tap has its own level, pan, filter type, cutoff, Q, pitch shift and pitch detune control
- The left and right channels can independently use a tap # to determine where feedback comes from (the feedback bypasses the filter and pitchshifting)
- The feedback slip controls allow the feedback to be ahead/behind the tap time for dragging effects, etc.
- If feedback tap is set to All, the mix of all 16 taps is fed back (use cautiously)
- If feedback tap is set to Ext, the feedback time input is used. Useful for sycing with say, the Hair Pick
- The Grain Number and Grain Size control how the all the pitch shifters work. "RAW" is uses one grain sample but without the a triangle window
- Reverse mode reverses direction of input buffer - time is based on feedback time
- Ping Pong feeds the L feedback into the right channel and vise versa
- FB Send and Returns allow you to insert FX into the feedback loop


## Quad Euclidean Rhythm

![Quad Euclidean Rhythm](./doc/qer.png)

- 4 Euclidiean rhythm based triggers
- CV control of Steps, Divisions and Offset, Padding, Accents and Accent Rotation
- QERs can be chained together to create arbitrarily long sequences. 
- If Chain Mode is Boss, the QER runs on start up, then stops if the track's Start input is patched, until a Start trigger is received - basically the first QER should be set to this
- If Chain Mode is Employee, the QER track will be idle until a Start trigger is received.
- Patch EoC (End of Cycle) outputs to next QER's Start Inputs
- Last QER's EoC should be patched back to first QER's Start Input to create a complete loop.
- May want to consider using an OR module (Qwelk has a nice one) so that mutiple QER's outs and accent outs can gate a single unit
- Each QER has its own clock input, so tempo changes can easily be created
- Mute Input keeps rhythm running just no output
- https://www.youtube.com/watch?v=ARMxz11z9FU is an example of how to patch a couple QERs together and drive some drum synths
- Normally each track advances one step every clock beat and are independent. If Time Sync is enabled, the track with the most steps becomes the master and the other tracks will fit their patterns to the timing of the master. This allows for complex polyrhythms (ie. 15 on 13 on 11 on 4, etc.)
- https://www.youtube.com/watch?v=eCErJMKAlVY is an example of the QER with Time Sync enabled
### Euclidean Rhythms
- Euclidean are based upon attempting to equally distribute the divisions among the steps available
- Basic example, with a step count of 16, and 2 divisions, the divisions will be on the 1 and the 9.
- A division of 4 would give you a basic 4 on the floor rhythm
- The Offset control lets you move the starting point of the rhythm to something other than the 1

## Quad Golumb Ruler Rhythm

![Quad Golumb Ruler Rhythm](./doc/qgrr.png)

- 4 Golumb Ruler rhythm based triggers
- For when QER is "too danceable" :)
- Features identical to the Quad Euclidean Rhythm and both units can be chained together
- 18 step sequencer since Golomb Ruler Rhythms like prime numbers having more than 17 steps allows for more patterns
### Golomb Ruler Rhythms
- Unlike Euclidean Rhythms which seek to evenly distribute the divisions, Golomb Rulers try to ensure unequal distribution
- Basic example, with a step count of 16, and 4 divisions, the divisions will be on the 1,4,9 and 13.


## Quantussy Cell

![Quantussy Cell](./doc/qc.png)

- This is based on work by Peter Blasser and Richard Brewster.
- Instantiate any number of cells (odd numbers work best - say 5 or 7)
- This is what is called a **Quantussy Ring**

![Quantussy Cell](./doc/qring.png)

- The Freq control sets the baseline value of the internal LFO
- The Castle output should be connected to the next Cell's CV Input.
- One of the outputs (usually triangle or saw) should be connected to the next Cell's Castle input
- Repeat for each cell. The last cell is connected back to the first cell
- Use any of the remaining wav outputs from any cell to provide semi-random bordering on chaotic CV
- Check out http://pugix.com/synth/eurorack-quantussy-cells/

## Roulette LFO

![Roulette LFO](./doc/rlfo.png)

- Based on the concept of a couple different types of Roulettes: https://en.wikipedia.org/wiki/Roulette_(curve)
- A circle rolling either inside a bigger circle (Hypotrochoid) or circle rolling outside a bigger circle (Epitrochoid)
- The ratio of the Big R to small r can create interesting patterns
- d is the distance of the 'pen' from the center of small circle.
- if d = r the shapes become a special form of the shapes - either a Hypocycloid or Epicycloid

## Seriously Slow LFO

![Seriously Slow LFO](./doc/sslfo.png)

- Waiting for the next Ice Age? Tidal Modulator too fast paced? This is the LFO for you.
- Generate oscillating CVs with range from 1 minute to 100 months
- NOTE: Pretty sure my math is correct, but 100 month LFOs have not been unit tested

## Vox Inhumana

![Vox Inhumana](./doc/voxinhumana.png)

- Generates vowel-ish sounds when given harmonically rich sources
- Pairs well with the Everlasting Glottal Stopper, but sawtooth and pulse waves work well
- Vowel/Voice formants are based on https://www.classes.cs.uchicago.edu/archive/1999/spring/CS295/Computing_Resources/Csound/CsManual3.48b1.HTML/Appendices/table3.html
- Fc allows changing the frequency of all formants at once, over a large range
- The CV of a formant allows a +/- 50% change of base vowel frequency
- The CV of amplitude allows the base level of the vowel/voice to be modified by about 2x.
- Changing the Fc of formants 1 & 2 can make the vowel sound more long or short

## Contributing

I welcome Issues and Pull Requests to this repository if you have suggestions for improvement.

These plugins are released into the public domain ([CC0](https://creativecommons.org/publicdomain/zero/1.0/)).