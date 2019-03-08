# arjo_modules

This plugin consists of 3 modules: `seq`, `count` and `switch`, each 3HP wide. 
The modules are simple on their own, but when combined they can become a powerful sequencer. It can easily be used for polyrhythmic sequences as well. 

All modules have a `CLK` (clock) and a `RST` (reset) input, which both take a pulse as input. 


<img src="https://i.imgur.com/xLtNqpK.png" height="400"> <img src="https://i.imgur.com/Ln6EuhC.png" height="400"> <img src="https://i.imgur.com/FZLgVO9.png" height="400">

- `seq` cycles through the 8 values of the knobs and outputs the current value. 
- `count` consists of 4 counters. Each output a pulse every x clock steps, where x can be controlled by the knob with a range of 1-16 steps. 
- `switch` cycles through the 8 values of the inputs and outputs the current value, just like `seq`. With the difference being that it has inputs instead of knobs. 

## Examples

### 32 step sequencer
2 `count` modules, 4 `seq` modules, 1 `switch` module

- Set the first `count` module on 4 steps and connect `CLK` to a pulse generator (LFO-1 in this example). 
- Set the second `count` module on 8 steps and connect its `CLK` to the output of the first `count` module. 
- Connect all `seq` modules in the same way: Connect `CLK` to the pulse generator, `RST` to the output of the first `count` module and the output to the 4 consecutive inputs on the `switch` module.
- Connect the switch module's `CLK` to the output of the first `count` module. Connect `RST` to the output of the second `count` module. Connect the output to anything (`V/OCT` of the VCO-1 in this example). 

![](https://i.imgur.com/h9fDwUI.jpg)

### 64 step sequencer
1 `count` module, 8 `seq` modules, 1 `switch` module

- Set the `count` module on 8 steps and connect `CLK` to a pulse generator (LFO-1 in this example)
- Connect all `seq` modules in the same way: Connect `CLK` to the pulse generator, `RST` to the output of the `count` module and the output to the 8 consecutive inputs on the `switch` module. 
- Connect the switch module's `CLK` to the output of the `count` module. Connect the output to anything (`V/OCT` of the VCO-1 in this example)

![](https://i.imgur.com/C8cd63O.jpg)
## Releases
https://github.com/ArjoNagelhout/arjo_modules/releases
