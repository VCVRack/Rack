
# Obxd_VCF

An adaption of Filatov Vadim's excellent Ob-Xd filter. Distributed under terms of the GNU General Public License V3.


# Scanner

A mixer that can seamlessly blend up to 16 input channels.

"pos" sets the center position

"pmod" sets the position modulation amount

"shape" selects the input window (when multiple inputs are mixed together). It goes from narrow sine to wide sine, to triangle, then to various pulse shapes.

"width" selects the number of neighbouring inputs (around the center position).

The post section is activated by pluggin a cable into the post output.
It's basically an experiment where I wanted to hear how the shape window sounds when applied to audio data.
I left it in since it turned out to be useful for synthesizing cymbal and hihat sounds (among other things).
The knob selects the window shape (same as the main shape parameter), and the switch toggles a window offset
(this used to be a bug in earlier versions but it sounded nice with some sounds).

NOTE: try modulating the position with the post output (feedback).


# Sway

A kind of slew-filtered noise generator, mainly designed for randomizing control voltages.

The "time" knobs select the minimum and maximum time (up to 60sec).

The "amp" knobs select the minimum and maximum amplification (-1..1).

The "s+o" knobs are used to apply a final scaling/amplification (-5..5) and offset (-5..5) to the output signal.

NOTE: when the min/max time is set to very small values, the module can be used to generate audio-rate noise.


# Known Issues

The graphics, especially the texts, look really bad. I currently have no idea to fix that.
It seems to be an issue with Inkscape, the nanosvg/nanovg SVG loader/renderer, my lack of experience with Inkscape, or all of the above.
