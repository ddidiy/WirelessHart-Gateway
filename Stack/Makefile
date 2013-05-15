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

export CPPLIB_PATH?=../../../cpplib
-include $(CPPLIB_PATH)/trunk/nbuild/Common.mk

-include ./Project.mk

ifneq ($(ECHO),@echo)
  $(error The CPPLIB_PATH is bad configured!)
endif



all: compile #run-test

.PHONY: clean

clean:
	$(RM) -r $(OUT_DIR)
	echo CLEAN DONE.
	

#########################
# COMPILATION targets
#


.PHONY: compile run-test

compile: compile-init $(LIB_EXE)


compile-init:
	$(ECHO) TOOLCHAIN=$(TOOLCHAIN)
	$(ECHO) LIB_EXE=$(LIB_EXE)

run-test: compile
	echo "Run tests:"
	$(LIB_EXE) --log_level=test_suite --catch_system_errors=yes --run_test=hart_stack*

$(OUT_DIR)/%.a: $(CPP_OBJS) $(C_OBJS)
	$(common-gcc-ar)
	echo COMPILE DONE

$(OUT_DIR)/%.so: $(CPP_OBJS) $(C_OBJS)
	$(LD) -shared -o"$@" $+ $(LFLAGS) 
	$(STRIP) --strip-all  "$@" 

$(OUT_DIR)/%.o: %.cpp
	$(common-g++-compile)

#force to compile in c++
$(OUT_DIR)/%.o: %.c
	$(common-cc-compile)
	
#$(OUT_DIR)/%.o: %.c
#	$(common-cc-compile)
	

#########################
# SVN targets
#	


#########################
# DEPENDENCIES targets
#	
ifneq ($(MAKECMDGOALS),clean)
-include $(CPP_DEPS)
-include $(C_DEPS)
endif
