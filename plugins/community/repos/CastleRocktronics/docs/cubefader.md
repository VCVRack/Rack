Cubefader
=========

<img align="right" src="images/cubefader.png">

## Overview

Cubefader is an 8 input to 1 output crossfader. A simple crossfader takes two inputs, and mixes between them from "left" to "right" allowing a smooth transition between them. Cubefader does the same thing, but instead it also mixes from front to back and top to bottom, allowing you to smoothly transition between 8 inputs.

## Getting started

First, make sure the toggle at the bottom-left is set to "unipolar" (we'll discuss this switch more under the [CV Polarity Switch](#cv-polarity-switch) section) and leave the X, Y and Z inputs unconnected. Connect OUT to your audio output. Now take an audio source (like the output of a VCO) and try connecting it to each corner of the cube. Only the left-bottom-front corner causes any sound to come out.

### X - Moving left-to-right

Try connecting different VCOs to the left-bottom-front and right-bottom-front corners of the cube. Now, when you add a signal to the X input, it will fade between these two corners with 0V being the left-bottom-front input only, and 10V being the right-bottom-front corner only.

### Y and Z - The rest of the cube

You will be unsurprised to find that the Y input fades from top to bottom and the Z input fades from front to back. If all CV inputs are 0V (just like when we had not yet attached any CVs) then the left-bottom-front input will be the only thing to make it to the output. If all CV inputs are 10V then this it is the right-top-back corner we will hear.

## CV Polarity Switch

The X, Y and Z inputs can be switched so that they accept bipolar CV inputs instead. In bipolar mode, the full -10V to 10V is needed to fade to the corners, and with no CV input at all you will hear all 8 inputs equally because as 0V now fades to the middle (it is 5V in unipolar mode)

## The CV trimpots

The X, Y and Z inputs each have trimpots above them. These boost, attenuate and/or invert the CV signal before it reaches the fading. This can be used to bump a 0-5V output up to 10V but be aware that anything above 10V or below 0V is clipped in unipolar mode. This is extended out to -10V to 10V in bipolar mode.
