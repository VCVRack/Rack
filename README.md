# Rack

*Rack* is the engine for the VCV open-source virtual Eurorack DAW.

![Rack screenshot](https://vcvrack.com/images/screenshot.png)

## Building

*If the build fails for you, please report the issue with a detailed error message to help the portability of Rack.*

Clone this repository and `cd` into it.
On Windows, use [MSYS2](http://www.msys2.org/) and launch a mingw64 shell.

Clone submodules.

	git submodule update --init --recursive

Build dependencies locally.

	cd dep
	make
	cd ..

Build Rack.

	make

Run Rack.

	make run

## Building plugins

Clone your favorite plugin in the `plugins/` directory. e.g.:

	cd plugins
	git clone https://github.com/VCVRack/Fundamental.git

Clone submodules.

	cd Fundamental
	git submodule update --init --recursive

Build plugin.

	make

## License

Rack source code by [Andrew Belt](https://andrewbelt.name/) licensed under the [BSD-3-Clause](LICENSE.txt)

Component Library graphics by [Grayscale](http://grayscale.info/) licensed under the [CC BY-NC 4.0](https://creativecommons.org/licenses/by-nc/4.0/)
