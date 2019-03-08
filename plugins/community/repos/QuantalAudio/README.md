
# Quantal Audio VCV Rack Plugins

This is a series of plugins for [VCV Rack](https://vcvrack.com/).

![Modules](https://github.com/sumpygump/quantal-audio/raw/master/doc/img/quantal-audio-mods.png)

Modules in this plugin pack:

## Horsehair VCO | 7HP

This is an oscillator module with 3 oscillators. The main two oscillators (A and B) are interpolated between square and saw waves. For each of the two main oscillators, you can modulate the octave for each independently as well as the shape (change smoothly between square and saw) and the pulse width (only affects the square wave component of the oscillation). The shape and pulse width can be modulated with CV inputs. A mix knob interpolates between osc A and osc B and also has a CV input. The main output will provide the mix output of osc A and B. There is also a sin output that provides a basic sine wave, good for a sub out to be processed separately with the main oscillator output.

## MULT | Buffered Multiple | 2HP

Two 1x3 voltage copies. With switch in the middle in the down position, it will turn into a 1x6 copier.

## MIX | Unity Mix | 2HP

Sum and average signals from 3 inputs into 1 output. With switch in middle in the down position, it will take 6 inputs to 2 outputs.

## DCH | Daisy Mix Channel | 2HP

This is a modular mixer! Use multiple copies and each one acts as a channel strip that daisy chained together and the last one in the chain connected to the Daisy Master. You can chain together up to 16 channels. Chain them together by connecting the daisy out of a module to a daisy in of the next module. The fader range is 0.0 to 1.0. Each channel has a CV input to control the fader and a mute button. Each channel also has an out that can be used to route the audio from the indivdual channel post fader and CV.

An example daisy mixer with 5 channels daisy-chained together:

![Daisy Mixer Example](https://github.com/sumpygump/quantal-audio/raw/master/doc/img/quantal-audio-daisy-mixer.png)

It is not recommended to send voltage signals in or out of the daisy plugs other than daisy-to-daisy connections. The way the daisy chain works is by making very low voltage in order to pass the signal along until it gets to the master bus which raises the voltage back up. Sending a higher signal into the daisy plugs will result in distortion.

## DSY-MX | Daisy Mix Master | 3HP

The Daisy mix master acts as a bus for daisy chained channel mods. It has a master fader knob with CV control and a mute button. The master bus will clamp signals to (+/-) 12 volt output signal.

## MIXER-2 | Master Mixer | 5HP

A 2-channel mixer with a mono/stereo switch. In mono mode, input channel 1 will be copied to channel two's output. This is useful if you want to copy a mono signal to route to modules that need stereo inputs (for example, L and R channels of Core Audio module).
