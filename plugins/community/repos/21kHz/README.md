# 21kHz 0.6.1 (Coming soon)

A couple of modules I made for [VCV Rack](https://vcvrack.com/). More to come. The following is a list of and documentation for each module in the plugin. Also, I've linked to audio demos next to the section title of some modules.

<img src="docs/pl.png" alt="drawing" height="420px"/>

## Palm Loop ([Audio Demo](https://clyp.it/d5zatc4a))

Palm Loop is a basic and CPU-friendly VCO, implementing through-zero FM and polyBLEP and polyBLAMP antialiasing.

The OCTAVE, COARSE, and FINE knobs change the oscillator frequency, which is C4 by default. The OCTAVE knob changes the frequency in octave increments (C0 to C8), the COARSE knob in half-step increments (-7 to +7), and the FINE knob within a continuous +/-1 half-step range.

The V/OCT input is the master pitch input. The EXP input is for exponential frequency modulation, and the LIN input is for through-zero linear frequency modulation, both having a dedicated attenuverter. The RESET input restarts each waveform output at the beginning of its cycle upon recieving a trigger. The reset is not antialiased.

There are five outputs. The top two are saw and sine, and the bottom three are square, triangle, and sine. The bottom three waveforms are pitched an octave lower.

**Tips**
- Since there's not much in the way of waveshaping, Palm Loop shines when doing FM, perhaps paired with a second. 
- The LIN input is for the classic glassy FM harmonics; use the EXP input for harsh inharmonic timbres.
- If you have one modulating another, RESET both on the same trigger to keep the timbre consistent across pitch changes.
- Mix or scan the outputs for varied waveshapes.

<img src="docs/d.png" alt="drawing" height="420px"/>

## *D*<sub>∞</sub>

A basic module for modifying V/OCT signals by transposition and inversion.

The OCTAVE knob transposes the signal in octave increments (-4 to +4), and the COARSE knob transposes it in half step increments (-7 to +7). The 1/2 # button raises the transposed signal by a quarter step, so quartertone transpositions can be achieved. When the INV button is on, the incoming signal is inverted about 0V before being transposed.

The rest of the controls determine when the transposition and inversion are done. By default, if there is no input at the TRIG port, the transposition is always active and the inversion active if the INV button is on. With the GATE button off, a trigger at the TRIG input will toggle the transposition between on and off. With the GATE button on, the transposition will only activate with a signal of >= 5.0V at the TRIG input (generally meant for 0-10V unipolar inputs). Finally, activating the button below INV means that the input will only be inverted when transposition is active (and the INV button is on). Otherwise, the signal will always be inverted if the INV button is on.

**Tips**
- Swap between differently transposed sequences with a sequential switch for harmonic movement.
- Turn on INV and the button below it, and transpose so that the inverted signal is in the same key as the incoming signal. An input at TRIG will create some nice melodic variation, especially if it is offset from the main rhythm.
