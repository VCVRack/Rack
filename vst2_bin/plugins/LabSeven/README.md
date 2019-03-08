
# LabSeven modules for VCV Rack

LabSeven modules is a collection of modules for [VCV Rack](https://vcvrack.com/) by Gernot Wurst, [labseven.prospectum.com](http://labseven.prospectum.com/).

See LICENSE.txt for license information.

### Compatibility

LabSeven modules are compatible with VCV Rack 0.6.X. releases.

If you like the modules you can support the development by making a donation, it will be appreciated!. Here's the link: [DONATE](https://www.paypal.me/labseven/)

<img src="http://labseven.prospectum.com/vco-3340_blue.png" height="190"><img src="http://labseven.prospectum.com/vco-3340_gray.png" height="190">

# LabSeven modules

### VCO-3340 Oscillator
The first Lab Seven module: An extended version of a classical VCO from a well-known 80s synthesizer. Comes with standard blue and 'classic gray' skins (simply rename svg files to change skins).

User interface features

    RANGE
        RANGE knob: Selects the pitch of the entire VCO in octave steps
        RANGE Input: Extends the range of the knob by +- 3 octaves from 128' to an upper limit of 0.5'.
            -3V: -3 octaves
            -2V: -2 octaves
            -1V: -1 octave
             0V: No change
            +1V: +1 octave
            +2V: +2 octaves
            +3V: +3 octaves
        Comment: The idea is to equip another module with a (-3, -2, L, M, H, +2, +3) switch or modulate the range input for arpeggiator-like effects.
        Comment: Maximum pitch is limited to 45% of the current sample rate (example: 44.1kHz -> 19,8kHz) with a hard threshold of 40kHz.
    MOD
        MOD fader: Selects the amount of pitch modulation.
        MOD Input: +- 5V -> +- 2.25 octaves of pitch modulation (like the original synth)
    V/OCT: Determines the pitch of the note to be played.
    PWM (pulse width modulation of the square waveform)
        PULSE WIDTH fader: Scales the amount of PWM. The effect depends on the selected PWM source.
        PULSE WIDTH selection
            LFO: Input +- 5V -> 10% to 90% pulse width
            MAN: Manually sets pulse width, ranging from 50% to 90%
            ENV: Input 0V to +10V -> 50% to 90% pulse width
    Source mixer
        Waveforms: Square (with PWM), sawtooth, triangle, sub oscillator (3 modes), coloured noise
        Individual waveform outputs are unattenuated. Faders only affect the MIX output.
        SUB OSC mode selection (SEL) input options
             0V: Manual selection
            +1V: 1OCT DOWN
            +2V: 2OCT DOWN
            +3V: 2OCT DOWN with 25% pulse

Algorithmic (internal) features

    Internal resolution: 192kHz
    VCO algorithm: BLIT (band limited impulse train) + leaky integrator
    Impulse: Original 3340 impulse reconstructed by Blackman-Harris kernel + leaky integration
    Coloured noise: High quality sample loop @192kHz
    Resampling/anti-aliasing: Windowed sinc interpolation

# Version history

### Version 0.6.1 (2019-02-08)
inital release

### Enjoy!
