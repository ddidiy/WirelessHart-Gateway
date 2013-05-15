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

#ifndef HostAppSession_h__
#define HostAppSession_h__

#include <boost/shared_ptr.hpp>
#include <boost/shared_array.hpp>
#include <WHartStack/WHartTypes.h>
#include <WHartStack/WHartStack.h>

#include <WHartStack/CommInterfaceOverIP/UDPServerIP.h>
#include <WHartStack/CommInterfaceOverIP/MessageIPSerializer.h>
#include <WHartStack/util/WHartCmdWrapper.h>
#include "GwRequest.h"

#include "GatewayTypes.h"

#include <list>
#include <map>

namespace hart7 {
namespace gateway {

	typedef std::map<WHartUniqueID, uint16_t, CompareUniqueID> CHostAppDevNotifMap;

	/**
	 * Session with a Host Application. Handles data flows.
	 */
	class CHostAppSession
	{
	public:
		CHostAppSession() { m_u16CrtNotifTransmit = 1; m_nMessageID = -1; m_nNivisSpecificCmdRecv = 0; }
	public:
		typedef boost::shared_ptr<CHostAppSession> Ptr;
		typedef boost::weak_ptr<CHostAppSession> WeakPtr;

	public:
		/**
		 * Adds a request for the host application connected on this session.
		 */
		bool RequestAdd (CGwRequest::Ptr & p_pReq);
		/**
		 * Removes a request for the host application connected on this session.
		 */
		void RequestRemove(CGwRequest::Ptr & p_pReq);

		/**
		 * True if message is Hart Wired PDU.
		 */
		bool IsWiredFormat() {  return m_nMessageID == transport::Packet::miHartWiredPDU; }

	public:
		CHostAppDevNotifMap		m_oDevNotifMaskMap;
		int						m_nNivisSpecificCmdRecv;
		uint16_t				m_u16CrtNotifTransmit;
		int						m_nMessageID;

		CHostAppReqTransMap m_oRequests;
		stack::transport::SessionIP::Ptr m_oRawSession;
	};

	typedef std::map<stack::transport::SessionIP::Ptr, CHostAppSession::Ptr> CHostAppSessionTransIdMap;

	/**
	 * Aggregator for Host Application sessions.
	 */
	class CHostAppSessions
	{
	public:
		/**
		 * Adds a Host APP session.
		 */
		void SessionAdd(const std::string & p_oHost, const stack::transport::SessionIP::Ptr & p_oRawSession);
		/**
		 * Removes a Host APP session.
		 */
		void SessionRemove(const stack::transport::SessionIP::Ptr & p_oRawSession);

		/**
		 * Adds a request for the host app connected on the required session.
		 */
		bool RequestAdd(const stack::transport::SessionIP::Ptr & p_oRawSession, CGwRequest::Ptr & p_oReq);
		/**
		 * Removes a request for the host app connected on the required session.
		 */
		void RequestRemove(const stack::transport::SessionIP::Ptr & p_oRawSession, CGwRequest::Ptr & p_oReq);

		/**
		 * Returns the Host APP session connected on the specified UDP session.
		 */
		CHostAppSession::Ptr GetSession(const stack::transport::SessionIP::Ptr & p_oRawSession);

		/**
		 * Returns all the Host APP sessions.
		 */
		CHostAppSessionTransIdMap& GetSessionsTable() { return m_oSessions; }
	private:
		CHostAppSessionTransIdMap m_oSessions;
	};


} // namespace gateway
} // namespace hart7

#endif // HostAppSession_h__

