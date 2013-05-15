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
 * RequestServiceFlowHandler.h
 *
 *  Created on: Sep 10, 2009
 *      Author: andrei.petrut
 */

#ifndef RequestServiceFlowHandler_H_
#define RequestServiceFlowHandler_H_

#include <boost/shared_ptr.hpp>
#include <ApplicationLayer/Model/NetworkLayerCommands.h>
#include <Model/Operations/EngineOperations.h>
#include <boost/function.hpp>

#include "IDefaultCommandHandler.h"
#include "../IRequestSend.h"
#include "../operations/WHOperationQueue.h"
#include <nlib/log.h>

namespace hart7 {
namespace nmanager {

/**
 * Creates a service between the requester and the requested peer.
 * Implements the Delayed Response mechanism until the service is set up.
 */
class RequestServiceFlowHandler
{
    LOG_DEF("h7.n.RequestServiceFlowHandler");

    public:

        typedef boost::shared_ptr<RequestServiceFlowHandler> Ptr;

        typedef boost::function1<void, WHartCommandWrapper&> OnFinishedCallback;

        /**
         * Delayed command handler implemented as a default handler for the requests that
         * come from the same device , have the same service ID and the same peer as the initial request.
         * Sends DR_Running until the service is completed.
         */
        struct DelayedCommandHandler: public IDefaultCommandHandler
        {
            public:

                DelayedCommandHandler() :
                    finishRequested(false), finished(false)
                {
                }

                virtual ~DelayedCommandHandler()
                {
                }

                typedef boost::shared_ptr<DelayedCommandHandler> Ptr;

                bool HandleCommand(const WHartAddress& src, const stack::WHartCommand& request,
                                   WHartCommandWrapper& response);

                bool CommandHandlerFinished();

                void FinishCommand(OnFinishedCallback callback);

            public:

                WHartAddress source;

                C799_RequestService_Req serviceRequest;

                boost::function0<void> CreateServiceCallback;

                bool finishRequested;

                OnFinishedCallback finishCallback;

                bool finished;
        };

        RequestServiceFlowHandler(CommonData& commondata, const RegisterDefaultHandlerCallback& registerDefault);

        /**
         * This method processes a Request Service request, and will register a default handler.
         */
        void ProcessRequest(const stack::WHartAddress& src, const C799_RequestService_Req& request,
                            WHartCommandWrapper& response);

    private:

        void CreateDelayedResponseInitiate(WHartCommandWrapper& response);

        void RegisterDefaultHandler(const stack::WHartAddress& src_, const C799_RequestService_Req& request);

        void CreateServiceWithPeer();

        void OnServiceCreated();

        void FinalMessageReceived(WHartCommandWrapper& response);

        uint32_t GetProcessedPeriod(uint32_t period);

    private:

        stack::WHartAddress src;

        C799_RequestService_Req serviceRequest;

        CommonData& commondata;

        NE::Model::Operations::EngineOperationsListPointer serviceOperations;

        /**
         * Called when the job is done
         */
        RegisterDefaultHandlerCallback registerDefault;

        DelayedCommandHandler::Ptr delayCommandHandler;

        uint32_t routeId;
};

}
}
#endif /* RequestServiceFlowHandler_H_ */
