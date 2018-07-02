# About composites

This is a somewhat ugly "architecture" that lets the same module "guts" run inside a VCV plugin, and also turn inside our unit test framework (which does not link against VCV Rack).

While this is slightly ugly, it is incredibly useful to us to be able to test and benchmark our plugins from a simple command line program.

You may look at the [FrequencyShifter composite](../composites/FrequencyShifter.h) as an example.

Here is [more on our tests](unit-test.md).

## To make a composite

Make a new class in the composites folder. This class is your composite class. This class must work in the test environment and the VCV Widget environment.

Templatize the class so that its base class is the template parameter. For example, in the FrequencyShifter class:
```
template <class TBase>
class FrequencyShifter : public TBase
{
...
}
```
Follow the link to look at [FrequencyShifter](composites/FrequencyShifter.h)

Create two constructors. One with no arguments for the tests to use, and the other with a `Module *` to use in the VCV plugin.

Put all the Input, Output, Param, Light enums into the composite class, rather than the normal Widget class.

To use this composite in a test, derive concrete class from TestComposite
```c++
using Shifter = FrequencyShifter<TestComposite>;
```
Now you may use this class in a test. The TestComposite base class give your tests access to the inputs and outputs of your "widget"
```c++
    Shifter fs;

    fs.setSampleRate(44100);
    fs.init();
    fs.inputs[Shifter::AUDIO_INPUT].value = 0;
```

## How it works

The class hierarchy is something like these crudely drawn UML diagrams
```
TestWidget -> I/O
    |
    ^
   / \
   ---
    |
 FrequencyShifter<TestWidget> -> enums
 ```

 ```
 ModuleWidget -> I/O -> Module
    |
    ^
   / \
   ---
    |
 FrequencyShifter<ModuleWidget> -> enums
```

The purpose of the composite's base class is to provide the inputs/outputs/etc... to the composite. The TestComposite base class has vectors of I/O exposed as public members that the tests may directly manipulate. The WidgetComposite base class just marshals references to the I/O that is already in the VCV provided Widget base class.

All of the enums for module I/O goes into the composite. So they are available to tests as composite.ENUM_NAME. They are also available to Modules as this->composite.ENUM_NAME.
