PACKAGES       = elements/test stmlib/utils elements/dsp elements stmlib/dsp

VPATH          = $(PACKAGES)

TARGET         = elements_test
BUILD_ROOT     = build/
BUILD_DIR      = $(BUILD_ROOT)$(TARGET)/
CC_FILES       = ominous_voice.cc \
		elements_test.cc \
		exciter.cc \
		multistage_envelope.cc \
		part.cc \
		resonator.cc \
		resources.cc \
		random.cc \
		tube.cc \
		units.cc \
		voice.cc
OBJ_FILES      = $(CC_FILES:.cc=.o)
OBJS           = $(patsubst %,$(BUILD_DIR)%,$(OBJ_FILES)) $(STARTUP_OBJ)
DEPS           = $(OBJS:.o=.d)
DEP_FILE       = $(BUILD_DIR)depends.mk

all:  elements_test

$(BUILD_DIR):
	mkdir -p $(BUILD_DIR)

$(BUILD_DIR)%.o: %.cc
	/opt/local/bin/g++-mp-4.7 -c -DTEST -g -Wl,-no_pie -Wall -Werror -msse2 -Wno-unused-variable -O2 -I. $< -o $@

$(BUILD_DIR)%.d: %.cc
	/opt/local/bin/g++-mp-4.7 -MM -DTEST -I. $< -MF $@ -MT $(@:.d=.o)

elements_test:  $(OBJS)
	/opt/local/bin/g++-mp-4.7 -g -o $(TARGET) $(OBJS) -Wl,-no_pie -lm -lprofiler -L/opt/local/lib

depends:  $(DEPS)
	cat $(DEPS) > $(DEP_FILE)

$(DEP_FILE):  $(BUILD_DIR) $(DEPS)
	cat $(DEPS) > $(DEP_FILE)

profile:	elements_test
	env CPUPROFILE_FREQUENCY=1000 CPUPROFILE=$(BUILD_DIR)/elements.prof ./elements_test && pprof --pdf ./elements_test $(BUILD_DIR)/elements.prof > profile.pdf && open profile.pdf

include $(DEP_FILE)
