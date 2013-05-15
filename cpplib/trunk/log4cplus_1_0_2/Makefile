#include common defines
export CPPLIB_PATH = ../..
-include $(CPPLIB_PATH)/trunk/nbuild/Common.mk

ifneq ($(NBUILD_COMMON_INCLUDED),yes)
  $(error The CPPLIB_PATH='$(CPPLIB_PATH)' is not configured correctly!)
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
compile: compile-init $(MAIN_AR)
copy-lib: compile
	$(CP) $(MAIN_AR) lib/$(TARGET_OSTYPE)/$(shell basename $(MAIN_AR))
	$(ECHO) COPY ALL SHARED LIBRARY DONE.
else ifeq ($(link),dynamic)
compile: compile-init $(MAIN_SHARED)
copy-lib: compile
	$(CP) $(MAIN_SHARED)_stripped lib/$(TARGET_OSTYPE)/$(shell basename $(MAIN_SHARED))
	$(ECHO) COPY ALL SHARED LIBRARY DONE.
endif

server: compile server/$(TARGET_OSTYPE)/logserver

server/$(TARGET_OSTYPE)/logserver: $(CPP_OBJS)
	@echo "Building Loggingserver.."
	$(common-gcc-link)

compile-init:
	@echo MAIN_SHARED=$(MAIN_SHARED)
	@echo MAIN_AR=$(MAIN_AR)

$(MAIN_SHARED): $(CPP_OBJS)
	$(LD) -shared -o"$@" $+
	$(STRIP) --strip-all -o"$@_stripped" "$@"

$(MAIN_AR): $(CPP_OBJS)
	$(common-gcc-ar)

$(OUT_DIR)/%.o: %.cxx
	$(common-g++-compile)

print-var:
	@echo CPP_SRCS=$(CPP_SRCS)
	@echo
	@echo CPP_OBJS=$(CPP_OBJS)
	@echo
	@echo C_OBJS=$(C_OBJS)
	@echo
	@echo CPP_DEPS=$(CPP_DEPS)
	@echo
	@echo CPP_INCLUDES=$(CPP_INCLUDES)
	@echo
	@echo CPP_FLAGS=$(CPP_FLAGS)
	@echo
	@echo LFLAGS=$(LFLAGS)
	@echo
	@echo CPP_FLAGS=$(CPP_FLAGS)
	@echo
	@echo LIBS=$(LIBS)
	@echo
	@echo STRIP=$(STRIP)

#########################
# DEPENDENCIES targets
#
ifneq ($(MAKECMDGOALS),clean)
-include $(CPP_DEPS)
endif

