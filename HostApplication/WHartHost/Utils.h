/*
* Copyright (C) 2013 Nivis LLC.
* Email:   opensource@nivis.com
* Website: http://www.nivis.com
*
* This program is free software: you can redistribute it and/or modify
* it under the terms of the GNU General Public License as published by
* the Free Software Foundation, version 3 of the License.
* 
* Redistribution and use in source and binary forms must retain this
* copyright notice.

* This program is distributed in the hope that it will be useful,
* but WITHOUT ANY WARRANTY; without even the implied warranty of
* MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
* GNU General Public License for more details.
*
* You should have received a copy of the GNU General Public License
* along with this program.  If not, see <http://www.gnu.org/licenses/>.
*
*/


#ifndef UTILS_H_
#define UTILS_H_

#include <list>
#include <string>
#include <WHartStack/WHartTypes.h>

//convert
const char * ConvertToHex(unsigned char val);
const char * ConvertToHex(unsigned short val);
const char * ConvertToHex(unsigned int val);
std::string GetDeviceVariableName(int p_nVariableCode);
WHartUniqueID MakeCorrectedUniqueID(uint16_t deviceCode, uint32_t deviceID, uint8_t masterMode);

//gcd
int ProcessGCD(const std::list<unsigned int> &numbers_);



#endif
