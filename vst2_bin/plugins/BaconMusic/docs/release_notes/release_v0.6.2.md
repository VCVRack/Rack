# BaconMusic v0.6.2 Release Notes

* Added sound samples for most of the modules in the README.

* ChipNoise
  * Fixed an important bit length bug that @alto777 found. 
  * Added control of short sequence selection; either the 31 long or which of the 93s.
  * For more, see https://github.com/baconpaul/BaconPlugs/issues/6
  * Update doc for ChipNoise

* KarplusStrongPoly
  * A new module which implements KarplusStrong plucked instrument synthesis polyphonically.

* QuantEyes
  * Don't trigger lights if there is no input

* PolyGnome
  * Added this new widget which allows for exact fractional polyrhythms in clocks.

* Glissinator
  * Fixed a bug found by @alto777 where a rapid decrease in the shift time would lock the module
  * Added a gate output for when you are in the gliss
  * Refactored to allow standalone tests of the stepper

* SampleDelay
  * Just a teensy utility plugin to do sample accurate delays between 1 and 99 samples

* I added a DMP Text Widget using Stewart C. Russell's Keypunch029 font (see README.md for license and links).
* As well as a single digit, there's a multi digit sevent segment light. Take a look at ChipNoise for how to use it.
* Background got some new features, like colored labels and filled rounded rects in the API

(required)
* Code Review all diffs from master

