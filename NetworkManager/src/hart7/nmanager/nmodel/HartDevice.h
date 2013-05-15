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
 * Device.h
 *
 *  Created on: Sep 2, 2009
 *      Author: andrei.petrut
 */

#ifndef NMANAGER_DEVICE_H_
#define NMANAGER_DEVICE_H_

#include <WHartStack/WHartTypes.h>
#include <nlib/log.h>

namespace hart7 {
namespace nmanager {

/**
 * Holds information about a device in the inner model.
 */
class HartDevice
{
    LOG_DEF("h7.n.Device");

    public:

        HartDevice()
        {
            //LOG_ERROR("Device - default");
            isGateway = false;
            isBackbone = false;
        }

        HartDevice(const HartDevice& other)
        {
            for (int i = 0; i < 5; longAddress.bytes[i] = other.longAddress.bytes[i], i++)
                ;
            nickname = other.nickname;
            isGateway = other.isGateway;
            isBackbone = other.isBackbone;
        }

        bool operator==(const HartDevice& other)
        {
            bool isEqual = true;
            for (int i = 0; i < 5; i++)
            {
                if (longAddress.bytes[i] != other.longAddress.bytes[i])
                {
                    isEqual = false;
                    break;
                }
            }
            return (isEqual || (nickname == other.nickname));
        }

        friend bool operator<(const HartDevice& lhs, const HartDevice& rhs)
        {
            if (lhs.nickname == rhs.nickname)
            {
                for (int i = 0; i < 5; i++)
                {
                    if (lhs.longAddress.bytes[i] != rhs.longAddress.bytes[i])
                        return lhs.longAddress.bytes[i] < rhs.longAddress.bytes[i];
                }
                return false;
            }
            return lhs.nickname < rhs.nickname;
        }

    public:

        WHartUniqueID longAddress;

        WHartShortAddress nickname;

        bool isGateway;

        bool isBackbone;
};

}
}

#endif /* NMANAGER_DEVICE_H_ */
