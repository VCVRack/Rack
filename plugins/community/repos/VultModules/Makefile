
SOURCES = $(wildcard src/*.cpp) $(wildcard src/*.c)

include ../../plugin.mk


dist: all
	mkdir -p dist/VultModules
	cp LICENSE* dist/VultModules/
	cp $(TARGET) dist/VultModules/
	cp -R res dist/VultModules/
