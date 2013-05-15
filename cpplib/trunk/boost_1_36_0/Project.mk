
OUT_DIR=out
DIST_VERSION=1_36_0

OUT_DIR := $(OUT_DIR)/$(if $(DEBUG),debug,release)/$(TARGET_OSTYPE)
_DIST_FLAGS=$(if $(DEBUG),mt-sd,mt-s)


BOOST_SUFFIX=$(CC_VERSION)-$(_DIST_FLAGS)-$(DIST_VERSION).a
DYN_SHARED =$(CC_VERSION)-$(if $(DEBUG),mt-d,mt)-$(DIST_VERSION).$(TARGET_OSTYPE_DLL_EXT)

CPP_INCLUDES = -I.

CPP_FLAGS = $(if $(DEBUG), -g3 -O0 -gdwarf-2 -ggdb -D_DEBUG, -O2)
CPP_FLAGS += -Wall -fmessage-length=0 
CPP_FLAGS += -DHAVE_SSTREAM -DHAVE_GETTIMEOFDAY -DBOOST_TEST_NO_MAIN



LFLAGS = 
LIBS =

ifeq ($(TOOLCHAIN),gcc-cygwin)
  # cygwin tool chain
  CPP_FLAGS += -D_WIN32_WINNT=0x0501

else ifeq ($(TOOLCHAIN),gcc-linux-pc)
  # linux-pc tool chain

else ifeq ($(TOOLCHAIN),gcc-linux-arm)
  # linux-arm tool chain

else ifeq ($(TOOLCHAIN),gcc-linux-m68k)  
 # linux-m68k ColdFire tool chain
  CPP_FLAGS += -DBOOST_SP_USE_PTHREADS -DBOOST_AC_USE_PTHREADS

else ifeq ($(TOOLCHAIN),mips-openwrt-linux)
  CPP_FLAGS += -fPIC
else ifeq ($(TOOLCHAIN),m68k-unknown-linux-uclibc)
  CPP_FLAGS += -DBOOST_SP_USE_PTHREADS -DBOOST_AC_USE_PTHREADS
else ifeq ($(TOOLCHAIN),powerpc-linux-gnu)
  CPP_FLAGS += -DBOOST_SP_USE_PTHREADS -DBOOST_AC_USE_PTHREADS
else ifeq ($(TOOLCHAIN),arm-iwmmx-linux-gnueabi)
  CPP_FLAGS += -DBOOST_SP_USE_PTHREADS -DBOOST_AC_USE_PTHREADS

endif

#boost thread
THREAD_MAIN = $(OUT_DIR)/libboost-thread-$(BOOST_SUFFIX)
THREAD_SHARED = $(OUT_DIR)/libboost-thread-$(DYN_SHARED)
THREAD_CPP_SRCS = $(shell find libs/thread/src -iname '*.cpp' | grep -i -v 'win32') 
THREAD_CPP_OBJS:=$(patsubst %.cpp, %.o, $(addprefix $(OUT_DIR)/, $(THREAD_CPP_SRCS)))

#boost signal
SIGNALS_MAIN = $(OUT_DIR)/libboost-signals-$(BOOST_SUFFIX)
SIGNALS_SHARED = $(OUT_DIR)/libboost-signals-$(DYN_SHARED)
SIGNALS_CPP_SRCS = $(shell find libs/signals/src -iname '*.cpp') 
SIGNALS_CPP_OBJS:=$(patsubst %.cpp, %.o, $(addprefix $(OUT_DIR)/, $(SIGNALS_CPP_SRCS)))

#boost graph
#GRAPH_MAIN = $(OUT_DIR)/libboost-graph-$(BOOST_SUFFIX)
#GRAPH_CPP_SRCS = $(shell find libs/graph/src -iname '*.cpp') 
#GRAPH_CPP_OBJS:=$(patsubst %.cpp, %.o, $(addprefix $(OUT_DIR)/, $(GRAPH_CPP_SRCS)))

#boost system
SYSTEM_MAIN = $(OUT_DIR)/libboost-system-$(BOOST_SUFFIX)
SYSTEM_SHARED = $(OUT_DIR)/libboost-system-$(DYN_SHARED)
SYSTEM_CPP_SRCS = $(shell find libs/system/src -iname '*.cpp') 
SYSTEM_CPP_OBJS:=$(patsubst %.cpp, %.o, $(addprefix $(OUT_DIR)/, $(SYSTEM_CPP_SRCS)))

#boost program_options
PROGRAM_OPTIONS_MAIN = $(OUT_DIR)/libboost-program_options-$(BOOST_SUFFIX)
PROGRAM_OPTIONS_SHARED = $(OUT_DIR)/libboost-program_options-$(DYN_SHARED)
PROGRAM_OPTIONS_CPP_SRCS = $(shell find libs/program_options/src -iname '*.cpp') 
PROGRAM_OPTIONS_CPP_OBJS:=$(patsubst %.cpp, %.o, $(addprefix $(OUT_DIR)/, $(PROGRAM_OPTIONS_CPP_SRCS)))

#boost test
TEST_MAIN = $(OUT_DIR)/libboost-test-$(BOOST_SUFFIX)
TEST_SHARED = $(OUT_DIR)/libboost-test-$(DYN_SHARED)
TEST_CPP_SRCS = $(shell find libs/test/src -iname '*.cpp') 
TEST_CPP_OBJS:=$(patsubst %.cpp, %.o, $(addprefix $(OUT_DIR)/, $(TEST_CPP_SRCS)))
