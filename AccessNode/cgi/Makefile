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



TOP:=..
PROJ_TOP:=.

#fs_tree=rel

export WWW:=1
include $(TOP)/sys_inc.mk

ifneq "$(HAS_SQLITE)" ""
SQLITE_LDFLAGS =-lsqlite3 -lpthread
SQLITE_CXXFLAGS=-I$(AUX_LIBS_DIR_INC)/sqlite
endif

#warning XXXXXXXXXXXXXXXXXXX"$(HAS_SQLITE)"

# USE -Wl,-rpath=/access_node/firmware/lib since LD_LIBRARY_PATH is not inherited
FLAGS = -Wl,-rpath=/access_node/firmware/lib $(SHRD_LIB_LINK) -ljson $(SQLITE_LDFLAGS)

rpc.cgi.FILES   = rpc JsonRPC Session Cookie
rpc.cgi.CXXFLAGS= -I$(_EXEDIR)/$(PROJ_TOP) -I.
rpc.cgi.LDFLAGS = $(OUTPUT_DIR)/$(host)/cgi/methods/librpcmarshal.a   $(OUTPUT_DIR)/$(host)/cgi/methods/librpcmethods.a $(FLAGS) -ldl
rpc.cgi.SUBDIRS = $(SHRD_DIR) methods
rpc.cgi.DEST    = rpc.cgi
rpc.cgi.TESTS   = one_test.cpp


scgi_svc.FILES   = scgi_svc JsonRPC Session Cookie Cgi
scgi_svc.CXXFLAGS= -DADDRESS="\"/scgi_svc\"" -I$(_EXEDIR)/$(PROJ_TOP) -I. -I$(TOP)
scgi_svc.LDFLAGS = $(rpc.cgi.LDFLAGS) -lpthread
scgi_svc.SUBDIRS = $(SHRD_DIR) methods
scgi_svc.DEST    = scgi_svc


upload.cgi.FILES   = upload Cgi JsonRPC Session Cookie
upload.cgi.CXXFLAGS= -I$(_EXEDIR)/$(PROJ_TOP) -I.
upload.cgi.LDFLAGS = $(rpc.cgi.LDFLAGS)
upload.cgi.SUBDIRS = $(SHRD_DIR) methods
upload.cgi.DEST    = upload.cgi


create_tgz.cgi.FILES = create_tgz
create_tgz.cgi.SUBDIRS= $(SHRD_DIR)
create_tgz.cgi.DEST  = create_tgz.cgi


TARGET.PROGRAMS = create_tgz.cgi scgi_svc
EXE = create_tgz.cgi scgi_svc


ifeq "$(hw)" "i386"
TARGET.PROGRAMS = create_tgz.cgi rpc.cgi
EXE = create_tgz.cgi rpc.cgi
endif

include $(TOP)/system.mk
