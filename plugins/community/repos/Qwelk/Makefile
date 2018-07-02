SLUG = Qwelk
VERSION = 0.6.0

SOURCES += $(wildcard src/*.cpp)

DISTRIBUTABLES += $(wildcard LICENSE*) res

RACK_DIR ?= ../..
include $(RACK_DIR)/plugin.mk


# # FLAGS will be passed to both the C and C++ compiler
# FLAGS +=
# CFLAGS +=
# CXXFLAGS +=

# # Careful about linking to libraries, since you can't assume much about the user's environment and library search path.
# # Static libraries are fine.
# LDFLAGS +=

# # Add .cpp and .c files to the build
# SOURCES = $(wildcard src/*.cpp)


# # Convenience target for including files in the distributable release
# DIST_NAME = Qwelk
# .PHONY: dist
# dist: all
# ifndef VERSION
# 	$(error VERSION must be defined when making distributables)
# endif
# 	mkdir -p dist/$(DIST_NAME)
# 	cp LICENSE* dist/$(DIST_NAME)/
# 	cp $(TARGET) dist/$(DIST_NAME)/
# 	cp -R res dist/$(DIST_NAME)/
# 	cp -R examples dist/$(DIST_NAME)/
# 	cd dist && zip -5 -r $(DIST_NAME)-$(VERSION)-$(ARCH).zip $(DIST_NAME)

# # Must include the VCV plugin Makefile framework
# include ../../plugin.mk
