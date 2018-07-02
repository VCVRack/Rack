# Squinky Labs modules for VCV Rack

All of our plugins are free and open source. The [instruction manual](booty-shifter.md) describes all of the released modules.

All of our released modules may be found in the [VCV Rack plugin manager] (https://vcvrack.com/plugins.html). This is by far the easiest way for most users to install our modules and keep them up to date.

It is also quite easy to clone this repo and build them yourself. In order to do this, however, you must first download and build [VCV Rack itself](https://github.com/VCVRack/Rack).

## Information for developer and experimenters

There are various test modules, test code, and other good things hidden away in this repo. We will try to point you to some that may be of interest.

Most of the documentation may be found in the [docs folder](../docs/.).

## Building source

As with all third-party modules for VCV, you must:

* Clone the VCV Rack repo.
* Build Rack from source.
* Clone SquinkyVCV in Rack’s plugins folder.
* `CD SquinkyVCV`
* `make`

## Experimental modules

At any given time, there may partially finished "experimental" modules in this repo. You can find up to date information on them [here](experimental.md).

## Unit testing framework

We have reasonably thorough tests for our code. Some of this might be of interest - it's [here](unit-test.md).