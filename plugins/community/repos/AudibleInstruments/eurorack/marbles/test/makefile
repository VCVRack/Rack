PACKAGES       = marbles/test stmlib/utils marbles/ramp marbles/random marbles stmlib/dsp

VPATH          = $(PACKAGES)

TARGET         = marbles_test
BUILD_ROOT     = build/
BUILD_DIR      = $(BUILD_ROOT)$(TARGET)/
CC_FILES       = marbles_test.cc \
		lag_processor.cc \
		output_channel.cc \
		quantizer.cc \
		discrete_distribution_quantizer.cc \
		ramp_extractor.cc \
		random.cc \
		resources.cc \
		units.cc \
		t_generator.cc \
		x_y_generator.cc
OBJ_FILES      = $(CC_FILES:.cc=.o)
OBJS           = $(patsubst %,$(BUILD_DIR)%,$(OBJ_FILES)) $(STARTUP_OBJ)
DEPS           = $(OBJS:.o=.d)
DEP_FILE       = $(BUILD_DIR)depends.mk

all:  marbles_test

$(BUILD_DIR):
	mkdir -p $(BUILD_DIR)

$(BUILD_DIR)%.o: %.cc
	g++ -c -DTEST -g -Wall -Werror -msse2 -Wno-unused-variable -O2 -I. $< -o $@

$(BUILD_DIR)%.d: %.cc
	g++ -MM -DTEST -I. $< -MF $@ -MT $(@:.d=.o)

marbles_test:  $(OBJS)
	g++ -g -o $(TARGET) $(OBJS) -Wl,-no_pie -lm -lprofiler -L/opt/local/lib

depends:  $(DEPS)
	cat $(DEPS) > $(DEP_FILE)

$(DEP_FILE):  $(BUILD_DIR) $(DEPS)
	cat $(DEPS) > $(DEP_FILE)

profile:	marbles_test
	env CPUPROFILE_FREQUENCY=1000 CPUPROFILE=$(BUILD_DIR)/marbles.prof ./marbles_test && pprof --pdf ./marbles_test $(BUILD_DIR)/marbles.prof > profile.pdf && open profile.pdf
	
clean:
	rm $(BUILD_DIR)*.*

include $(DEP_FILE)
