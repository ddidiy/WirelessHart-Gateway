
PROJECT_NAME=log4cplus
SRC_DIR=src
OUT_DIR=out
DIST_VERSION=1_0_2
_DIST_FLAGS=$(if $(DEBUG),mt-sd,mt-s)
CVSROOT = ':pserver:nicu.dascalu@cljsrv01:/ISA100'

OUT_DIR := $(OUT_DIR)/$(if $(DEBUG),debug,release)/$(TARGET_OSTYPE)
#Arhive will be Single-THREADED
MAIN_AR = $(OUT_DIR)/lib$(PROJECT_NAME)-$(CC_VERSION)-$(if $(DEBUG),st-sd,st-s)-$(DIST_VERSION).a
#MAIN_AR = $(OUT_DIR)/lib$(PROJECT_NAME)-$(CC_VERSION)-$(if $(DEBUG),mt-sd,mt-s)-$(DIST_VERSION).a
MAIN_SHARED = $(OUT_DIR)/lib$(PROJECT_NAME)-$(CC_VERSION)-$(if $(DEBUG),mt-d,mt)-$(DIST_VERSION).$(TARGET_OSTYPE_DLL_EXT)


CPP_INCLUDES = -I.

CPP_FLAGS = $(if $(DEBUG), -g3 -O0 -gdwarf-2 -ggdb -D_DEBUG, -Os)
CPP_FLAGS += -Wall -fmessage-length=0  -fPIC -fno-rtti 
#CPP_FLAGS += -Wall -fmessage-length=0  -fno-rtti # <--- TODO: Catalin - Verify effectiveness  
CPP_FLAGS += -DHAVE_SSTREAM -DHAVE_GETTIMEOFDAY -DLOG4CPLUS_SINGLE_THREADED
#log is disabled CPP_FLAGS += -DCPPLIB_LOG_LOG4CPLUS_ENABLED

LIBS  = -l$(PROJECT_NAME)-$(CC_VERSION)-$(_DIST_FLAGS)-$(DIST_VERSION)
#LIBS += -ldl
LFLAGS = -L$(OUT_DIR)

ifeq ($(TOOLCHAIN),gcc-cygwin)
  # cygwin tool chain
  CPP_FLAGS += -D_WIN32_WINNT=0x0501

else ifeq ($(TOOLCHAIN),gcc-linux-pc)
  # linux-pc tool chain 

else ifeq ($(TOOLCHAIN),gcc-linux-arm)
  # linux-arm tool chain

else ifeq ($(TOOLCHAIN),mips-openwrt-linux)
  # mips tool chain

else ifeq ($(TOOLCHAIN),gcc-linux-m68k)
  # linux-coldfire m68k tool chain

else ifeq ($(TOOLCHAIN),m68k-unknown-linux-uclibc)
  # m68k-unknown-linux-uclibc

else ifeq ($(TOOLCHAIN),powerpc-linux-gnu)
  # powerpc-linux-gnu 3eti

else
  abort "Undefined TOOLCHAIN=$(TOOLCHAIN) !"
endif


#//TODO [ovidiu.rauca] find a make compliant solution !! CPP_SRCS:= $(foreach dir, $(MY_SRC_DIR), $(wildcard $(dir)/*.cpp))
CPP_SRCS = $(shell find $(SRC_DIR) -iname '*.cxx' | grep -i -v -e 'nteventlogappender.cxx' -e 'socket-win32.cxx') 
CPP_OBJS:=$(patsubst %.cxx, %.o, $(addprefix $(OUT_DIR)/, $(CPP_SRCS))) 
CPP_DEPS=$(CPP_OBJS:.o=.d)
