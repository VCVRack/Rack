# VCV Rack

*Rack* is the main application for the VCV open-source virtual modular synthesizer.

This README includes instructions for building Rack from source. For information about the software, go to https://vcvrack.com/.

## The [Issue Tracker](https://github.com/VCVRack/Rack/issues?q=is%3Aissue+is%3Aopen+sort%3Aupdated-desc) is the official developer's forum

Bug reports, feature requests, questions, and discussions are welcome on the GitHub Issue Tracker for all repos under the VCVRack organization.
However, please search before posting to avoid duplicates, and limit to one issue per post.

Please vote on feature requests by using the Thumbs Up/Down reaction on the first post.

I rarely accept code contributions to Rack itself, so please notify me in advance if you wish to send a pull request.

## Setting up your development environment

Before building Rack, you must install build dependencies provided by your system's package manager.
Rack's own dependencies (GLEW, glfw, etc) do not need to be installed on your system, since specific versions are compiled locally during the build process.
However, you need proper tools to build Rack and these dependencies.

### Mac

Install [Xcode](https://developer.apple.com/xcode/).
Using [Homebrew](https://brew.sh/), install the build dependencies.
```
brew install git wget cmake autoconf automake libtool
```

### Windows

Install [MSYS2](http://www.msys2.org/) and launch the MinGW 64-bit shell (not the default MSYS shell).
```
pacman -S git wget make tar unzip zip mingw-w64-x86_64-gcc mingw-w64-x86_64-cmake autoconf automake mingw-w64-x86_64-libtool
```

### Linux

On Ubuntu 16.04:
```
sudo apt install git curl cmake libx11-dev libglu1-mesa-dev libxrandr-dev libxinerama-dev libxcursor-dev libxi-dev zlib1g-dev libasound2-dev libgtk2.0-dev libjack-jackd2-dev
```

On Arch Linux:
```
pacman -S git wget gcc make cmake tar unzip zip curl
```

## Building

*If the build fails for you, please report the issue with a detailed error message to help the portability of Rack.*

Clone this repository with `git clone https://github.com/VCVRack/Rack.git` and `cd Rack`.
Make sure there are no spaces in your absolute path, as this breaks many build systems.

Clone submodules.

	git submodule update --init --recursive

Build dependencies locally.
You may add `-j$(nproc)` to your make commands to parallelize builds across all CPU cores.

	make dep

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

	make dep
	make

## Licenses

See [LICENSE.md](LICENSE.md) for a description of all licenses for VCV Rack.
