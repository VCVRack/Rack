# SynthDevKit::EventEmitter

An Event Emitter.

## Methods

### EventEmitter( )

Constructor

### void EventEmitter::clear(bool all)

Clears all event listeners, if `all` is set to `true`, it even clears the
`EVENT_CLEAR` listener.

#### Parameters

|Parameter|Type|Description|
|---------|----|-----------|
|all|`bool`|whether to clear all (default: `false`)|

### void EventEmitter::on(int16_t event, function)

Sets up a listener for an event.

#### Parameters

|Parameter|Type|Description|
|---------|----|-----------|
|event|`int16_t`|event id (numeric value, from `1` to `2048`, or `EEType`)|
|function|`function`|`function` accepting `int16_t` and `float` arguments|

### void EventEmitter::removeListener(int16_t event, function)

Removes a listener from an event.

|Parameter|Type|Description|
|---------|----|-----------|
|event|`int16_t`|event id (numeric value, from `1` to `2048`, or `EEType`)|
|function|`function`|`function` accepting `int16_t` and `float` arguments|

### int16_t EventEmitter::listenerCount()

Returns the count of listeners for an event.

|Parameter|Type|Description|
|---------|----|-----------|
|event|`int16_t`|event id (numeric value, from `1` to `2048`, or `EEType`)|

_Returns:_

`int16_t` - number of listeners for the `event`.

### void EventEmitter::emit(int16_t event, float value)

Emits an event to all listeners.

|Parameter|Type|Description|
|---------|----|-----------|
|event|`int16_t`|event id (numeric value, from `1` to `2048`, or `EEType`)|
|value|`float`|value to emit to the listeners|

## Event Types

These are predefined events that will emit by default:

* `EVENT_CLEAR` - when `clear()` called
* `EVENT_FIRST` - emitted on first event
* `EVENT_EVEN` - emitted on all even events
* `EVENT_ODD` - emitted on all odd events
