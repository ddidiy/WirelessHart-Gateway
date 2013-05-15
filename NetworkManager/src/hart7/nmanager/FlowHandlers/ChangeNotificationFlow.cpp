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
 * ChangeNotificationFlow.cpp
 *
 *  Created on: Oct 27, 2009
 *      Author: Radu Pop
 */

#include "ChangeNotificationFlow.h"
#include <ApplicationLayer/Binarization/GatewayCommands.h>
#include <ApplicationLayer/Model/GatewayCommands.h>
#include <hart7/nmanager/operations/WHOperation.h>
#include <hart7/util/NMLog.h>
#include <WHartStack/WHartStack.h>

#define MAX_COMMAND_SIZE 256

namespace hart7 {
namespace nmanager {

using namespace hart7::nmanager::operations;

ChangeNotificationFlow::ChangeNotificationFlow(CommonData& commonData_, IRequestSend& requestSend_) :
    commonData(commonData_), requestSend(requestSend_)
{

}

ChangeNotificationFlow::~ChangeNotificationFlow()
{
}

void ChangeNotificationFlow::markForNotifyDevice(uint16_t address, uint16_t commandId, uint16_t peerAddress)
{
    LOG_TRACE("markForNotifyDevice() " << ToStr(address) << ", cmd: " << (int) commandId);
    DependentCommand dc(commandId, peerAddress);
    changedDevices[address].insert(dc);
}

void ChangeNotificationFlow::ProcessConfirm(WHartHandle requestHandle, const WHartLocalStatus& localStatus,
                                            const WHartCommandList& list)
{
    // do nothing
}

void ChangeNotificationFlow::ProcessIndicate(WHartHandle handle, const WHartAddress& src, WHartPriority priority,
                                             WHartTransportType transportType, const WHartCommandList& list)
{
    // do nothing
}

void ChangeNotificationFlow::notifyDevices()
{
    for (ChangedDevicesList::iterator itDev = changedDevices.begin(); itDev != this->changedDevices.end(); /* uses remove */)
    {
        C839_ChangeNotification_Resp changeNotificationResp;

        Address64 address64 = commonData.networkEngine.getAddress64((uint32_t) itDev->first);
        Device& device = commonData.networkEngine.getDevice((uint32_t) itDev->first);
        LOG_TRACE("notifyDevices() : process device " << ToStr(itDev->first) << ", cmdsNO : " << (int)itDev->second.size());
        if (device.deviceRequested832 == false)
        {
            LOG_TRACE("notifyDevices() : device " << ToStr(itDev->first) << " didn't receive 832");
            ++itDev;
            continue;
        }

        memcpy(changeNotificationResp.DeviceAddress, hart7::util::getUniqueIdFromAddress64(address64).bytes, 5);

        int i = -1;
        for (std::set<DependentCommand>::iterator itCmd = itDev->second.begin(); itCmd != itDev->second.end(); /* uses remove */)
        {

            LOG_TRACE("\tnotifyDevices() : process device " << ToStr(itDev->first)
                        << ", depNickname: " << ToStr(itCmd->dependentNickname)
                        << ", cmdId: " << std::dec << (int) itCmd->commandId );

            if (itCmd->dependentNickname != 0)
            {
                LOG_TRACE("notifyDevices() : dependentDevice: " << ToStr(itCmd->dependentNickname) << "!= 0");

                Device& depDevice = commonData.networkEngine.getDevice((uint32_t) itCmd->dependentNickname);
                if (depDevice.deviceRequested832 == false)
                {
                    LOG_TRACE("\tnotifyDevices() : dependentDevice: " << ToStr(itCmd->dependentNickname)
                                << " didn't received 832, cmdId: " << std::dec << (int) itCmd->commandId);
                    ++itCmd;
                    continue;
                }
            }

            LOG_TRACE("notifyDevices() : device " << ToStr(itDev->first) << " add command : " << (*itCmd).commandId);
            changeNotificationResp.ChangeNotifications[++i] = (*itCmd).commandId;
            itDev->second.erase(itCmd++);
            if (i >= 9)
            {
                LOG_TRACE("\tToo many notifications!!!!");
                break; // maximum 10 notifications in a command
            }
        }

        changeNotificationResp.ChangeNotificationNo = (i >= 0) ? i + 1 : 0;
        if (changeNotificationResp.ChangeNotificationNo < 1)
        {
            // avoid sending empty notifications
            ++itDev;
            continue;
        }

        uint8_t bytes[5]; memcpy(bytes, address64.value, 5);
        hart7::util::NMLog::logCommandResponse(CMDID_C839_ChangeNotification, 0, &changeNotificationResp, WHartAddress(bytes));

        WHartCommand commandsArray[] = { { CMDID_C839_ChangeNotification, 0, &changeNotificationResp } };
        WHartCommandList list = { 1, commandsArray };

        WHartAddress destinationAddress = stack::WHartAddress(Gateway_Nickname());

        try
        {
            NE::Model::Services::Service& service = commonData.utils.GetServiceTo(destinationAddress);

            /* //handle = */
            requestSend.TransmitRequest(destinationAddress, stack::whartpCommand,
                                        (stack::WHartServiceID) service.getServiceId(), stack::wharttPublishNotify,
                                        list, commonData.utils.GetSessionType(destinationAddress), *this);
        }
        catch (...)
        {
            LOG_WARN("No service found for destination=" << destinationAddress);
        }

        LOG_TRACE("notifyDevices() : end process device " << ToStr(itDev->first) << ", cmdsNO : " << (int)itDev->second.size());

        if (itDev->second.size() == 0)
        {
            changedDevices.erase(itDev++);
        }
        else
        {
            ++itDev;
        }
    }
}

}
}
