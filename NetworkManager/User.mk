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



################################
#	File to hold the user custom build section for Tests
#
# USER.tests.SRC - will contain the subset of test classes that must be compiled. 
################################

#USER.tests.SRC = $(shell find $(TEST_DIR)/Model/ -iname '*.cpp')
#USER.tests.SRC = $(TEST_DIR)/Model/Tdma/LinkEngineTests.cpp
#USER.tests.SRC = $(TEST_DIR)/hart7/nmanager/flows/TestJoinDeviceThroughDevice.cpp
#USER.tests.SRC = $(TEST_DIR)/hart7/util/TestDetermineIndexCount.cpp
USER.tests.SRC = $(TEST_DIR)/hart7/util/TestModulo.cpp


USER.tests.run_params = --log_level=test_suite --catch_system_errors=yes --report_level=detailed --build_info=yes --run_test=Isa100 1> TestOut.log 2> TestReport.log; cat TestOut.log; cat TestReport.log

test-print-var:
	@echo $(TEST_SRCS) 

#CUSTOM_CLASS = /src/Stats/DIOCmds

#custom-compile: clean-custom $(OUT_DIR)$(CUSTOM_CLASS).o

#clean-custom:
#	rm -f $(OUT_DIR)$(CUSTOM_CLASS).o
	
#precompiler: $(CUSTOM_CLASS).cpp
#	@$(MKDIR) $(@D)
#	@$(ECHO) "precompiling '$<' ... "
#	$(PREC) $(CPP_FLAGS) $(CPP_INCLUDES) -MMD -MP -o $(CUSTOM_CLASS).prec.cpp "$<"