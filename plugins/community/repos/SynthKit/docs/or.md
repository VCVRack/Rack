# Or

![Or Module](images/or.png)

The **or** module accepts two waveform inputs, executes an [OR](https://en.wikipedia.org/wiki/Bitwise_operation#OR) against both
waveforms, and outputs the resulting waveform.

In order to accomplish this, the voltage is multiplied by 10,000, and converted
to an integer, and then divided by 10,000 before returning the waveform.
