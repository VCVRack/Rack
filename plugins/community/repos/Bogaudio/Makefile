
SLUG=Bogaudio
VERSION=0.6.16
FLAGS += -DSLUG=$(SLUG) -DVERSION=$(VERSION)

ifdef REQUIRE_VERSION
FLAGS += -DREQUIRE_VERSION=$(REQUIRE_VERSION)
endif

ifdef EXPERIMENTAL
FLAGS += -DEXPERIMENTAL=1
endif

ifdef TEST
FLAGS += -DTEST=1
endif

SOURCES = $(wildcard src/*.cpp src/dsp/*cpp)
CXXFLAGS += -Isrc -Isrc/dsp

DISTRIBUTABLES += $(wildcard LICENSE* README*) res

RACK_DIR ?= ../..
include $(RACK_DIR)/plugin.mk

BENCHMARK_SOURCES = $(wildcard benchmarks/*.cpp src/dsp/*cpp)
BENCHMARK_OBJECTS = $(patsubst %, build/%.o, $(BENCHMARK_SOURCES))
BENCHMARK_DEPS = $(patsubst %, build/%.d, $(BENCHMARK_SOURCES))
-include $(BENCHMARK_DEPS)
benchmark: $(BENCHMARK_OBJECTS)
	$(CXX) -o $@ $^ -lbenchmark -lpthread
benchmark_clean:
	rm -f benchmark $(BENCHMARK_OBJECTS)

TESTMAIN_SOURCES = $(wildcard test/testmain.cpp src/dsp/*cpp)
TESTMAIN_OBJECTS = $(patsubst %, build/%.o, $(TESTMAIN_SOURCES))
TESTMAIN_DEPS = $(patsubst %, build/%.d, $(TESTMAIN_SOURCES))
-include $(TESTMAIN_DEPS)
testmain: $(TESTMAIN_OBJECTS)
	$(CXX) -o $@ $^ ../../build/src/util.cpp.o
testmain_clean:
	rm -f testmain $(TESTMAIN_OBJECTS)

PLOT_SOURCES = $(wildcard test/plot.cpp src/dsp/*cpp)
PLOT_OBJECTS = $(patsubst %, build/%.o, $(PLOT_SOURCES))
PLOT_DEPS = $(patsubst %, build/%.d, $(PLOT_SOURCES))
-include $(PLOT_DEPS)
plot: $(PLOT_OBJECTS)
	$(CXX) -o $@ $^
plotrun: plot
	./plot
plotrungp: plot
	./plot > plot.tmp && gnuplot -e "set yrange [0:1.1]; plot 'plot.tmp' using 1:2 with lines"
plot_clean:
	rm -f plot $(PLOT_OBJECTS)

clean: benchmark_clean testmain_clean plot_clean
