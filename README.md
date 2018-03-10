# Rack

*Rack* is the engine for the VCV open-source virtual modular synthesizer.

![Rack screenshot](https://vcvrack.com/images/screenshot.png)

This README includes instructions for building Rack from source. For information about the software, go to https://vcvrack.com/.

## The [Issue Tracker](https://github.com/VCVRack/Rack/issues?q=is%3Aissue+is%3Aopen+sort%3Aupdated-desc) *is* the official developer's forum

Bug reports, feature requests, and even *questions/discussions* are welcome on the GitHub Issue Tracker for all VCVRack repos.
However, please search before posting to avoid duplicates, and limit to one issue per post.

Please vote on feature requests by using the Thumbs Up/Down reaction on the first post.

I rarely accept code contributions to Rack itself, so please notify me in advance if you wish to send a pull request.

## Setting up your development environment

Rack's dependencies (GLEW, glfw, etc) do not need to be installed on your system, since specific versions are compiled locally during the build process. However, you need proper tools to build these dependencies.

### Mac

Install [Xcode](https://developer.apple.com/xcode/).
Install [CMake](https://cmake.org/) (for some of Rack's dependencies).

### Windows

Install [MSYS2](http://www.msys2.org/) and launch the mingw64 shell (not the default msys2 shell).
Install build dependencies with the pacman package manger.

	pacman -S git make tar unzip mingw-w64-x86_64-gcc mingw-w64-x86_64-cmake autoconf automake mingw-w64-x86_64-libtool

### Linux

With your distro's package manager, make sure you have installed `git`, `gcc`, `make`, `cmake`, `tar`, `unzip`, and `curl`.

## Building

*If the build fails for you, please report the issue with a detailed error message to help the portability of Rack.*

Clone this repository with `git clone https://github.com/VCVRack/Rack.git` and `cd Rack`.

The `master` branch contains the latest public code and breaks its plugin [API](https://en.wikipedia.org/wiki/Application_programming_interface) and [ABI](https://en.wikipedia.org/wiki/Application_binary_interface) frequently.
If you wish to build a previous version of Rack which is API/ABI-compatible with an official Rack release, check out the desired branch with `git checkout v0.5` for example.

Clone submodules.

	git submodule update --init --recursive

Build dependencies locally.
You may use make's `-j$(nproc)` flag to parallelize builds across all your CPU cores.

	make dep

You may use `make dep RTAUDIO_ALL_APIS=1` to attempt to build with all audio driver APIs enabled for your operating system, although this is unsupported.

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

## Licenses

All **source code** in this repository is licensed under [BSD-3-Clause](LICENSE.txt) by [Andrew Belt](https://andrewbelt.name/).

**Component Library graphics** in `res/ComponentLibrary` are licensed under [CC BY-NC 4.0](https://creativecommons.org/licenses/by-nc/4.0/) by [Grayscale](http://grayscale.info/). Commercial plugins must request a commercial license to use Component Library graphics.

**Core** panel graphics in `res/Core` are copyright © 2017 by Grayscale. You may not create derivative works of Core panels.

The **VCV logo and icon** are copyright © 2017 by Grayscale and may not be used in derivative works.

The **"VCV" brand name** is trademarked and may not be used for unofficial products. However, it is acceptable to use the phrase "for VCV Rack" for promotion of your plugin.
