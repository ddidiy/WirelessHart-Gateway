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

PROJECT_NAME=WHart_NM
WHART_STACK_DIR?=../WHart_Stack

AN_SRC?=../AccessNode/
AN_SHARED_LIB_DIR=$(AN_SRC)/out/$(host)/Shared/

SRC_DIR=src 
NET_ENGINE_DIR=$(NET_ENGINE_PATH)
OUT_DIR=out
TEST_DIR=test
SVNROOT = 'to be defined'


OUT_DIR := $(OUT_DIR)/$(if $(DEBUG),debug,release)/$(TARGET_OSTYPE)
#OUT_DIR := $(OUT_DIR)/release/$(TARGET_OSTYPE)


OUT_DIR := $(OUT_DIR)/NetworkManager
DIST_TAR_VERSION=$(shell grep '\#define[ ]*SYSTEM_MANAGER_VERSION' $(NET_ENGINE_DIR)/src/RunLib/Version.h | cut -f 2 -d \")
MAIN_EXE = $(OUT_DIR)/$(PROJECT_NAME).$(TARGET_OSTYPE_EXE_EXT)

TEST_MAIN_EXE = $(OUT_DIR)/Test_$(PROJECT_NAME).$(TARGET_OSTYPE_EXE_EXT)

log4cplus_version=1_0_2
ifeq ($(TOOLCHAIN),mips-openwrt-linux)
    log4cplus_version=1_0_4
endif

CPP_INCLUDES = -I$(WHART_STACK_DIR) -I$(WHART_STACK_DIR)/src -Isrc -I$(NET_ENGINE_DIR)/src
CPP_INCLUDES += -I$(CPPLIB_PATH)/trunk/nlib

CPP_INCLUDES += -I$(CPPLIB_PATH)/trunk/boost_1_36_0
CPP_INCLUDES += -I$(CPPLIB_PATH)/trunk/loki-0_1_6
CPP_INCLUDES += -I$(AN_SRC)

CPP_FLAGS = $(if $(DEBUG), -g3 -O0 -ggdb -gdwarf-2 -D_DEBUG, -Os -fno-inline)
#CPP_FLAGS = -g3 -O0 -D_DEBUG
CPP_FLAGS += -Wall -fmessage-length=0 -pipe 

   
CPP_FLAGS += -DHAVE_STDINT
CPP_FLAGS += -DBOOST_NO_TYPEID -DLOKI_FUNCTOR_IS_NOT_A_SMALLOBJECT
CPP_FLAGS += -DENABLE_NLIB_SFORMAT
#CPP_FLAGS += -E #just preprocessor
#CPP_FLAGS += -S #just assembler code 
CPP_FLAGS += $(if $(JOIN_REASON), -DJOIN_REASON,)

LFLAGS += -L$(CPPLIB_PATH)/trunk/boost_1_36_0/lib/$(TARGET_OSTYPE)
LFLAGS += -L$(NET_ENGINE_DIR)/out/$(if $(DEBUG),debug,release)/$(TARGET_OSTYPE)/NetworkEngine

_LIB_FLAGS=$(if $(DEBUG),mt-sd,mt-s)

LIBS += -lboost-test-$(CC_VERSION)-mt-s$(if $(DEBUG),d,)-1_36_0
LIBS += -lboost-system-$(CC_VERSION)-mt-s$(if $(DEBUG),d,)-1_36_0

LIBS += $(AN_SHARED_LIB_DIR)/libshared$(AN_SHARED_LIB_EXT)

ifeq ("$(use_log_an)","yes")
	CPP_FLAGS += -DACCESS_NODE_LOG_ENABLED
else
	#default
	CPP_FLAGS += -DCPPLIB_LOG_LOG4CPLUS_ENABLED #enable log4cplus

	CPP_INCLUDES += -I$(CPPLIB_PATH)/trunk/log4cplus_$(log4cplus_version)
	LFLAGS += -L$(CPPLIB_PATH)/trunk/log4cplus_$(log4cplus_version)/lib/$(TARGET_OSTYPE)
	LIBS += -llog4cplus-$(CC_VERSION)-st-s$(if $(DEBUG),d,)-$(log4cplus_version)	
endif

 
ifeq ("$(link_nlib-socket)","static")
	LIBS += $(CPPLIB_PATH)/trunk/nlib/lib/$(TARGET_OSTYPE)/libnlib-socket-$(CC_VERSION)-mt$(if $(DEBUG),-d,)-0_3_1.a
else
	LFLAGS +=  -L$(CPPLIB_PATH)/trunk/nlib/lib/$(TARGET_OSTYPE)	
	LIBS += -lnlib-socket-$(CC_VERSION)-mt$(if $(DEBUG),-d,)-0_3_1
endif


C_FLAGS = -std=c99

ifeq ($(TOOLCHAIN),gcc-cygwin)
  # cygwin tool chain
  CPP_FLAGS += -D_WIN32_WINNT=0x0501 # need it for asio
  CPP_FLAGS += -DIS_MACHINE_LITTLE_ENDIAN
  LIBS += -lws2_32 -lmswsock

else ifeq ($(TOOLCHAIN),gcc-linux-pc)
  # linux-pc tool chain
  CPP_FLAGS += -DIS_MACHINE_LITTLE_ENDIAN -D_X86_ -DHW_I386
  LIBS += -ldl -lpthread

else ifeq ($(TOOLCHAIN),gcc-linux-arm)
  # linux-arm tool chain
  CPP_FLAGS += -D_LINUX_UCLIBC_ -DIS_MACHINE_LITTLE_ENDIAN
  LIBS += -ldl -lpthread 
else ifeq ($(TOOLCHAIN),mips-openwrt-linux)
  CPP_FLAGS += -D_LINUX_UCLIBC_ -DIS_MACHINE_BIG_ENDIAN

  LIBS += -ldl -lpthread
else ifeq ($(TOOLCHAIN),m68k-unknown-linux-uclibc)
#  CPP_FLAGS += -DBOOST_SP_USE_PTHREADS -DBOOST_AC_USE_PTHREADS
  CPP_FLAGS += -D_LINUX_UCLIBC_ -DIS_MACHINE_BIG_ENDIAN -DBOOST_ASIO_DISABLE_EPOLL
  CPP_FLAGS += -DBOOST_SP_DISABLE_THREADS -DBOOST_DISABLE_THREADS 
  LIBS += -ldl -lpthread 
else ifeq ($(TOOLCHAIN),arm-iwmmx-linux-gnueabi)
#  CPP_FLAGS += -DBOOST_SP_USE_PTHREADS -DBOOST_AC_USE_PTHREADS
  CPP_FLAGS += -D_LINUX_UCLIBC_ -DIS_MACHINE_LITTLE_ENDIAN -DBOOST_ASIO_DISABLE_EPOLL
  CPP_FLAGS += -DBOOST_SP_DISABLE_THREADS -DBOOST_DISABLE_THREADS 
  LIBS += -ldl -lpthread 
else
  abort "Undefined TOOLCHAIN=$(TOOLCHAIN) !"
endif

ifeq ($(release),mcsapp)
	CPP_FLAGS += -DRELEASE_MCSAPP
else
	CPP_FLAGS += -DRELEASE_WHART
endif

-include ./User.mk
 

CPP_SRCS = $(shell find $(SRC_DIR) -iname '*.cpp' | grep -v 'test/' | grep -v 'WHartCmdWrapper') $(CPPLIB_PATH)/trunk/nlib/nlib/detail/bytes_print.cpp
CPP_OBJS = $(patsubst %.cpp, %.o, $(addprefix $(OUT_DIR)/, $(CPP_SRCS))) 
CPP_DEPS = $(CPP_OBJS:.o=.d)

ifdef USER.tests.SRC
# add the main class for tests and the possible utilitary classes
TEST_SRCS = $(USER.tests.SRC) $(TEST_DIR)/TestMain.cpp
else
TEST_SRCS = $(shell find $(TEST_DIR) -iname '*.cpp')
endif 	
		
TEST_OBJS = $(patsubst %.cpp, %.o, $(addprefix $(OUT_DIR)/, $(TEST_SRCS))) 
TEST_DEPS = $(TEST_OBJS:.o=.d) 

ifdef USER.tests.run_params
TEST_RUN_PARAMS = $(USER.tests.run_params)
else
#TEST_RUN_PARAMS = --log_level=test_suite --catch_system_errors=yes --run_test=hart_manager*
TEST_RUN_PARAMS = --log_level=error --catch_system_errors=yes --report_level=detailed --build_info=yes
endif
	 
TEST_OBJS = $(patsubst %.cpp, %.o, $(addprefix $(OUT_DIR)/, $(TEST_SRCS))) 
TEST_DEPS = $(TEST_OBJS:.o=.d) 

C_SRCS = $(shell find $(SRC_DIR) -iname '*.c' | grep -v "DeviceSpecific") 
C_OBJS = $(patsubst %.c, %.o, $(addprefix $(OUT_DIR)/, $(C_SRCS)))
C_DEPS = $(C_OBJS:.o=.d)

  
OTHER_DEPS = Project.mk Makefile

