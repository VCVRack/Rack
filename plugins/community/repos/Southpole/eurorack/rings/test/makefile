PACKAGES       = rings/test stmlib/utils rings/dsp rings stmlib/dsp

VPATH          = $(PACKAGES)

TARGET         = rings_test
BUILD_ROOT     = build/
BUILD_DIR      = $(BUILD_ROOT)$(TARGET)/
CC_FILES       = rings_test.cc \
		fm_voice.cc \
		part.cc \
		resonator.cc \
		resources.cc \
		random.cc \
		string.cc \
		string_synth_part.cc \
		units.cc
OBJ_FILES      = $(CC_FILES:.cc=.o)
OBJS           = $(patsubst %,$(BUILD_DIR)%,$(OBJ_FILES)) $(STARTUP_OBJ)
DEPS           = $(OBJS:.o=.d)
DEP_FILE       = $(BUILD_DIR)depends.mk

all:  rings_test

$(BUILD_DIR):
	mkdir -p $(BUILD_DIR)

$(BUILD_DIR)%.o: %.cc
	g++ -c -DTEST -g -Wall -Werror -msse2 -Wno-unused-variable -O2 -I. $< -o $@

$(BUILD_DIR)%.d: %.cc
	g++ -MM -DTEST -I. $< -MF $@ -MT $(@:.d=.o)

rings_test:  $(OBJS)
	g++ -g -o $(TARGET) $(OBJS) -Wl,-no_pie -lm -lprofiler -L/opt/local/lib

depends:  $(DEPS)
	cat $(DEPS) > $(DEP_FILE)

$(DEP_FILE):  $(BUILD_DIR) $(DEPS)
	cat $(DEPS) > $(DEP_FILE)

profile:	rings_test
	env CPUPROFILE_FREQUENCY=1000 CPUPROFILE=$(BUILD_DIR)/rings.prof ./rings_test && pprof --pdf ./rings_test $(BUILD_DIR)/rings.prof > profile.pdf && open profile.pdf
	
clean:
	rm $(BUILD_DIR)*.*

include $(DEP_FILE)
