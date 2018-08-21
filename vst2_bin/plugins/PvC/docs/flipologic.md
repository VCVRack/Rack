# FlipOLogic <img align="right" src="images/flipologic_100.png">
... is a 6HP Logic Gate / Clock Divider

## Details
combines a chain of flip flops with some logic gates.
the flipflops at the bottom provide 2/4/8 clock divisions of the FLIP Input.
divided outs are routed to the LOGIC Inputs at the top as well as to the side columns.


### I/O
LOGIC Ins A, B, C  

FLIP Input : triggers FLIP(triggers FLOP (triggers FLAP))  
left column (AND) Input  
right column (XOR) Input  

LOGIC Outs  
center: AND, NAND, OR, NOR, XOR, XNOR of the A,B,C inputs  
left: AND of left column input vs LOGIC Out  
right: XOR of right column input vs LOGIC Out  

### Controls


## Changes
0.5.8 - initial version  
