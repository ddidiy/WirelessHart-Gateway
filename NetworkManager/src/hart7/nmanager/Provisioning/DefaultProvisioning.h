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
 * DefaultProvisioning.h
 *
 *  Created on: Aug 4, 2010
 *      Author: Mihai.Stef
 */

#ifndef DEFAULTPROVISIONING_H_
#define DEFAULTPROVISIONING_H_

#include "../IProvisioning.h"
#include "../../../NMSettingsLogic.h"

namespace hart7 {
namespace nmanager {

/**
 * Default provisioning. Each device will have its own join key. Devices that are not provisioned will not join the network.
 */

class DefaultProvisioning : public IProvisioning
{
    public:
        DefaultProvisioning(hart7::util::NMSettingsLogic& settings);
        virtual ~DefaultProvisioning();

        void getProvisioning(const WHartUniqueID& uniqueID, NE::Model::SecurityKey& key);

    private:
        hart7::util::NMSettingsLogic& settingsLogic;

};


}
}
#endif /* DEFAULTPROVISIONING_H_ */
