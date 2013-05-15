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



#for help read help rule below

#gdb is suitable for debugging : compiled only with DEBUG configuration
#USAGE of gdb version:
#	- copy STRIPPED version (from an folder) to board
#	- copy UNSTRIPPED version (from original folder) to windows with cygwin
#	- on board, run gdbserver *:5555 <program_to_debug>
#	- run gdb from cygwin, and specify at target: tcp/ip, target ip, and 5555 (port)

# If you usually compile for only one project/host/compile type only,
# you can say so in a defaults file, named default.mk. This file contain
# default make parametaer so you don't have to type'em every time - just type
# make. However, if you want to compile for some different project/host/compile type
# you can, using command line
# default.mk contain default values for vars (dist, host, link)
# default.mk is local to every user (NOT on cvs)
#	a line (in default.mk file) to define a default value should look like:
#		export dist?=mesh
#		export host?=m68k-unknown-linux-uclibc
#		export link?=dynamic
#		export hw?=vr900
# if not exist is not a problem:)
TOP=.

include sys_inc.mk

.PHONY:	all sys_inc.mk Makefile FORCE

include dist_version.mk

# Modules_all should cotain the all modules and MUST include all modules from var Modules
Modules_all = Shared Tools cgi IsaGw WirelessHART  



#default
Modules = Shared Tools
ifneq "$(win)" "1"
Modules += cgi
endif

#-----------------------------------------------------------------------------
# HW
#-----------------------------------------------------------------------------


EXE_STRIP=exe_strip

#-----------------------------------------------------------------------------
# Dist
#-----------------------------------------------------------------------------


#-----------------------------------------------------------------------------
# release
#-----------------------------------------------------------------------------

ifeq "$(release)" "isa"
	Modules +=  IsaGw 

endif

ifeq "$(release)" "whart"
	Modules += WirelessHART
 
endif


all: $(dist)
	@echo "SW_VERSION=${SW_VERSION}" > last_build 

help:
	@echo "make [rule] [VAR=VALUE]..."
	@echo "	rule:"
	@echo "		all            - default"
	@echo "		help           - this message "
	@echo "		dep            - make depends files"
	@echo "		clean_out      - remove out folder -> force rebuild"
	@echo "		clean_dist     - remove dist folder an/"
	@echo "		clean          - clean out and dist"
	@echo "		clean_all	   - clean all old and new style"
	@echo "		clean_modules  - clean modules on old style"
	@echo "		cfast=<yes|no> - disable size optimizations and warnings"
	@echo "		                 disable header dependencies"
	@echo ""
	@echo "	Variables:"
	@echo "		The FIRST value in a list is the DEFAULT value."
	@echo "		dist=[mesh]  - default mesh"
	@echo "		host=[ m68k-unknown-linux-uclibc, i386] "
	@echo "		link=[dynamic, static]"
	@echo "		cc=[3.4, 2.95, 4.11]"
	@echo "		hw=[$(HW)]"
	@echo "		release=[$(RELEASES)]"
	@echo "		include=[<modules>] used for modules (folders separated by whitespaces) outside AccessNode"
	@echo "		Example: include=.~/c/compile_mh/Monitor_Host"
	@echo "	The \$$(host) variable will take a default value depending on \$$(hw) value."
	@echo "	Ex:  for hw=vr900 then host will be host=m68k-unknown-linux-uclibc"
	@echo ""
	@echo " system.mk:"
	@echo "	The functionality below is available when using 'include system.mk' "
	@$(MAKE) --no-print-directory -f system.mk help

$(Modules_all) : FORCE
	@set -e ; echo "   * $(request) [$@]" && \
	/usr/bin/time --format="   * $(request) [$@] (%U user, %S sys)"  $(MAKE) -C $@ $(request) && \
	echo

FORCE:

compile: clean_dist
	$(MAKE) $(Modules) request=$@ 

exe_copy: compile
	$(MAKE) $(Modules) request=$@

exe_strip: exe_copy
	$(MAKE) $(Modules) request=$@

$(dist): exe_strip an_copy
	@[ ! -d "$(AN_DIR)" ] && mkdir $(AN_DIR) || exit 0
	@echo -n $(DIST_VERSION) > $(AN_DIR)/version
	@echo -n $(link) >$(AN_DIR)/build_link_type
	@$(AN_CONFIG_TOP_DIR)/build_dist.sh $(dist) $(hw) "$(release)" "$(EXCLUDE.MODULES)"

an_copy:
	for file in $(include); do $(MAKE) -C $$file $@ ; done

dep lint :
	$(MAKE) $(Modules) request=$@


#  Cleaning rules
clean_modules:
	$(MAKE) $(Modules_all) request=clean_local

clean_exe:
	find $(OUTPUT_DIR) -type f  ! -regex ".*/objs/.*" -print | xargs /bin/rm -f -r

#clean_dist SHOULD rm ONLY folder an/ :
clean_dist:
	@echo " * RM an" && rm -Rf $(AN_DIR) || exit 0

clean_out:
	@echo " * RM $(OUTPUT_DIR)" && rm -Rf $(OUTPUT_DIR) || exit 0

clean: clean_dist clean_out clean_specific

clean_all: clean clean_modules
	find . -name "*.o" | xargs rm -rf

# end of file
