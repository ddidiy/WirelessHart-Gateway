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
 * DevicesJoinPriorityFlow.h
 *
 *  Created on: Dec 15, 2009
 *      Author: Radu Pop, Andrei Petrut
 */

#ifndef DEVICESJOINPRIORITYFLOW_H_
#define DEVICESJOINPRIORITYFLOW_H_

#include <nlib/log.h>

#include "../CommonData.h"
#include "../IResponseProcessor.h"
#include "Model/Operations/EngineOperations.h"
#include "Model/Services/Service.h"

namespace hart7 {

namespace nmanager {

namespace operations {

class WHOperationQueue;

}

/**
 * Changes the Join Priority of the devices as their resources are used or released.
 * @author Radu Pop
 */
class DevicesJoinPriorityFlow: public IResponseProcessor
{
    LOG_DEF("h7.n.DevicesJoinPriorityFlow");

    private:

        std::map<Uint16, uint8_t> changedDevicesList;

        CommonData& commonData;

    public:

        DevicesJoinPriorityFlow(CommonData& commonData);

        virtual ~DevicesJoinPriorityFlow();

        void ProcessConfirmedOperations(bool errorOccured);

        void ProcessConfirm(WHartHandle requestHandle, const WHartLocalStatus& localStatus,
                            const WHartCommandList& list);

        void ProcessIndicate(WHartHandle handle, const WHartAddress& src, WHartPriority priority,
                             WHartTransportType transportType, const WHartCommandList& list);

        /**
         * The join priority for the device with the given address has been changed.
         */
        void markJoinPriorityForDeviceAsChanged(Uint16 address, uint8_t joinPriority);

        /**
         * Send 811 command to all the devices that have join priority changes.
         */
        void updateDevices(hart7::nmanager::operations::WHOperationQueue& queue);
};

}

}

#endif /* DEVICESJOINPRIORITYFLOW_H_ */
