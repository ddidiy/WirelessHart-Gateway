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

#ifndef GwRequest_h__
#define GwRequest_h__

#include <boost/shared_ptr.hpp>
#include <boost/shared_array.hpp>
#include <WHartStack/WHartTypes.h>
#include <WHartStack/WHartStack.h>

#include <WHartStack/CommInterfaceOverIP/UDPServerIP.h>
#include <WHartStack/CommInterfaceOverIP/MessageIPSerializer.h>


#include "GatewayTypes.h"
#include <WHartStack/util/WHartCmdWrapper.h>
#include <Shared/Common.h>

#include <list>
#include <map>

namespace hart7 {
namespace gateway {

	using namespace stack;

	class CHostAppSession;
	class CService;

	/**
	 * Facade for sending requests through the stack. Supports sending all types of commands, either
	 * from the GW or on behalf of the Host Application.
	 */
	class CGwRequest
	{
	public:
		typedef boost::shared_ptr<CGwRequest> Ptr;

		enum {SourceUnk = 0, SourceHostApp, SourceLocalGw};
		enum
		{
			RequestStatus_Init,
			RequestStatus_PendingSend,
			RequestStatus_PendingRsp,
			RequestStatus_ResponseReceived,
			RequestStatus_Cancel
		};

	public:
		CGwRequest();
		~CGwRequest();
		transport::MessageIP::Ptr CreateResponse();
		static transport::MessageIP::Ptr CreateNotification(CHartCmdWrapper::Ptr & p_pRsp, int p_nMessageId, uint16_t p_u16TransactionId) ;
		static transport::MessageIP::Ptr CreateHostIndicate(const WHartUniqueID & p_oDeviceAddress, CHartCmdWrapperList & p_oRspList,
															int p_nMessageId, uint16_t p_u16TransactionId) ;

		/**
		 * @packet
		 * @isFromAppHostClient is true for packets received from App Host Clients(HostApp, HCF Tester, ...);
		 * otherwise is false;
		 */
		static CGwRequest::Ptr ExtractRequest(const stack::transport::MessageIP & packet, bool isFromAppHostClient);


		void SendRspList();
		void SendDR(uint8_t p_u8DRcode);
		void SendDR_DEAD();

		int adjustMetaResponse(CHartCmdWrapper::Ptr & p_pRsp, CHartCmdWrapper::Ptr & p_pCmd);

		static void SendNotification(CHostAppSession * p_pSession, CHartCmdWrapper::Ptr & p_pNotifCmd);
		static void SendHostIndicate(CHostAppSession * p_pSession, const WHartUniqueID & p_oDeviceAddress, CHartCmdWrapperList & p_oRspList);

		void MarkTrySend(int p_nHandle, int p_nRelTimeout);

		bool IsWiredMessage() const { return m_nMessageID == transport::Packet::miHartWiredPDU;}

		uint8_t MatchCacheBasedDRM(const WHartUniqueID & p_rUniqueID, CHartCmdWrapper::Ptr & p_pCmd );
		static const char* GetSourceName(int p_nSrc);

		/**
		 * Check if the unconfirmed GwRequest match the given confirmed GwRequest.
		 */
		bool MatchConfirmedGwRequest(const CGwRequest & gwRequest) const;

	private:

	public:
		int			m_nSrc;
		boost::weak_ptr<CHostAppSession> 	m_pHostAppSession;
		boost::weak_ptr<CService> 			m_pService;

		WHartUniqueID		m_oDevUniqueID;
		WHartShortAddress	m_u16NickNameToSendTo;

		uint16_t		m_u16TransactionID;

		uint8_t			m_u8Status;
		int				m_nMessageID;
		bool			m_bByPassIOCache;
		TMetaCmdsInfo*			m_pHostAppMetaCmd;

		CHartCmdWrapperList		m_oCmdList;
		CHartCmdWrapperList		m_oCmdExecList;
		CHartCmdWrapperList		m_oRspList;

		// Start retry timeout period
        clock_t     m_nStartRetryTimeout;

		clock_t		m_nNextTimeout;
		int			m_nRetryNo;//when 0 erase the request
		int			m_nHandle;
		int			m_nRequestStatus;

		uint8_t		errorCodeResponse;
	};

	typedef std::map<uint16_t, CGwRequest::Ptr> CHostAppReqTransMap;

	typedef std::list<CGwRequest::Ptr> CGwRequests;

	std::ostream& operator<<(std::ostream & out, const CGwRequest * p_pReq);

} // namespace gateway
} // namespace hart7

#endif // GwRequest_h__
