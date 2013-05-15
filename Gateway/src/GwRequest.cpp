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

#include <WHartGateway/GwRequest.h>
#include <WHartGateway/HostAppSession.h>
#include <WHartGateway/Service.h>
#include <Shared/UtilsSolo.h>

namespace hart7 {
namespace gateway {

CGwRequest::CGwRequest()
{
	m_u16NickNameToSendTo = g_cAddrNicknameInvalid;
	m_nSrc = CGwRequest::SourceUnk;
	m_u16TransactionID = 0;
	m_nHandle = 0;
	m_nStartRetryTimeout = GetClockTicks();
	m_nNextTimeout = m_nStartRetryTimeout;
	m_nRetryNo = 1;
	m_nRequestStatus = RequestStatus_PendingSend;
	m_u8Status = transport::HARTWiredPDU_Response::rcSuccess;
	m_nMessageID = 0;
	m_pHostAppMetaCmd = NULL;
	errorCodeResponse = transport::HARTWiredPDU_Response::rcSuccess;
}

CGwRequest::~CGwRequest()
{
	CService::Ptr pService = m_pService.lock();

	if (pService)
	{
		pService->NotifyRequestCompleted();
	}

	if (m_pHostAppMetaCmd)
	{
		delete m_pHostAppMetaCmd;
	}
}


transport::MessageIP::Ptr CGwRequest::CreateResponse()
{
	transport::MessageIP::Ptr pMsg;

	switch (m_nMessageID)
	{
	case transport::Packet::miHartWiredPDU:
		{
			transport::HARTWiredPDU_Response * pWiredMsg = new transport::HARTWiredPDU_Response;
			pWiredMsg->bypassIOCache = m_bByPassIOCache;

			pWiredMsg->status = transport::HARTWiredPDU_Response::rcErr_DRdead;
			if (!m_oRspList.empty() && m_u8Status == transport::HARTWiredPDU_Response::rcSuccess)
			{
				char pTempBuff[1024 * 64];
				int nLen = HartWired_WriteCmdList(pTempBuff, sizeof(pTempBuff), m_oDevUniqueID, m_oRspList
							, HART_DELIM_ADDR_TYPE_LONG | HART_DELIM_FRAME_TYPE_D2M);

				if (nLen>0)
				{
					pWiredMsg->hartPDU.insert(pWiredMsg->hartPDU.end(), pTempBuff, pTempBuff + nLen );
					pWiredMsg->status = transport::HARTWiredPDU_Response::rcSuccess;
				}
			}

			pMsg.reset(pWiredMsg);
		}
		break;
	case transport::Packet::miHartDirectPDU:
		{
			transport::HARTDirectPDU_Response * pDirectMsg = new transport::HARTDirectPDU_Response;
			pDirectMsg->bypassIOCache = m_bByPassIOCache;

			pDirectMsg->status = transport::HARTDirectPDU_Response::rcErr_DRdead;
			if (!m_oRspList.empty() && m_u8Status == transport::HARTWiredPDU_Response::rcSuccess)
			{
				char pTempBuff[1024*64];
				int nLen = HartDirect_WriteCmdList(pTempBuff, sizeof(pTempBuff), m_oDevUniqueID, m_oRspList);
				if (nLen > 0)
				{
					pDirectMsg->hartPDU.insert(pDirectMsg->hartPDU.end(), pTempBuff, pTempBuff+nLen );
					pDirectMsg->status = transport::HARTDirectPDU_Response::rcSuccess;
				}
			}

			pMsg.reset(pDirectMsg);
		}
		break;
	default:
		//log error
		break;
	}

	if (!pMsg)
	{
		return pMsg;
	}

	pMsg->transactionID = m_u16TransactionID;

	return pMsg;
}


transport::MessageIP::Ptr CGwRequest::CreateHostIndicate(const WHartUniqueID & p_oDeviceAddress, CHartCmdWrapperList & p_oRspList, int p_nMessageId, uint16_t p_u16TransactionId)
{
	transport::MessageIP::Ptr pMsg;

	switch (p_nMessageId)
	{
	case transport::Packet::miHartWiredPDU:
		{
			transport::HARTWiredPDU_Notify *pWiredMsg =  new transport::HARTWiredPDU_Notify;

			pWiredMsg->status = transport::HARTWiredPDU_Notify::rcErr_DRdead;
			if (!p_oRspList.empty())
			{
				char pTempBuff[1024*64];
				int nLen = HartWired_WriteCmdList(pTempBuff, sizeof(pTempBuff), p_oDeviceAddress, p_oRspList
					, HART_DELIM_ADDR_TYPE_LONG | HART_DELIM_FRAME_TYPE_BURST);

				if (nLen>0)
				{
					pWiredMsg->hartPDU.insert(pWiredMsg->hartPDU.end(), pTempBuff, pTempBuff+nLen );
					pWiredMsg->status = transport::HARTWiredPDU_Notify::rcSuccess;
				}
			}

			pMsg.reset(pWiredMsg);
		}
		break;
	case transport::Packet::miHartDirectPDU:
		{
			transport::HARTDirectPDU_Notify *pDirectMsg =  new transport::HARTDirectPDU_Notify;

			pDirectMsg->status = transport::HARTDirectPDU_Notify::rcErr_DRdead;
			if (!p_oRspList.empty())
			{
				char pTempBuff[1024*64];

				int nLen = HartDirect_WriteCmdList(pTempBuff, sizeof(pTempBuff), p_oDeviceAddress, p_oRspList);
				if (nLen > 0)
				{
					pDirectMsg->hartPDU.insert(pDirectMsg->hartPDU.end(), pTempBuff, pTempBuff+nLen );
					pDirectMsg->status = transport::HARTDirectPDU_Notify::rcSuccess;
				}
			}

			pMsg.reset(pDirectMsg);
		}
		break;
	default:
		//log error
		break;
	}

	if (!pMsg)
	{
		return pMsg;
	}

	pMsg->transactionID = p_u16TransactionId;

	return pMsg;
}


transport::MessageIP::Ptr CGwRequest::CreateNotification(CHartCmdWrapper::Ptr & p_pRsp, int p_nMessageId, uint16_t p_u16TransactionId)
{
	transport::MessageIP::Ptr pMsg;

	switch (p_nMessageId)
	{
	case transport::Packet::miHartWiredPDU:
		{
			transport::HARTWiredPDU_Notify * pWiredMsg =  new transport::HARTWiredPDU_Notify;

			pWiredMsg->status = transport::HARTWiredPDU_Notify::rcErr_DRdead;
			if (p_pRsp)
			{
				char pTempBuff[256];
				int nLen = p_pRsp->GetHartWired(pTempBuff, sizeof(pTempBuff), Gateway_UniqueID(), HART_DELIM_ADDR_TYPE_LONG | HART_DELIM_FRAME_TYPE_BURST);

				if (nLen>0)
				{
					pWiredMsg->hartPDU.insert(pWiredMsg->hartPDU.end(), pTempBuff, pTempBuff+nLen );
					pWiredMsg->status = transport::HARTWiredPDU_Notify::rcSuccess;
				}
			}

			pMsg.reset(pWiredMsg);
		}
		break;
	case transport::Packet::miHartDirectPDU:
		{
			transport::HARTDirectPDU_Notify * pDirectMsg =  new transport::HARTDirectPDU_Notify;

			pDirectMsg->status = transport::HARTDirectPDU_Notify::rcErr_DRdead;
			if (p_pRsp)
			{
				char pTempBuff[256];
				int nLen = p_pRsp->GetHartDirect(pTempBuff, sizeof(pTempBuff), Gateway_UniqueID());

				if (nLen > 0)
				{
					pDirectMsg->hartPDU.insert(pDirectMsg->hartPDU.end(), pTempBuff, pTempBuff+nLen );
					pDirectMsg->status = transport::HARTDirectPDU_Notify::rcSuccess;
				}
			}

			pMsg.reset(pDirectMsg);
		}
		break;
	default:
		//log error
		break;
	}

	if (!pMsg)
	{
		return pMsg;
	}

	pMsg->transactionID = p_u16TransactionId;

	return pMsg;
}

CGwRequest::Ptr CGwRequest::ExtractRequest(const stack::transport::MessageIP & packet, bool isFromAppHostClient)
{
	class HostAppProcessor : public stack::transport::IMessageIPVisitor
	{
		LOG_DEF_APP("hart7.gateway.HostAppProcessor");
	public:
	    bool m_isFromAppHostClient;
		CGwRequest::Ptr m_pQReq;
		bool m_bSuccess;
		uint8_t errorCodeResponse;

		HostAppProcessor() { m_isFromAppHostClient = false; errorCodeResponse = transport::HARTWiredPDU_Response::rcSuccess; }

	private:
		virtual void Visit(stack::transport::HARTWiredPDU_Request & req)
		{
			//LOG_INFO_APP("HARTWiredPDU_Request:" << req.ToString());
			int nPos = HartWired_ReadCmdList(false, req.hartPDU.data(), req.hartPDU.size(), m_isFromAppHostClient, m_pQReq->m_oDevUniqueID, m_pQReq->m_oCmdList, errorCodeResponse);
			// LOG_DEBUG("Visit - HARTWiredPDU_Request errorCodeResponse=" << errorCodeResponse);

			m_bSuccess = nPos > 0 && !m_pQReq->m_oCmdList.empty();

			if (m_bSuccess || errorCodeResponse == transport::HARTWiredPDU_Response::rcErr_TooFewDataBytes)
			{
				m_pQReq->m_bByPassIOCache = req.bypassIOCache;
				if ((*m_pQReq->m_oCmdList.begin())->MetaCmdInfoGet())
				{
					if (!m_pQReq->m_pHostAppMetaCmd)
					{
						m_pQReq->m_pHostAppMetaCmd = new TMetaCmdsInfo;
					}
					memcpy(m_pQReq->m_pHostAppMetaCmd, (*m_pQReq->m_oCmdList.begin())->MetaCmdInfoGet(), sizeof(TMetaCmdsInfo));
				}

				m_pQReq->m_nMessageID = transport::Packet::miHartWiredPDU;
			}
			else
			{
				LOG_WARN("ExtractRequest fail! HARTWiredPDU_Request=" << req);

				m_pQReq->m_bByPassIOCache = req.bypassIOCache;
				m_pQReq->m_nMessageID = transport::Packet::miHartWiredPDU;
			}
		}

		virtual void Visit(stack::transport::HARTDirectPDU_Request & req)
		{
			int nPos = HartDirect_ReadCmdList(false, req.hartPDU.data(), req.hartPDU.size(), m_isFromAppHostClient, m_pQReq->m_oDevUniqueID, m_pQReq->m_oCmdList, errorCodeResponse);

			m_bSuccess = nPos > 0 && !m_pQReq->m_oCmdList.empty();

			if (m_bSuccess || errorCodeResponse == transport::HARTWiredPDU_Response::rcErr_TooFewDataBytes)
			{
				m_pQReq->m_bByPassIOCache = req.bypassIOCache;

				if ((*m_pQReq->m_oCmdList.begin())->MetaCmdInfoGet())
				{
					if (!m_pQReq->m_pHostAppMetaCmd)
					{
						m_pQReq->m_pHostAppMetaCmd = new TMetaCmdsInfo;
					}
					memcpy (m_pQReq->m_pHostAppMetaCmd, (*m_pQReq->m_oCmdList.begin())->MetaCmdInfoGet(), sizeof(TMetaCmdsInfo));
				}

				m_pQReq->m_nMessageID = transport::Packet::miHartDirectPDU;
			}
			else
			{
				LOG_WARN("ExtractRequest fail! HARTDirectPDU_Request=" << req);

				m_pQReq->m_bByPassIOCache = req.bypassIOCache;
				m_pQReq->m_nMessageID = transport::Packet::miHartDirectPDU;
			}
		}

	public:
		bool Do(const stack::transport::MessageIP & packet, bool isFromAppHostClient)
		{
		    m_isFromAppHostClient = isFromAppHostClient;
			m_bSuccess = false;
			m_pQReq.reset(new CGwRequest);
			m_pQReq->m_u16TransactionID = packet.transactionID;
			m_pQReq->m_u8Status = packet.status;
			m_pQReq->m_nSrc = CGwRequest::SourceHostApp;

			const_cast<stack::transport::MessageIP&> (packet).Accept(*this);

			m_pQReq->errorCodeResponse = errorCodeResponse;

			if (errorCodeResponse == transport::HARTWiredPDU_Response::rcErr_TooFewDataBytes) {
				m_bSuccess = true;
				LOG_WARN("ExtractRequest fail! packet=" << packet << ", send response: rcErr_TooFewDataBytes");
			}

			return m_bSuccess;
		}
	};

	HostAppProcessor oProcessor;
	if (!oProcessor.Do(packet, isFromAppHostClient))
	{
		return CGwRequest::Ptr();
	}

	return oProcessor.m_pQReq;
}

void CGwRequest::SendDR_DEAD()
{
	if (m_nSrc != CGwRequest::SourceHostApp)
	{
		return;
	}

	if (IsWiredMessage())
	{
		SendDR(RCS_E35_DelayedResponseDead);
	}
	else
	{
		m_u8Status = transport::HARTDirectPDU_Notify::rcErr_DRdead;

		LOG_INFO_APP("HostApp send: " << this << " rsp=NONE req= " << CHartCmdWrapperListShortDetails(m_oCmdList) );

		CHostAppSession::Ptr pSession = m_pHostAppSession.lock();

		if (!pSession)
		{
			LOG_INFO_APP("HostApp session expired for req" << this);
			return;
		}

		pSession->m_oRawSession->SendMessage(*CreateResponse());
	}
}

void CGwRequest::SendDR(uint8_t p_u8DRcode)
{
	CHartCmdWrapperList::iterator itCmdExec = m_oCmdExecList.begin();

	for (; itCmdExec != m_oCmdExecList.end(); ++itCmdExec)
	{
		CHartCmdWrapper::Ptr pRsp (new CHartCmdWrapper);

		pRsp->LoadRaw((*itCmdExec)->GetCmdId(), 0, NULL, p_u8DRcode);

		if (m_pHostAppMetaCmd)
		{
			pRsp->MetaCmdInfoSet(m_pHostAppMetaCmd);
			pRsp->MetaCmdInfoGet()->m_i16MetaCmdResponseCode = p_u8DRcode;
		}

		m_oRspList.push_back(pRsp);
	}

	CHartCmdWrapperList::iterator itCmd = m_oCmdList.begin();

	for (; itCmd != m_oCmdList.end();  ++itCmd)
	{
		CHartCmdWrapper::Ptr pRsp ( new CHartCmdWrapper);

		pRsp->LoadRaw((*itCmd)->GetCmdId(), 0, NULL, p_u8DRcode);

		if (m_pHostAppMetaCmd)
		{
			pRsp->MetaCmdInfoSet(m_pHostAppMetaCmd);
			pRsp->MetaCmdInfoGet()->m_i16MetaCmdResponseCode = p_u8DRcode;
		}

		m_oRspList.push_back(pRsp);
	}

	SendRspList();

	m_oRspList.clear();
}


int CGwRequest::adjustMetaResponse(CHartCmdWrapper::Ptr & p_pRsp, CHartCmdWrapper::Ptr & p_pCmd )
{
	int nNivisSpecificCmdRecv = 0;
	if (!m_pHostAppMetaCmd)
	{
		p_pRsp->MetaCmdInfoDel(); // erase any way
		return nNivisSpecificCmdRecv;
	}

	if ( p_pCmd->MetaCmdInfoGet() && (p_pCmd->MetaCmdInfoGet()->m_u16MetaCmdId == CMDID_C64765_NivisMetaCommand))
	{
		nNivisSpecificCmdRecv = 1;
	}

	//test if is the same metaUniqueID, give priority to the response meta header if m_oMetaCmdUniqueId is null or same as in response
	if ( p_pRsp->MetaCmdInfoGet()
			&& ( m_pHostAppMetaCmd->m_oMetaCmdUniqueId == Zero_UniqueID() || m_pHostAppMetaCmd->m_oMetaCmdUniqueId == p_pRsp->MetaCmdInfoGet()->m_oMetaCmdUniqueId ))
	{
		return nNivisSpecificCmdRecv;
	}

	LOG_DEBUG_APP("CGwRequest::adjustMetaResponse: get meta info from cmd");
	p_pRsp->MetaCmdInfoSet(m_pHostAppMetaCmd);
	p_pRsp->MetaCmdInfoGet()->m_i16MetaCmdResponseCode = RCS_N00_Success;
	p_pRsp->MetaCmdInfoGet()->m_u8MetaCmdDeviceStatus = 0;

	//pRsp->SetResponseCode(RCS_N00_Success);
	//pRsp->SetDeviceStatus(0);
	return nNivisSpecificCmdRecv;
}

void CGwRequest::SendRspList()
{
	if (m_nSrc != CGwRequest::SourceHostApp)
	{
		return;
	}

	CHartCmdWrapperList::iterator itRsp = m_oRspList.begin();
	CHartCmdWrapperList::iterator itCmdExec = m_oCmdExecList.begin();

	int nNivisSpecificCmdRecv = 0;

	for (;itRsp != m_oRspList.end() && itCmdExec != m_oCmdExecList.end(); ++itRsp, ++itCmdExec)
	{
		nNivisSpecificCmdRecv = adjustMetaResponse(*itRsp, *itCmdExec);
	}

	CHartCmdWrapperList::iterator itCmd = m_oCmdList.begin();

	for (;itRsp != m_oRspList.end() && itCmd != m_oCmdList.end(); ++itRsp, ++itCmd)
	{
		nNivisSpecificCmdRecv = adjustMetaResponse(*itRsp, *itCmd);
	}


	if (!m_oRspList.empty())
	{
		LOG_INFO_APP("HostApp send: " << this << " rsp= " << CHartCmdWrapperListShortDetails(m_oRspList) );
        LOG_DEBUG_APP("HostApp send: " << this << " rsp= " << m_oRspList );
	}
	else
	{
		LOG_INFO_APP("HostApp send: " << this << " rsp=NONE req= " << CHartCmdWrapperListShortDetails(m_oCmdList) );
	}

	CHostAppSession::Ptr pSession = m_pHostAppSession.lock();

	if (!pSession)
	{
		LOG_INFO_APP("HostApp session expired for req" << this);
		return;
	}

	if (nNivisSpecificCmdRecv)
	{
		pSession->m_nNivisSpecificCmdRecv = 1;
	}

	pSession->m_oRawSession->SendMessage(*CreateResponse());
}



void CGwRequest::SendNotification(CHostAppSession * p_pSession, CHartCmdWrapper::Ptr & p_pNotifCmd)
{
    LOG_DEBUG_APP("HostApp send: Notification: " << *p_pNotifCmd);
	LOG_INFO_APP("HostApp send: Notification: " << CHartCmdWrapperShortDetails(*p_pNotifCmd.get()) );
	p_pSession->m_oRawSession->SendMessage(*CreateNotification(p_pNotifCmd, p_pSession->m_nMessageID, p_pSession->m_u16CrtNotifTransmit++));
}

void CGwRequest::SendHostIndicate(CHostAppSession * p_pSession, const WHartUniqueID & p_oDeviceAddress, CHartCmdWrapperList & p_oRspList)
{
	LOG_INFO_APP("HostApp send: HostIndicate: src=" << p_oDeviceAddress << " cmds=" << CHartCmdWrapperListShortDetails(p_oRspList) );
    LOG_DEBUG_APP("HostApp send: HostIndicate: src=" << p_oDeviceAddress << " cmds=" << p_oRspList);
	p_pSession->m_oRawSession->SendMessage(*CreateHostIndicate(p_oDeviceAddress, p_oRspList, p_pSession->m_nMessageID, p_pSession->m_u16CrtNotifTransmit++));
}

void CGwRequest::MarkTrySend(int p_nHandle, int p_nRelTimeout)
{
	if (p_nHandle)
	{
		m_nRequestStatus = CGwRequest::RequestStatus_PendingRsp ;
		m_nNextTimeout = GetClockTicks() + p_nRelTimeout;
		m_nRetryNo--;
	}
	else
	{
		m_nRequestStatus =  CGwRequest::RequestStatus_PendingSend;
		m_nNextTimeout = GetClockTicks() + sysconf( _SC_CLK_TCK );
	}

	m_nHandle = p_nHandle;
}

/**
 * Check if the unconfirmed GwRequest match the given confirmed GwRequest.
 */
bool CGwRequest::MatchConfirmedGwRequest(const CGwRequest & confirmedGwRequest) const
{
    bool matchGwRequest = false;

    if (m_nSrc == confirmedGwRequest.m_nSrc && m_oDevUniqueID == confirmedGwRequest.m_oDevUniqueID /*&& m_oCmdList.size() == gwRequest->m_oCmdList.size()*/)
    {
        CHartCmdWrapperList::const_iterator it = m_oCmdList.begin();
        CHartCmdWrapperList::const_iterator itParam = confirmedGwRequest.m_oCmdExecList.begin();

        for(; it != m_oCmdList.end() && itParam != confirmedGwRequest.m_oCmdExecList.end(); ++it)
        {
            if ( (*it)->GetCmdId() != (*itParam)->GetCmdId() )
            {
                break;
            }
            else
            {
                if ( (*it)->GetRawDataLen() != (*itParam)->GetRawDataLen() )
                {
                    break;
                }
                else
                {
                    if ( memcmp((*it)->GetRawData(), (*itParam)->GetRawData(), (*it)->GetRawDataLen()) != 0 )
                    {
                        break;
                    }
                    else
                    {
                        ++itParam;
                    }
                }
            }
        }

        if (it == m_oCmdList.end() && itParam == confirmedGwRequest.m_oCmdExecList.end())
        {
            matchGwRequest = true;
        }
    }
    return matchGwRequest;
}

uint8_t CGwRequest::MatchCacheBasedDRM(const WHartUniqueID & p_rUniqueID, CHartCmdWrapper::Ptr & p_pCmd)
{
	if ( !(p_rUniqueID == m_oDevUniqueID))
	{
		return RCS_E33_DelayedResponseInitiated;
	}

	CHartCmdWrapperList::iterator itCmdExec = m_oCmdExecList.begin();

	for (; itCmdExec != m_oCmdExecList.end(); ++itCmdExec)
	{
		CHartCmdWrapper::Ptr pCrtCmd = *itCmdExec;
		if (p_pCmd->GetCmdId() != pCrtCmd->GetCmdId())
		{
			continue;
		}

		/// TODO: improve the DRM detection, when and if it will be used
		if (p_pCmd->GetRawDataLen() != pCrtCmd->GetRawDataLen())
		{
			return RCS_E36_DelayedResponseConflict;
		}

		if (memcmp(p_pCmd->GetRawData(), pCrtCmd->GetRawData(), pCrtCmd->GetRawDataLen()) == 0)
		{
			return RCS_E34_DelayedResponseRunning;
		}
		else
			return RCS_E36_DelayedResponseConflict;

	}

	CHartCmdWrapperList::iterator itCmd = m_oCmdList.begin();

	for (; itCmd != m_oCmdList.end();  ++itCmd)
	{
		CHartCmdWrapper::Ptr pCrtCmd = *itCmd;
		if (p_pCmd->GetCmdId() != pCrtCmd->GetCmdId())
		{
			continue;
		}

		if (p_pCmd->GetRawDataLen() != pCrtCmd->GetRawDataLen())
		{
			return RCS_E36_DelayedResponseConflict;
		}

		if (memcmp(p_pCmd->GetRawData(), pCrtCmd->GetRawData(), pCrtCmd->GetRawDataLen()) == 0)
		{
			return RCS_E34_DelayedResponseRunning;
		}
		else
			return RCS_E36_DelayedResponseConflict;
	}

	return RCS_E33_DelayedResponseInitiated;
}


const char * CGwRequest::GetSourceName(int p_nSrc)
{

	switch(p_nSrc)
	{
	case SourceHostApp:
	    return "SourceHostApp";
	case SourceLocalGw:
		return "SourceLocalGw";
	default:
	    return "SourceUnk";
	}
	return "SourceUnk";
}


std::ostream & operator<< (std::ostream & out, const CGwRequest * p_pReq )
{
	if (!p_pReq)
	{
		out << " GwReq: NONE";
		return out;
	}
	out << " GwReq: " << (p_pReq->IsWiredMessage() ? "wired " : "direct ") << CGwRequest::GetSourceName(p_pReq->m_nSrc) << " transId=" << (int)p_pReq->m_u16TransactionID << " dest=" << p_pReq->m_oDevUniqueID;
	return out;
}


} // namespace gateway
} // namespace hart7
