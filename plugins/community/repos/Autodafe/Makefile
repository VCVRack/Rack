
SOURCES = $(wildcard src/*.cpp)

LDFLAGS += -Lsrc/stk/src -lstk

LDFLAGS += -Lsrc/Gamma/build/lib -lGamma


include ../../plugin.mk




dist: all
	mkdir -p dist/Autodafe
	cp LICENSE* dist/Autodafe/
	cp plugin.* dist/Autodafe/
	cp -R res dist/Autodafe/
