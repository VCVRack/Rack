This is eguitar by Gary Scavone, McGill University, 2012.

This is a program to create a simple electric guitar model using
the STK Guitar class.  The is model is derived in part from an
implementation made by Nicholas Donaldson at McGill University in
2009. The distortion model is poor, using a simple soft-clipping
expression provided by Charles R. Sullivan in "Extending the
Karplus-String Algorithm to Synthesize Electric Guitar Timbres with
Distortion and Feedback," Computer Music Journal, Vol.14 No.3, Fall
1990. Other distortion models would be better, such as that found
in Pakarinen and Yeh's "A Review of Digital Techniques for Modeling
Vacuum-Tube Guitar Amplifiers," Computer Music Journal, Vol 33
No. 2, Summer 2009.

This program performs simple voice management if all noteOn and
noteOff events are on channel 0.  Otherwise, channel values > 0 are
mapped to specific string numbers. By default, the program creates
a 6-string guitar.  If the normalized noteOn() velocity is < 0.2, a
string is undamped but not plucked (this is implemented in the
stk::Guitar class).  Thus, you can lightly depress a key on a MIDI
keyboard and then experiment with string coupling.

The Tcl/Tk GUI allows you to experiment with various parameter
settings and that can be used in conjunction with a MIDI keyboard
as: wish < tcl/EGuitar.tcl | ./eguitar -or -ip -im 1

For the moment, this program does not support pitch bends.

In the eguitar directory, type:

> make

to compile and then

> ElectricGuitar.bat

to run the program with the Tcl/Tk GUI.

There are many improvements that could be made to this project.  In
particular, you could record real body responses from different
guitars and use those with the Guitar class.  As well, you could
improve the distortion model and perhaps add some typical electric
guitar effects, such as an echo.  If you find any bugs, please let me
know!
