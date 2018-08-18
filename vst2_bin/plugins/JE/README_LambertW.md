LambertW
========

C++ implementation of the Lambert W(x) function.

The implementation grew out of the code that was first used in the mathematical
utilities of the [Offline Reconstruction Software
Framework](http://arxiv.org/abs/0707.1652) of the [Pierre Auger
Observatory](http://www.auger.org).

The work is described in the following publications:
* [arXiv:1003.1628](http://arxiv.org/abs/1003.1628)
* [arXiv:1209.0735](http://arxiv.org/abs/1209.0735) and [Comp. Phys. Comm. **183** (2012) 2622-2628](http://dx.doi.org/10.1016/j.cpc.2012.07.008)

This is currently the fastest known implementation of the Lambert W function on the planet.


## License

Released under dual licence: the GPL version 2 and the two-clause BSD license.

Scientific or technical publications resulting from projects using this code are required to add the [Comp. Phys. Comm. **183** (2012) 2622-2628](http://dx.doi.org/10.1016/j.cpc.2012.07.008) citation among their references.



## Usage

All the relevant code is placed into an `utl` namespace.

The two branches (0 and -1) of the Lambert function can be specified as template parameters, e.g.
```C++
const double w = utl::LambertW<-1>(x);
```
or as normal parameters
```C++
const double w = utl::LambertW(-1, x);
```
