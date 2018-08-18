
SOURCES = $(wildcard src/*.cpp)
SOURCES += $(wildcard common/*.cpp)
SOURCES += $(wildcard components/*.cpp)
SOURCES += $(wildcard ext/LambertW/*.cc)
SOURCES += $(wildcard utils/*.cpp)
SOURCES += $(wildcard widgets/*.cpp)

FLAGS += -I. -I./ext/LambertW -DJE_VERSION=0.5.1 -DJE_MANUFACTURER="Julien Eres"

include ../../plugin.mk

dist: all
	mkdir -p dist/JE
	cp LICENSE* dist/JE/
	cp plugin.* dist/JE/
	cp -R res dist/JE/
