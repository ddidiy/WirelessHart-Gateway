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

#include "SecurityManager.h"
#include "MockKeyGenerator.h"

namespace hart7 {
namespace security {

using namespace NE::Model;

std::string StreamToString(uint8_t* input, uint16_t len)
{
    std::ostringstream stream;
    for (uint16_t i = 0; i < len; i++)
    {
        stream << std::hex << ::std::setw(2) << ::std::setfill('0') << (int) *(input + i) << " ";
    }

    return stream.str();
}

SecurityManager::SecurityManager(KeyGenerator::Ptr keyGenerator_) :
    keyGenerator(keyGenerator_)
{
    if (!keyGenerator)
    {
        keyGenerator.reset(new hart7::security::MockKeyGenerator());
    }

    // will be cached as long as NM is running
    networkKey = keyGenerator->generateKey();
}

void SecurityManager::ChangeNetworkKey()
{

}

SecurityKey SecurityManager::GetNetworkKey()
{
    return networkKey;
}

SecurityKey SecurityManager::GetSessionKey(const Address64& device, bool renew)
{
    KeysMap::iterator it = sessionKeys.find(device);
    if ((renew == false) && (it != sessionKeys.end()))
    {
        return it->second;
    }
    else
    {
        sessionKeys.insert(std::make_pair(device, keyGenerator->generateKey()));
        return sessionKeys[device];
    }
}

}
}
