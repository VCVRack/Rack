# Must follow the format in the Naming section of https://vcvrack.com/manual/PluginDevelopmentTutorial.html
SLUG = SubmarineFree

# Must follow the format in the Versioning section of https://vcvrack.com/manual/PluginDevelopmentTutorial.html
VERSION = 0.6.8

# FLAGS will be passed to both the C and C++ compiler
FLAGS +=
CFLAGS +=
CXXFLAGS +=

# Careful about linking to shared libraries, since you can't assume much about the user's environment and library search path.
# Static libraries are fine.
LDFLAGS +=

# Add .cpp and .c files to the build
SOURCES += $(wildcard src/*.cpp) $(wildcard src/ComponentLibrary/*.cpp)

# Add files to the ZIP package when running `make dist`
# The compiled plugin is automatically added.
DISTRIBUTABLES += $(wildcard LICENSE*) res Scala manual WK_Custom.tunings.template

# If RACK_DIR is not defined when calling the Makefile, default to two levels above
RACK_DIR ?= ../..

# Include the VCV Rack plugin Makefile framework
include $(RACK_DIR)/plugin.mk

# Make resources

RESOURCES += $(subst src/res/,res/,$(wildcard src/res/*.svg))

res: $(RESOURCES)
	
%.svg: ../src/res/%.svg
ifeq (${SVG_TOOL},inkscape)
	inkscape -z -T -l=$@ $<
else
	touch $@
endif

