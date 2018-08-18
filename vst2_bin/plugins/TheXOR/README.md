# The XOR
Plugins for your joy. Starting from this version (0.6.2), all inputs and outputs are
color-coded as in this figure:

![legend](/res/png/legenda.png?raw=true "legend")

# Klee
A rather complete Klee Sequencer, with some enhancements...
More information: https://www.google.com/search?q=klee%20sequencer%20pdf

![Klee](/res/png/klee.png?raw=true "The Klee")

# M581
Inspired by the System 100 Sequencer
More information: https://www.google.com/search?q=m185%20step%20sequencer

![M581](/res/png/m581.png?raw=true "M581")

# Renato
X/Y Sequencer, dedicated to the great Renato!
Vaguely inspired by https://www.google.com/search?q=renee%20sequencer

![Renato!](/res/png/renato.png?raw=true "Renato")

# Z8K
4 colors for 4 sequencers. Matrix reloaded.
A tip: https://www.google.com/search?q=z8000%20sequencer

![Z8K](/res/png/z8k.png?raw=true "Z8K")

# Spiralone
Hypnotic!
Based on https://www.google.com/search?q=cyclic%20sequencer

![Spiralone](/res/png/spiralone.png?raw=true "Spiralone")

# Burst
Will likely cause serious damage to your ears
Idea taken from here: https://www.youtube.com/watch?v=YlnHFTrhJnM

![Burst](/res/png/burst.png?raw=true "Burst")

# pwmClockGen
Most of these sequencers have their gate outputs dependent
from the clock pulse cycle; for this reason there is a clock generator
with controllable PWM

![pwmClockGen](/res/png/pwmclockgen.png?raw=true "pwmClockGen")

# multiQuantizer
The CV outputs of all the sequencers are not quantized, so a Quantizer is handy, expecially
if it is modulable...

![multiQuantizer](/res/png/quantizer.png?raw=true "multiQuantizer")

# Uncertain
Unstable voltages
Recipe:
1/2 doepfer a-149-1
1/4 buchla 266e
1/4 gauss sigma control :-)

![Uncertain](/res/png/uncertain.png?raw=true "Uncertain")

# Attenuator
Calm down your boiling voltages

![Attenuator](/res/png/attenuator.png?raw=true "Attenuator")

# Boole
There are 10 kind of people: those who can understand boolean logic, and the others....

![Boole](/res/png/boole.png?raw=true "Boole")

# Switch
Circuit breaker

![Switch](/res/png/switch.png?raw=true "Switch")

# Mplex
Multiplexer

![Mplex](/res/png/mplex.png?raw=true "Mplex")

OSC!!!!!
For Microsoft Windows users only, there is the possibility to control the sequencers through OSC. 
To do so, you must compile the plugins with the command:

make -f makefile_osc

and compile oscServer, a background process that acts like a bridge between VCV and OSC (find id
@ https://github.com/The-XOR/oscServer).
You know that oscServer is connected because the 'DIGITAL' led in the sequencers goes lit.
oscServer is written in C# and needs at least Microsoft .NET Framerwork 4.5; in RackPlugins\digitalExt\osc\Repository
there are some Liine Lemur patches (jzml). If you want to program your own OSC patches, 
all the mappings are listed in RackPlugins\digitalExt\osc\RepositoryControl List.txt .
What else? Ah, yes, oscServer can control ALL the sequencers SIMULTANEOUSLY ;-) so you can program
(and send to me) a mega-patch able to control... everything. 
THIS FEATURE IS HIGHLY EXPERIMENTAL, I couldn't test it thoroughly and it could (and will) be buggy.
