***Note: This repo is unsupported until September 7. See you at [Knobcon](https://knobcon.com/)!***

# Rack

*Rack* is the engine for the VCV open-source virtual Eurorack DAW.

## Building

*If the build fails for you, please report the issue with a detailed error message to help the portability of Rack.*

Clone this repository and `cd` into it.
On Windows, use [MSYS2](http://www.msys2.org/) and launch a mingw32/64 shell.

Clone submodules.

	git submodule update --init

Build dependencies locally.

	cd dep
	make
	cd ..

Build Rack.

	make

## Building the plugins

change into the plugins directory and git clone all the plugin projects, e.g.

	cp plugins
	git clone https://github.com/VCVRack/Fundamental

for each plugin project:

	cd <plugin project>
	git submodule update --init
	make
	cd ..

for the *AudibleInstruments* project you also need to init submodules inside the eurorack directory:

	cd AudibleInstruments
	git submodule update --init
	cd eurorack
	git submodule update --init
	cd ..
	make
	cd ..

go back to the main directory and start Rack:

	cd ..
	make run

## License

Rack source code by [Andrew Belt](https://andrewbelt.name/) licensed under the [BSD-3-Clause](LICENSE.txt)

Component Library graphics by [Grayscale](http://grayscale.info/) licensed under the [CC BY-NC 4.0](https://creativecommons.org/licenses/by-nc/4.0/)
