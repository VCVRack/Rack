# SlimSeq <img align="right" src="images/slimseq_100.png">
... is a 10HP 16 step sequencer / 16-to-1 sequential switch  


## Details
compact 16 stepper with a few tricks.  
concept loosely based on Transient Modules 8s.  
code based on martin lueders' sequential switch.  

### I/O
CLK trigger: jump to next position  
REV switch: forward/reverse (in random mode: walking/hopping)  
RND switch: normal/random direction  
HLD switch: hold mode  
RST trigger: jump to zero position  

16 cv inputs  

1 cv output  

### Display/Controls

#### per step
LED - shows current step (full brightness) and reset position (dimmed)  
  Knob - adjust voltage [-5..5] / attenuvert input signal (if connected)  
Label - set reset position (in hold/unclocked mode sets current position)  

#### center section
LED - shows output level  
  Knob - attenuvert output signal


## Changes
__0.6.0__ - new 10HP layout  
__0.5.8__ - initial version  
