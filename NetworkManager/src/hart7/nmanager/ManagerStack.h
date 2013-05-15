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
 * ManagerStack.h
 *
 *  Created on: Dec 05, 2008
 *      Author: Radu Pop
 */

#ifndef ManagerStack_H_
#define ManagerStack_H_

#include <WHartStack/WHartStack.h>
#include <WHartStack/WHartSubApplicationData.h>
#include <WHartStack/WHartTransportData.h>
#include <WHartStack/WHartNetworkData.h>
#include <WHartStack/WHartDataLinkData.h>
#include <WHartStack/WHartLocalManagement.h>
#include <ApplicationLayer/ApplicationCommand.h>
#include <ApplicationLayer/Model/DataLinkLayerCommands.h>
#include <ApplicationLayer/Model/GatewayCommands.h>
#include <ApplicationLayer/Model/UniversalCommands.h>
#include "AllNetworkManagerCommands.h"

#include "../../NMSettingsLogic.h"
#include <nlib/log.h>
#include <nlib/socket/ServiceSocket.h>
#include <nlib/timer/Timer.h>
#include <string>
#include <boost/function.hpp>
#include "IProvisioning.h"

namespace hart7 {

namespace nmanager {

typedef std::basic_string<uint8_t> Payload;

using namespace hart7::stack;

/**
 * Application layer for the stack.
 */
class ManagerStack: public WHartApplication
{

    LOG_DEF("h7.n.ManagerStack");

    public:

        static const WHartHandle LOCALMANAGEMENT_HANDLE = 0xFFFF;

        ManagerStack(hart7::util::NMSettingsLogic& settingsLogic);

        ~ManagerStack();

        /**
         * Transmit request.
         * @to - destination
         * @serviceID
         * @req - command request
         *
         * @return the handled associated with request, an confirm will have same handle (for mapping response to request)
         */
        WHartHandle TransmitRequest(const WHartAddress& to, WHartPriority priority, WHartTransportType transportType,
                                    WHartServiceID serviceID, const WHartCommandList& req,
                                    WHartSessionKey::SessionKeyCode sessionCode);

        /**
         * Call back confirm.
         * @res is filled only for ack service
         */
        boost::function3<void, WHartHandle, WHartLocalStatus, const WHartCommandList& /*res*/> TransmitConfirmCallBack;

        /**
         * Call back indicate.
         * @res is filled only for ack service
         */
        boost::function5<void, WHartHandle, const WHartAddress& /*src*/, WHartPriority /*priority*/,
                    WHartTransportType /*transportType*/, const WHartCommandList& /*res*/> TransmitIndicateCallBack;

        /**
         * Transmit response.
         */
        void TransmitResponse(WHartHandle handle, WHartServiceID serviceID, const WHartCommandList& res,
                              WHartSessionKey::SessionKeyCode sessionCode);

        bool CanSendToDestination(const WHartAddress& dest);

        /**
         *
         */
        void SetKeepAliveHandler(const boost::function1<void, uint32_t>& keepAliveHandler);

        void Start();

        void Stop();

        void WriteNickname(C2009_NM_WriteUniqueIdNickname_Req& req);

        void WriteKey(C963_WriteSession_Req& req);

        void WriteRoute(C974_WriteRoute_Req& req);

        void WriteService(C973_WriteService_Req& req);

        void DeleteService(C801_DeleteService_Req& req);

        void AddGraph(C969_WriteGraphNeighbourPair_Req& req);

        void DeleteGraph(C970_DeleteGraphConnection_Req& req);

        void StartGWSession(int networkManagerListenPort, int networkManagerMinRangePort,
                            int networkManagerMaxRangePort);

        void NewGWSession(const std::string& /*host*/, const stack::transport::SessionIP::Ptr);

        void GWSessionClosed(const stack::transport::SessionIP::Ptr);

        void initGWService();

        /**
         * Creates session with gateway
         */
        void initGWJoinKey();

        /**
         * Creates session with aps
         */
        void initAPsJoinKey();

        void initJoinKeys();

        /**
         * Called from network layer when join flow is detected
         */
        void OnJoin(const WHartAddress& address);

        void setProvisioning();

    private:

        void TransmitConfirm(WHartHandle requestHandle, const WHartLocalStatus& localStatus,
                             DeviceIndicatedStatus status, const WHartCommandList& list);

        void TransmitIndicate(WHartHandle handle, const WHartAddress& src, WHartPriority priority,
                              DeviceIndicatedStatus status, WHartTransportType transportType,
                              const WHartCommandList& list);

        void PeriodicTask();

    private:

        void ProcessCommands(const WHartCommandList& req);

    private:

        // must be before localManagement, since localManagement uses it
        nlib::socket::ServiceSocket::Ptr service;

        hart7::util::NMSettingsLogic& settingsLogic;

        stack::CommonData commonData;

    public:

        stack::transport::WHartTransportData transport;

    private:

        stack::datalink::WHartDataLinkData datalink;

        transport::ServerIP::Ptr server; //listen for gw session

        nlib::socket::Timer::Ptr timer;

        nlib::timer::TimerFactory::Ptr timerFactory;

        boost::function1<void, uint32_t> keepAliveHandler;

    public:

        stack::network::WHartNetworkData network;

        stack::subapp::WHartSubApplicationData subapp;

        stack::LocalManagement localManagement;

        hart7::nmanager::IProvisioning::Ptr provisioning;

};

}
}

#endif /* ManagerStack_H_ */
