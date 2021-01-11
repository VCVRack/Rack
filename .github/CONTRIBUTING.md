
VCV Rack is [open-source](https://opensource.org/osd) but not [open-contribution](https://opensource.guide/how-to-contribute/).
VCV is unable to accept free code contributions to Rack for the following reasons.
- **Quality.**
Most contributions to open-source projects typically only contain code, but writing code is just a small percentage of the effort required to maintain a large software project.
Additional tasks for fully supporting a feature include
	- debating the best design before any code is written
	- considering all use cases and corner cases of the implementation
	- generalizability to allow other features to be built on top if needed
	- backward compatibility with Rack's plugin [API/ABI](https://vcvrack.com/manual/Version) and user patches.
	- testing across all supported operating systems and hardware
	- dedication to support the feature for >4 years
- **Time.**
In the past, free code contributions have cost far more time to review, iterate, fix, and test than writing the implementation from scratch.
There have been exceptions to this, but they are rare.
- **Legal.**
A proprietary fork of VCV Rack is planned (see [*Rack for DAWs*](https://vcvrack.com/manual/FAQ.html#is-vcv-rack-available-as-a-vst-au-aax-plugin-for-daws)), so VCV must own all GPL-licensed code that is included in Rack.
To accept a contribution, all authors of the contribution need to either
	- declare the patch under the [CC0](https://creativecommons.org/publicdomain/zero/1.0/) license.
	- complete a copyright reassignment form.
	- perform the work under a paid agreement.

Except in exceptional circumstances, contributions are only accepted as paid work under detailed guidelines.

However there are several areas you may volunteer to benefit the Rack project.
- Create proper [bug reports and feature requests](https://vcvrack.com/manual/Issues).
- Answer questions in the [VCV communities](https://vcvrack.com/manual/Communities).
- Develop and maintain your own [Rack plugins](https://vcvrack.com/manual/PluginDevelopmentTutorial).
- Contribute to Rack's open-source dependencies, such as [GLFW](https://www.glfw.org/), [nanovg](https://github.com/memononen/nanovg), [RtAudio](https://www.music.mcgill.ca/~gary/rtaudio/), and [RtMidi](https://www.music.mcgill.ca/~gary/rtmidi/).
