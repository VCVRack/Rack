# qwelk VCVRack modules

![Roster](/img/modules.png?raw=true "Qwelk Modules")

## Chaos and Automaton
Chaos and Automaton are both based on [elementary cellular automata](http://mathworld.wolfram.com/ElementaryCellularAutomaton.html).
Their RULE is determined by their 8 inputs (as a binary number, MSB being the bottommost input). Next generation is produced
when a trigger signal is received over the STEP inputs.
Chaos allows you to mess with its current generation via its TRIG inputs which upon receiving trigger (positive) signal have their values set to ON.
SCAN affects how the binary number (output through the VALUE outputs) is generated, negative SCAN (red light) means MSb is on top
and positive SCAN (green light) means MSB is at the bottom of the module (the defult).
You can also tinker with SCAN, STEP and the cells themselves by clicking on them. 
Consider these modules as 8-bit machines. In order to patch [Rule 30](http://www.wolframalpha.com/input/?i=rule+30) into these
modules you must first look at the 8-bit binary number representing 30: 00011110 (the MSb is the leftmost digit). Now, considering
the input MSb is located at the bottom of these modules you will need to have positive signals present at the second, third, fourth and the fifth
RULE inputs just as, reading from right to left, the second, third, fourth and the fifth bits of the binary word above.
Try toggling the cells on and off on the BLUE column manually while providing a clock signal of some sort to the STEP input to see the modules come to life.

## NEWS
NEWS takes the input signal and interprets it as directions (hence the name, North East West South) and 'plots' it.
Note that when patching inputs with knobs above them, the input is ADDED to the knob's value. That's not and attenuator.
Here's what the inputs and switches do:

	in:	The input signal to plot
	hold:	Sample & Hold on the input signal. Whenever triggered the signal present as in is stored
		and plotted until the next trigger.
	o:	The origin from which plotting begins. 
	int:	The intensity of the output when the module is not running in GATE MODE. See below.
	wrap:	This changes the way the input signal is looked at.
	bi:	When down, the module's base is shifted down to -5v.
		Might appear useless until you run the module in audio rate.
	m:	Selects between the two available plotting modes. See which one you prefer.
	g:	GATE MODE. When in the upper position, the module acts as a sequencer, providing Gate outputs.
		When switched, the module's output range is divided into parts inversely proportional to 'int'.
		This way you can get gate signals of varying peak values.
		Try modulating 'int' with a simple LFO while in this mode for fun times.
	r:	Simplifies the input to fuck.
		For those interested, this basically rounds the decimal signal value to an integer.
		I don't know why I haven't removed this. 
		EDIT: It's actually not that bad!
	c:	Clamp! This module is designed in a way that when the intensity of an output peaks beyond +10v,
		the output resets and readjusts (to output value - 10v, in a way.)
		When toggled down, if the intensity of an output peaks, the output stays there and doesn't reset.
	s:	Smoothes out the output values' transients.

## Byte
The Byte module constructs a binary number out of the patched input signals.
The MSb is the bottommost input by default when not patched and switched manually, and once patched the bottommost
upon receiving a positive signal and the topmost upon receiving a negative signal.
Note that the COUNT output outputs the number of positive signals over the number of _patched_ inputs! i.e. if only one input
is patched and the signal present at it is negative, COUNT will output 10v. But if two of the inputs are patched
and only one of them has a positive signal present, COUNT will output 5v, as that's (number of positive signals:1 / number of _patched_ inputs:2) * 10.

## Wrap
Wrap is Routing module. Whatever you patch in will be present at the corresponding output. You can change the position of the corresponding output by feeding a +/- signal to the wrap input.  

## Column
Column is a Mixer/Averager. Righthand outputs output the lefthand input signal unaffected, so you can patch a bunch of column together side-by-side for some kind of a matrix mixer. Each input positioned in the middle and exactly above each lefthand input is an UPSTREAM input. Here is what the switches do:

	avg:	When toggled down, the module runs as an averager. 
	X:	When toggled down, if operating as mixer, the lefthand-side inputs are multiplied by the inputs above them (the UPSTREAM inputs).
		If operating as an averager, the UPSTREAM inputs will act as weights of the lefthand inputs and the module outputs
		Weighted Averages.

### Can you really make music with these modules? They look boring.
Yes. [Sort of.](https://co-dependent.bandcamp.com/album/code147)
