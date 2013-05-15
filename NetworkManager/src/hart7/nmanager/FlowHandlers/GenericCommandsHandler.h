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
 * GenericCommandsHandler.h
 *
 *  Created on: May 22, 2009
 *      Author: andrei.petrut
 */

#ifndef GENERICCOMMANDSHANDLER_H_
#define GENERICCOMMANDSHANDLER_H_

#include <nlib/log.h>
#include <WHartStack/WHartStack.h>
#include <vector>

#include "RequestServiceFlowHandler.h"
#include "GatewayRequestsHandler.h"
#include "IDefaultCommandHandler.h"
#include "ReportsHandler.h"
#include "../CommonData.h"

namespace hart7 {
namespace nmanager {

using namespace stack;

/**
 * Generic handler for requests / responses/ bursts.
 * Dispatches health reports and alarms to their respective handlers.
 * Offers default handlers for requests (used in Delayed Response mechanism).
 */
class GenericCommandsHandler: public IResponseProcessor
{
    LOG_DEF("h7.n.GenericCommandsHandler");

    public:

        GenericCommandsHandler(CommonData& commonData_, IRequestSend& requestSend_) :
            commonData(commonData_), requestSend(requestSend_), reportsHandler(commonData_),
                        gatewayRequestsHandler(commonData_)
        {
        }

        virtual ~GenericCommandsHandler()
        {
        }

        /**
         * Interface method called with requests from the stack.
         */
        void HandleRequests(WHartHandle handle, const WHartAddress& source, const WHartCommandList& commandsList);

        /**
         * Interface method called with responses from the stack.
         */
        void HandleResponses(WHartHandle handle, const WHartAddress& source, const WHartCommandList& commandsList);

        /**
         * Interface method called with publish-notifys from the stack.
         */
        void HandlePublishes(WHartHandle handle, const WHartAddress& source, const WHartCommandList& commandsList);

        /**
         * Registers a default handler. Any request will be first passed to this default handler.
         */
        void RegisterDefaultHandler(IDefaultCommandHandler::Ptr defaultHandler);

        /**
         * Interface method.
         */
        virtual void ProcessConfirm(WHartHandle requestHandle, const WHartLocalStatus& localStatus,
                                    const WHartCommandList& list);
        /**
         * Interface method.
         */

        virtual void ProcessIndicate(WHartHandle handle, const WHartAddress& src, WHartPriority priority,
                                     WHartTransportType transportType, const WHartCommandList& list);


    private:

        void HandleGWRequests(WHartHandle handle, const WHartAddress& source, const WHartCommandList& commandsList);

        void HandleDeviceRequests(WHartHandle handle, const WHartAddress& source, const WHartCommandList& commandsList);

        bool HandleDefaults(const WHartAddress& src, const WHartCommand& command, WHartCommandWrapper& response);

       bool generateGWNotification(C64765_NivisMetaCommand_Resp* resp, WHartCommand& command, WHartAddress address);

        std::vector<RequestServiceFlowHandler::Ptr> serviceRequests;

        std::vector<IDefaultCommandHandler::Ptr> defaultHandlers;

        CommonData& commonData;

        IRequestSend& requestSend;

        ReportsHandler reportsHandler;

        GatewayRequestsHandler gatewayRequestsHandler;

};

}
}
#endif /* GENERICCOMMANDSHANDLER_H_ */
