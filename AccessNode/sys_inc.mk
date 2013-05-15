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




#for explanations about default.mk look in main Makefile
-include $(TOP)/default.mk
-include ~/default_home_an.mk
##################################




#####################################################################################
# HW Specific assign default value to host variable
####################################################################################
export RELEASES:= isa whart
export HW:= vr900 pc i386

dist?=mesh


ifeq "$(hw)" "vr900"
	host?=m68k-unknown-linux-uclibc
	TARGET_OSTYPE?=linux-m68k
endif

ifeq "$(hw)" "pc"
	override  hw:=i386
	TARGET_OSTYPE?=linux-pc
endif

ifeq "$(hw)" "i386"
	host?=i386
	link?=static
	fs_tree?=rel
	TARGET_OSTYPE?=linux-pc
endif

#variables which could appear on cmd line
#an - fs tree for AN (ER)
#rel - relative fs tree on i386
fs_tree  ?=an
static_libc ?=0

ifndef link
link     ?=dynamic
#ifneq ($(findstring exe_strip,$(MAKECMDGOALS)),)
#else ifneq ($(findstring exe_copy,$(MAKECMDGOALS)),)
#else ifneq ($(findstring clean_local,$(MAKECMDGOALS)),)
#else
#$(warning *** WARN: $$(link) undefined. Defaulting to $(link).)
#endif
endif

TOP:=$(realpath $(TOP))

OUTPUT_DIR			?=$(abspath $(TOP)/out)
AN_DIR      		?=$(abspath $(TOP)/an)
AN_LIB_DIR  		?=$(abspath $(AN_DIR)/lib)
SHRD_DIR    		?=$(abspath $(TOP)/Shared)
RMP_SHRD_DIR		?=$(abspath $(TOP)/NivisRMP/RMP_Shared)
OUT_SHRD_DIR		?=$(OUTPUT_DIR)/$(host)/Shared
OUT_RMP_SHRD_DIR	?=$(OUTPUT_DIR)/$(host)/NivisRMP/RMP_Shared
AN_CONFIG_TOP_DIR	?=$(abspath $(TOP)/config/)
AN_CONFIG_RELEASE_DIR  	?=$(AN_CONFIG_TOP_DIR)/FW_$(dist)_HW_$(hw)/release_$(release)

EXE_DST_DIR :=$(abspath $(AN_DIR))
ifdef WWW
EXE_DST_DIR :=$(abspath $(AN_DIR)/www/wwwroot)
endif

AUX_LIBS_DIR_BASE		?=$(TOP)/../AuxLibs
AUX_LIBS_DIR_INC		 =$(AUX_LIBS_DIR_BASE)/include
AUX_LIBS_DIR_LIB		 =$(AUX_LIBS_DIR_BASE)/lib/$(host)

LIBISA_DIR_SRC ?=$(abspath $(TOP)/IsaGw/ISA100)
LIBISA_DIR_OUT ?=$(OUTPUT_DIR)/$(host)/IsaGw/ISA100
LIBGSAP_DIR_SRC ?=$(abspath $(TOP)/ProtocolTranslators/LibGSAP)
LIBGSAP_DIR_OUT ?=$(OUTPUT_DIR)/$(host)/ProtocolTranslators/LibGSAP

CPPLIB_DIR			?=$(TOP)/../cpplib/trunk

BOOST_INCLUDE_PATH	?=$(CPPLIB_DIR)/boost_1_36_0/
BOOST_LIB_PATH		?=$(CPPLIB_DIR)/boost_1_36_0/lib

BOOST_CXXFLAGS= -I$(BOOST_INCLUDE_PATH)

ifeq "$(hw)" "vr900"
	BOOST_CXXFLAGS += -DBOOST_SP_DISABLE_THREADS -DBOOST_DISABLE_THREADS
endif


#variable "link" MUST NOT change after this point
ifeq "$(link)" "static"
	_LIBEXT :=.a
	SHRD_LIB_LINK=$(OUT_SHRD_DIR)/libshared$(_LIBEXT) -lpthread
	RMP_SHRD_LIB_LINK=$(OUT_RMP_SHRD_DIR)/librmp_shared$(_LIBEXT)
	LIBISA_LINK=$(LIBISA_DIR_OUT)/libisa100$(_LIBEXT)
	AXTLS_LIB_LINK=$(AUX_LIBS_DIR_LIB)/libaxtls$(_LIBEXT)
	LIBGSAP_LINK=$(LIBGSAP_DIR_OUT)/libgsap$(_LIBEXT)
	PTHREAD_LIB_LINK=libpthread$(_LIBEXT)
	LIBAGENTPP_LINK=$(AUX_LIBS_DIR_LIB)/libagent++$(_LIBEXT) $(AUX_LIBS_DIR_LIB)/libsnmp++$(_LIBEXT)  $(AUX_LIBS_DIR_LIB)/libdes$(_LIBEXT)
	LIB_NETFILTER_Q_LINK=$(AUX_LIBS_DIR_LIB)/libnetfilter_queue$(_LIBEXT) $(AUX_LIBS_DIR_LIB)/libnfnetlink$(_LIBEXT)
    LIB_PYTHON_LINK=$(AUX_LIBS_DIR_LIB)/libpython$(_LIBEXT) -pthread -lm -ldl -lutil -export-dynamic
else
	_LIBEXT :=.so
	SHRD_LIB_LINK=-L$(OUT_SHRD_DIR) -lshared -lpthread
	RMP_SHRD_LIB_LINK=-L$(OUT_RMP_SHRD_DIR) -lrmp_shared
	LIBISA_LINK=-L$(LIBISA_DIR_OUT) -lisa100
	AXTLS_LIB_LINK=-L$(AUX_LIBS_DIR_LIB) -laxtls
	LIBGSAP_LINK=-L$(LIBGSAP_DIR_OUT) -lgsap
	PTHREAD_LIB_LINK=-lpthread
	LIBAGENTPP_LINK=-L$(AUX_LIBS_DIR_LIB) -lagent++ -lsnmp++ $(AUX_LIBS_DIR_LIB)/libdes.a
	LIB_NETFILTER_Q_LINK=-L$(AUX_LIBS_DIR_LIB) -lnetfilter_queue -lnfnetlink
    LIB_PYTHON_LINK=-L$(AUX_LIBS_DIR_LIB)/libpython$(_LIBEXT) -lpthread -ldl -lutil -lm -xlinker -export-dynamic
endif
SHRD_LIB=$(SHRD_LIB_LINK)

HASSQLITE:=vr900_isa;vr900_whart;
HAS_SQLITE:=$(findstring $(hw)_$(release);,$(HASSQLITE))


define GLOB
$(shell find $(1) -iname $(2) | sed -e 's/\.cpp$$\|\.c$$//g;')
endef

PTHREAD_UTIL_OBJS=$(SHRD_DIR)/PThreadUtil/PThreadWrapper $(SHRD_DIR)/PThreadUtil/MutexWrapper $(SHRD_DIR)/PThreadUtil/SemaphoreWrapper


#GLOBAL_DEP:=Makefile system.mk sys_inc.mk

export host hw dist fs_tree static_libc
export AN_DIR AN_LIB_DIR SHRD_DIR OUT_SHRD_DIR EXE_DST_DIR OUTPUT_DIR _EXEDIR _OBJDIR LIBISA_DIR_SRC LIBISA_DIR_OUT

export _LIBEXT SHRD_LIB_LINK SHRD_LIB LIBISA_LINK BOOST_INCLUDE_PATH BOOST_LIB_PATH CPPLIB_DIR GLOBAL_DEP LIBGSAP_DIR_OUT LIBGSAP_LINK


#####################################################################################
#{{{Host Specific PATH and tool(compiler,linker,strip) names.
#####################################################################################
ifeq "$(host)" "m68k-unknown-linux-uclibc"
	override PATH:=/opt/nivis/$(host)/bin:$(PATH)
	CC_DIR:=/opt/nivis/$(host)/bin/
	CC:=$(host)-gcc
	CXX:=$(host)-g++
	AR:=$(host)-ar
	LD:=$(host)-ld
	STRIP:=$(host)-strip
	RANLIB:=$(host)-ranlib
	ADDR2LINE:=$(host)-addr2line
endif
ifeq "$(host)" "i386"
	ZLIB:= -lz -L/usr/lib/
	CC:=gcc
	#CXX:=c++
endif

#}}}++++++++++++++++++++++++++++++++++++++++++++++++++++

export PATH
export CC_VERSION=$(shell $(CC_DIR)$(CC) --version | grep -e '^[0-9a-zA-Z_-]*gcc (.*)' | sed -e 's/[0-9a-zA-Z_-]*gcc (.*) \([0-9]\)\.\([0-9]\).*/gcc\1\2/')

#####################################################################################
# {{{ ccache(compiler cache) support for faster compile.
#####################################################################################
ifneq "$(ccache)" ""
# If this will be adopted, then common cache directory will save disk space.
# groupadd -g 1234 ccache
# mkdir common_directory
# chgrp ccache common_directory
# chmod g+s common_directory
# CCACHE_DIR=common_directory
# CCACHE_UMASK=002
# CCACHE_LOGFILE=logfile
CC:=ccache $(CC)
CXX:=ccache $(CXX)
endif
#}}}++++++++++++++++++++++++++++++++++++++++++++++++++++
