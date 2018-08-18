# Formula

A formula module for VCV Rack, by Frank Buss, based on BokontepByteBeatMachine.

This plugin provides 4 inputs: w, x, y and z. In the text field you can write a
formula for the output. For example `x+y` would be a simple adder.

![alt text](add.png "Add example")

Some functions take 2 arguments. For example you could use the max function, to
get either the input x or the input y, depending on which voltage is higher:
`max(x, y)`.

The red LED is blinking if there is a parsing error. If it is on, the formula is
running.

The integrated knob is available as the variable `k`. The integrated -1 / 0 / +1
radio buttons are available as the variable `b`.

The frequency field is another formula, all variables can be used for it. The
result of this formula is used as the frequency for an integrated sawtooth
oscillator, available as the variable `p` (phase). It starts at 0 and wraps
around at 1.

It works with CV and audio signals. The output is clamped to -5V/+5V, if the
clamp toggle button is pressed. Some more examples for what you can use it:

# Waveform Generator

You can also use functions, see later for a list of all available functions, and
you can use the variable `pi`. In combination with the integrated sawtooth, this
can be used as a mathematically wave generator. The sawtooth has to be a simple
generator, with no anti-aliasing algorithms, because the falling edge of the
ramp needs to be with no smoothing in one sample. Otherwise there would be
samples from the rising part created, which results in distortions. The
integrated oscillator has this property.

So a sine wave with the usual 10Vpp can be generated with `sin(2*pi*p/5)*5`. In
the freq field you can enter the frequency formula, which could be a simple
number as well, like 440 for 440 Hz.

More complex formulas are possible as well, you can enter multiple lines. For
example this approximates the square wave function with additive synthesis with
the base frequency and the first 3 harmonics of the square wave:

```
4*(
sin(2*pi*p)+
sin(6*pi*p)/3+
sin(10*pi*p)/5+
sin(14*pi*p)/7
)
```

![alt text](formula.png "Additive synthesis example")

The variable `k` in the frequency field allows to use the integrated knob to
adjust the frequency. You could use any of the inputs as well to modify it and
to build a full featured VCO.

# Level Converter

Another wider range of applications is parameter adjustment. For example the
VCO-1 in Fundamental has a V/OCT input. This means when you create a new
instance with the initial frequency for C4 (261.626 Hz) and apply 1 V, the
frequency will be increased by one octave to C5 (523.252 Hz).

But what if you have a linear V/Hz CV signal and want to convert it in a way
that it can be used with a V/OCT input? Then you need to calculate the log to
base 2 of the input level, which is the function log2. So for `x=1.5` it would
be exactly `1.5 * 261.626 Hz = 392.439 Hz`.

![alt text](oct-hz.png "Level converter")

# Relational operators

The relational operators (<, >, <= and >=) allow other interesting applications.
For example if you want to select one of two signals, based on the CV level of
another signal (a multiplexer), then you could use the formula
`((w<=5)*x+(w>5)*y)*b`. This would output `x`, if the level at the `w` input is
5V or less, otherwise it outputs `y`. It works because if `w<=5` is valid, the
term evaluates to 1, otherwise to 0. Additionally the multiplication with the
variable `b` allows to mute the signal, if the 0 radio button is selected.

![alt text](mux.png "Multiplexer")

# Boolean operators

Finally there are the boolean operators `&` for logical-and, `|` for logical-or
and `!`for not. This can be useful, if you want to test two signals with the
relational operators, like if x and y has some minimum value at the same time,
only then output the value of z. Or you could even use it to implement a 10 step
sequencer with chromatic scale:

```
((p>=0.0&p<0.1)*1+(p>=0.1&p<0.2)*5+
(p>=0.2&p<0.3)*8+(p>=0.3&p<0.4)*9+
(p>=0.4&p<0.5)*10+(p>=0.5&p<0.6)*12+
(p>=0.6&p<0.7)*10+(p>=0.7&p<0.8)*9+
(p>=0.8&p<0.9)*8+(p>=0.9&p<1.0)*5)
/12
```

The knob can be used to change the frequency. Try it and listen to it, then
change the factors interactively. Unlike a normal step sequencer, you can adjust
the length of the indivual steps as well by changing the p time windows, lots of
fun :-)

![alt text](sequencer.png "Sequencer")

# Quantizer

You can use the `floor` function to implement a simple quantizer. This function
rounds the value down to the next integer value. For example `floor(1.8)=1`. For
a chromatic quantizer, only the voltages 0, 1/12, 2/12 etc. are allowed. This
can be enforced with this formula: `floor(k*12)/12`. With the variable `k` you
can use the integrated knob to adjust the output in the range from -1 to 1
octave, using a chromatic scale. Of course you can use the inputs as well
instead of the knob, or add both, like `floor(x)+k`, which would quantize the
input `x`, but would allow a fine adjustment with the knob.

![alt text](quantizer.png "Quantizer")

# The geeky details

The following functions are implemented:

acos, asin, atan, atan2, cos, cosh, exp, fabs, fmod, log, log2, log10, pow, sin,
sinh, tan, tanh, sqrt, ceil, floor, max, and min. See here for a detailed
description of each function: http://www.cplusplus.com/reference/cmath/

The full BNF grammar for the parser looks like this:

```
expression = and-expression [or-operator and-expression]
and-expression = equal-expression [and-operator equal-expression]
equal-expression = relational-expression [equal-operator relational-expression]
relational-expression = sum [relational-operator sum]
sum = [sign] term {additive-operator term}
term = factor {multiplicative-operator factor}
factor = power {power-operator power}
power = power_operand {power-operator power_operand}
power_operand = unsigned-real | variable | (expression) | function-call
or-operator = |
and-operator = &
equal-operator = == | !=
relational-operator = < | > | <= | >=
additive-operator = + | -
multiplicative-operator = * | / | %
power-operator = ^
not-operator = !
variable = [a-z, A-Z, _]+ [a-z, A-Z, 0-9, _]*
unsigned-real = [0-9]* [.] [0-9]* [real-exponent]
real-exponent = [e|E] [+|-] [0-9]+
```

I wrote the formula library in 2001, here is the original page with a function
plotter as another example: http://www.frank-buss.de/formula/index.html
