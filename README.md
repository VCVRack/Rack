*Note: This software is in semi-public alpha. If you have stumbled upon this project, feel free to try it out and report bugs to the GitHub issue tracker. However, with more users it becomes difficult to make breaking changes, so please don't spread this to your friends and the Internet just yet, until the official announcement has been made.*

	░█▀▄░█▀█░█▀▀░█░█
	░█▀▄░█▀█░█░░░█▀▄
	░▀░▀░▀░▀░▀▀▀░▀░▀

Eurorack-style modular DAW

## Building

Install dependencies

- [GLEW](http://www.glfw.org/)
- [GLFW](http://glew.sourceforge.net/)
- [jansson](http://www.digip.org/jansson/)
- [portaudio](http://www.portaudio.com/)
- [portmidi](http://portmedia.sourceforge.net/portmidi/)
- GTK+-2.0 if Linux (for file open/save dialog)

Run `make ARCH=linux` or `make ARCH=windows` or `make ARCH=apple`

If the build breaks because you think I've missed a step, feel free to post an issue.
