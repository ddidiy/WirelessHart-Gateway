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

/*
 * ScriptLineParser.cpp
 *
 *  Created on: Jul 22, 2010
 *      Author: andrei.petrut
 */

#include "ScriptLineParser.h"
#include <Model/NetworkEngine.h>
#include "../DevicesManager.h"

const char delimiters[] = {' '};

template<typename T> T ParseHex(std::string value, std::string prepend = "")
{
    std::stringstream str;
    long outValue;
    str << prepend << value;
    str >> std::hex >> outValue;

    return (T)outValue;
}



template<typename T> T Parse(std::string value)
{
	std::stringstream str;
	long outValue;
	str << value;
	str >> outValue;

	return (T)outValue;
}


ScriptLineParser::ScriptLineParser(hart7::nmanager::DevicesManager& devicesManager_) : tokenizerFunc(delimiters), devicesManager(devicesManager_)
{
}

void ScriptLineParser::ParseLine(const std::string& line)
{
    LOG_DEBUG("Executing Script '" << line << "'");
	Tokenizer tokenizer(line, tokenizerFunc);

	int index = 0;
	for (Tokenizer::iterator it = tokenizer.begin(); it != tokenizer.end(); ++it, ++index)
	{
		if (index == 0)
		{
			try
			{
				ProcessCommand(line, it, tokenizer.end());
			}
			catch (std::exception& ex)
			{
				LOG_WARN("Error while processing command '" << line << "'. ex=" << ex.what());
			}
			break;
		}
	}
}


void ScriptLineParser::ProcessCommand(const std::string& line, Tokenizer::iterator firstParam, Tokenizer::iterator end)
{
	if (*firstParam == "DEV_DEL")
	{
		if (++firstParam != end)
		{
			uint16_t address = ParseHex<uint16_t>(*firstParam, "0x");
			LOG_INFO("Executing DEV_DEL of " << ToStr(address));

			devicesManager.DeleteDevice(address);
		}
	}
	else if (*firstParam == "DEV_WRITE_NETID")
	{
	    uint16_t address = 0xFFFF;
	    uint16_t networkID = 0xFFFF;
	    bool haveParams = false;
	    if (++firstParam != end)
	    {
	        address = ParseHex<uint16_t>(*firstParam, "0x");
	        if (++firstParam != end)
	        {
	            networkID = ParseHex<uint16_t>(*firstParam, "0x");
	            haveParams = true;
	        }
	    }

	    if (haveParams)
	    {
	        if (NE::Model::NetworkEngine::instance().getDevicesTable().existsConfirmedDevice(address))
	        {
	            LOG_INFO("Executing DEV_WRITE_NETID addr=" << std::hex << address << " netId=" << std::hex << networkID);

	            devicesManager.ChangeNetworkID(address, networkID);
	        }
	        else
	        {
	            LOG_INFO("DEV_WRITE_NETID - device " << std::hex << address << " not operational!");
	        }
	    }
	    else
	    {
	        LOG_INFO("Not enough parameters for command:'" << line << "'");
	    }
	}
}








