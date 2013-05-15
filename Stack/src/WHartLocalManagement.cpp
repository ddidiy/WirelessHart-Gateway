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
 * LocalManagement.cpp
 *
 *  Created on: Jan 13, 2009
 *      Author: Andy
 */
#include <WHartStack/WHartLocalManagement.h>

#include <boost/bind.hpp>


namespace hart7 {
namespace stack {

const int TIMER_INTERVAL = 1000;// 250;

LocalManagement::LocalManagement(CommonData& commonData_, subapp::WHartSubApplicationData& subApplication_, transport::WHartTransportData& transport_,
		network::WHartNetworkData& network_, datalink::WHartDataLinkData& dataLink_, nlib::socket::ServiceSocket::Ptr service_,
		nlib::timer::TimerFactory::Ptr timerFactory_)
	: commonData(commonData_), subApplication(subApplication_), transport(transport_), network(network_), dataLink(dataLink_), service(service_),
	timerFactory(timerFactory_)
{
	//    timer = timerFactory->CreateDeadlineTimer(TIMER_INTERVAL);
	//    timer->Elapsed = boost::bind(&LocalManagement::TimeElapsed, this, _1);
	timer = service->CreateDeadLineTimer();
	timer->Elapsed = boost::bind(&LocalManagement::TimeElapsed, this, _1);
	timer->Start(TIMER_INTERVAL);
}

void LocalManagement::TimeElapsed(int time)
{
	dataLink.TimeElapsed(time);
	transport.TimePassed(time);
}

void LocalManagement::ResetStack()
{
	dataLink.Reset();
	network.Reset();
	transport.Reset();
	commonData.isJoined = false;
}

struct RemoveJoinSession
{
	bool operator()(const hart7::stack::network::Session& session)
	{
		return (session.sessionKey.keyCode == WHartSessionKey::joinKeyed);
	}
};

void LocalManagement::RemoveJoinKeys()
{

	network.sessionTable.erase(std::remove_if(network.sessionTable.begin(), network.sessionTable.end(), RemoveJoinSession()),
			network.sessionTable.end());
}


void LocalManagement::WriteKey(C963_WriteSession_Req& req) {
    C963_WriteSession_Resp resp;
    network.WriteSession(&req, &resp);
    LOG_DEBUG("Add session type:" << (int) req.m_eSessionType << " for: 0x" << std::hex << req.m_unPeerNickname <<
    		" with uniqueID=0x" << WHartAddress(req.m_aPeerUniqueID));
}

void LocalManagement::WriteNickname(WHartUniqueID uniqueID, WHartShortAddress nickname, bool isTR)
{
	//TODO check if always master
	transport.RemoveTableEntry(WHartAddress(nickname), true);
	transport.AddTableEntryWithUniqueId(uniqueID, nickname, true, (isTR ? false : true));
}

void LocalManagement::WriteRoute(C974_WriteRoute_Req& req)
{
	C974_WriteRoute_Resp resp;
	network.WriteRoute(&req, &resp);
	LOG_DEBUG("Add route:" << (int)req.m_ucRouteID << " for: 0x" << std::hex << req.m_unPeerNickname);
}

void LocalManagement::DeleteRoute(C975_DeleteRoute_Req& req)
{
	C975_DeleteRoute_Resp resp;
	network.DeleteRoute(&req, &resp);
	LOG_DEBUG("Delete route:" << (int)req.m_ucRouteID);
}

int LocalManagement::ReadRouteList(C802_ReadRouteList_Req* req, C802_ReadRouteList_Resp* resp)
{
	return network.ReadRouteList(req, resp);
}

void LocalManagement::WriteSourceRoute(C976_WriteSourceRoute_Req& req)
{
	C976_WriteSourceRoute_Resp resp;
	network.WriteSourceRoute(&req, &resp);
	LOG_DEBUG("Add source route:" << (int)req.m_ucRouteID);
}

void LocalManagement::DeleteSourceRoute(C977_DeleteSourceRoute_Req& req)
{
	C977_DeleteSourceRoute_Resp resp;
	network.DeleteSourceRoute(&req, &resp);
	LOG_DEBUG("Delete source route:" << (int)req.m_ucRouteID);
}

int LocalManagement::ReadSourceRoute(C803_ReadSourceRoute_Req* req, C803_ReadSourceRoute_Resp* resp)
{
	return network.ReadSourceRoute(req, resp);
}

void LocalManagement::WriteService(C973_WriteService_Req& req) {
    C973_WriteService_Resp resp;
    network.WriteService(&req, &resp);
    LOG_DEBUG("Add service:" << (int) req.m_ucServiceID << " for: 0x" << std::hex << req.m_unPeerNickname);
}

void LocalManagement::DeleteService(C801_DeleteService_Req& req)
{
    C801_DeleteService_Resp resp;
	network.DeleteService(&req, &resp);
    LOG_DEBUG("Removed service:" << (int) req.m_ucServiceId);
}


void LocalManagement::AddGraph(C969_WriteGraphNeighbourPair_Req& req) {
    dataLink.AddConnection(req.m_unGraphID, req.m_unNeighborNickname);
    LOG_DEBUG("Add graph:" << (int) req.m_unGraphID << " for: 0x" << std::hex << req.m_unNeighborNickname);
}

void LocalManagement::DeleteGraph(C970_DeleteGraphConnection_Req& req) {
    dataLink.DeleteConnection(req.m_unGraphID, req.m_unNeighborNickname);
    LOG_DEBUG("Delete graph:" << (int) req.m_unGraphID << " for: 0x" << std::hex << req.m_unNeighborNickname);
}

void LocalManagement::SetOnJoinCallback(boost::function1<void, WHartAddress&> callback){
    network.OnJoinCallback = callback;
    LOG_DEBUG("OnJoinCallback set");
}

void LocalManagement::SetSession(C963_WriteSession_Req& req){
    network.SetSession(&req);
    LOG_DEBUG("Set session type:" << (int) req.m_eSessionType << " for: 0x" << std::hex << req.m_unPeerNickname <<
            " with uniqueID=0x" << WHartAddress(req.m_aPeerUniqueID));
}

void LocalManagement::DeleteSession(const WHartAddress& address, WHartSessionKey::SessionKeyCode sessionType)
{
	network.DeleteSession(address, sessionType);
}


}
}
