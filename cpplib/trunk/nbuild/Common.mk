
#############################
# required varibles
#
# TOOLCHAIN=gcc-cygwin|gcc-linux-pc|gcc-linux-arm|gcc-linux-m68k|iar-windows|vcl-windows|gcc-linux-trinity2
#
# CVSROOT - for cvs
# LFLAGS, LIBS - for link
# CPP_INCLUDES, CPP_FLAGS - for g++ compilation
# C_INCLUDES, C_FLAGS - for cc compilation
#############################
# TOOL CHAINS

export NBUILD_COMMON_INCLUDED=yes
 #used to know if this file has been included.

#generic tools (*nix style)
MKDIR = mkdir -p
CP = cp
RM = rm -f
CHMOD = chmod
CVS = cvs -d :pserver:nicu.dascalu@cljsrv01:/ISA100
TAR = tar -czf
UNTAR = tar -xzf
ECHO = @echo
link?=static

MTFLAGS=
ifeq ($(link),static)
MTFLAGS:=mt-s$(if $(DEBUG),d)
else ifeq ($(link),dynamic)
MTFLAGS:=mt$(if $(DEBUG),-d)
endif
export MTFLAGS

###############
# Common flags for all toolchains

CPP_FLAGS = $(if $(DEBUG), -g3 -O0 -gdwarf-2 -ggdb -D_DEBUG, -Os -fno-inline)


ifeq "$(hw)" "atom"
	export TOOLCHAIN?=
	export host?=i586-wrs-linux-gnu-i686-glibc_cgl

else ifeq "$(hw)" "mips"
	export TOOLCHAIN?=mips-openwrt-linux
	export host?=mips-openwrt-linux

else ifeq "$(hw)" "vr900"
	export TOOLCHAIN?=m68k-unknown-linux-uclibc
	export host?=m68k-unknown-linux-uclibc
else ifeq "$(hw)" "pf"
	export TOOLCHAIN?=arm-iwmmx-linux-gnueabi
	export host?=arm-iwmmx-linux-gnueabi

else ifeq "$(hw)" "trinity2"
	export TOOLCHAIN?=gcc-linux-trinity2
	export host?=arm-926ejs-linux-gnueabi
	export link_shared?=static

else ifeq "$(hw)" "an6"
	export TOOLCHAIN?=gcc-linux-arm
	export host?=arm

else ifeq "$(hw)" "an5"
	export TOOLCHAIN?=gcc-linux-arm
	export host?=arm

else ifeq "$(hw)" "cyg"
	export TOOLCHAIN?=gcc-cygwin
	export host?=cyg
	export fs_tree?=rel

else ifeq "$(hw)" "pc"
	export TOOLCHAIN?=gcc-linux-pc
	export host?=i386
	export fs_tree?=rel
	override  hw:=i386

else ifeq "$(hw)" "i386"
	export TOOLCHAIN?=gcc-linux-pc
	export host?=i386
	export fs_tree?=rel

else ifeq "$(hw)" "3eti"
	export TOOLCHAIN?=powerpc-linux-gnu
	export host?=powerpc-linux-gnu
	export fs_tree?=rel
endif

ifeq "$(link_shared)" "dynamic"
	AN_SHARED_LIB_EXT :=.so	
else
	#default
	AN_SHARED_LIB_EXT :=.a	
endif 

##############
# Flags for each toolchain

ifeq ($(TOOLCHAIN),gcc-cygwin)

	export host?=cyg
	export hw?=cyg
	export fs_tree?=rel

  # cygwin tool chain
  TIME_=$(CPPLIB_PATH)/trunk/nbuild/tools/cygwin/time.exe
  
#TIME_LINE=$(TIME_) --format="compiled in (%U user, %S sys) '$<'" 
TIME_LINE=
  CC = $(TIME_LINE) gcc -c
  CPP = $(TIME_LINE) g++ -c #colorgcc -c
  CPP_ = $(TIME_LINE) g++ -c
  LD = $(TIME_LINE) g++
  AR = $(TIME_LINE) ar
  STRIP =  $(TIME_LINE) strip
  
  SQLITE = $(CPPLIB_PATH)/trunk/nbuild/tools/cygwin/sqlite3.exe
  TARGET_OSTYPE=cygwin
  TARGET_OSTYPE_EXE_EXT=exe
  TARGET_OSTYPE_DLL_EXT=dll



else ifeq ($(TOOLCHAIN),gcc-linux-pc)
	export hw?=i386
	export host?=i386
	export fs_tree?=rel

  # linux-pc tool chain
  TIME_ = /usr/bin/time
  
#TIME_LINE=$(TIME_) --format="compiled in (%U user, %S sys) '$<'" 
TIME_LINE=

  CC = $(TIME_LINE) gcc -c
  CPP = $(TIME_LINE) g++ -c #colorgcc -c
  CPP_ = $(TIME_LINE) g++ -c
  LD = $(TIME_LINE) g++
  AR = $(TIME_LINE) ar
  STRIP = $(TIME_LINE)  strip
  
  SQLITE = $(CPPLIB_PATH)/trunk/nbuild/tools/linux-pc/sqlite3.bin
  TARGET_OSTYPE=linux-pc
  TARGET_OSTYPE_EXE_EXT=o
  TARGET_OSTYPE_DLL_EXT=so

else ifeq ($(TOOLCHAIN),gcc-linux-arm)	

	#default on an6
	export hw?=an6	
	export host?=arm
	

 # linux-arm tool chain
  TIME_ = /usr/bin/time

  _TOOLCHAIN_ARM=/usr/local/arm-linux-uclibc/bin
  CC = $(_TOOLCHAIN_ARM)/gcc -c
  CPP = $(_TOOLCHAIN_ARM)/g++ -c
  CPP_ = $(_TOOLCHAIN_ARM)/g++ -c
  LD = $(_TOOLCHAIN_ARM)/g++
  AR = $(_TOOLCHAIN_ARM)/ar
  STRIP = $(_TOOLCHAIN_ARM)/strip

  SQLITE = $(CPPLIB_PATH)/trunk/nbuild/tools/linux-pc/sqlite3.bin
  TARGET_OSTYPE=linux-arm
  TARGET_OSTYPE_EXE_EXT=o
  TARGET_OSTYPE_DLL_EXT=so

else ifeq ($(TOOLCHAIN),gcc-linux-m68k)	
 # linux-m68k ColdFire tool chain
  TIME_ = /usr/bin/time

  _TOOLCHAIN_M68K=m68k-unknown-linux-uclibc
  _TOOLCHAIN_M68K_PATH=/opt/nivis/$(_TOOLCHAIN_M68K)/bin
  CC = $(_TOOLCHAIN_M68K_PATH)/$(_TOOLCHAIN_M68K)-gcc -c
  CPP = $(_TOOLCHAIN_M68K_PATH)/$(_TOOLCHAIN_M68K)-g++ -c
  CPP_ = $(_TOOLCHAIN_M68K_PATH)/$(_TOOLCHAIN_M68K)-g++ -c
  LD = $(_TOOLCHAIN_M68K_PATH)/$(_TOOLCHAIN_M68K)-g++
  AR = $(_TOOLCHAIN_M68K_PATH)/$(_TOOLCHAIN_M68K)-ar
  STRIP = $(_TOOLCHAIN_M68K_PATH)/$(_TOOLCHAIN_M68K)-strip

  SQLITE = $(CPPLIB_PATH)/trunk/nbuild/tools/linux-pc/sqlite3.bin
  TARGET_OSTYPE=linux-m68k
  TARGET_OSTYPE_EXE_EXT=o
  TARGET_OSTYPE_DLL_EXT=so

else ifeq ($(TOOLCHAIN),gcc-linux-trinity2)	
 # linux-m68k ColdFire tool chain
	export hw?=trinity2
	export host?=arm-926ejs-linux-gnueabi
	export link_shared?=static

  TIME_ = /usr/bin/time

  _TOOLCHAIN_TRINITY2=arm-926ejs-linux-gnueabi
  _TOOLCHAIN_TRINITY2_PATH=/opt/trinity2/bin
  CC = $(_TOOLCHAIN_TRINITY2_PATH)/$(_TOOLCHAIN_TRINITY2)-gcc -c
  CPP = $(_TOOLCHAIN_TRINITY2_PATH)/$(_TOOLCHAIN_TRINITY2)-g++ -c
  CPP_ = $(_TOOLCHAIN_TRINITY2_PATH)/$(_TOOLCHAIN_TRINITY2)-g++ -c
  LD = $(_TOOLCHAIN_TRINITY2_PATH)/$(_TOOLCHAIN_TRINITY2)-g++
  AR = $(_TOOLCHAIN_TRINITY2_PATH)/$(_TOOLCHAIN_TRINITY2)-ar
  STRIP = $(_TOOLCHAIN_TRINITY2_PATH)/$(_TOOLCHAIN_TRINITY2)-strip

  SQLITE = $(CPPLIB_PATH)/trunk/nbuild/tools/linux-pc/sqlite3.bin
  TARGET_OSTYPE=linux-trinity2
  TARGET_OSTYPE_EXE_EXT=o
  TARGET_OSTYPE_DLL_EXT=so

else ifeq ($(TOOLCHAIN),mips-openwrt-linux)
	export host?=mips-openwrt-linux
	export hw?=mips
   # mips-openwrt-linux tool chain
   TIME_ = /usr/bin/time
   TUPLE=mips-openwrt-linux
   _TOOLCHAIN_MIPS=/opt/nivis/$(TUPLE)/bin/
   CC = $(_TOOLCHAIN_MIPS)/$(TUPLE)-gcc -c
   CPP = $(_TOOLCHAIN_MIPS)/$(TUPLE)-g++ -c
   CPP_ = $(_TOOLCHAIN_MIPS)/$(TUPLE)-g++ -c
   LD = $(_TOOLCHAIN_MIPS)/$(TUPLE)-g++
   AR = $(_TOOLCHAIN_MIPS)/$(TUPLE)-ar
   STRIP = $(_TOOLCHAIN_MIPS)/$(TUPLE)-strip
 
   SQLITE = $(CPPLIB_PATH)/trunk/nbuild/tools/linux-pc/sqlite3.bin
 #  TARGET_OSTYPE=m68k-unknown-linux-uclibc
   TARGET_OSTYPE=linux-mips
   TARGET_OSTYPE_EXE_EXT=o
   TARGET_OSTYPE_DLL_EXT=so

 else ifeq ($(TOOLCHAIN),m68k-unknown-linux-uclibc)
	export host?=m68k-unknown-linux-uclibc
	export hw?=vr900
   # m68k-unknown-linux-uclibc tool chain
   TIME_ = /usr/bin/time
   TUPLE=m68k-unknown-linux-uclibc
   _TOOLCHAIN_M68K=/opt/nivis/$(TUPLE)/bin/
   CC = $(_TOOLCHAIN_M68K)/$(TUPLE)-gcc -c
   CPP = $(_TOOLCHAIN_M68K)/$(TUPLE)-g++ -c
   CPP_ = $(_TOOLCHAIN_M68K)/$(TUPLE)-g++ -c
   LD = $(_TOOLCHAIN_M68K)/$(TUPLE)-g++
   AR = $(_TOOLCHAIN_M68K)/$(TUPLE)-ar
   STRIP = $(_TOOLCHAIN_M68K)/$(TUPLE)-strip
 
   SQLITE = $(CPPLIB_PATH)/trunk/nbuild/tools/linux-pc/sqlite3.bin
 #  TARGET_OSTYPE=m68k-unknown-linux-uclibc
   TARGET_OSTYPE=linux-m68k
   TARGET_OSTYPE_EXE_EXT=o
   TARGET_OSTYPE_DLL_EXT=so

 else ifeq ($(TOOLCHAIN),powerpc-linux-gnu)
	export host?=powerpc-linux-gnu
	export hw?=3eti
   TIME_ = /usr/bin/time
   TUPLE=powerpc-linux-gnu
   _TOOLCHAIN_3ETI_P1025=/opt/freescale/usr/local/3eti/$(TUPLE)/bin/
   CC = $(_TOOLCHAIN_3ETI_P1025)/$(TUPLE)-gcc -c
   CPP = $(_TOOLCHAIN_3ETI_P1025)/$(TUPLE)-g++ -c
   CPP_ = $(_TOOLCHAIN_3ETI_P1025)/$(TUPLE)-g++ -c
   LD = $(_TOOLCHAIN_3ETI_P1025)/$(TUPLE)-g++
   AR = $(_TOOLCHAIN_3ETI_P1025)/$(TUPLE)-ar
   STRIP = $(_TOOLCHAIN_3ETI_P1025)/$(TUPLE)-strip
 
   SQLITE = $(CPPLIB_PATH)/trunk/nbuild/tools/linux-pc/sqlite3.bin
   TARGET_OSTYPE=linux-3eti
   TARGET_OSTYPE_EXE_EXT=o
   TARGET_OSTYPE_DLL_EXT=so

 else ifeq ($(TOOLCHAIN),arm-iwmmx-linux-gnueabi)
	export host?=arm-iwmmx-linux-gnueabi
	export hw?=pf
   # m68k-unknown-linux-uclibc tool chain
   TIME_ = /usr/bin/time
   TUPLE=arm-iwmmx-linux-gnueabi
   _TOOLCHAIN_PF=/opt/nivis/${TUPLE}/bin/
   CC = $(_TOOLCHAIN_PF)/$(TUPLE)-gcc -c
   CPP = $(_TOOLCHAIN_PF)/$(TUPLE)-g++ -c
   CPP_ = $(_TOOLCHAIN_PF)/$(TUPLE)-g++ -c
   LD = $(_TOOLCHAIN_PF)/$(TUPLE)-g++
   AR = $(_TOOLCHAIN_PF)/$(TUPLE)-ar
   STRIP = $(_TOOLCHAIN_PF)/$(TUPLE)-strip
   # SQLITE = $(CPPLIB_PATH)/trunk/nbuild/tools/linux-pc/sqlite3.bin
   # TARGET_OSTYPE=m68k-unknown-linux-uclibc
   TARGET_OSTYPE=linux-arm-iwmmx
   TARGET_OSTYPE_EXE_EXT=o
   TARGET_OSTYPE_DLL_EXT=so

else ifeq ($(TOOLCHAIN),vcl-windows)
 # x86 Win32 tool chain
  TIME_=$(CPPLIB_PATH)/trunk/nbuild/tools/cygwin/time.exe
  CC = cl.exe
  #CPP = $(TIME_) --format="compiled in (%U user, %S sys) '$<'"  cl.exe /nologo /c /TP
  CPP = cl.exe /nologo /c /TP
  CPP_ = cl.exe
  LD = link.exe
  AR = link.exe
  STRIP = strip_not_defined

  SQLITE = $(CPPLIB_PATH)/trunk/nbuild/tools/cygwin/sqlite3.bin
  TARGET_OSTYPE=vcl-windows
  TARGET_OSTYPE_EXE_EXT=exe
  TARGET_OSTYPE_DLL_EXT=dll

  CC_VERSION=vc9.0

# not define yet : else ifeq($(TOOLCHAIN),iar-windows)
  # iar tool chain

else ifneq ($(MAKECMDGOALS),clean)
   $(error Unknown TOOLCHAIN='$(TOOLCHAIN)'! Available toolchains : [gcc-cygwin | gcc-linux-pc | gcc-linux-arm | vcl-windows | gcc-linux-m68k | m68k-unknown-linux-uclibc | gcc-linux-trinity2 | powerpc-linux-gnu ])
   
endif
  
 CC_VERSION?=$(shell $(CC) --version | grep -e '^[0-9a-zA-Z_-]*gcc (.*)' | sed -e 's/[0-9a-zA-Z_-]*gcc (.*) \([0-9]\)\.\([0-9]\).*/gcc\1\2/')

#############################
# GCC COMPILE/LINK defines

define common-gcc-compile
  @$(MKDIR) $(@D)
  @$(ECHO) "compiling '$<' ... "
  $(CPP) $(CPP_FLAGS) $(CPP_INCLUDES) -MMD -MP -MF"$(@:%.o=%.d)" -MT"$(@:%.o=%.d)" -o"$@" "$<"
endef

define common-g++-compile
  @$(MKDIR) -p $(@D)
  @$(ECHO) "compiling '$<' ... "
  $(CPP_) $(CPP_FLAGS) $(CPP_INCLUDES) -MMD -MP -MF"$(@:%.o=%.d)" -MT"$(@:%.o=%.d)" -o"$@" "$<"
endef

define common-cc-compile
  $(MKDIR) -p $(@D)
  @$(ECHO) -n "compiling '$<' ... "
  $(CC) $(C_FLAGS) $(CPP_FLAGS) $(CPP_INCLUDES) -MMD -MP -MF"$(@:%.o=%.d)" -MT"$(@:%.o=%.d)" -o"$@" "$<" 
endef

define common-gcc-link
  $(ECHO) "linking '$@' ..."
  $(LD) -o"$@" $+ $(LFLAGS) $(LIBS)
endef

define common-gcc-ar
  $(ECHO) "ar-ing '$@' ..."
  $(AR) -r "$@" $+ 
endef


#############################
# CVS defines
#//FIXME [nicu.dascalu] CVSROOT should be defined from outside
#CVSROOT ?= :pserver:nicu.dascalu@cljsrv01:/ISA100
#CVS = cvs -d $(CVSROOOT)
CVS = cvs -d :pserver:nicu.dascalu@cljsrv01:/ISA100


define common-cvs-update
  $(CVS) update -dPR 2>&1 | grep -v -i -e '^cvs server'
endef

define common-cvs-query
  $(CVS) -n update -dPR 2>&1 | grep -v -i -e '^cvs server'
endef


#############################
# SVN defines
