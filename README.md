# Rack

*Rack* is the engine for the VCV open-source virtual Eurorack DAW.

![Rack screenshot](https://vcvrack.com/images/screenshot.png)

This README includes instructions for building Rack from source. For information about the software, go to https://vcvrack.com/.

## The [Issue Tracker](https://github.com/VCVRack/Rack/issues) *is* the official developer's forum

Bug reports, feature requests, and even *questions/discussions* are welcome on the GitHub Issue Tracker for all VCVRack repos.

Please vote on feature requests by using the Thumbs Up/Down reaction on the first post.

## Setting up your development environment

Rack's dependencies (GLEW, glfw, etc) do not need to be installed on your system, since specific versions are compiled locally during the build process. However, you need proper tools to build these dependencies.

### Mac

Install [Xcode](https://developer.apple.com/xcode/) or *command line developer tools* with `xcode-select --install`.
Install [CMake](https://cmake.org/), preferably from [Homebrew](https://brew.sh/).

### Windows

Install [MSYS2](http://www.msys2.org/) and launch the mingw64 shell (not the default msys2 shell).
Install build dependencies with the pacman package manger.

	pacman -S git make tar unzip mingw-w64-x86_64-gcc mingw-w64-x86_64-cmake

### Linux

With your distro's package manager, make sure you have installed `gcc`, `make`, `cmake`, `tar`, and `unzip`.

## Building

*If the build fails for you, please report the issue with a detailed error message to help the portability of Rack.*

Clone this repository and `cd` into it.

Clone submodules.

	git submodule update --init --recursive

Build dependencies locally.
You may use make's `-j$(nproc)` flag to parallelize builds across all your CPU cores.

	make dep

You should see a message that all dependencies built successfully.

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
