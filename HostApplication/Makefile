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

export CPPLIB_PATH?=../cpplib
-include $(CPPLIB_PATH)/trunk/nbuild/Common.mk

-include ./Project.mk

ifneq ($(ECHO),@echo)
  $(error The CPPLIB_PATH is bad configured!)
endif



all: compile #run-test

.PHONY: clean strip

clean:
	$(RM) -r $(shell dirname $(OUT_DIR))
	$(ECHO) 'CLEAN DONE dir=$(shell dirname $(OUT_DIR))'
	

#########################
# COMPILATION targets
#
.PHONY: compile run

compile: compile-init $(MAIN_EXE)


compile-init:
	$(ECHO) TOOLCHAIN=$(TOOLCHAIN)
	$(ECHO) MAIN_EXE=$(MAIN_EXE)
	
run: $(MAIN_EXE)
	@clear
	@rm -rf logs
	$(MKDIR) logs
	LD_LIBRARY_PATH=. $(MAIN_EXE) > /dev/null &
	multitail -CS log4cplus -i logs/gw_debug.log
	

run-test: compile
	$(ECHO) "Run tests:"
	LD_LIBRARY_PATH=$(CPPLIB_PATH)/trunk/nlib/lib/$(TARGET_OSTYPE):\
		$(CPPLIB_PATH)/trunk/log4cplus_1_0_2/lib/$(TARGET_OSTYPE) $(MAIN_EXE) --log_level=test_suite --catch_system_errors=yes --run_test=hart_stack*
	
$(MAIN_EXE): $(CPP_OBJS) $(C_OBJS)
	$(common-gcc-link)
	$(STRIP) --strip-all $(MAIN_EXE)
	$(ECHO) COMPILE DONE
	

$(OUT_DIR)/%.o: %.cpp
	$(common-g++-compile) 
	#-fno-rtti

#force to compile in c++
#$(OUT_DIR)/%.o: %.c
#	$(common-g++-compile)
	
$(OUT_DIR)/%.o: %.c
	$(common-cc-compile)
	

#########################
# DIST RELEASE
#

DISTR_OUT_DIR=distr
DISTR_MAIN_EXE=$(DISTR_OUT_DIR)/$(PROJECT_NAME).$(TARGET_OSTYPE_EXE_EXT)

distr-compile: distr-init $(DISTR_MAIN_EXE)


distr-init:
	$(MKDIR) -p $(DISTR_OUT_DIR)

$(DISTR_MAIN_EXE): $(DISTR_OUT_DIR)/single_file.o $(C_OBJS)
	$(common-gcc-link)
	#$(STRIP) --strip-all -o $(DISTR_MAIN_EXE)_stripped $(DISTR_MAIN_EXE)
	$(ECHO) DISTR COMPILE DONE

$(DISTR_OUT_DIR)/single_file.o: $(DISTR_OUT_DIR)/single_file.cpp
	$(common-g++-compile) -fno-rtti

$(DISTR_OUT_DIR)/single_file.cpp: $(CPP_SRCS)
	@cat $+ > $@


#########################
# DEPENDENCIES targets
#	
ifneq ($(MAKECMDGOALS),clean)
-include $(CPP_DEPS)
-include $(C_DEPS)
endif
