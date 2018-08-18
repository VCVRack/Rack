# Add (simple additive synthesis)

![Add](images/add.png)

The **ADD** module accepts two waveform inputs, adds the waveforms together,
and outputs the resulting waveform.

This module provides two separated sets of inputs and outputs, and does not clip.
This means that if you have two high voltages coming in, the result could go
above or below the prescribed -5v/5v.  If this happens, some clipping may occur,
or you could overdrive other modules.
