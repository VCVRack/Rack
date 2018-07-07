PACKAGES       =  warps/dsp warps/test stmlib/utils stmlib/dsp warps

VPATH          = $(PACKAGES)

TARGET         = warps_test
BUILD_ROOT     = build/
BUILD_DIR      = $(BUILD_ROOT)$(TARGET)/
CC_FILES       = warps_test.cc \
		filter_bank.cc \
		modulator.cc \
		oscillator.cc \
		random.cc \
		resources.cc \
		units.cc \
		vocoder.cc
OBJ_FILES      = $(CC_FILES:.cc=.o)
OBJS           = $(patsubst %,$(BUILD_DIR)%,$(OBJ_FILES)) $(STARTUP_OBJ)
DEPS           = $(OBJS:.o=.d)
DEP_FILE       = $(BUILD_DIR)depends.mk

all:  clouds_test

$(BUILD_DIR):
	mkdir -p $(BUILD_DIR)

$(BUILD_DIR)%.o: %.cc
	g++ -c -O2 -std=c++11 -DTEST -g -Wall -Werror -I. $< -o $@

$(BUILD_DIR)%.d: %.cc
	g++ -MM -DTEST -I. $< -MF $@ -MT $(@:.d=.o)

clouds_test:  $(OBJS)
	g++ -o $(TARGET) $(OBJS)

depends:  $(DEPS)
	cat $(DEPS) > $(DEP_FILE)

$(DEP_FILE):  $(BUILD_DIR) $(DEPS)
	cat $(DEPS) > $(DEP_FILE)

include $(DEP_FILE)
