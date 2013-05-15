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



# Makefile for Shared folder
export TOP=..

-include $(TOP)/sys_inc.mk

shared.FILES = app Common Utils UtilsSolo  IdsBooking  DevMem ModulesActivity \
		FileLock  PersistentInteger PersistentMsgId CmdLineArgs\
		IniParser Config CfgBuffVar  PacketStream ProtocolPacket h \
		pipe link Socket TcpSocket  UdpSocket ServerSocket \
		log log_callback linklog log2flash  AlarmEx SignalsMgr \
		SimpleStream MicroSec Packet BuffFile TimeWrapper Tun


#shared.TESTS  = TPacketStream
shared.CXXFLAGS= $(BOOST_CXXFLAGS)  -I$(TOP)

TARGET.LIBRARIES := shared

include $(TOP)/system.mk


GLOBAL.LDFLAGS = $(SHRD_LIB_LINK) 
#../UnitTest/test_fmwk/mockpp/lib/libmockpp.so.36

#TPacketStream.FILES=PacketStream
