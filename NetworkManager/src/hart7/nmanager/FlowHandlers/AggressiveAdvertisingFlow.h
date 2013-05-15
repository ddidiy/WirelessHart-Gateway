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
 * AggressiveAdvertisingFlow.h
 *
 *  Created on: Oct 9, 2010
 *      Author: Radu Marginean
 */

#ifndef AGGRESIVEADVERTISINGFLOW_H_
#define AGGRESIVEADVERTISINGFLOW_H_

#include "FlowHandler.h"
#include <boost/function.hpp>
#include <Common/logging.h>
#include "Model/Operations/EngineOperation.h"
#include <nlib/timer/Timer.h>

namespace hart7 {

namespace nmanager {

/**
 * This flow is used to initiate aggressive advertising on TR
 * for some given time period than change back to the default.
 * @author Radu Marginean
 */

class AggressiveAdvertisingFlow : public FlowHandler
{

    public:
        LOG_DEF("h7.n.AggressiveAdvertisingFlow")
        ;

        /**
         * Constructor.
         */
        AggressiveAdvertisingFlow(IRequestSend& requestSend_, CommonData& commonData_, operations::WHOperationQueue& operationsQueue_);

        virtual ~AggressiveAdvertisingFlow();

        /**
         * Handles confirms for each command.
         */
        void ProcessConfirm(WHartHandle requestHandle, const WHartLocalStatus& localStatus,
                            const WHartCommandList& list);

        /**
         * Handles requests received. Does nothing in this flow.
         */
        void ProcessIndicate(WHartHandle handle, const WHartAddress& src, WHartPriority priority,
                            WHartTransportType transportType, const WHartCommandList& list);

        /**
         * Handles the confirmation of a command set.
         */
        void ProcessConfirmedOperations(bool errorOccured);

        /**
         * Called when a change of interval is requested. Used by start, stop, etc...
         */
        bool ChangeTrancieverAddvInterval (uint32_t timerInterval, bool agressive);

        /**
         * Initiates the aggressive advertise flow. Flow ends after a specified time.
         */
        bool InitiateAggresiveAdvertising (uint32_t intervalMsecs);

        /**
         * Stops an aggressive advertise flow before time.
         */
        bool StopAggressiveAdvertising();

        /**
         * Notifies flow handler of passed time.
         */
        void TimePassed(uint32_t timeMsec);

    private:

        uint32_t aggressiveAdvTimerInterval;

        uint32_t defaultAdvTimerInterval;

        uint32_t remainingTimer;

};

}

}

#endif /* AGGRESIVEADVERTISINGFLOW_H_ */
