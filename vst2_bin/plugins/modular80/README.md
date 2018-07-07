# A collection of modules for VCV Rack

This repository contains a collection of modules for [VCV Rack](https://vcvrack.com/),
the open-source virtual modular synthesizer.

The minimum supported VCV Rack version is **0.6**.

# Overview of modules

![modular80](/modular80.png)

## Logistiker

The `Logistiker` module is based on the [Logistic Map](https://en.wikipedia.org/wiki/Logistic_map),
a non-linear dynamic equation, which for certain input parameters exhibits [chaotic behavior](https://en.wikipedia.org/wiki/Chaos_theory).

The **RATE** knob controls the update rate of the internal clock. It has no function, if an
external clock signal is connected to the **EXT CLOCK** input.

The **R** knob, and the corresponding input, controls the **R** variable of the equation.
The [Wikipedia page](https://en.wikipedia.org/wiki/Logistic_map) has a good overview of the effect
of different **R** values. The default value corresponds to the onset of *Chaos* in the system.

The **X0** knob sets the initial starting value for the **X** variable of the equation.

If the **RESET** button is pressed, or a positive edge arrives at the **RESET** input,
the model starts over from the value set by the **X0** knob. The reset takes effect at the
next rising edge of the (internal or external) clock signal.

[YoutTube Module Demo](https://youtu.be/xGSvLBChjzk)

# License

Source code licensed under BSD-3-Clause by Christoph Scholtes.
