# mtsch plugins

My modules for [VCVRack](https://github.com/VCVRack/Rack). If you find any bugs,
or make something cool with them, let me know.

### Rationals

![rationals](images/Rationals.png)

Modify a volt/octave input by a rational number. Outputs constants on no input.
The rationals can be controlled by CV inputs at the top. Useful for making
music in [just intonation](https://en.wikipedia.org/wiki/Just_intonation) (see
[`examples`](examples) for basic examples).

Multiple Rationals can be daisy chained, see
[`examples/rationals-bassline.vcv`](examples/rationals-bassline.vcv). Use
Sum to combine outputs of multiple Rationals.

### Trigger Panic!

![triggerpanic](images/TriggerPanic.png)

A triggered/tempo synchronized delay. Starts playing/writing into a buffer on
trigger input. Mix and feedback are CV controlled. The buffer holds 10 seconds
of audio. Trigger it with a regular clock to get a tempo delay (see
[`examples/triggerpanic-rationals-dubtechno.vcv`](examples/triggerpanic-rationals-dubtechno.vcv)).
Trigger it with an irregular clock to produce unpredictable glitchy noises (see
[`examples/triggerpanic-breakcore.vcv`](examples/triggerpanic-breakcore.vcv)).

Optionally, you can use the AUX input and output to feed audio into the feedback
loop or thread the feedback loop through an effect chain.

### Sum

![sum](images/Sum.png)

Add, subtract or mute signals.

Note that this adds the signals directly, so make sure to attentuate the output
signal when adding audio signals.

## Thanks

Digital display in Rationals taken from
[luckyxxl](https://github.com/luckyxxl/vcv_luckyxxl).

Thanks to [alikins](https://github.com/alikins) for porting to the v0.6 API.
