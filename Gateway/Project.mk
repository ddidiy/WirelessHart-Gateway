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




#-include ~/default_home.mk
#-include default_project.mk

PROJECT_NAME=WHart_GW
SRC_DIR=src 
OUT_DIR=out
SVNROOT = 'to be defined'

WHART_STACK_DIR?=../Stack
AN_SRC?=../../../isa_work/AccessNode/

VERSION="\"local\""
release?=whart
-include $(AN_SRC)/dist_version.mk  


OUT_DIR := $(OUT_DIR)/$(if $(DEBUG),debug,release)/$(TARGET_OSTYPE)
ifeq "$(link_stack)" "dynamic"
	WHART_LIB:= $(WHART_STACK_DIR)/$(OUT_DIR)/Stack/WHart_Stack.so
else
	#default
	WHART_LIB:= $(WHART_STACK_DIR)/$(OUT_DIR)/Stack/WHart_Stack.a	
endif 

OUT_DIR := $(OUT_DIR)/Gateway
DIST_DIR=$(OUT_DIR)/dist
MAIN_EXE = $(OUT_DIR)/$(PROJECT_NAME).$(TARGET_OSTYPE_EXE_EXT)


AN_SHARED_LIB_DIR=$(AN_SRC)/out/$(host)/Shared/

CPP_INCLUDES = -I. -I$(WHART_STACK_DIR) -I$(WHART_STACK_DIR)/src -I$(AN_SRC)
#-I$(AN_SHARED_INC)
CPP_INCLUDES += -I$(CPPLIB_PATH)/trunk/nlib
CPP_INCLUDES += -I$(CPPLIB_PATH)/trunk/log4cplus_1_0_2
CPP_INCLUDES += -I$(CPPLIB_PATH)/trunk/boost_1_36_0
CPP_INCLUDES += -I$(CPPLIB_PATH)/trunk/loki-0_1_6


LFLAGS += -L$(CPPLIB_PATH)/trunk/boost_1_36_0/lib/$(TARGET_OSTYPE)



#LIBS += -llog4cplus-$(CC_VERSION)-mt-sd-1_0_2
#LIBS += -llog4cplus-$(CC_VERSION)-mt-1_0_2
#LIBS += -lboost-thread-$(CC_VERSION)-mt-s-1_36_0
LIBS += -lboost-system-$(CC_VERSION)-mt-s$(if $(DEBUG),d,)-1_36_0
#LIBS += -lboost-program_options-$(CC_VERSION)-mt-s-1_36_0
LIBS += $(AN_SHARED_LIB_DIR)/libshared$(AN_SHARED_LIB_EXT) $(WHART_LIB)

CPP_FLAGS = $(if $(DEBUG), -g3 -O0 -D_DEBUG, -Os)
CPP_FLAGS += -fmessage-length=0 
#CPP_FLAGS += -fno-rtti #-fno-default-inline -frepo #-fno-implicit-templates -fno-implicit-inline-templates
CPP_FLAGS += -DACCESS_NODE_LOG_ENABLED #-DCPPLIB_LOG_LOG4CPLUS_ENABLED #enable log4cplus
CPP_FLAGS += -DHAVE_STDINT -DENABLE_NLIB_SFORMAT
CPP_FLAGS += -DBOOST_NO_TYPEID -DLOKI_FUNCTOR_IS_NOT_A_SMALLOBJECT 
CPP_FLAGS += -DVERSION=$(VERSION)
#CPP_FLAGS += -E #just preprocessor
#CPP_FLAGS += -S #just assembler code


C_FLAGS = -std=c99 

ifeq ($(TOOLCHAIN),gcc-cygwin)
	
	LIBS += $(CPPLIB_PATH)/trunk/nlib/lib/$(TARGET_OSTYPE)/libnlib-socket-$(CC_VERSION)-mt$(if $(DEBUG),-d,)-0_3_1.a
else
	LFLAGS +=  -L$(CPPLIB_PATH)/trunk/nlib/lib/$(TARGET_OSTYPE)	
	LIBS += -lnlib-socket-$(CC_VERSION)-mt$(if $(DEBUG),-d,)-0_3_1
endif


ifeq ($(TOOLCHAIN),gcc-cygwin)
  # cygwin tool chain
  CPP_FLAGS += -D_WIN32_WINNT=0x0501 #for asio need it
  CPP_FLAGS += -DIS_MACHINE_LITTLE_ENDIAN  -DFS_TREE_REL 
  LIBS += -lws2_32 -lmswsock --enable-auto-import 

else ifeq ($(TOOLCHAIN),gcc-linux-pc)
  # linux-pc tool chain
  CPP_FLAGS += -DIS_MACHINE_LITTLE_ENDIAN -D_X86_ -DHW_I386
  LIBS +=  -lpthread -ldl
  
  #CPP_FLAGS += -ffunction-sections -fdata-sections -mno-apcs-frame -mno-thumb-interwork
  #LFLAGS += -Wl,--no-whole-archive -Wl,--relax -Wl,--gc-sections

else ifeq ($(TOOLCHAIN),gcc-linux-arm)
  # linux-arm tool chain
  CPP_FLAGS += -D_LINUX_UCLIBC_ -DIS_MACHINE_LITTLE_ENDIAN 
  CPP_FLAGS += -fno-inline-functions-called-once -fno-builtin -finline-limit=45 -ffunction-sections -fdata-sections -mno-apcs-frame -mno-thumb-interwork
  LIBS += -ldl -lpthread 
  LFLAGS += -Wl,-Map=arm.map -Wl,--cref

else ifeq ($(TOOLCHAIN),mips-openwrt-linux)
  CPP_FLAGS += -D_LINUX_UCLIBC_ -DIS_MACHINE_BIG_ENDIAN
  CPP_FLAGS += -DBOOST_SP_DISABLE_THREADS -DBOOST_DISABLE_THREADS
  LIBS += -ldl -lpthread

else ifeq ($(TOOLCHAIN),m68k-unknown-linux-uclibc)
#  CPP_FLAGS += -DBOOST_SP_USE_PTHREADS -DBOOST_AC_USE_PTHREADS
  CPP_FLAGS += -DIS_MACHINE_BIG_ENDIAN 
  CPP_FLAGS += -DBOOST_SP_DISABLE_THREADS -DBOOST_DISABLE_THREADS  
	
		_CCFLAGS:=	-Wall \
		-Wredundant-decls \
		-Wshadow \
		-Wsign-compare -ffunction-sections -fdata-sections

C_FLAGS += $(_CCFLAGS)
CPP_FLAGS += $(_CCFLAGS)
LFLAGS += -Wl,--no-whole-archive -Wl,--relax -Wl,--gc-sections
  LIBS += -ldl -lpthread 
endif

ifeq ($(release),mcsapp)
	CPP_FLAGS += -DRELEASE_MCSAPP
else
	CPP_FLAGS += -DRELEASE_WHART
endif


#$$(addprefix $(_OBJDIR)/,$(addsuffix .o, $(subst ../,,$($(1).FILES)) ))

#//TODO [ovidiu.rauca] find a make compliente solution !! 
#CPP_SRCS:= $(foreach dir, $(MY_SRC_DIR), $(wildcard $(dir)/*.cpp))

CPP_SRCS = $(shell find $(SRC_DIR) -iname '*.cpp' | grep -v -e 'TCPSessionIP.cpp' ) $(CPPLIB_PATH)/trunk/nlib/nlib/detail/bytes_print.cpp
#CPP_OBJS = $(patsubst %.cpp, %.o, $(addprefix $(OUT_DIR)/,  $(subst ../,,$(CPP_SRCS)))) 
CPP_OBJS = $(patsubst %.cpp, %.o, $(addprefix $(OUT_DIR)/, $(CPP_SRCS))) 
CPP_DEPS = $(CPP_OBJS:.o=.d)

#C_SRCS = $(shell find $(SRC_DIR) -iname '*.c' | grep -e 'CommonResponseCode.c' -e 'ApplicationCommand.c' \
# -e 'C000' -e 'C020' -e 'C787' -e 'C961' -e 'C962' -e 'C963' -e'C965' -e'C969') 

C_SRCS = $(shell find $(SRC_DIR) -iname '*.c') 
C_OBJS = $(patsubst %.c, %.o, $(addprefix $(OUT_DIR)/, $(C_SRCS)))
C_DEPS = $(C_OBJS:.o=.d)

OTHER_DEPS = Project.mk Makefile

