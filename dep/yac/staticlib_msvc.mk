#
# Common lib makefile for "msvc" / win32 target
#

#
#
# In order to extend existing targets, add 
# your targets to the
#
#       HELP_RULES - extends "help" target
#    INSTALL_RULES - extends "install" target
#        BIN_RULES - extends "bin" target
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
TARGET=$(TARGET_BASENAME).lib
endif

CPPFLAGS += -I"$(VSVR_BASE_DIR)/dep/yac" -DWIN32 $(EXTRAFLAGS)
# -DYAC_FORCE_NO_TLS
EXTRALIBS += 
#-DLL -MAP 


#
# Build plugin library
#
bin: $(BIN_RULES) $(ALL_OBJ) 
#	$(LD) $(LDFLAGS_SIZE) -OUT:$(TARGET) $(ALL_OBJ) $(EXTRALIBS)
	$(LIB) -OUT:$(TARGET) $(LDFLAGS_SIZE) $(ALL_OBJ) $(EXTRALIBS)
	@ls -1l $(TARGET)
	@echo "Build finished at `date +%H:%M`."


.cpp.o:
	$(CC) $(CPPFLAGS) $(OPTFLAGS_PLUGIN) -c "$<" -Fo"$@"

.cc.o:
	$(CC) $(CPPFLAGS) $(OPTFLAGS_PLUGIN) -c "$<" -Fo"$@"

.c.o:
	$(CC) $(CPPFLAGS) $(OPTFLAGS_PLUGIN) -c "$<" -Fo"$@"


#
# Install plugin
#  (Note: cannot depend on "bin" target since this would overwrite the UPX compressed file..)
#
.PHONY: install
install: $(INSTALL_RULES)
	$(CP) $(TARGET) $(LIB_INSTALL_PREFIX)
	@echo "[...] $(TARGET) installed to \"$(LIB_INSTALL_PREFIX)\".";





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
