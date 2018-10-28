# Gray Code: eclectic clock divider

![gray code image](./gray-code.png)

## About Gray Code
A cool feature of gray codes is that only one bit changes at a time. Having only one “thing” change at a time can be interesting for music, so we are hoping you will find some good things to do with it.

WikiPedia has a very good article on gray codes: https://en.wikipedia.org/wiki/Gray_code

Our Gray Code module has only one control. It selects between standard gray code and balanced gray codes. With a standard gray code, the lower bits change much more often than the high bits. You can see it counting up. With the balanced gray codes, all the bits change more or less the same amount, but of course no two ever change at the same time.

Each bit of the 8-bit gray code comes out to an output jack. The LED beside the output shows when it goes high and low.

There is an additional output that adds up all the bits with a binary weighting, kind of like a DAC.

The external clock input must be driven with a clock - there is no internal clock.

Now let your imagination run wild!