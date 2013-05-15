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
 * LocalManagement.h
 *
 *  Created on: Jan 13, 2009
 *      Author: Andy
 */

#ifndef LOCALMANAGEMENT_H_
#define LOCALMANAGEMENT_H_


#include <WHartStack/WHartStack.h>
#include <WHartStack/WHartSubApplicationData.h>
#include <WHartStack/WHartTransportData.h>
#include <WHartStack/WHartNetworkData.h>
#include <WHartStack/WHartDataLinkData.h>

#include <nlib/log.h>
#include <nlib/socket/ServiceSocket.h>
#include <nlib/timer/Timer.h>

namespace hart7 {
namespace stack {

class LocalManagement
{
	LOG_DEF("h7.s.LocalManagement");
public:
	LocalManagement(CommonData& commonData, subapp::WHartSubApplicationData& subApplication, transport::WHartTransportData& transport,
			network::WHartNetworkData& network, datalink::WHartDataLinkData& dataLink,
			nlib::socket::ServiceSocket::Ptr service, nlib::timer::TimerFactory::Ptr timerFactory);

    void WriteNickname(WHartUniqueID uniqueID, WHartShortAddress nickname, bool isTR);

    void WriteKey(C963_WriteSession_Req& req);

    void WriteRoute(C974_WriteRoute_Req& req);

    void DeleteRoute(C975_DeleteRoute_Req& req);

	int ReadRouteList(C802_ReadRouteList_Req* req, C802_ReadRouteList_Resp* resp);

    void WriteSourceRoute(C976_WriteSourceRoute_Req& req);

    void DeleteSourceRoute(C977_DeleteSourceRoute_Req& req);

    int ReadSourceRoute(C803_ReadSourceRoute_Req* req, C803_ReadSourceRoute_Resp* resp);

    void WriteService(C973_WriteService_Req& req);

    void DeleteService(C801_DeleteService_Req& req);

    void AddGraph(C969_WriteGraphNeighbourPair_Req& req);

    void DeleteGraph(C970_DeleteGraphConnection_Req& req);

	void ResetStack();

	void RemoveJoinKeys();

	void SetOnJoinCallback(boost::function1<void,  WHartAddress&> callback);

	void DeleteSession(const WHartAddress& address, WHartSessionKey::SessionKeyCode sessionType);

	void SetSession(C963_WriteSession_Req& req);

private:
	void NewConnectionSucceeded(WHartUniqueID id, bool isMaster);
	void TimeElapsed(int time);

private:
	CommonData& commonData;
	subapp::WHartSubApplicationData& subApplication;
	transport::WHartTransportData& transport;
	network::WHartNetworkData& network;
	datalink::WHartDataLinkData& dataLink;
	nlib::socket::ServiceSocket::Ptr service;
	nlib::timer::TimerFactory::Ptr timerFactory;
    nlib::socket::Timer::Ptr timer;
};

} // namespace stack
} // namespace hart7

#endif /* LOCALMANAGEMENT_H_ */
