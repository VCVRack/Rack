#
# Common plugin makefile for "msvc" / win32 target
#
#  Also see plugin_common.mk

#
#
# In order to extend existing targets, add 
# your targets to the
#
#       HELP_RULES - extends "help" target
#    INSTALL_RULES - extends "install" target
#        BIN_RULES - extends "bin" target
#    BIN_POST_EVAL - evaluated after building "bin" target
#      CLEAN_RULES - extends "clean" target
#  REALCLEAN_RULES - extends "realclean" target
#
# variables  
#
#


#
# Determine TARGET filename if only TARGET_BASENAME name is known
#
ifeq ("$(TARGET)","")
TARGET=$(TARGET_BASENAME).dll
endif

#

CPPFLAGS += -I"$(VSVR_BASE_DIR)/dep/yac" -DWIN32 $(EXTRAFLAGS)
# -DYAC_FORCE_NO_TLS
EXTRALIBS += -DLL -MAP 


#
# Build plugin library
#
.PHONY: bin
bin: $(BIN_RULES) $(ALL_OBJ) 
	$(LD) $(LDFLAGS_SIZE) -OUT:$(TARGET) $(ALL_OBJ) $(EXTRALIBS)
	@echo "Build finished at `date +%H:%M`."
	@ls -l $(TARGET)
	$(call BIN_POST_FXN)
#	$(foreach postfxn,$(BIN_POST_FXNS),$(call $(postfxn)))
#	@echo "done"


.cpp.o:
	$(CC) $(CPPFLAGS) $(OPTFLAGS_PLUGIN) -c $< -Fo"$@"

.cc.o:
	$(CC) $(CPPFLAGS) $(OPTFLAGS_PLUGIN) -c $< -Fo"$@"

.c.o:
	$(CC) $(CPPFLAGS) $(OPTFLAGS_PLUGIN) -c $< -Fo"$@"



#
# Make clean and remove backup files
#
.PHONY: realclean
realclean: $(REALCLEAN_RULES) clean
	$(RM) `$(FIND) . -name \*\~` $(TARGET) *.exp *.lib *.plg *.ncb *.map *.manifest


#
# Remove object files and targets.
#
.PHONY: clean
clean: $(CLEAN_RULES)
	@echo "[...] cleaning up.."
	$(RM) $(ALL_OBJ) $(TARGET)
