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
 * CommandsProcessor.h
 *
 *  Created on: May 22, 2009
 *      Author: andrei.petrut
 */

#ifndef COMMANDSPROCESSOR_H_
#define COMMANDSPROCESSOR_H_

#include <WHartStack/WHartStack.h>
#include <boost/function.hpp>
#include <map>

#include "IResponseProcessor.h"
#include "IRequestSend.h"
#include "FlowHandlers/GenericCommandsHandler.h"
#include "FlowHandlers/IDefaultCommandHandler.h"
#include "CommonData.h"

#include <nlib/log.h>

namespace hart7 {
namespace nmanager {

using namespace hart7::stack;

/**
 * This class acts as a dispatcher of commands. It receives commands, and dispatches them to the appropriate handler.
 * Pairs requests with responses. Identifies join requests.
 */
class CommandsProcessor: public IRequestSend
{
    public:
	LOG_DEF("CP");
        CommandsProcessor(CommonData& commonData) :
            genericCommandsHandler(commonData, *this), isSendingCommand(false), errorOccured(false)
        {
        }

        typedef boost::function2<void, const WHartAddress&, const WHartCommandList&> HandleJoinRequestCallback;

        typedef boost::function6<WHartHandle, const WHartAddress&, WHartPriority, WHartTransportType, WHartServiceID,
                    const WHartCommandList&, WHartSessionKey::SessionKeyCode> TransmitRequestCallback;

        typedef boost::function4<void, WHartHandle, WHartServiceID, const WHartCommandList&,
                    WHartSessionKey::SessionKeyCode> TransmitResponseCallback;

        typedef boost::function1<bool, const WHartAddress&> CanSendToCallback;

        void ProcessConfirm(WHartHandle requestHandle, const WHartLocalStatus& localStatus,
                            const WHartCommandList& list);

        void ProcessIndicate(WHartHandle handle, const WHartAddress& src, WHartPriority priority,
                             WHartTransportType transportType, const WHartCommandList& list);

        virtual void CancelRequest(WHartHandle handle);

    private:

        bool IsRegistrationRequest(const WHartCommandList& commands);

        virtual WHartHandle TransmitRequest(const WHartAddress& dest, WHartPriority priority, WHartServiceID serviceID,
                                            WHartTransportType transportType, const WHartCommandList& list,
                                            WHartSessionKey::SessionKeyCode sessionCode,
                                            IResponseProcessor& responseProcessor);

        virtual void TransmitResponse(WHartHandle indicatedHandle, WHartServiceID serviceID,
                                      const WHartCommandList& list, WHartSessionKey::SessionKeyCode sessionCode);

        virtual bool CanSendToAddress(const WHartAddress& dest);

    public:

        HandleJoinRequestCallback HandleJoinRequest;

        TransmitRequestCallback TransmitRequestEvent;

        TransmitResponseCallback TransmitResponseEvent;

        CanSendToCallback CanSendToDestinationEvent;

    private:

        struct IResponseProcessorPlaceholder
        {
                IResponseProcessor& ResponseProcessor;

                IResponseProcessorPlaceholder(IResponseProcessor& responseProcessor_) :
                    ResponseProcessor(responseProcessor_)
                {
                }

                IResponseProcessorPlaceholder(const IResponseProcessorPlaceholder& other) :
                    ResponseProcessor(other.ResponseProcessor)
                {

                }
        };

        std::map<WHartHandle, IResponseProcessorPlaceholder> responseProcessors;

        std::vector<IDefaultCommandHandler::Ptr> defaultHandlers;

        GenericCommandsHandler genericCommandsHandler;

        bool isSendingCommand;
        bool errorOccured;
        WHartLocalStatus lastStatus;

};

}
}
#endif /* COMMANDSPROCESSOR_H_ */
