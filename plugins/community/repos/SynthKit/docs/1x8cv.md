# 1x8 CV

![1x8 CV Module](images/1x8cv.png)

The **1x8 CV** module accepts a single input and outputs what it receives on
each of the 8 output ports.  Each output is controlled by a gate input, when
the input is low (less than `1.7`), then the output will be `0`.  If the gate
input is high (`1.7` and above), then the input is ouput on the port.
