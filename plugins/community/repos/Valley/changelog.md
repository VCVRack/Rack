# ValleyRackFree Change
Log

## Changes

### 0.6.9

    • [Fixed] Corrected a mistake where the modulation depth and shape input jacks were mixed up.

### 0.6.8

    • [Fixed] Corrected the dry gain staging mistake that 0.6.7 introduced. We never speak of 0.6.7 again....

### 0.6.7

    • [Fixed] "Diffuse input" bypass switch correctly taps from after the pre-delay

### 0.6.6

    • [Update] Plateau now features a CV input for Predelay. The sensitivity for this input is set
      in the context menu.

### 0.6.5

    • [Fix] Fixed stereo input bug on Plateau

### 0.6.4

    • [New Module] Plateau reverb! A plate reverb with a twist. Can add large expansive textures to your
    sounds, as well as be able to be tuned and excited at very short reverb times.
    • [Update] Performance optimisations to Topograph and µGraph.
    • [Update] All modules now have a dark jack look.

### 0.6.3

    • [Fixed] Dexter sample rate bug. If Dexter is inserted at a sample rate other than
    44.1K, the tuning would be incorrect until the sample rate was changed.

### 0.6.2

#### [New Module] µGraph
    • µGraph is a micro version of Topograph.

#### Topograph
    • [Update] Fixed hanging gate issue when using external clock in gate mode.

#### Dexter
    • [New] Full inversion mode for chords that span more than 1 octave. Notes can now be optionally
    inverted above the highest note rather than exactly 1 octave.

### 0.5.7
    • [Major Update] Warning, this update will break old patches.
    Open "Please READ ME first.pdf" for a guide on how to fix your old patches.

### 0.5.6
    • [Fixed] Dexter is now tuned to C not A. Please adjust Dexter to be in tune with your current patches.

### 0.5.5

Say hello to Dexter ;)

### 0.5.4

#### Topograph
    • [New] CV trigger for Run
    • [New] Run can either be Toggled (default) or Momentary.
    • [New] Dynamic text that displays BPM and lengths for each channel in Euclidean mode.

### 0.5.3

#### Topograph
    • [New] Swing!
    • [New] Alternate, light panel graphic. Accessible from the right click menu.
    • [Update] Graphical tweaks.

### 0.5.2

#### Topograph
    • [New] Two new drums modes: Olivier and Euclidean. Current drum modes are Henri, Olivier and Euclidean.
    • [New] Three ext clock resolution modes. Module can respond to 4, 8 and 24 ppqn clocks.
    • [New] Gate modes. Toggle between 1ms pulses and 50% duty cycle gate mode on the trigger outputs.
    • [New] Accent out alt modes. Can toggle between independent accents or [Accent / Clock / Reset] modes.
    • [Update] Brighter button graphics.
    • [Update] Ext clock input trigger sensitivity improved. User complained that some modules could not
    trigger it with certain gate signals.

### 0.5.1
    • [Update] Fixed CV control.

## Todo

#### Topograph
    • Fix text flicker
#### Trixie
    • Finalise the design and prepare for release
