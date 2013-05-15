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



TOP:=../..
PROJ_TOP:=..
DIRS:=core
SCHEMAS:=
HEADERS:=

# override the linkage since we're not copying the libraries onto An
override link=static
include $(TOP)/sys_inc.mk

# librpcmethods.a
rpcmethods.FILES=	\
	UserSet		\
	core/UserImpl	\
	core/md5	\
	core/ConfigImpl	\
	core/MiscImpl	\
	core/FileImpl

rpcmethods.CXXFLAGS=-I$(TOP) -I$(PROJ_TOP) -I$(_EXEDIR)/$(PROJ_TOP) -I$(AUX_LIBS_DIR_INC)/sqlite
rpcmethods.DEPS=$(_EXEDIR)/methods.h $(_EXEDIR)/MethodsHash.h $(_EXEDIR)/MethodsACL.h copy_schemas

#
# librpcmarshal.a
#
rpcmarshal.FILES=	\
	core/UserUnmarshaller	\
	core/ConfigUnmarshaller	\
	core/MiscUnmarshaller	\
	core/FileUnmarshaller

rpcmarshal.CXXFLAGS=$(rpcmethods.CXXFLAGS)
rpcmarshal.DEPS=$(rpcmethods.DEPS)


#
# Release specific files

# ISA
ifeq "$(release)" "isa"
DIRS+=isa100
EXTRA_INCLUDE:=\#include \"isa100/methods.h\"
rpcmethods.FILES+=  isa100/Isa100Impl
rpcmarshal.FILES+=  isa100/Isa100Unmarshaller
rpcmethods.FILES+=  isa100/MhImpl
rpcmarshal.FILES+=  isa100/MhUnmarshaller
rpcmethods.FILES+=  isa100/SmImpl
rpcmarshal.FILES+=  isa100/SmUnmarshaller
rpcmethods.FILES+=  isa100/BbrImpl
rpcmarshal.FILES+=  isa100/BbrUnmarshaller
rpcmethods.FILES+=  isa100/modbus/GwImpl
rpcmarshal.FILES+=  isa100/modbus/GwUnmarshaller
rpcmethods.FILES+=  isa100/modbus/ServerImpl
rpcmarshal.FILES+=  isa100/modbus/ServerUnmarshaller
endif


# WHART use isa until we find out what we need
ifeq "$(release)" "whart"
DIRS+=isa100
EXTRA_INCLUDE:=\#include \"isa100/methods.h\"
rpcmethods.FILES+=  isa100/Isa100Impl
rpcmarshal.FILES+=  isa100/Isa100Unmarshaller
rpcmethods.FILES+=  isa100/MhImpl
rpcmarshal.FILES+=  isa100/MhUnmarshaller
rpcmethods.FILES+=  isa100/SmImpl
rpcmarshal.FILES+=  isa100/SmUnmarshaller
rpcmethods.FILES+=  isa100/BbrImpl
rpcmarshal.FILES+=  isa100/BbrUnmarshaller
rpcmethods.FILES+=  isa100/modbus/GwImpl
rpcmarshal.FILES+=  isa100/modbus/GwUnmarshaller
rpcmethods.FILES+=  isa100/modbus/ServerImpl
rpcmarshal.FILES+=  isa100/modbus/ServerUnmarshaller
endif


ifneq "$(HAS_SQLITE)" ""
rpcmethods.FILES+= core/SqldalImpl
rpcmarshal.FILES+= core/SqldalUnmarshaller ../../WCI/scrsrv/Csv
endif


TARGET.LIBRARIES = rpcmethods rpcmarshal

include $(TOP)/system.mk

.PHONY: $(_EXEDIR)/methods.json
SCHEMAS+=$(addsuffix /methods.json,$(DIRS))
HEADERS+=$(addsuffix /methods.h,$(DIRS))

copy_schemas: $(_EXEDIR)/methods.json
	dest="$(TOP)/an/www/wwwroot/" ; [ ! -d "$$dest" ] && mkdir -p $$dest || echo ; cp $< $$dest

$(_EXEDIR)/methods.json: $(SCHEMAS)
	$(MKOBJDIR)
	set -e ; ./rpcgen.py --hw=$(hw) --include=`pwd` --merge --output=$@ $(SCHEMAS)


$(_EXEDIR)/methods.h: $(_EXEDIR)/methods.json
	$(MKOBJDIR)
	set -e ; for i in $(HEADERS); do \
	mkdir -p $(_EXEDIR)/`dirname $$i` ;\
	./rpcgen.py --hw=$(hw) --include=`pwd` --methods.h --output=$(_EXEDIR)/$$i --namespace=`dirname $$i` `dirname $$i`/methods.json ; done
	./rpcgen.py --hw=$(hw) --include=`pwd` -T --output=$@ --namespaces='$(DIRS)' $^

$(_EXEDIR)/methods.gperf: $(_EXEDIR)/methods.json
	$(MKOBJDIR)
	set -e ; ./rpcgen.py --hw=$(hw) --gperf --output=$@ $^

$(_EXEDIR)/MethodsACL.h: $(_EXEDIR)/methods.json
	$(MKOBJDIR)
	@set -e ; ./rpcgen.py --hw=$(hw) --acl --output=$@ $^

$(_EXEDIR)/MethodsHash.h: $(_EXEDIR)/methods.gperf
	$(MKOBJDIR)
	gperf -CGD -N HasMethod -Z JsonRPCMethods -K MethodName -L C++ -t $^ > $@

