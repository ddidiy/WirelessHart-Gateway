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

#ifndef SECURITYMANAGER_H_
#define SECURITYMANAGER_H_

#include <map>
#include <boost/noncopyable.hpp>
#include "Common/NETypes.h"
#include "Common/logging.h"
#include "Common/NEAddress.h"
#include "Common/NEException.h"
#include "Misc/Convert/Convert.h"
#include "Model/SecurityKey.h"
#include "Misc/Marshall/NetworkOrderBytesWriter.h"

#include "KeyGenerator.h"
#include "SecurityException.h"
#include <stdint.h>

using namespace NE::Common;
using namespace NE::Misc::Marshall;

namespace hart7 {

namespace security {

class DeviceSecurityInfo;
/**
 * Security Manager is the only entity that generates security keys in system.
 * It deals with device provisioning, device join and device service.
 *
 * @author Ioan-Vasile Pocol
 * @version 1.0
 */

class SecurityManager: boost::noncopyable
{
        LOG_DEF("h7.s.SecurityManager");

    public:

        SecurityManager(KeyGenerator::Ptr keyGenerator);

        virtual ~SecurityManager()
        {
        }

        /*
         * Read the keys from persistence medium.
         */
        //        int readKeys();

        /**
         * Network key must be unique on each NM start.
         */
        SecurityKey GetNetworkKey();

        void ChangeNetworkKey();

        /**
         * This key must be unique on each device and on each device rejoin.
         */
        SecurityKey GetSessionKey(const Address64& device, bool renew = false);

    private:

        typedef map<Address64, SecurityKey> KeysMap;

        /**
         * Unique on each NM run.
         */
        SecurityKey networkKey;

        /**
         * Session keys.
         */
        KeysMap sessionKeys;

        /**
         * The key generator.
         */
        KeyGenerator::Ptr keyGenerator;
};

}
}

#endif /*SECURITYMANAGER_H_*/
