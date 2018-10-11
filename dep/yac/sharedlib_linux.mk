#
# Common plugin makefile for Linux / GCC target
#
#
#

#
# Determine TARGET filename if only PLUGIN name is known
#
ifeq ("$(TARGET)","")
TARGET=$(TARGET_BASENAME).so
endif

CPPFLAGS += -fPIC -I"dep/yac" $(EXTRAFLAGS)
CFLAGS += -fPIC -I"dep/yac" $(EXTRAFLAGS)
EXTRALIBS += -L$(CROSS_ROOT)/usr/lib -lm 

###########include sharedlib_common_gcc.mk
#
# Plugin targets for all GCC based platforms
#

CPPFLAGS += -Wall -I"dep/yac" -I$(CROSS_ROOT)/usr/include 
CFLAGS += -Wall -I$(CROSS_ROOT)/usr/include


#
# Build plugin library
#
bin: $(BIN_RULES) $(ALL_OBJ) 
	$(CPP) -shared -o "$(TARGET)" -Wl,-soname,$(TARGET) $(ALL_OBJ) $(EXTRALIBS)
ifneq ($(DEBUG),y)
	$(STRIP) "$(TARGET)"
endif
	$(call BIN_POST_FXN)
	@echo "Build finished at `date +%H:%M`."


.cpp.o:
	$(CPP) $(CPPFLAGS) $(OPTFLAGS) $(DBGFLAGS) -c $< -o $@ 

.cc.o:
	$(CPP) $(CPPFLAGS) $(OPTFLAGS) $(DBGFLAGS) -c $< -o $@ 

.c.o:
	$(CC) $(CFLAGS) $(OPTFLAGS) $(DBGFLAGS) -c $< -o $@ 


#
# Make clean and remove backup files
#
.PHONY: realclean
realclean: $(REALCLEAN_RULES) clean
	$(RM) `$(FIND) . -name \*\~` *.map

.PHONY: clean
clean: $(CLEAN_RULES)
	@echo "[...] cleaning up.."
	$(RM) $(ALL_OBJ) $(TARGET)
