# JW-Modules

`	           d(-_-)b        Useful, Musical, and Fun     ¯\_(ツ)_/¯`

Modules for [VCV Rack](https://vcvrack.com/) by Jeremy Wentworth (http://jeremywentworth.com)

Add JW-Modules through [the plugin manager](https://vcvrack.com/plugins.html) or download latest [release from here](https://github.com/jeremywen/JW-Modules/releases) and manually install it.  You can see these modules used in [my youtube videos here](https://www.youtube.com/user/jeremywen/videos).

Donations: [Paypal](https://www.paypal.me/jeremywen)

![All](./doc/all-img9.gif)

## Thing Thing
![thingthing](https://user-images.githubusercontent.com/408586/34801602-8f2ba8e4-f637-11e7-9948-ccb6fdbec6c5.gif)

 * inputs 1 through 4 are +/-5v
 * 5th input and knob are the BALL size uh huh huh :)
 * 6th input and knob zoOM in
 * pairs well with the angle outputs on [Vult Modules Caudal](https://modlfo.github.io/VultModules/caudal/)

## NoteSeq

![NoteSeq](./doc/NoteSeq-img1.png)

#### Basics

* Left side of module is for sequencer control, generation, and modification.
* Right side of module is mainly for controlling the outputs.
* You can click and drag cells in the grid to toggle them on and off.
* Here is a video of a [patch from init](https://www.youtube.com/watch?v=P6cNFfb0w1s)

#### Left

*  **Clock Input:** when source sends a trigger, the sequence moves to the next step
*  **Step Button:** when clicked, the sequence moves to the next step
*  **Length Knob:** the sequence length indicated by the vertical purple line
*  **Mode:** the play mode changes how it moved through a sequence
*  **Reset Input and Button:** resets to first step in sequence (based on the play mode)
*  **Clear Input and Button:** clears the grid
*  **RND Mode:** changes what happens when you trigger the random input or button 
*  **Random Trig Input and Button:** clears the grid and generates random notes (based on the rnd mode)
*  **Random Amount Input and Knob:** determines how full the grid will be when generating random notes
*  **Shift Up Trig Input and Button:** moves the cells in the grid up one row (notes wrap around to the bottom)
*  **Shift Down Trig Input and Button:** moves the cells in the grid down one row (notes wrap around to the top)
*  **Rotate Right Input and Button:** rotates the cells in the grid clockwise 90 degrees
*  **Rotate Left Input and Button:** rotates the cells in the grid counter-clockwise 90 degrees
*  **Flip Horiz Input and Button:** flips the cells in the grid right to left
*  **Flip Vert Input and Button:** flips the cells in the grid top to bottom
*  **Life Switch:** when switched to the right this turns on [conway's game of life](https://en.wikipedia.org/wiki/Conway%27s_Game_of_Life)
*  **Life Speed Knob:** determines how fast to iterate the game of life (clock divided based on clock input)

#### Right

*  **Poly Outputs:** Send out the lowest 16 active notes up from the 'Lowest Note' OR send out the lowest 16 rows up from the 'Lowest Note'.  This is based on the 'Include Inactive Switch' (See Below).  The lights indicate which outputs are sending out values.
*  **Mono Outputs:** can be used to select one note from a column of many notes.
*  **Highest Note Knob:** will set the orange line right above the highest note sent out the poly outputs.
*  **Lowest Note Knob:** will set the yellow line right below the lowest note sent out the poly outputs.
*  **Include Inactive Switch:** [Demo Video](https://www.youtube.com/watch?v=EkFzYxhhn-g) When switched to the right, even if the note isn't 'active'(turned on) then the outputs still directly line up to the 16 lowest notes. 
So for example, you can program a drum beat with a kick at the lowest note and a snare on the 4th row up then connect your outputs to the lowest output and the 4th 
output row up and they will work more like a midi sequencer. When 'Include Inactive' is switched to the left it grabs the 16 lowest notes which are 'active'.
*  **Octave Knob:** octave for the lowest note in the grid
*  **Note Knob:** root note if scaling pitch sent to "OUT"
*  **Scale Knob:** current musical scale or none if turned up all the way to the last value

## Bouncy Balls

![Bouncy Balls](./doc/bouncy-balls-img3.png)

#### Basics

* The top section contains all the inputs.
* The bottom sections contains all the outputs.
* The colors of the inputs/outputs correspond to the ball colors.
* The top two rows have buttons next to the inputs to for manually triggering.
* The next three rows have knobs next to the inputs for manually controlling the values.
* Click in the dark area to lock/unlock the paddle.
* Click the 'ON' button to toggle the paddle visibilty.

#### Inputs

*  **RST:** starts ball back at center with initial velocity
*  **TRIG:** speeds up ball based on velocity settings
*  **VELX:** velocity in the x direction, to the right is positive (towards east), to the left is negative (towards west)
*  **VELY:** velocity in the y direction, to the right is positive (towards south), to the left is negative (towards north)
*  **MULT:** velocity multiplier

#### Outputs

*  **X:** x position for each ball
*  **Y:** y position for each ball
*  **N:** (north) trigger out for each ball that hits the top
*  **E:** (east) trigger out for each ball that hits the right side
*  **S:** (south) trigger out for each ball that hits the bottom
*  **W:** (west) trigger out for each ball that hits the left side
*  **EDGE:** trigger out for each ball that hits any side
*  **PAD:** trigger out for each ball that hits the white pad

#### Modifying the Output Signal

*  **X Scale Knob:** multiplies x output by a value between 0.01 and 1.0
*  **Y Scale Knob:** multiplies y output by a value between 0.01 and 1.0
*  **X Offset Knob:** +/- 5v added to X Output
*  **Y Offset Knob:** +/- 5v added to Y Output

## GridSeq

![GridSeq](./doc/GridSeq-img9.png)

[Video](https://www.youtube.com/watch?v=Bnxzqi5jwcU)

[Example Patch](./doc/gridseq-example-patch-1.png)

#### 16 Step Sequencer

When a direction input is sent a trigger and a cell is entered, a pitch will be sent to the OUT port if the cell's gate is on.

#### Top Direction Inputs 

  *  **Right Input:** on trigger move right one cell and send out value if gate is on for that cell
  *  **Left Input:** on trigger move left one cell and send out value if gate is on for that cell
  *  **Down Input:** on trigger move down one cell and send out value if gate is on for that cell
  *  **Up Input:** on trigger move up one cell and send out value if gate is on for that cell
  *  **RND:** on trigger move one cell in a random direction and send out value if gate is on for that cell
  *  **REPEAT:**  stay on current cell and send out value if gate is on for that cell

  **NOTE 1:** You can click the arrows or words above the inputs to trigger them.

#### Left

  *  **RUN:** values are sent to outputs if running
  *  **RESET Button:** move to top left cell on click
  *  **RESET Input:** move to top left cell on trigger
  *  **GATE OUT:** sends out gate if current cell gate is on and sequencer is running
  *  **OUT:** sends out the current value (pitch knob) for the current cell if the gate is on and sequencer is running

#### Bottom

  *  **ROOT Knob:** root note if scaling pitch sent to "OUT"
  *  **SCALE Knob:** current musical scale or none if turned up all the way to the last value
  *  **RND GATES Button** randomize gate only for each cell
  *  **RND GATES Input** on trigger randomize gate only for each cell
  *  **RND NOTES Button** randomize pitch only for each cell
  *  **RND NOTES Input** on trigger, randomize pitch only for each cell (NOTE: knobs don't update on 'random notes' cv in. If you want knobs to update after cv into 'random notes', right click the random notes button.)

#### Right Click Context Menu

  ![Menu](./doc/GridSeq-Menu-img2.png)

  * **Trigger:** same as SEQ3
  * **Retrigger:** same as SEQ3
  * **Continuous:** same as SEQ3
  * **Ignore Gate for V/OCT Out:** If you want the pitch to continue changing even if the gates are not on, you can right click the module and check 
  'Ignore Gate for V/OCT Out'.  This can create interesting effects, for example, when gate is triggering an envelope with a long release.


## XY Pad

![XYPad](./doc/XYPag-img4.png)

[Video](https://www.youtube.com/watch?v=SF0lwKFIXqo)

Draw your own LFO.  The recorded path is saved and opened.

#### Top
  
  *  **Rnd Left Button:** will cycle through different types of designs and generate random ones
  *  **Rnd Right Button:** random variation of the last design will be generated (so click the left button until you find one you like, then click the right button to make a variation of that one.)
  *  **X Scale Knob:** multiplies x output by a value between 0.01 and 1.0
  *  **Y Scale Knob:** multiplies y output by a value between 0.01 and 1.0
  *  **X Offset Knob:** +/- 5v added to X Output
  *  **Y Offset Knob:** +/- 5v added to Y Output

#### Bottom
  
  *  **Gate In:** plays recorded path while gate is high, retriggers on new gate
  *  **Auto:** loops playback of the recorded path right after releasing the mouse button
      * Or if you just performed a motion while Auto was off, turning it on will play it back
      * Or if Auto is on and you want to stop playback, turn Auto off
  *  **Speed:** plays recorded path from 1x up to 10x faster
  *  **Mult:** multiplies the current speed by larger amounts
  *  **X Output:** +/- 5v based on x position
  *  **Y Output:** +/- 5v based on y position
  *  **-X:** +/- 5v based on inverted x position (darker crosshairs)
  *  **-Y:** +/- 5v based on inverted y position (darker crosshairs)
  *  **Gate Out:** 10v gate out while mouse pressed

#### Right Click Context Menu

Change the playback mode by right clicking the module and selecting of of the playback options:

![Menu](./doc/XYPad-Menu-img4.png)

## SimpleClock

![SimpleClock](./doc/SimpleClock-img6.png)

[Video](https://www.youtube.com/watch?v=DCustAy7xVc)

_Outputs 1/4 notes because this was like vcv rack standard er something at first but I don't agree.  Working on a module with optional note divisions..._

#### Knobs 

  *  **Clock knob:** determines speed of clock
  *  **Random Reset knob:** If down all the way, this does nothing.  If turned up, the chances of sending out a reset trigger on a clock step is increased.

#### Outputs

  *  **Clock output:** Clock sends out a trigger at a certain interval based on the clock knob position.
  *  **Reset output:** trigger is sent out when the clock is started by clicking 'run' or if random reset knob is turned up
  *  **/4 output:** clock divided by 4 ticks
  *  **/8 output:** clock divided by 8 ticks
  *  **/16 output:** clock divided by 16 ticks
  *  **/32 output:** clock divided by 32 ticks

## Quantizer

![Quantizer](./doc/Quantizer-img1.png)

  *  **Root Knob:** root note if scaling pitch sent to "OUT"
  *  **Scale Knob:** current musical scale or none if turned up all the way to the last value

## FullScope

Scope in lissajous mode which takes up the full width and height of the module.  Credit goes to Andrew Belt for the [Fundamental:Scope](https://github.com/VCVRack/Fundamental/blob/v0.4.0/src/Scope.cpp) code.  I just modied that code slightly.

![FullScope](./doc/FullScope-img2.png)

Drag the right side of the module to resize it!

  * Inputs in bottom left corner
    * X input (same as Fundamental Scope)
    * Y input (same as Fundamental Scope)
    * Color input
    * Rotation input
    * Time input
  * Knobs in bottom right corner (same knobs exist in Fundamental Scope)
    * X Position
    * Y Position
    * X Scale
    * Y Scale
    * Rotation
    * Time

#### Right Click Context Menu

Change to the normal scope by unchecking lissajous mode.

![FullScope Menu](./doc/FullScope-Menu-img1.png)

## MinMax

Created to show minumum and maximum voltages in a larger font than the Fundamental Scope 

![MinMax](./doc/MinMax-img1.png)

## WaveHead

Move WavHead up and down based on voltage in.  

![WaveHead](./doc/WavHead-img2.png)

Right click to:

* invert direction 
* change to bidirectional -5 to +5 volts.
* change to snow mode

![WaveHead Menu](./doc/WavHead-menu-img2.png)

## Other goofy crap

![goofy](https://user-images.githubusercontent.com/408586/34752974-6934555a-f583-11e7-8fd1-bc378e455839.gif)

## Building

Compile Rack from source, following the instructions at https://github.com/VCVRack/Rack.

Check out JW-Modules into the `plugins/` directory

Then run:

	make

To make plugin zip:

	make dist

SEE ALSO [Dev FAQ for VCV Rack](https://github.com/jeremywen/JW-Modules/wiki/Dev-FAQ-for-VCV-Rack)
