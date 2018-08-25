
# Stochasm - Stochastic Computing-style Bitstream Synthesis Modules

Stochastic computing is a method of analog computing where values are represented as the probability of a high value in a stream of random bits.

Bitstreams can be combined using relatively simple gating logic, for example, the multiplication of two streams where -1.0, 1.0 is mapped to false and true can be accomplished by XNORing the two streams. This module operates at 32bps, that is, 32 bit operations per sample frame. This was chosen to evenually enable passing complete bitstreams bitween modules.

## Modules

### Dual Resonator

The Dual Resonator Module provides two instances of a Stochastic Resonance Circuit. The circuit consists of two stochastic integrators (configured as a second order low pass filter) connected to a delay line (the chamber). The circuit mimics how molecules of air behave in a cylinder similar to a pipe organ or a woodwind instrument.

At rest, the delay lines fill up with random noise but upon gating the circuit, values are fed from the end delay line through the second order filter network back to the start of the delay line. Although controlled with knobs, the filter parameters are fixed to powers of 2 since they represent binary counters compared against an LFSR which is masked. In order for the filter to resonate and hence the whole system, it is necessary that **Filter 1** be the same size as or smaller than **Filter 2**. The fundamental frequency of the circuit is determined by the size of **Chamber** which can be controlled through the dial or through the 1V/Oct input. The signal is low pass filtered twice by a single pole recursive IIR filter, although it's probably worth running through more filters depending on how intelligible you want the tones.

The **Bits** output will eventually transfer the actual bit values generated per sequence (as an int written to the field instead of the required float). This will eventually enable linking stochastic circuits and having them interact with each other's raw bitstream, albeit at a 32 sample delay.

Things to experiment with include dragging up the **Chamber** size for fixed filter values since the filters have a maximum frequency they support for a given delay size and will become somewhat unstable. You can also drag down **Filter 1** for a given **Filter 2** value, which will exascerbate the resonance of the filter causing more energy and hence instability.

## Future Work

At this stage although the fundamentals are understood, it remains to be investigated how the stochastic modules perform in the real world. Help would be appreciated mapping configuration values to specific tonal frequencies. There may be support in the future for **Filter 1** and **Filter 2** to have continuous values but this will depend how stochastic dividers work (since you need dividers to amplify signals in a stochastic system).

Contributions appreciated.
