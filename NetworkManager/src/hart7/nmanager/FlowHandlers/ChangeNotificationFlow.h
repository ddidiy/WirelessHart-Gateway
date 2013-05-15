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
 * ChangeNotificationFlow.h
 *
 *  Created on: Oct 27, 2009
 *      Author: Radu Pop
 */

#ifndef CHANGENOTIFICATIONFLOW_H_
#define CHANGENOTIFICATIONFLOW_H_

#include "../CommonData.h"
#include "../operations/WHOperationQueue.h"
#include <nlib/log.h>

namespace hart7 {
namespace nmanager {

struct DependentCommand
{
    public:

        DependentCommand(uint16_t commandId_, uint16_t dependentNickname_) :
            commandId(commandId_), dependentNickname(dependentNickname_)
        {
        }

        uint16_t commandId;

        // if the operation has a dependent peer this field will contain it; otherwise will be 0.
        uint16_t dependentNickname;

        bool operator==(const DependentCommand& other)
        {
            return (commandId == other.commandId) && (dependentNickname == other.dependentNickname);
        }

        friend bool operator<(const DependentCommand& lhs, const DependentCommand& rhs)
        {
            if (lhs.dependentNickname == rhs.dependentNickname)
            {
                return lhs.commandId < rhs.commandId;
            }

            return (lhs.dependentNickname < rhs.dependentNickname);
        }
};

/**
 * The purpose of this flow is to support the "Command 839 Change Notification".
 * Every time a change is detected in NetworkEngine (a device changes its neighbors, routes, graphs, etc),
 * this flow will be notified and the address of the device will be cached.
 * From time to time the cached devices will be notified that a change has been made and the GW will
 * have to send a certain command to NM to get those changes.
 */
class ChangeNotificationFlow: public IResponseProcessor
{
        LOG_DEF("h7.n.ChangeNotificationFlow")
        ;

    private:

        typedef std::map<Uint16, std::set<DependentCommand> > ChangedDevicesList;

        // caches all devices and the changes (as command ids) that have changed
        ChangedDevicesList changedDevices;

        CommonData& commonData;

        IRequestSend& requestSend;

    public:

        ChangeNotificationFlow(CommonData& commonData, IRequestSend& requestSend);

        virtual ~ChangeNotificationFlow();

        void ProcessConfirm(WHartHandle requestHandle, const WHartLocalStatus& localStatus,
                            const WHartCommandList& list);

        void ProcessIndicate(WHartHandle handle, const WHartAddress& src, WHartPriority priority,
                             WHartTransportType transportType, const WHartCommandList& list);

        /**
         * The device with the given address has changes that can be get with the given command.
         */
        void markForNotifyDevice(uint16_t address, uint16_t commandId, uint16_t peerAddress);

        /**
         * This will check all the cached devices sets and send the notifications to the GW.
         */
        void notifyDevices();

};

}
}
#endif /* CHANGENOTIFICATIONFLOW_H_ */
