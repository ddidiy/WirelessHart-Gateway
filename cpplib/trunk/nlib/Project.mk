
PROJECT_NAME=nlib
DIST_VERSION=0_3_1
OUT_DIR=out

OUT_DIR := $(OUT_DIR)/$(if $(DEBUG),debug,release)/$(TARGET_OSTYPE)
DIST_DIR=$(OUT_DIR)/dist


CPP_INCLUDES = -I$(CPPLIB_PATH)/trunk/boost_1_36_0
CPP_INCLUDES += -I$(CPPLIB_PATH)/trunk/log4cplus_1_0_2
CPP_INCLUDES += -I$(CPPLIB_PATH)/trunk/loki-0_1_6
CPP_INCLUDES += -I.

CPP_FLAGS = $(if $(DEBUG), -g3 -O0 -gdwarf-2 -ggdb -D_DEBUG, -O3)
CPP_FLAGS += -Wall -fmessage-length=0 -fPIC
CPP_FLAGS += -DHAVE_SSTREAM -DHAVE_GETTIMEOFDAY
CPP_FLAGS += -DBOOST_NO_TYPEID
CPP_FLAGS += -DLOKI_FUNCTOR_IS_NOT_A_SMALLOBJECT -DSQLITE_THREADSAFE=0
#disable CPP_FLAGS += -DCPPLIB_LOG_LOG4CPLUS_ENABLED

LFLAGS = -L$(CPPLIB_PATH)/trunk/boost_1_36_0/lib/$(TARGET_OSTYPE)
LIBS = -lboost-system-$(CC_VERSION)-mt-$(if $(DEBUG),sd,s)-1_36_0

ifeq ($(TOOLCHAIN),gcc-cygwin)
  # cygwin tool chain
  CPP_FLAGS += -D_WIN32_WINNT=0x0501
  LIBS += -lws2_32 -lmswsock

else ifeq ($(TOOLCHAIN),gcc-linux-pc)
  # linux-pc tool chain

else ifeq ($(TOOLCHAIN),gcc-linux-arm)
  # linux-arm tool chain
  CPP_FLAGS += -DSQLITE_DISABLE_LFS
  CPP_FLAGS += -ffunction-sections -fdata-sections -mno-apcs-frame -mno-thumb-interwork

  LFLAGS += -Wl,--no-whole-archive -Wl,--relax -Wl,--gc-sections

else ifeq ($(TOOLCHAIN),m68k-unknown-linux-uclibc)
  # linux-m68k tool chain
  CPP_FLAGS += -DBOOST_SP_DISABLE_THREADS -DBOOST_AC_DISABLE_THREADS 

else ifeq ($(TOOLCHAIN),powerpc-linux-gnu)
  # linux-m68k tool chain
  CPP_FLAGS += -DBOOST_SP_DISABLE_THREADS -DBOOST_AC_DISABLE_THREADS -msoft-float 
endif


#nlib socket
SOCKET_MAIN_SHARED = $(OUT_DIR)/libnlib-socket-$(CC_VERSION)-$(if $(DEBUG),mt-d,mt)-$(DIST_VERSION).$(TARGET_OSTYPE_DLL_EXT)
SOCKET_MAIN_STATIC = $(OUT_DIR)/libnlib-socket-$(CC_VERSION)-$(if $(DEBUG),mt-d,mt)-$(DIST_VERSION).a
SOCKET_CPP_SRCS = $(shell find libs/socket/src -iname '*.cpp') $(shell find libs/timer/src -iname '*.cpp') $(shell find nlib/detail/ -iname '*.cpp') 
SOCKET_CPP_OBJS =$(patsubst %.cpp, %.o, $(addprefix $(OUT_DIR)/, $(SOCKET_CPP_SRCS)))


#nlib sqlitexx
SQLITEXX_MAIN_SHARED = $(OUT_DIR)/libnlib-sqlitexx-$(CC_VERSION)-$(if $(DEBUG),mt-d,mt)-$(DIST_VERSION).$(TARGET_OSTYPE_DLL_EXT)
SQLITEXX_MAIN_STATIC = $(OUT_DIR)/libnlib-sqlitexx-$(CC_VERSION)-$(if $(DEBUG),mt-d,mt)-$(DIST_VERSION).a
SQLITEXX_CPP_SRCS = $(shell find libs/sqlitexx/src -iname '*.cpp') 
SQLITEXX_CPP_OBJS =$(patsubst %.cpp, %.o, $(addprefix $(OUT_DIR)/, $(SQLITEXX_CPP_SRCS)))

#probably nobody needs cpplib sqlite
#ifneq ($(TOOLCHAIN),m68k-unknown-linux-uclibc)
#ifneq ($(TOOLCHAIN),mips-openwrt-linux)
#	SQLITEXX_C_SRCS = $(shell find libs/sqlitexx/src -iname '*.c') 
#endif	
#endif

SQLITEXX_C_OBJS = $(patsubst %.c, %.o, $(addprefix $(OUT_DIR)/, $(SQLITEXX_C_SRCS)))
