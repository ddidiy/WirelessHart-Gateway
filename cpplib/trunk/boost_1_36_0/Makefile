#include common defines
export CPPLIB_PATH = ../..
-include $(CPPLIB_PATH)/trunk/nbuild/Common.mk

ifneq ($(NBUILD_COMMON_INCLUDED),yes)
  $(error The CPPLIB_PATH=$(CPPLIB_PATH) is not configured correctly!)
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

ifeq ($(link),static)
compile: $(THREAD_MAIN) $(SIGNALS_MAIN) $(SYSTEM_MAIN) $(PROGRAM_OPTIONS_MAIN) $(TEST_MAIN)
copy-lib: compile
	$(CP) $(THREAD_MAIN) lib/$(TARGET_OSTYPE)
	$(CP) $(SIGNALS_MAIN) lib/$(TARGET_OSTYPE)
	#$(CP) $(GRAPH_MAIN) lib/$(TARGET_OSTYPE)
	$(CP) $(SYSTEM_MAIN) lib/$(TARGET_OSTYPE)
	$(CP) $(PROGRAM_OPTIONS_MAIN) lib/$(TARGET_OSTYPE)
	$(CP) $(TEST_MAIN) lib/$(TARGET_OSTYPE)
	$(ECHO) COPY ALL STATIC LIBRARY DONE.
else ifeq ($(link),dynamic)
compile: $(THREAD_SHARED) $(SIGNALS_SHARED) $(SYSTEM_SHARED) $(PROGRAM_OPTIONS_SHARED) $(TEST_SHARED)
copy-lib: compile
	$(CP) $(THREAD_SHARED) lib/$(TARGET_OSTYPE)
	$(CP) $(SIGNALS_SHARED) lib/$(TARGET_OSTYPE)
	#$(CP) $(GRAPH_SHARED) lib/$(TARGET_OSTYPE)
	$(CP) $(SYSTEM_SHARED) lib/$(TARGET_OSTYPE)
	$(CP) $(PROGRAM_OPTIONS_SHARED) lib/$(TARGET_OSTYPE)
	$(CP) $(TEST_SHARED) lib/$(TARGET_OSTYPE)
	$(ECHO) COPY ALL STATIC LIBRARY DONE.
endif



$(THREAD_MAIN): $(THREAD_CPP_OBJS)
	$(common-gcc-ar)

$(THREAD_SHARED): $(THREAD_CPP_OBJS)
	$(LD) -shared -o"$@" $+
	$(STRIP) --strip-all -o"$@_stripped" "$@"

$(SIGNALS_MAIN): $(SIGNALS_CPP_OBJS)
	$(common-gcc-ar)

$(SIGNALS_SHARED): $(SIGNALS_CPP_OBJS)
	$(LD) -shared -o"$@" $+
	$(STRIP) --strip-all -o"$@_stripped" "$@"

#$(GRAPH_MAIN): $(GRAPH_CPP_OBJS)
#	$(common-gcc-ar)

$(SYSTEM_MAIN): $(SYSTEM_CPP_OBJS)
	$(common-gcc-ar)

$(SYSTEM_SHARED): $(SYSTEM_CPP_OBJS)
	$(LD) -shared -o"$@" $+
	$(STRIP) --strip-all -o"$@_stripped" "$@"

$(PROGRAM_OPTIONS_MAIN): $(PROGRAM_OPTIONS_CPP_OBJS)
	$(common-gcc-ar)

$(PROGRAM_OPTIONS_SHARED): $(PROGRAM_OPTIONS_CPP_OBJS)
	$(LD) -shared -o"$@" $+
	$(STRIP) --strip-all -o"$@_stripped" "$@"

$(TEST_MAIN): $(TEST_CPP_OBJS)
	$(common-gcc-ar)

$(TEST_SHARED): $(TEST_CPP_OBJS)
	$(LD) -shared -o"$@" $+
	$(STRIP) --strip-all -o"$@_stripped" "$@"

$(OUT_DIR)/%.o: %.cpp
	$(common-g++-compile)


#########################
# DEPENDENCIES targets
#
ifneq ($(MAKECMDGOALS),clean)
#disabled -include $(CPP_DEPS)
endif

