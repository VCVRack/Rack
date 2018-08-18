# SynthDevKit::Clock

A clock module that handles a number of divisions up to `CLOCK_LIMIT` (currently
set at `1024`).

Division occurs as follows (for a clock with 8 divisions):

```
    1  2  3  4  5  6  7  8
/1  x  x  x  x  x  x  x  x
/2     x     x     x     x
/3        x        x      
/4           x           x
/5              x         
/6                 x      
/7                    x   
/8                       x
```

One thing to note is that currently this module uses one half of a clock interval
to determine whether the clock should be `high` or `low`.

Input:

```
________        ________
       |________|      |
```

Output:

```
____            ____
   |____________|  |____
```

## Methods

### Clock(uint16_t count, float threshold)

Constructor

#### Parameters

|Parameter|Type|Description|
|---------|----|-----------|
|count|`uint16_t`|number of divisions for the clock module|
|threshold|`float`|threshold of voltage that a trigger fires at|

### bool *Clock::update(float value)

Processes a clock update, and returns a pointer to an array of boolean results,
one for each division.  These will be `true` if the division is `high`, and `false`
if the division is `low`.

#### Parameters

|Parameter|Type|Description|
|---------|----|-----------|
|value|`float`|current voltage value|

_Returns:_

`bool *` - pointer to an array of `bool` values

### Clock::reset()

Resets the clock module to starting state.
