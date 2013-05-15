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
 * IRequestSend.h
 *
 *  Created on: May 25, 2009
 *      Author: andrei.petrut
 */

#ifndef IREQUESTSEND_H_
#define IREQUESTSEND_H_
#include <WHartStack/WHartStack.h>
#include "IResponseProcessor.h"

namespace hart7 {
namespace nmanager {

using namespace hart7::stack;

/**
 * Interface for sending requests through the stack. Offers means to transmit a request or a response.
 */
class IRequestSend
{
    public:

        virtual ~IRequestSend()
        {
        }

        /**
         * Transmit a request through the stack. Returns the stack handle of the request.
         */
        virtual WHartHandle TransmitRequest(const WHartAddress& dest, WHartPriority priority, WHartServiceID serviceID,
                                            WHartTransportType transportType, const WHartCommandList& list,
                                            WHartSessionKey::SessionKeyCode sessionCode,
                                            IResponseProcessor& responseProcessor) = 0;

        /**
         * Transmit a response to an indicated request through the stack.
         */
        virtual void TransmitResponse(WHartHandle indicatedHandle, WHartServiceID serviceID,
                                      const WHartCommandList& list, WHartSessionKey::SessionKeyCode sessionCode) = 0;

        /**
         * Cancel a reqeust with a specified handle.
         */
        virtual void CancelRequest(WHartHandle handle) = 0;

        /**
         * Test the stack to see if a packet can be sent to a destination. Limitations come from the transport layer.
         */
        virtual bool CanSendToAddress(const WHartAddress& dest) = 0;
};

}
}

#endif /* IREQUESTSEND_H_ */
