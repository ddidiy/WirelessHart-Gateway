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
 * OperationsResponseCodes.cpp
 *
 *  Created on: Apr 16, 2010
 *      Author: andrei.petrut
 */
#include "OperationsResponseCodes.h"
#include "../AllNetworkManagerCommands.h"

namespace hart7 {
namespace nmanager {
namespace operations {

OperationsResponseCodes::OperationsResponseCodes(bool reportAllAsError_) :
    reportAllAsError(reportAllAsError_)
{
}

bool OperationsResponseCodes::IsResponseCodeConsideredError(uint16_t cmdId, uint8_t responseCode)
{
    if (responseCode == 0)
    {
        return false;
    }

    if (reportAllAsError && responseCode != 0)
    {
        return true;
    }

    switch (cmdId)
    {
        case CMDID_C795_WriteTimerInterval:
            switch (responseCode)
            {
                case C795_W08:
                    return false;
                default:
                break;
            }
        break;

        case CMDID_C967_WriteLink:
            switch (responseCode)
            {
                case C967_E66:
                    return false;
                default:
                break;
            }
        break;
        case CMDID_C968_DeleteLink:
            switch (responseCode)
            {
                case C968_E65:
                    return false;
                default:
                break;
            }
        break;

        case CMDID_C970_DeleteGraphConnection:
            switch (responseCode)
            {
                case C970_E65:
                    return false;
                default:
                break;
            }
        break;

        default:
        break;
    }

    return true;
}

}
}
}
