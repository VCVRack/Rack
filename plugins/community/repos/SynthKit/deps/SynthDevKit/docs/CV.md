# SynthDevKit::CV

A trigger module that provides state management of triggers and trigger intervals
based on voltage.

## Methods

### CV(float threshold)

Constructor

#### Parameters

|Parameter|Type|Description|
|---------|----|-----------|
|threshold|`float`|threshold of voltage that a trigger fires at|

### void CV::update(float value)

Updates the state of the trigger manager, should be called with every step or
poll.

#### Parameters

|Parameter|Type|Description|
|---------|----|-----------|
|value|`float`|current voltage value|

### bool CV::newTrigger()

Returns a boolean value as to a new trigger event has occurred with the latest
update.

_Returns:_

`bool` - whether a new trigger event has been detected

### void CV::reset()

Resets the engine to initial state.

### float CV::currentValue()

Returns the current value that the engine has processed.

_Returns:_

`float` - most recent voltage value from `update`

### uint32_t CV::triggerInterval()

The number of steps between trigger events.  This will only be available after
the second trigger has fired.  This value is calculated on every trigger, and
will adjust to input changes, but will always be one trigger behind in its
calculations.

_Returns:_

`uint32_t` - number of steps between the last two triggers

### uint32_t CV::triggerTotal()

The number of triggers that have been detected since instantiation or last
reset.

_Returns:_

`uint32_t` - number of triggers detected

### bool CV::isHigh()

Whether a trigger event is still considered above the threshold.

_Returns:_

`bool` - whether a trigger can still be considered active

### bool CV::isLow()

Whether a trigger event is no longer considered above the threshold.

_Returns:_

`bool` - whether a trigger can no longer be considered active
