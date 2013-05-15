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
#DO NOT INCLUDE from main Makefile

#for explanations about default.mk look in main Makefile
#default.mk is search in the dir of Makefile which include common.mk -> Shared, NodeConnection, etc
-include $(TOP)/default.mk

#####################################################################################
#{{{HW Specific assign default value to host variable
#####################################################################################
ifeq "$(hw)" "vr900"
  host?=m68k-unknown-linux-uclibc
endif


#vars which could appear on cmd line
export host?=m68k-unknown-linux-uclibc
export link?=dynamic

#an - fs tree for AN (ER)
#rel - relative fs tree on i386 or cyg.
export fs_tree?=an

#export cc?=2.95
export cc?=3.4
export static_libc?=0

TOP?=..
export AN_DIR?=$(TOP)/an/
export AN_LIB_DIR?=$(AN_DIR)lib/
export SHRD?=$(TOP)/Shared/
_OUTPUT_DIR:=out
export OUT_SHRD_DIR?=$(abspath $(TOP)/$(_OUTPUT_DIR)/$(host)/Shared)
#override this in each section, if necessary
LD?=ld
STRIP?=strip
AR?=ar
RANLIB?=ranlib

CXXFLAGS?=
CFLAGS?=
LDFLAGS?=


VERSION?="\"local\""
# common to all compilers/projects
SHRD_CFLAGS =  -g -Wall -DVERSION=$(VERSION)
SHRD_LDFLAGS = -g


ifeq "$(board_version)" "4.1"
	SHRD_CFLAGS+= -DBOARD_VERSION_4_1
endif


ifeq "$(dist)" "mesh"
	SHRD_CFLAGS+= -DMESH
endif

ifdef DEBUG
	SHRD_CFLAGS+=-DDEBUG=$(DEBUG)
endif

ifeq "$(fs_tree)" "rel"
	SHRD_CFLAGS+= -DFS_TREE_REL
endif

ifdef DUMPRTL
	SHRD_CFLAGS+= -dr
endif

ifeq "$(static_libc)" "1"
	SHRD_LDFLAGS+= -static
endif

ifeq "$(link)" "static"
	SHRD_LIB_LINK=$(OUT_SHRD_DIR)/libshared.a
	SHRD_LIB=$(OUT_SHRD_DIR)/libshared.a
endif

ifeq "$(link)" "dynamic"
	SHRD_LIB_LINK=$(OUT_SHRD_DIR)/libshared.so
	SHRD_LIB=$(OUT_SHRD_DIR)/libshared.so
endif


ifeq "$(host)" "m68k-unknown-linux-uclibc"
	override PATH:=/opt/nivis/$(host)/bin:$(PATH)
	CC=$(host)-gcc
	CXX=$(host)-g++
	AR=$(host)-ar
	LD=$(host)-ld
	STRIP=$(host)-strip
	RANLIB=$(host)-ranlib
endif

ifeq "$(host)" "i386"
	CXX=g++
	CC=gcc

	ZLIB= -lz -L/usr/lib/
	UDNSLIB = $(TOP)/Shared/lib/i386/libudns.a
	SHRD_CFLAGS+= -DI386
	LIBDIR=$(TOP)/Shared/lib/i386/
	#TARGET_ARCH=-DI386
endif


ifndef _OTHER_INCLUDED_
CXXFLAGS+=$(SHRD_CFLAGS) -I/usr/src/linux/include #-I/opt/m68k-uclinux/m68k-uclinux/include/c++/4.2.3
CFLAGS+=$(SHRD_CFLAGS)  -I/usr/src/linux/include #-I/opt/m68k-uclinux/m68k-uclinux/include/c++/4.2.3
LDFLAGS+=$(SHRD_LDFLAGS)
endif

export CPP='$(CC) -E'
export CXX CC LD STRIP AR RANLIB SHRD_LIB_LINK SHRD_LIB CXXFLAGS CFLAGS LDFLAGS ZLIB UDNSLIB


LAST_B := $(shell [ -f last_build ] && cat last_build)
TMP := $(shell echo $(host)$(link) > last_build )

ifneq "$(host)$(link)" "$(LAST_B)"
    TMP := $(shell make clean_local host=$(host) link=$(link))
endif

########################################################
# dep
########################################################
dep:
	$(CXX) -MM *.cpp > dep.mk

########################################################
# clean_local
########################################################
clean_local:
	rm -f *.o *.lob *.tmp $(EXE) $(AUX_FILES) $(LIBS) *.rtl

# vim:ft=make
