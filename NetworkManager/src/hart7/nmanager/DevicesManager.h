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
 * DevicesManager.h
 *
 *  Created on: May 25, 2009
 *      Author: andrei.petrut
 */

#ifndef DEVICESMANAGER_H_
#define DEVICESMANAGER_H_

#include <map>
#include <nlib/log.h>
#include <WHartStack/WHartStack.h>

#include "CommonData.h"
#include "FlowHandlers/JoinFlowHandler.h"
#include "FlowHandlers/QuarantineToOperationalFlowHandler.h"
#include "FlowHandlers/ChangeNotificationFlow.h"
#include "FlowHandlers/EvaluateRouteFlow.h"
#include "FlowHandlers/VisibleEdgeFlow.h"
#include "IPeriodicTask.h"
#include "operations/WHOperationQueue.h"
#include "alarms/NodeAlarmVerifier.h"
#include "alarms/AlarmsProcessor.h"
#include "reports/ReportListener.h"
#include "reports/ReportsProcessor.h"
#include "FlowHandlers/NewAdvertiseLinkOnTrFlow.h"
#include "nmodel/HartDevice.h"
#include "FlowHandlers/EvaluateClkSourceFlow.h"

#include "FlowHandlers/AggressiveAdvertisingFlow.h"

#include "script/ScriptManager.h"

namespace hart7 {
namespace nmanager {

/**
 * Handles all flows and operations for the devices.
 * Join flows are initiated from here.
 * Q2O flows are initiated from here.
 * TR advertise bandwidth is handled from here.
 * Route evaluations are initiated from here.
 */
class DevicesManager: public IPeriodicTask
{
        LOG_DEF("h7.n.DevicesManager");

    public:

        /**
         * Constructor.
         */
        DevicesManager(IRequestSend& requestSend, CommonData& commonData,
                       operations::WHOperationQueue& operationsQueue);

        virtual ~DevicesManager()
        {
        }
        /**
         * Handles a Join request. Initiates a J Req flow.
         */
        void DeviceRegistrationRequest(const stack::WHartAddress& source, const stack::WHartCommandList& commands);

        /**
         * Marks the device was changed, for change notifications.
         */
        void markAsChangedFromOperation(IEngineOperation& operation, uint32_t peerAddress);

        /**
         * Marks the device was changed, for change notifications.
         */
        void markAsChanged(uint16_t address, uint16_t commandId);

        /**
         * Returns the number of active join flow handlers.
         */
        uint8_t getJoinsInProgressNo(Address32 address32, Address32 parentAddress32, std::set<Address32>& addresses);

        static bool saveTopoloyToDisk;

        /**
         * Handles the SIGUSR2 signal. Logs all tables.
         */
        static void HandlerSIGUSR2(int signal);

        static bool LogAllInfo;

        /**
         * Reload config was requested, set up for it.
         */
        void markForReloadingConfiguration();


        /// SCRIPT Utils
        /**
         * Deletes a device from the network.
         */
        void DeleteDevice(uint16_t nickname);
        /**
         * Changes the NetworkID of a device.
         */
        void ChangeNetworkID(uint16_t nickname, uint16_t newNetId);

        /// END SCRIPT Utils

        /**
         * This callback is invoked when the config files must be read.
         */
        boost::function0<void> managerReloadConfigFilesHandler;


    private:

        void JoinFinished(const HartDevice & device, bool status);

        void newAdvertiseLinkOnTrFlowFinished(bool errorOccurred);

        void OperationalFinished(const HartDevice & device, bool status);

        void PeriodicTask();

        void CheckDevicesReadyForOperational();

        void SendDeviceToOperational(const HartDevice & device);

        void stopJoin(const stack::WHartAddress & source);

        void checkVisibleEdges();

        // the handler called when a visible edge flow ends
        void VisibleEdgeFinished(uint16_t src, uint16_t dest, bool status);

        void checkTransceiverAdvertiseBandwidth();

        void stopAdvLinkFlowOnTR(uint16_t device);

        void compactLinks();

        void ConfirmChangeNetworkIDOperations(bool success, NE::Model::Operations::EngineOperationsListPointer operations);

    private:

        IRequestSend& requestSend;

        CommonData& commonData;

        operations::WHOperationQueue& operationsQueue;

        std::map<stack::WHartAddress, JoinFlowHandler> joinHandlers;

        typedef std::map<stack::WHartAddress, QuarantineToOperationalFlowHandler> QuarantineHandlersMap;

        QuarantineHandlersMap quarantineHandlers;

        std::set<HartDevice> quarantinedDevices;

        ChangeNotificationFlow changeNotification;

        EvaluateRouteFlow evaluateRoute;

        std::map<VisibleEdge, VisibleEdgeFlowPointer> visibleEdgeHandlers;

        NodeAlarmVerifierPointer nodeAlarmVerifier;

        AlarmsProcessor alarmsProcessor;

        ReportsProcessor reportsProcessor;

        NewAdvertiseLinkOnTrFlowPointer flowTr;

        NE::Model::Operations::EngineOperationsListPointer advertiseEngineOperations;

        ScriptManager scriptManager;

        EvaluateClkSourceFlow evaluateClkSrc;

        AggressiveAdvertisingFlow agressiveAdvFlow;
        /**
         * Set to true when the config files must be read.
         */
        bool flagMarkedForReloadingConfiguration;
};

}
}
#endif /* DEVICESMANAGER_H_ */
