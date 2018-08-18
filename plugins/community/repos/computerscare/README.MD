
# computerscare modules

Modules for VCV Rack.  I take great pride in the graphic design.

## Father & Son Patch Sequencer



If you've ever wished that patch cables could be sequenced, this is the module for you.

![ComputerscarePatchSequencer](./doc/computerscare-patch-sequencer-basic.png)

Father & Son Patch Sequencer allows for the sequencing of connections.  It's somewhat like a giant sequential switch.

### Buttons:
**Patch Matrix:** Grid of 100 buttons.  When the button is lit in green, it means that the input column and output row is connected for the step that is being edited.  When the button is lit in red, it means that the input column and output row is connected for the currently active step.

**Next Active Step:** Moves to the next active step.  If the sequence length is set to greater than 1, the red lights in the patch matrix will change.

**Previous/Next Editing Step:** Move to edit the previous or next step.  Note that this will always cycle from 1 to 16 no matter what number of steps the knob is set to.

### Knobs:
**Number of Steps:** Choose between 1 and 16 steps.

### Inputs:
**Clock:** Moves to the next active step.  If the currently active step is equal to the number-of-steps, it will go back to step 1.  Does the same thing as the 'Next Active Step Button'.

**Randomize:** Randomizes the patch matrix for the current editing step.  Randomly enables exactly one input for each output.  Does the same thing as the 'randomize' selection from the right-click menu.

**Input Column:** 10 input jacks for any sort of signal you want.

### Outputs:
**Output Row:** 10 output jacks which output the sum of the signals in that particular column.

### Possible applications:

**Triggers:**

**Envelopes / CV:**

**Audio:**


*Inspired by Strum's Patch Matrix, Bidoo's ACnE Mixer, and Fundamental Sequential Switch*

## Debug
A simple module for debugging.  Displays a list of the last 16 voltages read from IN, with the most recent one at the top.

![ComputerscareDebug](./doc/computerscare-debug-basic.png)

### Inputs:
**Trigger:** When a trigger signal is detected, the voltage at the Input will be placed at the top of the list of voltages, and the voltage at the bottom will be discarded from the display.

**Input:** Any sort of signal you wish to probe.

**Clear:** Reset the list of voltages back to all zeros.

### Buttons:
**Trigger:** Manual Trigger

**Clear:** Manual Clear

