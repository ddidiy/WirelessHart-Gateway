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
 * OperationsResponseCodes.h
 *
 *  Created on: Apr 16, 2010
 *      Author: andrei.petrut
 */

#ifndef OPERATIONSRESPONSECODES_H_
#define OPERATIONSRESPONSECODES_H_

#include <WHartStack/WHartTypes.h>

namespace hart7 {
namespace nmanager {
namespace operations {

/**
 * Processes operation response codes, to see if the response code is an error, or can be considered as OK.
 */
class OperationsResponseCodes
{
public:
        /**
         * Constructor. If <reportAllAsError> is true, any RC != 0 is reported as an error.
         */
        OperationsResponseCodes(bool reportAllAsError);

        /**
         * True if the response code for the command ID is an error. False otherwise.
         */
        bool IsResponseCodeConsideredError(uint16_t cmdId, uint8_t responseCode);

private:
	bool reportAllAsError;
};

}
}
}

#endif /* OPERATIONSRESPONSECODES_H_ */
