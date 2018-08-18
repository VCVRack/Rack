
# SynthKit

![ADD](docs/images/add.png) ![SUBTRACT](docs/images/subtract.png) ![AND](docs/images/and.png) ![OR](docs/images/or.png) ![1x8](docs/images/1x8.png) ![1x8 CV](docs/images/1x8cv.png) ![CLOCK DIVIDER](docs/images/clock_divider.png) ![ROTATING CLOCK DIVIDER](docs/images/rotating.png) ![SHIFTING CLOCK DIVIDER](docs/images/shifting.png) ![PRIME CLOCK DIVIDER](docs/images/prime.png) ![FIBONACCI CLOCK DIVIDER](docs/images/fibonacci.png) ![SEQ-4](docs/images/seq4.png) ![SEQ-8](docs/images/seq8.png)

A series of modules for VCVRack.  This are meant to be basic building blocks
of synthesis, that will help you create bigger and more complicated
synthesizers.

Full documentation is [available](docs/README.md).

## Building

Building requires [SynthDevKit](https://github.com/JerrySievert/SynthDevKit),
which will be checked out as part of the build initialization.

```
$ git clone https://github.com/JerrySievert/SynthKit
$ cd SynthKit
$ git submodule init
$ git submodule update
$ make
```

Special thanks to @jonheal (Jon Heal), who designed and provided the look and
feel of SynthKit.  His hard work is what made this a cohesive set of useful
modules.
