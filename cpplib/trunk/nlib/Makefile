
#include common defines
export CPPLIB_PATH =../..
-include $(CPPLIB_PATH)/trunk/nbuild/Common.mk

ifneq ($(NBUILD_COMMON_INCLUDED),yes)
  $(error The CPPLIB_PATH=$(CPPLIB_PATH) is not configured correctly!)
endif

ifeq ($(DEBUG),true)
  $(error DEBUG Not supported yet!)
endif


-include ./Project.mk

.PHONY: all clean

all: copy-lib

clean:
	$(RM) -r $(OUT_DIR)
	$(ECHO) CLEAN DONE.


#########################
# COMPILATION targets
#
.PHONY: compile copy-lib

#compile: $(SOCKET_MAIN_SHARED) $(SQLITEXX_MAIN_SHARED)
compile: $(SOCKET_MAIN_SHARED) $(SOCKET_MAIN_STATIC) 

#copy-lib: $(SOCKET_MAIN_SHARED)_stripped $(SQLITEXX_MAIN_SHARED)_stripped
#	$(CP) $(SOCKET_MAIN_SHARED)_stripped lib/$(TARGET_OSTYPE)/$(shell basename $(SOCKET_MAIN_SHARED))
#	$(CP) $(SQLITEXX_MAIN_SHARED)_stripped lib/$(TARGET_OSTYPE)/$(shell basename $(SQLITEXX_MAIN_SHARED))
#	$(ECHO) COPY ALL SHARED LIBRARY DONE.
copy-lib: $(SOCKET_MAIN_STATIC) $(SOCKET_MAIN_SHARED)_stripped 
	$(CP) $(SOCKET_MAIN_SHARED)_stripped lib/$(TARGET_OSTYPE)/$(shell basename $(SOCKET_MAIN_SHARED))
	$(CP) $(SOCKET_MAIN_STATIC) lib/$(TARGET_OSTYPE)/$(shell basename $(SOCKET_MAIN_STATIC))
#	$(CP) $(SQLITEXX_MAIN_STATIC) lib/$(TARGET_OSTYPE)/$(shell basename $(SQLITEXX_MAIN_STATIC))
	$(ECHO) COPY ALL SHARED LIBRARY DONE.


$(SQLITEXX_MAIN_SHARED): $(SQLITEXX_CPP_OBJS) $(SQLITEXX_C_OBJS)
	$(LD) -shared -o"$@" $+ $(LFLAGS) $(LIBS)


$(SOCKET_MAIN_SHARED): $(SOCKET_CPP_OBJS)
	$(LD) -shared -o"$@" $+ $(LFLAGS) $(LIBS)


$(SQLITEXX_MAIN_SHARED)_stripped: $(SQLITEXX_MAIN_SHARED)
	$(STRIP) --strip-all -o"$@" "$<"


$(SOCKET_MAIN_SHARED)_stripped: $(SOCKET_MAIN_SHARED)
	$(STRIP) --strip-all -o"$@" "$<"

$(SQLITEXX_MAIN_STATIC): $(SQLITEXX_CPP_OBJS) $(SQLITEXX_C_OBJS)
	$(common-gcc-ar)


$(SOCKET_MAIN_STATIC): $(SOCKET_CPP_OBJS) 
	$(common-gcc-ar)

$(OUT_DIR)/%.o: %.cpp
	$(common-g++-compile)

$(OUT_DIR)/%.o: %.c
	$(common-cc-compile)

#########################
# DEPENDENCIES targets
#
ifneq ($(MAKECMDGOALS),clean)
#disabled -include $(CPP_DEPS)
endif
