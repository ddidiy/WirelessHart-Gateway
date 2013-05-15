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
 * ServicesTypes.h
 *
 *  Created on: Sep 18, 2009
 *      Author: ioanpocol
 */

#ifndef SERVICESTYPES_H_
#define SERVICESTYPES_H_

#include "Common/NETypes.h"

namespace NE {

namespace Model {

namespace Services {

#define MAX_FREQUENCY 15
#define MAX_STAR_INDEX 25

namespace ApplicationDomain {
/**
 * Enumeration with possible application domain.
 */
enum ApplicationDomainEnum {
    PUBLISH = 0, EVENT = 1, MAINTENANCE = 2, BLOCK_TRANSFFER = 3
};
}

namespace DeleteReason {
/**
 * Enumeration with possible application domain.
 */
enum DeleteReasonEnum {
    REQUESTED_BY_PEER = 0, SERVICE_CAN_NOT_BE_ESTABLISHED = 1, NETWORK_FAILURE = 2
};
}

}
}
}

#endif /* SERVICESTYPES_H_ */
