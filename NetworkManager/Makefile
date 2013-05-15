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



default: compile

#include common defines
-include ~/default_home.mk
-include default_project.mk

export CPPLIB_PATH?=../cpplib
-include $(CPPLIB_PATH)/trunk/nbuild/Common.mk

export NET_ENGINE_PATH = ../NetworkEngine

-include ./Project.mk

ifneq ($(ECHO),@echo)
  $(error The CPPLIB_PATH is bad configured!)
endif

all: compile

.PHONY: clean compile compile-init compile-test run-test print-var

clean:
	$(RM) -r $(shell dirname $(OUT_DIR))
	echo 'CLEAN DONE dir=$(shell dirname $(OUT_DIR))'	

#########################
# COMPILATION targets
#

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
	
	@echo TEST_SRCS=$(TEST_SRCS)
	@echo 
	@echo TEST_OBJS=$(TEST_OBJS)
	@echo 
	@echo TEST_DEPS=$(TEST_DEPS)
	@echo 
	
	@echo CPP_FLAGS=$(CPP_FLAGS)
	@echo 
	@echo LFLAGS=$(LFLAGS)
	@echo 
	@echo CPP_FLAGS=$(CPP_FLAGS)
	@echo 
	@echo LIBS=$(LIBS)
	@echo 	
	@echo DIST_TAR_VERSION=$(DIST_TAR_VERSION)
	
compile: compile-init $(MAIN_EXE)

compile-init:
	echo TARGET_OSTYPE=$(TARGET_OSTYPE)
	echo OUT_DIR=$(OUT_DIR)
	echo TOOLCHAIN=$(TOOLCHAIN)
	echo MAIN_EXE=$(MAIN_EXE)

#run:
#	$(MAIN_EXE)
	
run: $(MAIN_EXE)
	@clear
	@rm -rf logs
	$(MKDIR) logs
	LD_LIBRARY_PATH=. $(MAIN_EXE) > /dev/null &
	multitail -CS log4cplus -i logs/nm_debug.log

compile-test: $(TEST_MAIN_EXE)

run-test: 
	echo "Run tests:"
	echo TEST_MAIN_EXE= $(TEST_MAIN_EXE)
	$(TEST_MAIN_EXE) $(TEST_RUN_PARAMS)

######### NM - executable

$(MAIN_EXE): $(CPP_OBJS) $(C_OBJS) $(NET_ENGINE_DIR)/out/$(if $(DEBUG),debug,release)/$(TARGET_OSTYPE)/NetworkEngine/libNetworkEngine-$(CC_VERSION)-$(_LIB_FLAGS)-$(DIST_TAR_VERSION).a
	$(common-gcc-link)
	$(STRIP) --strip-all -R.comment -R.note -o $(MAIN_EXE)_stripped $(MAIN_EXE)
	echo COMPILE DONE

$(OUT_DIR)/%.o: %.cpp
	$(common-g++-compile)
	

######### NM - test executable	
$(TEST_MAIN_EXE): $(filter-out %/src/main.o,$(CPP_OBJS)) $(C_OBJS) $(TEST_OBJS) $(NET_ENGINE_DIR)/out/$(if $(DEBUG),debug,release)/$(TARGET_OSTYPE)/libNetworkEngine-$(CC_VERSION)-$(_LIB_FLAGS)-$(DIST_TAR_VERSION).a
	$(common-gcc-link)
	
$(OUT_DIR)/test/%.o: test/%.cpp  
	@$(MKDIR) -p $(@D)
	$(CPP) $(CPP_FLAGS) $(CPP_INCLUDES) -Isrc -Icute -MMD -MP -MF"$(@:%.o=%.d)" -MT"$(@:%.o=%.d)" -o"$@" "$<"
	echo "test-compiled '$<'"
	echo
	
#force to compile in c++
#$(OUT_DIR)/%.o: %.c
#	$(common-g++-compile)
	
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
