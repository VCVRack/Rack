# Thread Booster

Thread booster boosts the thread priority of the audio streaming thread in VCV Rack. The intention is to make sure that if your computer strains to produce all that audio, audio will get priority in the competition for the CPU over other things like drawing the screen, or showing ads in your web browser.

If you aren't a computer programmer, this may sound like some hocus-pocus. But it is a very basic technique used in many audio applications. Google "audio thread priority" for some random instances.

If some other task is competing for CPU and the audio loses out, it may be unable to keep up with the audio interface. This will cause occasional data underruns, which often sound like pops and clicks.

Now, while raising the audio thread priority is a fundamentally sound thing to do, it is not going to solve all audio problems. Some users report that Thread Booster does not help at all. Others report significant improvement.

On all three operating systems we have been able to get the "real-time" setting to work (although read below - it's not always technically "real time").

## Notes for Linux

Linux has two challenges for thread booster:

* The non-real-time settings set by the POSIX Pthread API don't work.
* Raising thread priority to real-time in Linux is a privileged operation.

The impact of the first challenge is that the middle "boosted" setting might not do much in Linux. We recommend the real-time setting.

You can always set the real time priority of Rack if run as root, but we do not recommend that. Instead you may use `setcap` to give VCV Rack permission to use real-time priority.

```bash
sudo setcap cap_sys_nice=ep <path to Rack>
```

After giving Rack this ability, it will stay set until the Rack executable file is changed, either by downloading a new one, or building a new one on top of it.

## Notes on Windows

Unlike Linux, the middle boost setting works well on Windows. The realtime setting works well also, although it is not the very highest setting that windows calls realtime. Instead it sets the Rack process as a whole to HIGH_PRIORITY_CLASS, and the sets the audio thread to THREAD_PRIORITY_TIME_CRITICAL. We have found this to give a very good boost without requiring that you run Rack as administrator (which is not recommended).

We only test on Windows 7. If you have issues with other versions of Windows, please let us know.

## Notes on OS X

As usual, it just works on OS X.

## Caveats and limitations

If you try this plugin, be aware that changing Rack’s thread priority behind its back may cause some things to misbehave. We have not seen this happen, but it's a real possibility. Running at an elevated priority could make something in VCV or the plugins misbehave or glitch.

Another limitation is that Thread Booster does not boost the thread(s) that move data between Rack's audio engine and the Audio (sound card) Module(s).

## Is it dangerous to run at "realtime" priority?

If you scour the Internet you will find some warnings about boosting thread priorities to super high thread priorities, saying that there is a danger your computer will become unresponsive, possibly even needing to be re-booted.

While in general this is true, it is not true that raising the priority of a single thread will do this, as a single thread can at worst use all the cycles of a single core. For the last 10 years or so all CPUs have more than one core, so there will always be plenty of CPU left over.

Most audio programs in fact do raise the priority of their audio threads, and of course work fine. So, while we don't guarantee that Thread Booster will fix all clicks and pops, we do believe that it is very unlikely to do any harm.

## Questions for testers

Please use our [GitHub issues](https://github.com/squinkylabs/SquinkyVCV/issues) to send us feedback. Some things we are curious about:

* With a fully loaded session that starts to make pops and clicks, does thread booster fix it?
* If you are able to run realtime, is there any noticeable difference between boosted and real-time?
* For all reports, please list operating system and version (i.e. Windows-10, OSX 10.12, Ubuntu 16.04).

## Known issues

On the Mac, Boosting and the switching back to normal will not restore the original priority. It will actually be running at a lower priority.

When the Thread Booster comes up initially, no LEDs are illuminated - the normal LED should be.
