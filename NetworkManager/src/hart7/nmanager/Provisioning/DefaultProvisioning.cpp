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
 * DefaultProvisioning.cpp
 *
 *  Created on: Aug 4, 2010
 *      Author: Mihai.Stef
 */

#include "DefaultProvisioning.h"
#include "../../util/ManagerUtils.h"


namespace hart7 {
namespace nmanager {

DefaultProvisioning::DefaultProvisioning(hart7::util::NMSettingsLogic& settings) : settingsLogic(settings)
{

}

DefaultProvisioning::~DefaultProvisioning()
{
}


void DefaultProvisioning::getProvisioning(const WHartUniqueID& uniqueID, NE::Model::SecurityKey& key)
{
    hart7::util::NMSettingsLogic::SecurityProvisioning::iterator it_provisioning = settingsLogic.provisioningKeys.begin();
    Address64 address = hart7::util::getAddress64FromUniqueId(uniqueID);
    for (; it_provisioning != settingsLogic.provisioningKeys.end(); it_provisioning++)
    {
        if (it_provisioning->first ==  address)
        {
            key = it_provisioning->second;
            return;
        }
    }
    throw ProvisioningException();
}

}
}
