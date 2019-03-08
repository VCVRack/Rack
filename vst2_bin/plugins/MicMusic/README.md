
# VCV Rack MicMusic plugins

## Distortion "CuTeR"
![](https://github.com/very-cool-name/MicMusic-VCV/blob/master/doc/cuter_layout.png)

It cuts all waveshape above HIGH and below LOW levels.

```
_____________________________________________________ HIGH
       /     \             /     \             /   
      /       \           /       \           /    
     /         \         /         \         /     
    /           \       /           \       /      
   /             \     /             \     /       
  /               \   /               \   /        
 /                 \ /                 \ /            LOW
‾‾‾‾‾‾‾‾‾‾‾‾‾‾‾‾‾‾‾‾‾‾‾‾‾‾‾‾‾‾‾‾‾‾‾‾‾‾‾‾‾‾‾‾‾‾‾‾‾‾‾‾‾   
```
Pic. 1 CuTeR functionality

HIGH and LOW levels are controlled with corresponding knobs.
Also CV inputs are available to modulate HIGH and LOW levels.
CV knobs vary amount of modulation.

## Adder "SeVen"
![](https://github.com/very-cool-name/MicMusic-VCV/blob/master/doc/seven_layout.png)

Adds values from seven channels.
If no input is connected - channel outputs constant voltage. Zero by default, knob regulates it up to 1.
If input is connected - knob regulates input volume. Switch regulates sign of addition. Light toggle is channel mute.
Bottom light toggle is mute for all channels.
Based on [A-185-2 Precision CV Adder](http://www.doepfer.de/a1852.htm).
