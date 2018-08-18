# Rack Components

Custom components and resources for VCV Rack Modules.

## Usage

Include as one of your dependencies:

```c
#include "../deps/rack-components/knobs.hpp"
```

Make sure you add `rack-components/res` to your `Makefile` for distribution.

## Components

### Knobs

#### RCKnobRed

A simple 10mm red knob.

#### RCKnobRedSnap

Snap version of the 10mm red knob.

#### RCKnobRedLarge

A simple 12mm red knob.

#### RCKnobRedLargeSnap

Snap version of the 12mm red knob.

### Jacks

#### RCJackSmallRed

A smaller red jack that will fit 2 across in a 3HP module.

#### RCJackSmallDark

A smaller dark ringed jack that will fit 2 across in a 3HP module.

#### JLHHexScrew

A hex screw, courtesy of Jon Heal.
