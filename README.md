# Rack

*Rack* is the engine for the VCV open-source virtual Eurorack DAW.

![Rack screenshot](https://vcvrack.com/images/screenshot.png)

This README includes instructions for building Rack from source. For information about the software, go to https://vcvrack.com/.

## The [Issue Tracker](https://github.com/VCVRack/Rack/issues) *is* the official developer's forum

Bug reports, feature requests, and even *questions/discussions* are welcome on the GitHub Issue Tracker for all VCVRack repos.
However, please search before posting to avoid duplicates, and limit to one issue per post.

You may vote on feature requests by using the Thumbs Up/Down reaction on the first post.

I rarely accept Pull Requests, so please notify me in advance to plan your contribution before writing code.

## Setting up your development environment

Rack's dependencies (GLEW, glfw, etc) do not need to be installed on your system, since specific versions are compiled locally during the build process. However, you need proper tools to build these dependencies.

### Mac

Install [Xcode](https://developer.apple.com/xcode/).
Install [CMake](https://cmake.org/) (for some of Rack's dependencies) and wget, preferably from [Homebrew](https://brew.sh/).

### Windows

Install [MSYS2](http://www.msys2.org/) and launch the mingw64 shell (not the default msys2 shell).
Install build dependencies with the pacman package manger.

	pacman -S git make tar unzip mingw-w64-x86_64-gcc mingw-w64-x86_64-cmake

### Linux

With your distro's package manager, make sure you have installed `gcc`, `make`, `cmake`, `tar`, and `unzip`.

## Building

*If the build fails for you, please report the issue with a detailed error message to help the portability of Rack.*

Clone this repository and `cd` into it.
If you would like to build a previous version of Rack instead of the master branch, check out the desired tag with `git checkout v0.4.0` for example.

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

Be sure to check out and build the version of Rack you wish to build your plugins against.

You must clone the plugin in Rack's `plugins/` directory, e.g.

	cd plugins
	git clone https://github.com/VCVRack/Fundamental.git

Clone submodules.

	cd Fundamental
	git submodule update --init --recursive

Build plugin.

	make

## License

Rack source code by [Andrew Belt](https://andrewbelt.name/) licensed under [BSD-3-Clause](LICENSE.txt)

Component Library graphics by [Grayscale](http://grayscale.info/) licensed under [CC BY-NC 4.0](https://creativecommons.org/licenses/by-nc/4.0/)
