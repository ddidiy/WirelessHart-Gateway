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
 * NewAdvertiseLinkOnTrFlow.h
 *
 *  Created on: Jun 22, 2010
 *      Author: radu
 */

#ifndef NEWADVERTISELINKONTRFLOW_H_
#define NEWADVERTISELINKONTRFLOW_H_

#include "FlowHandler.h"
#include "Model/Operations/EngineOperation.h"
#include <boost/function.hpp>
#include <Common/logging.h>

namespace hart7 {

namespace nmanager {

/**
 * This flow is used to send a new advertise link operation to the TR.
 * An advertise link is a regular Tx link with peer FFFF whose sole purpose is to offer the TR
 * an occasion to advertise.
 * @author Radu Pop
 */
class NewAdvertiseLinkOnTrFlow: public FlowHandler
{
    public:
        LOG_DEF("h7.n.NewAdvertiseLinkOnTrFlow")
        ;

        /**
         * Constructor.
         */
        NewAdvertiseLinkOnTrFlow(IRequestSend& requestSend_, CommonData& commonData_,
                                 operations::WHOperationQueue& operationsQueue_);

        virtual ~NewAdvertiseLinkOnTrFlow();

        typedef boost::function1<void, bool> NewAdvertiseLinkOnTrFlowFinishedHandler;

        /**
         * Callback for the flow finish.
         */
        NewAdvertiseLinkOnTrFlowFinishedHandler newAdvertiseLinkOnTrFlowFinished;

        /**
         * Creates a new advertise link on the TR.
         */
        bool sendNewAdvLink(Address32 brAddress);

        /**
         * Removes an advertise link from the TR.
         */
        void sendRemoveAdvLink(Address32 brAddress, uint32_t allocationHandler);

        virtual void ProcessConfirmedOperations(bool errorOccured);

        /**
         * Stops flow.
         */
        void StopFlow();


        NE::Model::Operations::EngineOperationsListPointer advertiseEngineOperations;

    private:
        uint32_t lastOperationsHandle;

};

typedef boost::shared_ptr<NewAdvertiseLinkOnTrFlow> NewAdvertiseLinkOnTrFlowPointer;

}

}

#endif /* NEWADVERTISELINKONTRFLOW_H_ */
