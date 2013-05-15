#
# Copyright (C) 2013 Nivis LLC.
# Email:   opensource@nivis.com
# Website: http://www.nivis.com
#
# This program is free software: you can redistribute it and/or modify
# it under the terms of the GNU General Public License as published by
# the Free Software Foundation, version 3 of the License.
#
# Redistribution and use in source and binary forms must retain this
# copyright notice.
#
# This program is distributed in the hope that it will be useful,
# but WITHOUT ANY WARRANTY; without even the implied warranty of
# MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
# GNU General Public License for more details.
#
# You should have received a copy of the GNU General Public License
# along with this program.  If not, see <http://www.gnu.org/licenses/>.
#
#




#include common defines
-include ~/default_home.mk
-include default_project.mk

export CPPLIB_PATH ?= ../cpplib
-include $(CPPLIB_PATH)/trunk/nbuild/Common.mk
#-include $(CPPLIB_PATH)/build/Common.mk


all: compile
-include ./Project.mk

clean:
	$(RM) -r $(OUT_DIR_BASE)

.PHONY: compile compile-init compile-test run-test print-var testClean clean

#########################
# COMPILATION targets
#

ifeq ($(link),static)
compile: compile-init $(MAIN_AR)
else ifeq ($(link),dynamic)
compile: compile-init $(MAIN_SHARED)
endif

$(MAIN_AR): $(CPP_OBJS) $(C_OBJS)
	$(common-gcc-ar)
	echo done

$(MAIN_SHARED): $(CPP_OBJS)  $(C_OBJS)
	$(LD) -shared -o"$@" $+
	$(STRIP) --strip-all -o"$@_stripped" "$@"

compile-init:
	@echo MAIN_AR=$(MAIN_AR)

compile-test: testClean $(TEST_MAIN_EXE)

TMPDATE := $(shell date +%Y.%m.%d_%H.%M.%S.%N)

run-test:
	@echo $(TMPDATE)
	rm -f logs/*
	$(TEST_MAIN_EXE) $(TEST_RUN_PARAMS)
	@echo $(TMPDATE)
	

print-var:
	@echo SRC_DIR=$(SRC_DIR)
	@echo
	@echo CPP_SRCS=$(CPP_SRCS)
	@echo 
	@echo C_SRCS=$(C_SRCS)
	@echo
	@echo CPP_OBJS=$(TEST_OBJS)
	@echo
	@echo MAIN_AR=$(MAIN_AR)
	@echo
	@echo CPP_INCLUDES=$(CPP_INCLUDES)
	@echo
	
	@echo DIST_VERSION=$(DIST_VERSION)
	@echo
	@echo DIST_DIR=$(DIST_DIR)
	@echo
	@echo LIBS=$(LIBS)
	@echo
	@echo LFLAGS=$(LFLAGS)
	@echo
	@echo CPP_FLAGS=$(CPP_FLAGS)

	@echo
	@echo TEST_SRCS=$(TEST_SRCS)
	@echo
	@echo TEST_OBJS=$(TEST_OBJS)
	@echo
	@echo TEST_DEPS=$(TEST_DEPS)
 
	@echo
	@echo 
	

######### NE-library

$(OUT_DIR)/%.o: %.cpp
	$(common-g++-compile)

######### NE-test executable

testClean:
	rm -f $(TEST_MAIN_EXE)
	echo "Force cleaned $(TEST_MAIN_EXE)"

#$(TEST_MAIN_EXE): $(filter-out %/src/Main.o,$(CPP_OBJS)) $(TEST_OBJS) out/$(if $(DEBUG),debug,release)/$(TARGET_OSTYPE)/libNetworkEngine-$(CC_VERSION)-$(_LIB_FLAGS)-$(DIST_VERSION).a
#$(TEST_MAIN_EXE): $(TEST_OBJS) $(MAIN_AR)
$(TEST_MAIN_EXE): $(CPP_OBJS) $(C_OBJS) $(TEST_OBJS) out/$(if $(DEBUG),debug,release)/$(TARGET_OSTYPE)/libNetworkEngine-$(CC_VERSION)-$(_LIB_FLAGS)-$(DIST_VERSION).a
	$(common-gcc-link)
	echo Done

$(OUT_DIR)/tests/%.o: tests/%.cpp
	@$(MKDIR) -p $(@D)
	$(CPP) $(CPP_FLAGS) $(CPP_INCLUDES) -Isrc -Icute -MMD -MP -MF"$(@:%.o=%.d)" -MT"$(@:%.o=%.d)" -o"$@" "$<"
	echo "test-compiled '$<'"

$(OUT_DIR)/%.o: %.c 
	$(common-cc-compile)

#########################
# DEPENDENCIES targets
#
ifneq ($(MAKECMDGOALS),clean)
-include $(CPP_DEPS)
-include $(TEST_DEPS)
-include $(C_DEPS)
endif

