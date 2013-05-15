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
 * IProvisioning.h
 *
 *  Created on: Aug 3, 2010
 *      Author: Mihai.Stef
 */

#ifndef IPROVISIONING_H_
#define IPROVISIONING_H_

#include <boost/shared_ptr.hpp>
#include <WHartStack/WHartTypes.h>
#include <Model/SecurityKey.h>

namespace hart7 {
namespace nmanager {
/**
 * Generic exception thrown from provisioning.
 */
class ProvisioningException: public std::exception
{
    virtual const char* what() const throw()
    {
        return "Provisioning exception throw";
    }
};

/**
 * Interface for provisioning. Provides means to get join key for a specified UniqueID.
 */
class IProvisioning
{
    public:
        typedef boost::shared_ptr<IProvisioning> Ptr;
    public:
        virtual void getProvisioning(const WHartUniqueID &uniqueID, NE::Model::SecurityKey& key) = 0;
};

}
}

#endif /* IPROVISIONING_H_ */
