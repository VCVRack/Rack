# Some notes on our unit tests

The ./test folder contains a unit test program and test-support classes.

In general most of this software is fairly unremarkable code that supports unit testing of the various parts of our plugins.

We find that reasonably thorough unit testing is even more valuable for plugins than for "normal" code. Some of the reasons are:

* Without unit tests, it can be very easy to make filters that don’t have the response they should ("oops – forget to multiply by 2 pi"), lookup tables aren’t as accurate as they should be, numeric approximations have gross discontinuities, etc.
* Unit tests may be built and debugged with a conventional IDE, whereas (at least for us) debugging a plugin with gdb from inside a running instance of VCV Rack is more difficult.
* It is easy to measure CPU usage in a unit test, whereas it is difficult to impossible to measure while running in VCV Rack.

This last point is very important, as one of the basic rules of code optimization is that a programmers intuition (guess) as to where the bottlenecks are is remarkably unreliable – you must drive your optimizations from real data. For example, we discovered that over half the CPU time in our vocal filter was in evaluating assertions!

As stated above, much of the test code is unremarkable, other than its existence. There are a couple of modules however that might be useful to others.

[MeasureTime](../test/MeasureTime.h) is used to measure the CPU usage of any arbitrary code. It takes a simple lambda and profiles it.

[Composite pattern](composites.md) allows us to run our plugin code inside a test application as well as inside a VCV Track plugin module.

[Assert Library](../test/asserts.h) is a very basic collection of assertion macros loosely based on the Chai Assert framework.

We have enhanced [Makefile](../Makefile) in several useful ways with the addition of [test.mk](../test.mk):

* Default `make` will generate a plugin with assertions disabled.
* `make test` will generate test.exe, our unit test application, with asserts enabled.
* `make perf` will generate perf.exe, our unit test application, with asserts disabled.
* `make cleantest` is like `make clean`, but for out test code.
