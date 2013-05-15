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



# Makefile for sqlitexx lib
export TOP=../..

-include $(TOP)/sys_inc.mk

sqlitexx.FILES = Connection RawData sqlitexx_log
sqlitexx.CXXFLAGS= $(BOOST_CXXFLAGS) -I$(CPPLIB_DIR)/nlib/ -I$(TOP)

TARGET.LIBRARIES := sqlitexx

include $(TOP)/system.mk
