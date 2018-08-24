# Bidoo's plugins for [VCVRack](https://vcvrack.com)

<!-- Version and License Badges -->
![Version](https://img.shields.io/badge/version-0.6.10-green.svg?style=flat-square)
![License](https://img.shields.io/badge/license-BSD3-blue.svg?style=flat-square)
![Language](https://img.shields.io/badge/language-C++-yellow.svg?style=flat-square)

![pack](/images/pack.png?raw=true "pack")

## How to

You can find information on that plugins pack in the [wiki](https://github.com/sebastien-bouffier/Bidoo/wiki). When doing tests it happens that I record a video so you may find some ideas on how to use those modules [here](https://www.youtube.com/bidoo).

## Last changes

21/08/2018 => 0.6.10

rabBIT redesign

20/08/2018 => 0.6.9

rabBIT is a 8 bit reducer/reverser

09/07/2018

Changed the way wav files are loaded and saved => OUAIve and cANARd. Changed the way onsets are detected in cANARd. Fix play mode saving on close for OUAIve.

This version is compliant with the last version I have of Rack SDK so maybe my pack will be available thru Rack again in 0.6.2.

13/05/2018 => 0.6.6

antN goes away from mpg123 and is based now on minimp3 so maybe my pack will be available thru Rack again.

Some changes on μ that has an offset param now. I changed the fine tuning of step length so it is easier to setup. Step length is 100% by default.

11/05/2018 => 0.6.5

μ. can be viewed as a step in a sequence. Link some of them and you will build a full sequence path. When moving the cursor over the knobs and ports the display shows a description and the current value of the focused object. As usual black ports are inputs and the red ones outputs. To start experimenting link some units with bpm ports so the first one will become the master tempo for the chain. Link gate and CV at the bottom to create the path for the signal. Link step end ports with step start ones to build the order of steps. At that stage you can use the alternate end of step output port to fork the path and adjust the corresponding probability. Maybe you need that under some circumstances a step is stopped .. use the inhibit port. Once the chain is made set the tempo with the first step (one fast knob and a dedicated one for decimals). Then set the length of each step (two controls too as for bpm) 100% means a quarter for a 4/4 signature, 50% one eighth, 200% an half etc... To introduce swing use decimal values for the step length. Now choose the length of the trigs in percentage of the step length, the CV value, the probability of the trig to be played, the probability of the alternate port to be use instead of the standard end of step port, the trig repeat count and the distance between retrigs. If the trig length is greater than the distance then the trigs will merge. You have two small led buttons between gate and CV in and out ports. They activate the stack mode on those ports. By default a step plays its trigs when active and let the signal pass thru the ports when not active. If you activate the stack mode the incoming signal will be added to the trigs data when the step is active. You can manually launch a step with the left top led button and mute the step with the right one. Modulation ports are available for almost all parameters.

This release comes with another module Σ which is a simple merge tool .. essential with μ in order to merge end of steps ports or gate and CV ones.

## License

The license is a BSD 3-Clause with the addition that any commercial use requires explicit permission of the author. That applies for the source code.

For the image resources (all SVG files), any use requires explicit permission of the author.

## Donate

If you enjoy those modules you can support the development by making a donation. Here's the link: [DONATE](https://paypal.me/sebastienbouffier)
