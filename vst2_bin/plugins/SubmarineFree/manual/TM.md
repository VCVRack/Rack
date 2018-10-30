# Torpedo Message Merge
#### TM-105 - 5 port Torpedo Message Merge

![View of the Torpedo Message Merge](TM.png "Torpedo Message Merge")

## Basic Operation

Because [Torpedo](https://github.com/david-c14/Torpedo) signals are data streams, they cannot be mixed without corrupting the data in the stream.

The TM-105 is a message buffer with 5 separate inputs and a single Torpedo output. As messages array at the input ports, they are buffered until the output port is free to send them on. Up to 5 messages may be buffered at once, if the buffer fills, messages will be dropped. A set of leds indicate how full the buffer is at any one time.
