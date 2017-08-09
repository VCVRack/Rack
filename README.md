***Note: This repo is unsupported until September 7. All issues and pull requests will be ignored until then.***

# Rack

Open source virtual Eurorack DAW

## Building

Install dependencies

- [GLEW](http://www.glfw.org/)
- [GLFW](http://glew.sourceforge.net/)
- [jansson](http://www.digip.org/jansson/)
- [portaudio](http://www.portaudio.com/)
- [portmidi](http://portmedia.sourceforge.net/portmidi/)
- [libsamplerate](http://www.mega-nerd.com/SRC/)
- GTK+-2.0 if Linux (for file open/save dialog)

Run `make ARCH=lin` or `make ARCH=win` or `make ARCH=mac`

If the build breaks because you think I've missed a step, feel free to post an issue.

## License

Rack source code by [Andrew Belt](https://andrewbelt.name/): [BSD-3-Clause](LICENSE.txt)

Component Library graphics by [Grayscale](http://grayscale.info/): [CC BY-NC 4.0](https://creativecommons.org/licenses/by-nc/4.0/)
