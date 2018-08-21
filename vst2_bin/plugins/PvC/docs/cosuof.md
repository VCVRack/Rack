# CoSuOf <img align="right" src="images/cosuof_100.png">
... is a 4HP Comparator / Substractor / Offsetter

## Details
inspired by the D-A167

### I/O
POS Input  
NEG Input  
  
SUM Output : POS IN - NEG IN + OFFSET (!)  
GATE(/NOT) Outputs : GATE is high when SUM is over 0, low when 0 or below.  

### Controls

Input Attenuators: Input x [0..1]  
Sum Offset: [-10..10]V  
Gap: Hysteresis Control for the Gate 

## Changes
0.6.0 - 4HP layout, experiment: gated outs  
0.5.8 - initial version  
