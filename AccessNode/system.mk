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



# INPUT VARIABLES(MUST BE RECURSIVELY EXPANDED VARIABLES[1]):
#    TOP                - directory of system.mk file and default.mk
#    PROJ_TOP           - directory of project Makefile
#    TARGET.SUBDIRS     - list of modules. A module is not an executable name.
#    TARGET.LIBRARIES   - list of libraries ( without extension and without lib prefix)
#    TARGET.PROGRAMS    - list of executable targets (with no extension)
#    <exename>.EXE_SRC  - other source files needed for <exename>
#    <exename>.SUBDIRS  - list of subdirectories for recursive make
#    <exename>.FILES    - list of object file targets (with no extension)
#    <exename>.DEPS     - list of dependency targets
#    <exename>.CXXFLAGS - compilation flags
#    <exename>.CFLAGS     for OS specific or test files, -I.. is added
#                         in addition to this, there are also defined:
#    <exename>.LIBS     - list of libraries to be used.
#                         Depending on $(link), the appropiate `ld' options
#                         will be generated.
#    <exename>.TESTS    - list of unittests, without exension.
#
# TARGETS:
#    all                - default target;
#                         compiles OBJ_LIST and builds TARGET.PROGRAMS, TARGET.SUBDIRS and TARGET.LIBRARIES
#    tests              - build tests
#    todo               - grep after TODOs
#
# LINKS
#    [1]: http://www.gnu.org/software/make/manual/make.html#Flavors
#

##################################
-include $(TOP)/sys_inc.mk
##################################
export SHELL=/bin/bash
.SUFFIXES:

ifeq ($(findstring help,$(MAKECMDGOALS)),)

ifndef host
$(error ERR: $$(host) undefined. Please specified hw or host variables in comand line)
endif

ifndef TOP
TOP      ?= ..
$(error ERR: $$(TOP) undefined)
endif

# validate link type
ifeq "$(findstring _$(link)_,_static_dynamic_)" ""
$(error Unknown link type:"$(link)")
endif

endif

TARGET.SUBDIRS  ?=
TARGET.PROGRAMS ?=
TARGET.LIBRARIES?=
VERSION?="\"local\""
ifneq (,$(sudo))
export _SUDO:=sudo
endif

ifeq ($(findstring help,$(MAKECMDGOALS)),)
ifeq "$(TARGET.PROGRAMS)$(TARGET.LIBRARIES)$(TARGET.SUBDIRS)" ""
$(error Nothing to do: empty TARGET.PROGRAMS, TARGET.LIBRARIES or TARGET.SUBDIRS in Makefile!)
endif
endif

#override this in each section, if necessary
CC      ?=gcc
CXX     ?=g++
LD      ?=ld
STRIP   ?=strip
AR      ?=ar
RANLIB  ?=ranlib
CPP     ?='$(CC) -E'
ADDR2LINE?=addr2line

CFLAGS  ?=
CXXFLAGS?=
LDFLAGS ?=

cfast ?=no

ifeq "$(cfast)" "yes"
nodeps?=yes
optimize?=no
memcheck?=no
endif

nodeps?=no
optimize?=yes
memcheck ?=no


# common/global to all compilers/projects
VERSION ?="\"local\""

UDNSLIB ?= $(_LIBDIR)/libudns.a

TOP:=$(realpath $(TOP))

#Local variables
CWD:=$(shell pwd)
_OUTPUT_TUPLE:=$(OUTPUT_DIR)/$(host)
# obtain trailing string by removing the common string $(TOP) 
_MODULE_DIR:= $(subst $(TOP),,$(CURDIR))
_EXEDIR    := $(_OUTPUT_TUPLE)/$(_MODULE_DIR)
_OBJDIR    := $(_EXEDIR)/objs
_LIBDIR    ?= $(AUX_LIBS_DIR_LIB)
_LIBOUTDIR := $(_OUTPUT_TUPLE)/libs$(_LIBEXT)
_MKFILES   := $(addprefix $(TOP)/,$(GLOBAL_DEP))
_INCLUDED  ?=

#$(warning *** WARN: TOP=$(TOP))
#$(warning *** WARN: CWD=$(CWD))
#$(warning *** WARN: _MODULE_DIR=$(_MODULE_DIR))
#$(warning *** WARN: _OUTPUT_TUPLE=$(_OUTPUT_TUPLE))
#$(warning *** WARN: _OBJDIR=$(_OBJDIR))
#$(warning *** WARN: _EXEDIR=$(_EXEDIR))

_CXXTESTGEN = $(TOP)/UnitTest/test_fmwk/cxxtestgen.pl
_TABS=""
ifneq "$(MAKELEVEL)" "0"
_TABS=$(word $(MAKELEVEL),\x20 \x20\x20 \x20\x20\x20\x20\x20\x20 \x20\x20\x20\x20\x20\x20\x20\x20 \x20\x20\x20\x20\x20\x20\x20\x20\x20\x20)
endif

ECHO :=printf "$(_TABS)%s\n"
SHELL_SET_X := set +x

ifneq ($(findstring -s,$(MAKEFLAGS)),)
DISP := sil
Q := @
SECHO:= -@false
export MAKE_IS_SILENT := y
else
SHELL_SET_X := set -x
export MAKE_IS_SILENT := n
SECHO:=@$(ECHO)
DISP :=pur
Q:=
endif

# if make is run in parralel, then use locking
ifneq ($(findstring -j,$(MAKEFLAGS)),)
_LOCKEXT   :=_lock
endif

LIBS:=$(addprefix lib,$(addsuffix .a,$(TARGET.LIBRARIES)))
LIBS+=$(addprefix lib,$(addsuffix .so,$(TARGET.LIBRARIES)))


######################################################
#{{{ Defines
######################################################

define MKOBJDIR
	mkdir -p $(subst ../,,$(@D))
endef

define MKANDIR
	@set -e ; [ ! -d "$(AN_DIR)" ] && mkdir $(AN_DIR) ;\
	[ ! -d "$(AN_LIB_DIR)" ] && mkdir -p $(AN_LIB_DIR)  ; \
	[ ! -d "$(EXE_DST_DIR)" ] && mkdir -p $(EXE_DST_DIR) || exit 0
endef

define MKDEPS
	@[ -f "$(_OBJDIR)/$(*F).d"  ] && cp $(_OBJDIR)/$(*F).d $(_OBJDIR)/$(*F).P && \
	sed -e 's/#.*//' -e 's/^[^:]*: *//' -e 's/ *\\$$//' \
		-e '/^$$/ d' -e 's/$$/ :/' < $(_OBJDIR)/$(*F).d >> $(_OBJDIR)/$(*F).P && \
	rm -f $(_OBJDIR)/$(*F).d \
	|| exit 0
endef

ifeq "$(nodeps)" "yes"
define MKDEPS
endef
endif
#}}}++++++++++++++++++++++++++++++++++++++++++++++++++++


define EXPAND_LIBS
$(if $(filter $(link),dynamic),$(addprefix -l,$(1)))
endef

#$(info "$(link)",$(findstring "dynamic",$(link)))

#####################################################################################
#{{{ TARGET.PROGRAMS command sequence
#####################################################################################
define CMPL_PROGRAM
runtests tests clean_local clean_tests $(1):_MODULE=$(1)

VPATH=$(TOP)
$(1).TMPFILES=$$(addprefix $(_OBJDIR)/,$(addsuffix .o, $(subst ../,,$($(1).FILES)) ))

$(1): $(_MKFILES) $$($(1).SUBDIRS) $$($(1).DEPS) $$($(1).TMPFILES)
ifneq (,$(value $(1).FILES))
	@printf " %b* LINK   [$(1)]\t$(_EXEDIR)/$(1)\n" "$(_TABS)"
	$(CXX) $$(subst ../,,$$($(1).TMPFILES)) -L$(_LIBDIR) $$($(1).LDFLAGS) $(call EXPAND_LIBS,$($(1).LIBS)) $(LDFLAGS) -o $(_EXEDIR)/$(1)
ifneq (,$(run))
	$(_SUDO) $(_EXEDIR)/$(1)
endif
endif
$$($(1).SUBDIRS)::
	+$(MAKE) -C $$@
endef
#}}}++++++++++++++++++++++++++++++++++++++++++++++++++++



#####################################################################################
#{{{ Build TARGET.LIBRARIES
# When PARALLEL building, put a critical section around
# SUBDIRS and FILES compilation.
#####################################################################################
define CMPL_LIBRARY
tests clean_local clean_tests $(_EXEDIR)/lib$(1)$(_LIBEXT) $(1)_lock:_MODULE=$(1)
VPATH=$(TOP)
$(1).TMPFILES=$$(addprefix $(_OBJDIR)/,$(addsuffix .o, $(subst ../,,$($(1).FILES)) ))

$(_EXEDIR)/lib$(1)$(_LIBEXT)_lock:
	@( flock -x 200 ; $(MAKE) $(_EXEDIR)/lib$(1)$(_LIBEXT)  )200>/tmp/$(USER).$(1)$(_LOCKEXT)


$$($(1).DEPS): $$($(1).SUBDIRS)
$$($(1).TMPFILES): $$($(1).DEPS)

$(_EXEDIR)/lib$(1)$(_LIBEXT): $(_MKFILES) $$($(1).TMPFILES)
	@printf " %b* $(link) [$(1)]\t$$@\n" "$(_TABS)"
ifeq "$(link)" "static"
	$(AR) rc $$@ $$(subst ../,,$$($(1).TMPFILES))
	$(RANLIB) $$@
else
	$(CXX) $$(subst ../,,$$($(1).TMPFILES)) $($(1).LDFLAGS) $(LDFLAGS) -fPIC -shared -o $$@
endif
$$($(1).SUBDIRS):: 
	+$(MAKE) -C $$@
endef
#}}}++++++++++++++++++++++++++++++++++++++++++++++++++++





#####################################################################################
# {{{ CFLAGS, CXXFLAGS, LDFLAGS
# _INCLUDED guards for multiple inclusions of system.mk
#####################################################################################
ifndef _INCLUDED
export _INCLUDED:=1
_USEDEBUG:=-g -ggdb3

	# common compilers(c/c++) flags
	_CCFLAGS:=$(_USEDEBUG) \
		-DVERSION=$(VERSION) \
		-Wall \
		-Wredundant-decls \
		-Wshadow \
		-Wsign-compare \
		-I$(TOP) -I$(AUX_LIBS_DIR_INC)
		#-Wunreachable-code
		#-Winit-self \

	# compiler optimization flags
	_OPTCFLAGS := -Os
	_EXTCFLAGS := -ffunction-sections -fdata-sections
	_EXTLDFLAGS:= -Wl,--no-whole-archive -Wl,--relax -Wl,--gc-sections

	ifeq "$(coverage)" "yes"
		_CCFLAGS+=-fprofile-arcs -ftest-coverage
		LDFLAGS +=-lgcov -fprofile-arcs -ftest-coverage
	endif

	ifeq "$(profile)" "yes"
		_CCFLAGS += -pg -O0
		ifeq "$(hw)" "pc"
			LDFLAGS += -pg
		endif
		optimize=no
	endif

	ifeq "$(optimize)" "yes"
		_CCFLAGS += $(_OPTCFLAGS)
		LDFLAGS +=$(_OPTLDFLAGS)
	else
		_CCFLAGS += -O0
		LDFLAGS += -O0 -fno-stack-protector
	endif

	ifeq "$(memcheck)" "yes"
		_CCFLAGS += -DDEBUG_MODE
	endif

	CFLAGS  += $(_CCFLAGS) -Wmissing-declarations
	CXXFLAGS+= $(_CCFLAGS) 
	LDFLAGS	+= $(_USEDEBUG) 
#	LDFLAGS	+= $(_USEDEBUG) -Wl,-Map=cucu.map -Wl,--cref
endif

ifneq "$(nodeps)" "yes"
GEN_DEP=-MD -Wp,-MD,$(_OBJDIR)/$(*F).d
endif
#}}}++++++++++++++++++++++++++++++++++++++++++++++++++++



#####################################################################################
# {{{ Generate MACROS for distribution, architecture from `dist`,`host` variables
#####################################################################################

ifdef dist
	TARGET_ARCH:=-D$(shell echo $(dist) | tr '[a-z]' '[A-Z]')
endif

ifdef host
	export ARCH:=$(shell echo $(host) | cut -f1 -d'-')
	TARGET_ARCH += -D$(shell echo $(ARCH) | tr '[a-z]' '[A-Z]')
endif

ifdef hw
ifeq (,$(findstring $(hw),$(HW)))
$(error "Unknown hardware $(hw)")
endif
	TARGET_ARCH+= -DHW_$(shell echo $(hw) | cut -f1 -d'-' | tr '[a-z]' '[A-Z]')
endif

ifdef release
ifeq (,$(findstring $(release),$(RELEASES)))
$(error "Unknown release $(release)")
endif
	TARGET_ARCH+= -DRELEASE_$(shell echo $(release) | tr '[a-z]' '[A-Z]')
endif

ifeq "$(fs_tree)" "rel"
	TARGET_ARCH+= -DFS_TREE_REL
endif

ifdef DUMPRTL
	TARGET_ARCH+= -dr
endif

ifeq "$(static_libc)" "1"
	LDFLAGS+= -static
endif

#}}}++++++++++++++++++++++++++++++++++++++++++++++++++++



######################################################
#{{{ Excludes
######################################################

ifdef exclude
TARGET.SUBDIRS:=$(filter-out $(exclude),$(TARGET.SUBDIRS))
EXE :=$(filter-out $(exclude),$(EXE))
endif
#}}}++++++++++++++++++++++++++++++++++++++++++++++++++++



export fs_tree static_libc cc link host
export GEN_DEP CXX CC LD STRIP AR RANLIB SHRD_LIB_LINK SHRD_LIB CFLAGS CXXFLAGS LDFLAGS ZLIB UDNSLIB SHRD_LIB _LIBEXT
.PHONY: all compile clean clean_local exe_copy exe_strip tests todo help $(TARGET.SUBDIRS)



#####################################################################################
#{{{ TARGETS
#####################################################################################
compile:all
all: $(TARGET.SUBDIRS) $(TARGET.PROGRAMS) $(_LIBS)  $(addsuffix $(_LIBEXT)$(_LOCKEXT), $(addprefix $(_EXEDIR)/lib, $(TARGET.LIBRARIES)) )

$(TARGET.SUBDIRS):
	+$(MAKE) -C $@

ifdef TARGET.PROGRAMS
$(foreach prog,$(TARGET.PROGRAMS),$(eval $(call CMPL_PROGRAM,$(prog))) )
endif

ifdef TARGET.LIBRARIES
$(foreach prog,$(TARGET.LIBRARIES),$(eval $(call CMPL_LIBRARY,$(prog))) )
endif


ifdef TARGET.ALL
$(foreach prog,$(TARGET.ALL),$(eval $(call CMPL_PROGRAM,$(prog))) )
endif

tests runtests:_EXEDIR:=$(realpath $(TOP))/$(_OUTPUT_TUPLE)/tests
runtests:export run:=yes
tests:
ifdef $($(_MODULE).SUBDIRS)
	$(MAKE) -C $($(_MODULE).SUBDIRS)
endif
	$(MAKE) -C 'tests'

runtests: tests
#}}}++++++++++++++++++++++++++++++++++++++++++++++++++++



#####################################################################################
#{{{ Generic rule for .c and .cpp (please do not break lines in this section)
#####################################################################################
$(_OBJDIR)/%.o: %.cpp
	$(MKOBJDIR)
	@printf " %b* CXX    [$(_MODULE)] %25s\t-> $(subst ../,,$@)\n" "$(_TABS)" $<
	$(Q)$(CXX) $(CXXFLAGS) $(CPPFLAGS) $($(_MODULE).CXXFLAGS) \
	$(TARGET_ARCH) $(GEN_DEP) $< -c -o $(subst ../,,$@)
	$(MKDEPS)

$(_OBJDIR)/%.o: %.c
	$(MKOBJDIR)
	@printf " %b* CC     [$(_MODULE)] %25s\t-> $(subst ../,,$@)\n" "$(_TABS)" $<
	$(Q)$(CC)  $(CFLAGS)   $(CPPFLAGS) $($(_MODULE).CFLAGS)   $(TARGET_ARCH) $(GEN_DEP)    $< -c -o $(subst ../,,$@)
	$(MKDEPS)
#}}}++++++++++++++++++++++++++++++++++++++++++++++++++++



#####################################################################################
#{{{ Generic rule for UnitCxx tests
#####################################################################################
tests runtests: export CXXFLAGS+=-I$(TOP)/UnitTest/test_fmwk -I$(TOP)/UnitTest/test_fmwk/mockpp/include/ -I. -I$(TOP)
tests runtests: export LDFLAGS+=-L$(TOP)/UnitTest/test_fmwk/mockpp/lib/ -Wl,-rpath=$(TOP)/UnitTest/test_fmwk/mockpp/lib/
T%_ucxx: T%_ucxx.cpp
	$(MAKE) $(addprefix $(_OBJDIR)/,$(addsuffix .o,$($(@F).FILES)))
	$(CXX) -L$(_LIBDIR) $(LDFLAGS) $(TARGET_ARCH) $($(@F).LDFLAGS) \
		$(addprefix $(_OBJDIR)/,$(addsuffix .o,$($(@F).FILES))) $(_OBJDIR)/$(<:.cpp=.o) \
		$(GLOBAL.LDFLAGS) -o $(_EXEDIR)/$@

T%_ucxx.cpp: T%_ucxx.h
	$(_CXXTESTGEN) --error-printer $< -o $@

#tests/%: tests/%.cpp $(_OBJDIR)/%.o
#}}}++++++++++++++++++++++++++++++++++++++++++++++++++++



#####################################################################################
#{{{ clean
#####################################################################################
clean: clean_tests clean_module
#}}}++++++++++++++++++++++++++++++++++++++++++++++++++++



#####################################################################################
#{{{ clean_local
# Removes all the objects of the modules in current directory.
# make clean_local in subdirs too.
#####################################################################################
clean_local:
	$(Q)rm -rf *.o *.lob *.tmp *.rtl $(_OBJDIR) $(EXE) $(LIBS) $($(_MODULE).DEPS)
ifneq " " "$(LIBS)"
	$(Q)rm -rf $(addprefix $(realpath $(SHRD_DIR))/,$(LIBS))
endif
ifneq "" "$(TARGET.LIBRARIES)"
	@[ "" = "$($(_MODULE).KEEP)" ] && \
	printf " %b* RM [lib$(_MODULE){.so,.a}]\n" "$(_TABS)" && \
	$(Q)rm -f lib$(_MODULE){.so,.a} || exit 0
else
	@[ "" = "$($(_MODULE).KEEP)" ] && \
	printf " %b* RM [$(_MODULE) $(TARGET.PROGRAMS)]\n" "$(_TABS)" && \
	$(Q)rm -f $(_MODULE) $(TARGET.PROGRAMS) || exit 0
endif
	@for i in $(TARGET.SUBDIRS) $($(_MODULE).SUBDIRS) ; do \
	printf " %b* $@ [$$i]\n" "$(_TABS)" ;\
	[ -d $$i ] && $(MAKE) -C $$i clean_local || exit 0; done
#}}}++++++++++++++++++++++++++++++++++++++++++++++++++++



#####################################################################################
#{{{ clean_module_exe
# removes only the executables from _EXEDIR
#####################################################################################
clean_module_exe:
	[ -d "$(_EXEDIR)" ] && find $(_EXEDIR) -type f  ! -regex ".*/objs/.*" -print | xargs /bin/rm -f -r \
	|| exit 0
#}}}++++++++++++++++++++++++++++++++++++++++++++++++++++



#####################################################################################
#{{{ clean_module
# remove the whole _EXEDIR
#####################################################################################
clean_module:
	@printf " %b* RM [$(_EXEDIR)]\n" "$(_TABS)" && \
	$(Q)rm -rf  $(_EXEDIR)
#}}}++++++++++++++++++++++++++++++++++++++++++++++++++++



#####################################################################################
#{{{ clean_tests
#####################################################################################
clean_tests:
	rm -f $(addprefix tests/,$($(_MODULE).TESTS))
	rm -f $(addsuffix .cpp,$(addprefix tests/,$($(_MODULE).TESTS)))
#}}}++++++++++++++++++++++++++++++++++++++++++++++++++++



#####################################################################################
#{{{ exe_copy
#####################################################################################
exe_copy: $(EXE_COPY_HOOK)
	$(MKANDIR)
	@for file in $(TARGET.SUBDIRS) ; do $(MAKE) -C $$file exe_copy ; done
	@set -e ; [ ! -z "$(TARGET.PROGRAMS)" ] && \
		printf " %b* COPY $(TARGET.PROGRAMS) -> $(EXE_DST_DIR)\n" "$(_TABS)" &&\
		cp -f $(addprefix $(_EXEDIR)/,$(TARGET.PROGRAMS)) $(EXE_DST_DIR)	\
		|| exit 0
	@set -e ; [ ! -z "$(TARGET.LIBRARIES)" ] && \
		printf " %b* COPY $(TARGET.LIBRARIES) -> $(AN_LIB_DIR)\n" "$(_TABS)" && \
		cp $(addprefix $(_EXEDIR)/lib,$(addsuffix $(_LIBEXT),$(TARGET.LIBRARIES))) $(AN_LIB_DIR) \
		|| exit 0
#}}}++++++++++++++++++++++++++++++++++++++++++++++++++++



#####################################################################################
#{{{ exe_strip
#####################################################################################
exe_strip:
	@for file in $(TARGET.SUBDIRS) ; do $(MAKE) -C $$file exe_strip ; done
	@[ ! -z "$(TARGET.PROGRAMS)" ]  && \
		printf " %b* STRIP $(TARGET.PROGRAMS)\n" "$(_TABS)" && \
		$(STRIP) --strip-all -R.comment -R.note $(addprefix $(EXE_DST_DIR)/, $(TARGET.PROGRAMS)) \
		|| exit 0
	@[ ! -z "$(TARGET.LIBRARIES)" ] && \
		printf " %b* STRIP $(TARGET.LIBRARIES)\n" '$(_TABS)' && \
		$(STRIP) --strip-unneeded $(addprefix $(AN_LIB_DIR)/lib,$(addsuffix $(_LIBEXT),$(TARGET.LIBRARIES))) \
		|| exit 0
#}}}++++++++++++++++++++++++++++++++++++++++++++++++++++



#####################################################################################
#{{{ todo
#####################################################################################
todo:
	@grep --exclude=system.mk -RnIi "todo\|\#if 0" *
#}}}++++++++++++++++++++++++++++++++++++++++++++++++++++



#####################################################################################
#{{{ HEEEEELp
#####################################################################################
help:
	$(SECHO) "	exclude=PATH"
	$(SECHO) "	nodeps=(yes|no)		default:no"
	$(SECHO) "		Disable header based dependencies."
	$(SECHO) "	optimize=(yes|no)	default:yes"
	$(SECHO) "		Use size optimizations."
	$(SECHO) "	cfast=(yes|no)		default:no"
	$(SECHO) "		Compile fast: nodeps:yes + optimize:no."
	$(SECHO) "	coverage=(yes|no)	default:no"
	$(SECHO) "		Activate code coverage."
	$(SECHO) "	profile=(yes|no)	default:no"
	$(SECHO) "	memcheck=(yes|no)	default:no"
	$(SECHO) "		Activate DEBUG_MODE macro and memory leak hooks"
	$(SECHO) "	ccache=(any|)		default:no"
	$(SECHO) "		Activate ccache support"
	$(SECHO) "	addr2line e=backbone b='HEX Addresses'"
	$(SECHO) "	The 'clean_module' rule removes the \$$(TOP)/\$$(OUTPUT_DIR)/\$$(host)/\$$(module)/ folder."
	$(SECHO) "	Note: one of the variables \$$(host) OR \$$(hw) MUST be specified."
	$(SECHO) "	Ex  : make host=i386 clean_module"
	$(SECHO) "	hw=[$(HW)]"
#}}}++++++++++++++++++++++++++++++++++++++++++++++++++++

addr2line:
	@if [ ! -f "$(_EXEDIR)/$(e)" ]; then \
		echo "No such file:$(_EXEDIR)/$(e)"; exit 0; fi && \
	if [ "`file $(_EXEDIR)/$(e) | grep 'not stripped'`" = "" ] ; then \
	echo "ERROR: $(_EXEDIR)/$(e) is stripped"; exit 0; fi && \
	$(ADDR2LINE) -f -e $(_EXEDIR)/$(e) $(b)

indent:
	find . -iname "*.cpp" -o -iname "*.h" -o -iname "*.c" | xargs astyle --brackets=break  --indent=tab=8


-include $(_OBJDIR)/*.P
# vim:ft=make
