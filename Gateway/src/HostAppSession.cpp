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

#include <WHartGateway/HostAppSession.h>

namespace hart7 {
namespace gateway {


bool CHostAppSession::RequestAdd(CGwRequest::Ptr & p_pReq )
{
	CHostAppReqTransMap::iterator itReq = m_oRequests.find(p_pReq->m_u16TransactionID);

	if (itReq != m_oRequests.end())
	{
		LOG_ERROR_APP("A request with same transactionID already in queue; transactionID=" << (int)p_pReq->m_u16TransactionID);
		return false;
	}

	m_oRequests.insert(std::make_pair(p_pReq->m_u16TransactionID, p_pReq));

	return true;
}

void CHostAppSession::RequestRemove(CGwRequest::Ptr & p_pReq)
{
	CHostAppReqTransMap::iterator itReq = m_oRequests.find(p_pReq->m_u16TransactionID);

	if (itReq == m_oRequests.end())
	{
		LOG_ERROR_APP("Request with transactionID=" << (int)p_pReq->m_u16TransactionID << " already removed.");
		return;
	}

	m_oRequests.erase(itReq);
}

void CHostAppSessions::SessionAdd(const std::string & p_oHost, const stack::transport::SessionIP::Ptr & p_oRawSession)
{
	CHostAppSessionTransIdMap::iterator itSession = m_oSessions.find(p_oRawSession);

	if (itSession != m_oSessions.end())
	{
		LOG_ERROR_APP("A session already exists; session=" << *p_oRawSession);
		return;
	}

	CHostAppSession::Ptr pSession (new CHostAppSession);

	pSession->m_oRawSession = p_oRawSession;
	m_oSessions.insert(std::make_pair(p_oRawSession, pSession));
}

void CHostAppSessions::SessionRemove(const stack::transport::SessionIP::Ptr & p_oRawSession)
{
	CHostAppSessionTransIdMap::iterator itSession = m_oSessions.find(p_oRawSession);

	if (itSession == m_oSessions.end())
	{
		LOG_ERROR_APP("CHostAppSessions::SessionRemove: already removed");
		return;
	}

	CHostAppSession::Ptr pSession = itSession->second;

	m_oSessions.erase(itSession);

	//mark as cancel all pending requests, and not remove directly in order to allow that responses to be cached when will come
	CHostAppReqTransMap::iterator itReq = pSession->m_oRequests.begin();

	for (;itReq != pSession->m_oRequests.end();itReq++)
	{
		itReq->second->m_nRequestStatus = CGwRequest::RequestStatus_Cancel;
	}
}

void CHostAppSessions::RequestRemove(const stack::transport::SessionIP::Ptr & p_oRawSession, CGwRequest::Ptr & p_pReq )
{
	CHostAppSessionTransIdMap::iterator itSession = m_oSessions.find(p_oRawSession);

	if (itSession == m_oSessions.end())
	{
		//the session does not exist
		return ;
	}

	CHostAppSession::Ptr pSession = itSession->second;

	pSession->RequestRemove(p_pReq);
}

bool CHostAppSessions::RequestAdd(const stack::transport::SessionIP::Ptr & p_oRawSession, CGwRequest::Ptr & p_pReq)
{
	CHostAppSessionTransIdMap::iterator itSession = m_oSessions.find(p_oRawSession);

	if (itSession == m_oSessions.end())
	{
		LOG_ERROR_APP("The session does not exist; session=" << *p_oRawSession);
		return false;
	}

	return itSession->second->RequestAdd(p_pReq);
}

CHostAppSession::Ptr CHostAppSessions::GetSession(const stack::transport::SessionIP::Ptr & p_oRawSession)
{
	CHostAppSession::Ptr pSession;

	CHostAppSessionTransIdMap::iterator itSession = m_oSessions.find(p_oRawSession);

	if (itSession != m_oSessions.end())
	{
		pSession = itSession->second;;
	}

	return pSession;
}

} // namespace gateway
} // namespace hart7
