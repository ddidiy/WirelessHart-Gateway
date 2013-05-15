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
 * TerminateServiceFlowHandler.h
 *
 *  Created on: Sep 18, 2009
 *      Author: andrei.petrut
 */

#ifndef TERMINATESERVICEFLOWHANDLER_H_
#define TERMINATESERVICEFLOWHANDLER_H_

#include "../CommonData.h"
#include "IDefaultCommandHandler.h"
#include <WHartStack/WHartStack.h>

namespace hart7 {
namespace nmanager {

/**
 * Flow is responsible with removing a service upon request from the source.
 */
class TerminateServiceFlowHandler
{
    LOG_DEF("h7.n.TerminateServiceFlowHandler");

    private:

        CommonData& commonData;

    public:

        /**
         * Constructor.
         */
        TerminateServiceFlowHandler(CommonData& commondata);

        /**
         * Processes a C801 Delete Service request. Removes the service. No DR mechanism.
         */
        void ProcessRequest(const stack::WHartAddress& src, const C801_DeleteService_Req& request,
                            WHartCommandWrapper& response);
};

}
}

#endif /* TERMINATESERVICEFLOWHANDLER_H_ */
