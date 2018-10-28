# Thread Booster<a name="booster"></a>

![thread booster image](./thread-booster.png)

Thread booster raises the priority of VCV Rack's audio rendering thread. In many cases this decreases the annoying pops, ticks, and dropouts that many users are experiencing.

Many users have reported that Thread Booster helps significantly. Others have reported that it does not help at all. No one has reported a detrimental effect.

For a deeper dive into the Thread Booster, you should read [this document](./thread-booster.md).

Thread Booster has a UI that lets you boost the priority of the audio thread. There are three arbitrary settings: normal, boosted, and real time. When the switch is in the bottom position, the plugin does nothing; the audio thread keeps its default priority. In the boost (middle) position, it sets the thread priority to the highest priority non-real-time setting. In the real-time position it attempts to set it to the highest possible priority, or near it.

If setting the priority fails, the red error light lights up, and the priority stays where it was last.

To use Thread Booster, just insert an instance into VCV Rack, then adjust the boost switch. In general we recommend the "real time" setting, if it is available on your computer.

Once Thread booster is in your session, it will boost all the audio processing - it doesn't matter if other modules are added before or after - they all get boosted.

Linux users - you must read [the detailed document](./thread-booster.md) to use this module.

Note to users who downloaded the original version of Thread Booster: we've improved it a bit since then, especially on Linux and Windows.
