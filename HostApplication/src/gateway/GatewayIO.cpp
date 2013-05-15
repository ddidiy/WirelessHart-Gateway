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

#include <WHartHost/gateway/GatewayIO.h>
#include <WHartStack/CommInterfaceOverIP/MessageIP.h>

#include <WHartHost/model/MAC.h>

#include "GatewayTrackingMng.h"
#include "GatewayChannel.h"
#include "GatewayProcessor.h"
#include "Table.h"

#include <boost/bind.hpp>


#include <WHartHost/Utils.h>

namespace hart7 {
namespace gateway {



GatewayIO::GatewayIO(const nlib::TimeSpan &requestTimedOut, bool allowSameCmdOnDev):
        m_allowSameCmdOnDev(allowSameCmdOnDev), m_requestTimedOut(requestTimedOut)
{
	m_pChannel = new GatewayChannel();
	m_pTrackingMng = new GatewayTrackingMng();

	//bind tracking
	m_pTrackingMng->m_sendMessage = boost::bind(&GatewayChannel::SendMessage, m_pChannel, _1);
	m_pTrackingMng->m_messageReceived = boost::bind(&GatewayIO::ReceiveMessage, this, _1, _2);
	m_pTrackingMng->m_timedOutReceived = boost::bind(&GatewayIO::ReceiveTimedOut, this, _1);
	m_pTrackingMng->m_gwDisconnReceived = boost::bind(&GatewayIO::ReceiveDisconnected, this, _1);


	//bind Channel
	m_pChannel->m_gwDisconnected.push_back(boost::bind(&GatewayTrackingMng::GWDisconnReceived, m_pTrackingMng));
	m_pChannel->m_messageReceived = boost::bind(&GatewayTrackingMng::MessageReceived, m_pTrackingMng, _1);

	LOG_DEBUG_APP("[GatewayIO]: the composeReqParseResp table size = " << g_nComposeReqParseRespSize);
	stack::CHartCmdWrapper::SetTable_ComposeReqParseResp(g_composeReqParseResp, g_nComposeReqParseRespSize);

}

GatewayIO::~GatewayIO()
{
	delete m_pChannel;
	delete m_pTrackingMng;
}


//functionality
void GatewayIO::Start(const std::string &hostIP, int hostPort, int localPort)
{
	//delegate pattern
	m_pChannel->Start(hostIP, hostPort, localPort);
}
void GatewayIO::Run()
{
	//delegate pattern
	m_pChannel->Run();
}
void GatewayIO::RunOnce()
{
	//delegate pattern
	m_pChannel->RunOnce();
}
void GatewayIO::Stop()
{
	//delegate pattern
	m_pChannel->Stop();
}

//for observers
void GatewayIO::AddForGWDisconnectNotify(Loki::Function<void(void)> funct)
{
	m_pChannel->m_gwDisconnected.push_back(funct);
}
void GatewayIO::AddForGWConnectNotify(Loki::Function<void(const std::string&, int)> funct)
{
	m_pChannel->m_gwConnected.push_back(funct);
}
nlib::timer::DeadlineTimer::Ptr GatewayIO::GetTimer(int periodTime/*ms*/)
{
	//delegate pattern
	return m_pChannel->GetTimer(periodTime);
}


//register for notifications
void GatewayIO::RegisterForNoBurstRspNotif(const hostapp::AppNoBurstRspNotificationPtr noBurstRspNotifPtr)
{
	//delegate pattern
	m_pTrackingMng->RegisterForNoBurstRspNotif(noBurstRspNotifPtr);
}
void GatewayIO::UnRegisterForNoBurstRspNotif()
{
	//delegate pattern
	m_pTrackingMng->UnRegisterForNoBurstRspNotif();
}
void GatewayIO::RegisterTopologyMsgFromDev(const hostapp::AppTopologyNotificationPtr topoNotificationPtr)
{
	//delegate pattern
	m_pTrackingMng->RegisterTopologyMsgFromDev(topoNotificationPtr);
}
void GatewayIO::UnRegisterTopologyMsgFromDev()
{
	//delegate pattern
	m_pTrackingMng->UnRegisterTopologyMsgFromDev();
}
void GatewayIO::RegisterBurstMsgFromDev(const hostapp::AppBurstNotificationPtr burstNotificationPtr, const WHartUniqueID &whAddr)
{
	//delegate pattern
	m_pTrackingMng->RegisterBurstMsgFromDev(burstNotificationPtr, whAddr);
}
void GatewayIO::UnRegisterBurstMsgFromDev(const WHartUniqueID &whAddr)
{
	//delegate pattern
	m_pTrackingMng->UnRegisterBurstMsgFromDev(whAddr);
}
void GatewayIO::RegisterReportsMsgFromDev(const hostapp::AppReportsNotificationPtr	reportsNotificationPtr)
{
	//delegate pattern
	m_pTrackingMng->RegisterReportsMsgFromDev(reportsNotificationPtr);
}
void GatewayIO::UnRegisterReportsMsgFromDev()
{
	//delegate pattern
	m_pTrackingMng->UnRegisterReportsMsgFromDev();
}
void GatewayIO::RegisterDevConfigMsgFromDev(const hostapp::AppDevConfigNotificationPtr devConfigNotificationPtr)
{
	//delegate pattern
	m_pTrackingMng->RegisterDevConfigMsgFromDev(devConfigNotificationPtr);
}
void GatewayIO::UnRegisterDevConfigMsgFromDev()
{
	//delegate pattern
	m_pTrackingMng->UnRegisterDevConfigMsgFromDev();
}



//send data
void GatewayIO::SendWHRequest(AppData &appData, const WHartUniqueID &whAddr, const stack::WHartCommand &whCmd, bool bypassIOCache /*= false*/, bool forceSameCmdOnDev /*= false*/)
{

	appData.whCmdID = whCmd.commandID;
	memcpy(appData.uniqueId.bytes, whAddr.bytes, sizeof(appData.uniqueId.bytes));
	if (forceSameCmdOnDev == false && m_allowSameCmdOnDev==false &&
			m_pTrackingMng->IsWHCmdSent(appData.whCmdID, appData.uniqueId))
	{
		std::string str = "Already exists a wireless_cmd=";
		char tmp[50];
		sprintf(tmp, "%d", appData.whCmdID);
		str+=tmp;
		str+=" in pending list for device=";
		str+=hostapp::MAC(appData.uniqueId.bytes).ToString();
		LOG_WARN_APP("[GatewayIO]: SendMessage: failed! " << str << " NewMessage=" << *appData.appCmd  << ". DeviceUniqueID=" << appData.uniqueId);
		return;
	}

	stack::transport::HARTDirectPDU_Request req;

	if (SerializeWHCommand(whAddr, whCmd, req) < 0)
		THROW_EXCEPTION1(GWSerializeException, "Gateway serialize request error!");

	req.bypassIOCache = bypassIOCache;
	m_pTrackingMng->SendMessage(appData, m_requestTimedOut, req);
}
void GatewayIO::SendWHRequest (AppData &appData, /* DevAddress */ const WHartUniqueID &whAddr , /* cmdNo */ int p_cmdNo, std::string& p_dataBytes, bool bypassIOCache/*= false*/)
{
	hart7::stack::transport::HARTDirectPDU_Request msg;

	unsigned short cmdNo;
	unsigned char dataBytesNo;

	cmdNo = htons(p_cmdNo);
	dataBytesNo =  (p_dataBytes.size() / 2);

	msg.status = 0;

	std::basic_ostringstream<unsigned char> data;
	data.write(whAddr.bytes, sizeof(whAddr.bytes));
	data.write((unsigned char*)&(cmdNo), sizeof(cmdNo));
	data.write((unsigned char*)& (dataBytesNo), sizeof(dataBytesNo));


	for(size_t i = 0; i< p_dataBytes.size(); i+=2)
	{
		char byte[3];
		byte[0] =  p_dataBytes[i];
		byte[1] =  p_dataBytes[i+1];
		byte[2] = '\0';

		int aux;
		sscanf(byte, "%x", &aux);

		unsigned char temp_buf_hex = (unsigned char)(aux);

		data.write((unsigned char * ) &temp_buf_hex, sizeof(temp_buf_hex));
	}
	msg.hartPDU = data.str();
	msg.bypassIOCache = bypassIOCache;

	m_pTrackingMng->SendMessage(appData, m_requestTimedOut, msg);
}
void GatewayIO::SendWHRequest(AppData &p_rAppData, const WHartUniqueID &p_rWhAddr, stack::CHartCmdWrapperList& p_rRequestsList, bool p_bBypassIOCache/* = false*/)
{
	stack::transport::HARTDirectPDU_Request req;

	char pTempBuff[1024*64];

	int nLen = stack::HartDirect_WriteCmdList(pTempBuff, sizeof(pTempBuff), p_rWhAddr, p_rRequestsList);

	if (nLen > 0)
	{	req.hartPDU.insert(req.hartPDU.end(), pTempBuff, pTempBuff + nLen );
		req.bypassIOCache = p_bBypassIOCache;
		p_rAppData.cmdType = AppData::CmdType_Multiple;
		p_rAppData.uniqueId = p_rWhAddr;
		m_pTrackingMng->SendMessage(p_rAppData, m_requestTimedOut, req);
	}
	else
	{	LOG_ERROR_APP("[GatewayIO]: Sending multiple commands request failed. DeviceUniqueID=" << p_rAppData.uniqueId);
	}
}

//receive data
void GatewayIO::RegisterForRecvMsg(Loki::Function<void(GWResponse&)> funct)
{
	m_recvMsg = funct;
}
void GatewayIO::ReceiveMessage(const stack::transport::MessageIP& msg, const AppData &appData)
{
	stack::CHartCmdWrapperList cmdWrappersList;

	stack::WHartCommand whCmd;
	GWResponse	resp;

	if (msg.status != stack::transport::HARTDirectPDU_Response::rcSuccess)
	{	switch ( msg.status )
		{
		case stack::transport::HARTDirectPDU_Response::rcErr_AccessRestricted:
			LOG_ERROR_APP("[GatewayIO]: TransId=" << (int)msg.transactionID
				<< " Received response with RC 16 - Access Restricted (Server resources exhausted). DeviceUniqueID=" << appData.uniqueId);
			resp.hostErr = HostTimedOut;
			break;
		case stack::transport::HARTDirectPDU_Response::rcErr_DRdead:
			LOG_ERROR_APP("[GatewayIO]: TransId=" << (int)msg.transactionID
				<< " Received response with RC 35  - Device Not Connected Or No Response. DeviceUniqueID=" << appData.uniqueId);
			resp.hostErr = HostTimedOut;
			break;
		case stack::transport::HARTDirectPDU_Response::rcErr_TooFewDataBytes:
			LOG_ERROR_APP("[GatewayIO]: TransId=" << (int)msg.transactionID
				<< " Received response with RC 5 : Too Few Data Bytes Received. DeviceUniqueID=" << appData.uniqueId);
			resp.hostErr = HostUnserializationFail;
			break;
		default:
			LOG_ERROR_APP("[GatewayIO]: TransId=" << (int)msg.transactionID
				<< " Received response with RC " << (int)(msg.status) << ". DeviceUniqueID=" << appData.uniqueId);
			resp.hostErr = HostUnserializationFail;
			break;
		}

		resp.appData = appData;
		resp.whCmd	 = whCmd;

		m_recvMsg(resp);
		return;
	}

	//unserialize message
	int data_length = 0;
	std::string hex_temp;

	switch(appData.cmdType)
	{
	case AppData::CmdType_Normal:
		{
			resp.hostErr = MsgUnserializer().Unserialize(msg, m_parser, whCmd) < 0 ?
									HostUnserializationFail : HostSuccess;
		}
		break;
	case AppData::CmdType_General:
		{
			std::basic_string<boost::uint8_t> hartPDU = ((stack::transport::HARTDirectPDU_Response&)msg).hartPDU;

			whCmd.commandID = ntohs(*((unsigned short*)&hartPDU[WHART_CMD_NO_OFFSET]));
			whCmd.responseCode = hartPDU[WHART_RESPONSE_CODE_OFFSET];

			hex_temp += ConvertToHex((unsigned short)whCmd.commandID);
			hex_temp += ConvertToHex((unsigned char)whCmd.responseCode);

			data_length = hartPDU[WHART_DATA_LENGTH_OFFSET]- 2;

			for (int i = 0; i< data_length; i++)
				hex_temp += ConvertToHex((unsigned char)hartPDU[WHART_RESP_DATA_BUFFER_OFFSET + i]);

			whCmd.command = (void *)hex_temp.c_str();
			resp.hostErr = HostSuccess;
		}
		break;
	case AppData::CmdType_Meta_Notification:
		{
			resp.pCmdWrappersList = NULL;
			resp.hostErr = HostSuccess;

			std::basic_string<boost::uint8_t> oData = ((stack::transport::HARTDirectPDU_Notify&)msg).hartPDU;
			uint8_t errorCodeResponse = 0;
			if (stack::HartDirect_ReadCmdList (true, oData.c_str(), oData.size(), true, const_cast<AppData&>(appData).uniqueId, cmdWrappersList, errorCodeResponse) > 0)
			{
				if (cmdWrappersList.size() == 1)
				{
					stack::CHartCmdWrapperList::iterator cmdIt = cmdWrappersList.begin();
					whCmd.commandID = (*cmdIt)->GetCmdId();
					whCmd.responseCode = (*cmdIt)->GetResponseCode();
					uint8_t u8RspCode = (*cmdIt)->GetParsedFromRaw();

					if (u8RspCode == RCS_N00_Success)
					{
						whCmd.command = (*cmdIt)->GetParsedData();
						resp.appData = appData;
						resp.whCmd	 = whCmd;
						m_recvMsg(resp);
						return;
					}

					LOG_ERROR_APP("[GatewayIO]: received an invalid packet for cmdID=" << (int)whCmd.commandID << ". DeviceUniqueID=" << appData.uniqueId);
				}
				else
				{
					LOG_ERROR_APP("[GatewayIO]: notification -> metacommand has more than one response. DeviceUniqueID=" << appData.uniqueId);
				}
			}
			resp.hostErr = HostUnserializationFail;
		}
	case AppData::CmdType_Multiple:
		{
			whCmd.commandID = stack::CHartCmdWrapper::g_sCmdIdInvalid;
			whCmd.responseCode = stack::CHartCmdWrapper::g_sRspCodeNotRsp;

			resp.hostErr = HostSuccess;
			std::basic_string<boost::uint8_t> oData = ((stack::transport::HARTDirectPDU_Response&)msg).hartPDU;

			uint8_t errorCodeResponse = 0;
			if (stack::HartDirect_ReadCmdList (true, oData.c_str(), oData.size(), true, const_cast<AppData&>(appData).uniqueId, cmdWrappersList, errorCodeResponse) > 0)
			{	resp.pCmdWrappersList = &cmdWrappersList;
			}
			break;
		}
	default:
		LOG_ERROR_APP("[GatewayIO]: Resp -> invalid cmdType! DeviceUniqueID=" << appData.uniqueId);
		return;
	}

	resp.appData = appData;
	resp.whCmd	 = whCmd;

	m_recvMsg(resp);
}
void GatewayIO::ReceiveTimedOut(const AppData &appData)
{
	GWResponse		resp;
	resp.appData = appData;
	resp.hostErr = HostTimedOut;

	m_recvMsg(resp);
}
void GatewayIO::ReceiveDisconnected(const AppData &appData)
{
	GWResponse		resp;
	resp.appData = appData;
	resp.hostErr = HostDisconnected;

	m_recvMsg(resp);
}



}
}

