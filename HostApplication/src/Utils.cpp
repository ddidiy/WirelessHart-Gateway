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


#include <iostream>
#include <WHartHost/Utils.h>
#include <nlib/log.h>
#include <WHartStack/WHartStack.h>
#include <src/ApplicationLayer/Model/CommonTables.h>
#include <src/ApplicationLayer/Model/CommonPracticeCommands.h>

//convert
static char aHEX[]={'0', '1', '2', '3', '4', '5', '6', '7', '8', '9', 'A', 'B', 'C', 'D', 'E', 'F'}; 
const char * ConvertToHex(unsigned char val)
{
	static char hexVal[3] ={0,0,0};
	hexVal[0] = aHEX[(val >> 4)];
	hexVal[1] = aHEX[(val & 0x0F)];
	return hexVal;
}
const char * ConvertToHex(unsigned short val)
{
	static char hexVal[5] ={0,0,0,0,0};
	strcpy(hexVal, ConvertToHex((unsigned char)(val >> 8)));
	strcat(hexVal, ConvertToHex((unsigned char)(val & 0x00FF)));
	return hexVal;
}
const char * ConvertToHex(unsigned int val)
{
	static char hexVal[9] ={0,0,0,0,0,0,0,0,0};
	strcpy(hexVal, ConvertToHex((unsigned short)(val >> 16)));
	strcat(hexVal, ConvertToHex((unsigned short)(val & 0x0000FFFF)));
	return hexVal;
}

std::string GetDeviceVariableName(int p_nVariableCode)
{
	switch (p_nVariableCode) {
		case DeviceVariableCodes_BatteryLife: 			// 243
			return "BatteryLife";
		case DeviceVariableCodes_PercentRange: 			// 244
			return "PercentRange";
		case DeviceVariableCodes_LoopCurrent: 			// 245
			return "LoopCurrent";
		case DeviceVariableCodes_PrimaryVariable: 		// 246
			return "PrimaryVar";
		case DeviceVariableCodes_SecondaryVariable: 	// 247
			return "SecondaryVar";
		case DeviceVariableCodes_TertiaryVariable:		// 248
			return "TertiaryVar";
		case DeviceVariableCodes_QuaternaryVariable:	// 249
			return "QuaternaryVar";
		case DeviceVariableCodes_None:					// 250
			return "N/A";
		default:
			std::stringstream ss;
			ss << "var" << p_nVariableCode;
			return ss.str();
	}
}


//gcd
static int GCD(std::list<unsigned int> &numbers, unsigned int prevMin)
{
	unsigned int nextMin = prevMin;
    bool finished = true;
    for (std::list<unsigned int>::iterator it = numbers.begin(); it != numbers.end(); ++it)
    {
        if ((*it) > prevMin)
        {
            finished = false;
            if (((*it) -= prevMin) < prevMin)
                nextMin = ((*it) < nextMin) ? (*it) : nextMin; //there could be nextMin set with a value greater than a new one
        }
    }
    if (finished)
        return prevMin;
    return GCD(numbers, nextMin);
}

int ProcessGCD(const std::list<unsigned int> &numbers_)
{
	std::list<unsigned int> numbers = numbers_;
    unsigned int min = 0xFFFFFFFF;
    for (std::list<unsigned int>::iterator it = numbers.begin(); it != numbers.end();)
    {
        if ((*it) <= 0)
        {
            LOG_ERROR_APP("[GCD]: number should not be equal or less than 0");
            it = numbers.erase(it);
            if (it == numbers.end())
                break;
        }
        if ((*it) < min)
            min = (*it);
        ++it;
    }
	return GCD(numbers, min);
}

WHartUniqueID MakeCorrectedUniqueID(uint16_t deviceCode, uint32_t deviceID, uint8_t masterMode)
{
    WHartUniqueID oAddressField = hart7::stack::MakeUniqueID(deviceCode, deviceID);
    //first bit is master address : secondary master or primary master
    oAddressField.bytes[0] |= (masterMode != 0 ? (1<<7) : 0);
    //second bit is burst mode - change to 0
    oAddressField.bytes[0] &= (~(1<<6));

    return oAddressField;
}

