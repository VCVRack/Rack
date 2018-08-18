# SynthDevKit

SynthDevKit is a set of utilities for development of software and hardware synthesizers.

All effort is made to test correctness and functionality.  SynthDevKit is meant
to be a dependency, not a stand-alone project.

[![Build Status](https://travis-ci.org/JerrySievert/SynthDevKit.svg?branch=master)](https://travis-ci.org/JerrySievert/SynthDevKit)
[![Coverage Status](https://coveralls.io/repos/github/JerrySievert/SynthDevKit/badge.svg?branch=master)](https://coveralls.io/github/JerrySievert/SynthDevKit?branch=master)

## Building and Testing

SynthDevKit lives on [GitHub](https://github.com/JerrySievert/SynthDevKit).

### Building

```
$ git clone https://github.com/JerrySievert/SynthDevKit.git
$ cd SynthDevKit
$ make
```

This will compile the source, ready for direct inclusion in your project.

### Testing

SynthDevKit includes tests that check the functionality of the modules.

Building the test runner is simple, and running with the `-s` flag will run the
tests in `spec` mode.

```
$ make test
$ ./testrunner -s
```

### Documentation

These modules live in the `SynthDevKit` namespace.

Documentation is available for the [CV](docs/CV.md) and [Clock](docs/Clock.md)
modules.

New: [EventEmitter](docs/EventEmitter.md).

There are currently no examples, but you can see them in use in [SynthKit](https://github.com/JerrySievert/SynthKit/).
