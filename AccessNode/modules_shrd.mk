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



#this file is included from modules Makefiles
#DO NOT INCLUDE from Shared Makefile
.PHONY: clean exe_strip exe_copy
LINT=/cygdrive/c/home/lint/Lint-nt +v  -i"C:\home\lint\config" std.lnt



%.lob : %.cpp
	$(LINT) -u -zero -os\($<.tmp\) $< -oo


% : %.cpp $(SHRD_LIB)
	$(CXX) $(LDFLAGS) -o $@  $< $(SHRD_LIB_LINK)


########################################################
# SHRD_LIB
########################################################
$(SHRD_LIB) :
	$(MAKE) -C $(SHRD) compile

########################################################
# exe_copy
########################################################
exe_copy :
	if ! test -d $(AN_DIR) ; then mkdir $(AN_DIR); fi
	cp -f $(EXE) $(AN_DIR)


########################################################
# exe_strip
########################################################
exe_strip :
	$(STRIP) --strip-all -R.comment -R.note $(addprefix $(AN_DIR), $(EXE))



########################################################
# clean
########################################################
clean: clean_local
	$(MAKE) -C $(SHRD) clean


LAST_B_SH := $(shell [ -f $(SHRD)last_build ] && cat $(SHRD)last_build)
TMP := $(shell echo $(host)$(link) > $(SHRD)last_build )

ifneq "$(host)$(link)" "$(LAST_B_SH)"
     TMP := $(shell make clean host=$(host) link=$(link) )
endif

