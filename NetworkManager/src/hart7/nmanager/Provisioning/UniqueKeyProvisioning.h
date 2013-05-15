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
 * UniqueKeyProvisioning.h
 *
 *  Created on: Aug 4, 2010
 *      Author: Mihai.Stef
 */

#ifndef UNIQUEKEYPROVISIONING_H_
#define UNIQUEKEYPROVISIONING_H_

#include "../IProvisioning.h"

namespace hart7 {
namespace nmanager {

/**
 * A single unique join key is used for every device.
 */
class UniqueKeyProvisioning: public IProvisioning
{
    public:

        void getProvisioning(const WHartUniqueID& uniqueID, NE::Model::SecurityKey& key);
        UniqueKeyProvisioning(const NE::Model::SecurityKey& key);
        virtual ~UniqueKeyProvisioning();

    private:
        NE::Model::SecurityKey UniqueKey;


};

}
}

#endif /* UNIQUEKEYPROVISIONING_H_ */
