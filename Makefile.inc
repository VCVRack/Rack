FLAGS += -MMD


ifeq ($(ARCH), lin)
CC ?= gcc
CXX ?= g++
FLAGS += -DARCH_LIN
endif

ifeq ($(ARCH), mac)
CC ?= clang
CXX ?= clang++
FLAGS += -DARCH_MAC
CXXFLAGS += -stdlib=libc++
LDFLAGS += -stdlib=libc++
endif

ifeq ($(ARCH), win)
CC ?= x86_64-w64-mingw32-gcc
CXX ?= x86_64-w64-mingw32-g++
FLAGS += -DARCH_WIN -D_USE_MATH_DEFINES
endif



CFLAGS := $(FLAGS) $(CFLAGS)
CXXFLAGS := $(FLAGS) $(CXXFLAGS)
OBJECTS += $(patsubst %, build/%.o, $(SOURCES))
DEPS = $(patsubst %, build/%.d, $(SOURCES))


# Final targets

$(TARGET): $(OBJECTS)
	$(CXX) -o $@ $^ $(LDFLAGS)

# Object targets

-include $(DEPS)

build/%.c.o: %.c
	@mkdir -p $(@D)
	$(CC) $(CFLAGS) -c -o $@ $<

build/%.cpp.o: %.cpp
	@mkdir -p $(@D)
	$(CXX) $(CXXFLAGS) -c -o $@ $<

build/%.cc.o: %.cc
	@mkdir -p $(@D)
	$(CXX) $(CXXFLAGS) -c -o $@ $<

build/%.m.o: %.m
	@mkdir -p $(@D)
	$(CC) $(CFLAGS) -c -o $@ $<
