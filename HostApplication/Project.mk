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




-include ~/default_home.mk
-include default_project.mk

PROJECT_NAME=MonitorHost
SRC_DIR=src 
OUT_DIR=out
SVNROOT = 'to be defined'

WHART_STACK_DIR?=../Stack
AN_SRC?=../AccessNode/
AN_SHARED_LIB_DIR=$(AN_SRC)/out/$(host)/Shared/
AN_AUX_LIBS_BASE?=$(AN_SRC)/../AuxLibs
AN_AUX_LIBS_INC=$(AN_AUX_LIBS_BASE)/include
AN_AUX_LIBS_LIB=$(AN_AUX_LIBS_BASE)/lib/$(host)

VERSION="\"local\""
release?=whart
-include $(AN_SRC)dist_version.mk  

force_sqlite?=0

OUT_DIR := $(OUT_DIR)/$(if $(DEBUG),debug,release)/$(TARGET_OSTYPE)
ifeq "$(link_stack)" "dynamic"
	WHART_LIB:= $(WHART_STACK_DIR)/$(OUT_DIR)/Stack/WHart_Stack.so
else
	#default
	WHART_LIB:= $(WHART_STACK_DIR)/$(OUT_DIR)/Stack/WHart_Stack.a	
endif 

OUT_DIR := $(OUT_DIR)/Host
DIST_DIR=$(OUT_DIR)/dist
MAIN_EXE = $(OUT_DIR)/$(PROJECT_NAME)

ifeq "$(link_shared)" "static"
	SQLITEXX_LIB_LINK=$(AN_SHARED_LIB_DIR)/SqliteUtil/libsqlitexx.a
else
	SQLITEXX_LIB_LINK= -L$(AN_SHARED_LIB_DIR)/SqliteUtil/ -lsqlitexx 
endif

CPP_INCLUDES = -I. -I../Stack -I../Stack/src -I$(AN_SRC) -I../Gateway -I$(AN_AUX_LIBS_INC)
CPP_INCLUDES += -I$(CPPLIB_PATH)/trunk/nlib
CPP_INCLUDES += -I$(CPPLIB_PATH)/trunk/log4cplus_1_0_2
CPP_INCLUDES += -I$(CPPLIB_PATH)/trunk/boost_1_36_0
CPP_INCLUDES += -I$(CPPLIB_PATH)/trunk/loki-0_1_6

LFLAGS =  -L$(CPPLIB_PATH)/trunk/nlib/lib/$(TARGET_OSTYPE)
LFLAGS += -L$(CPPLIB_PATH)/trunk/log4cplus_1_0_2/lib/$(TARGET_OSTYPE)
LFLAGS += -L$(CPPLIB_PATH)/trunk/boost_1_36_0/lib/$(TARGET_OSTYPE)
LFLAGS += -L$(CPPLIB_PATH)/trunk/mysql-5.0.67/lib/$(TARGET_OSTYPE)


LIBS = -lnlib-socket-$(CC_VERSION)-mt-0_3_1
#LIBS += -lnlib-sqlitexx-$(CC_VERSION)-mt-0_3_1
#LIBS += -llog4cplus-$(CC_VERSION)-mt-1_0_2
#LIBS += -lboost-thread-$(CC_VERSION)-mt-s-1_36_0
LIBS += -lboost-system-$(CC_VERSION)-mt-s-1_36_0
#LIBS += -lboost-program_options-$(CC_VERSION)-mt-s-1_36_0
LIBS += $(AN_SHARED_LIB_DIR)/libshared$(AN_SHARED_LIB_EXT) $(WHART_LIB)

CPP_FLAGS = $(if $(DEBUG), -g3 -O0 -D_DEBUG -DENABLE_NLIB_SFORMAT, -Os)
CPP_FLAGS += -fmessage-length=0 
#CPP_FLAGS += -fno-rtti #-fno-default-inline -frepo #-fno-implicit-templates -fno-implicit-inline-templates
#CPP_FLAGS += -fno-rtti
#CPP_FLAGS += -DCPPLIB_LOG_LOG4CPLUS_ENABLED #enable log4cplus
CPP_FLAGS += -DHAVE_STDINT 
CPP_FLAGS += -DBOOST_NO_TYPEID -DLOKI_FUNCTOR_IS_NOT_A_SMALLOBJECT
#CPP_FLAGS += -E #just preprocessor
#CPP_FLAGS += -S #just assembler code
CPP_FLAGS += -DACCESS_NODE_LOG_ENABLED 
CPP_FLAGS += -DVERSION=$(VERSION)

_CCFLAGS:=	-Wall \
		-Wredundant-decls \
		-Wshadow \
		-Wsign-compare -ffunction-sections -fdata-sections

C_FLAGS = -std=c99 $(_CCFLAGS)
CPP_FLAGS += $(_CCFLAGS)
LFLAGS += -Wl,--no-whole-archive -Wl,--relax -Wl,--gc-sections


ifeq ($(TOOLCHAIN),gcc-cygwin)
  
  LFLAGS += -L$(AN_SRC)out/cyg/Shared/

  # cygwin tool chain
  CPP_FLAGS += -D_WIN32_WINNT=0x0501 #for asio need it
  CPP_FLAGS += -D__USE_W32_SOCKETS -D__INSIDE_CYGWIN_NET__
  CPP_FLAGS += -DIS_MACHINE_LITTLE_ENDIAN
  
  
  CPP_FLAGS += -DUSE_MYSQL_DATABASE
  LIBS += -lmysqlclient-$(CC_VERSION)-mt-s-5.0.67
  
  LIBS += -lws2_32 -lmswsock
  

else ifeq ($(TOOLCHAIN),gcc-linux-pc)

  LFLAGS += -L$(AN_SRC)/out/i386/Shared/
  #LFLAGS += -Wl,--no-whole-archive -Wl,--relax -Wl,--gc-sections
  
  # linux-pc tool chain
  CPP_FLAGS += -DIS_MACHINE_LITTLE_ENDIAN -D_X86_
  
  #CPP_FLAGS += -ffunction-sections -fdata-sections -mno-apcs-frame -mno-thumb-interwork
  
  LIBS += -ldl -lpthread
  LIBS += -L/usr/lib
	ifneq "$(force_sqlite)"  "1"
	  CPP_FLAGS += -DUSE_MYSQL_DATABASE
	  LIBS +=  -L$(AN_AUX_LIBS_LIB)/db/my_sql/ -lmysqlclient
	  LIBS += -L$(AN_AUX_LIBS_LIB) -lsqlite3
	  #LIBS += -lmysqlclient-$(CC_VERSION)-mt-s-5.0.67
	else
		CPP_FLAGS += -DUSE_SQLITE_DATABASE -DSQLITE_DISABLE_LFS -DSQLITE_THREADSAFE=0 -DHW_VR900
		LIBS += $(SQLITEXX_LIB_LINK) -L$(AN_AUX_LIBS_LIB) -lsqlite3
	endif  
else ifeq ($(TOOLCHAIN),gcc-linux-arm)
  LFLAGS += -Wl,--no-whole-archive -Wl,--relax -Wl,--gc-sections -Wl,-Map=arm.map -Wl,--cref
  # linux-arm tool chain
  CPP_FLAGS += -D_LINUX_UCLIBC_ -DIS_MACHINE_LITTLE_ENDIAN
  CPP_FLAGS += -fno-inline-functions-called-once -fno-builtin -finline-limit=45 -ffunction-sections -fdata-sections -mno-apcs-frame -mno-thumb-interwork
  LIBS += -ldl -lpthread 
else ifeq ($(TOOLCHAIN),mips-openwrt-linux)
  CPP_FLAGS += -DUSE_SQLITE_DATABASE -DSQLITE_DISABLE_LFS -DSQLITE_THREADSAFE=0 -DMIPS
  CPP_FLAGS += -D_LINUX_UCLIBC_ -DIS_MACHINE_BIG_ENDIAN
  LIBS += $(SQLITEXX_LIB_LINK) -L$(AN_AUX_LIBS_LIB) -lsqlite3
  LIBS += -ldl -lpthread 
else ifeq ($(TOOLCHAIN),m68k-unknown-linux-uclibc)
  LFLAGS += -L$(AN_SRC)/out/m68k-unknown-linux-uclibc/Shared
  CPP_FLAGS += -DIS_MACHINE_BIG_ENDIAN
  CPP_FLAGS += -DBOOST_SP_DISABLE_THREADS -DBOOST_DISABLE_THREADS
  CPP_FLAGS += -DUSE_SQLITE_DATABASE -DSQLITE_DISABLE_LFS -DSQLITE_THREADSAFE=0 -DHW_VR900
  LIBS += $(SQLITEXX_LIB_LINK) -L$(AN_AUX_LIBS_LIB) -lsqlite3
  LIBS += -ldl -lpthread
endif


#//TODO [ovidiu.rauca] find a make compliente solution !! 
#CPP_SRCS:= $(foreach dir, $(MY_SRC_DIR), $(wildcard $(dir)/*.cpp))

CPP_SRCS = $(shell find $(SRC_DIR) -iname '*.cpp') $(CPPLIB_PATH)/trunk/nlib/nlib/detail/bytes_print.cpp
CPP_OBJS = $(patsubst %.cpp, %.o, $(addprefix $(OUT_DIR)/, $(CPP_SRCS))) 
CPP_DEPS = $(CPP_OBJS:.o=.d)

C_SRCS = $(shell find $(SRC_DIR) -iname '*.c' | grep -v -e  sqlite3.c) 

C_OBJS = $(patsubst %.c, %.o, $(addprefix $(OUT_DIR)/, $(C_SRCS)))
C_DEPS = $(C_OBJS:.o=.d)

OTHER_DEPS = Project.mk Makefile

