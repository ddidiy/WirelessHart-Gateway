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
PROJ_TOP:=.



whaccesspoint.FILES   = App Cfg main EventLoop TtyDownLink HartUpLink HartUpHdr HartUpMsgQueue MsgQueue
whaccesspoint.CXXFLAGS= $(ENCASING) -I$(TOP)  $(BOOST_CXXFLAGS)
whaccesspoint.SUBDIRS = $(SHRD_DIR) 
whaccesspoint.LDFLAGS = $(SHRD_LIB_LINK) -Wl,--traditional-format
whaccesspoint.TESTS   = TMsgQueue
#TMsgMultiQueue THartUpMsgQueue THartUpMsgMultiQueue

TARGET.PROGRAMS=whaccesspoint
include $(TOP)/system.mk


GLOBAL.LDFLAGS = $(SHRD_LIB_LINK) -Wl,--traditional-format

TMsgQueue.FILES   = MsgQueue tests/time_wrapper
TMsgQueue.SUBDIRS = $(SHRD_DIR)
TMsgQueue.LDFLAGS = -Wl,-wrap,time


TMsgMultiQueue.FILES   = $(TMsgQueue.FILES)
THartUpMsgQueue.FILES   = HartUpMsgQueue $(TMsgQueue.FILES)
THartUpMsgMultiQueue.FILES   = HartUpMsgQueue $(TMsgQueue.FILES)

