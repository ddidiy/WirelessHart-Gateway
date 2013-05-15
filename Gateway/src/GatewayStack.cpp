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
 * GatewayStack.cpp
 *
 *  Created on: Nov 28, 2008
 *      Author: nicu.dascalu
 */
#include <WHartGateway/GwUtil.h>
#include <WHartGateway/GatewayStack.h>
#include <WHartGateway/AllGatewayCommands.h>
#include <WHartGateway/GatewayTypes.h>
#include <ApplicationLayer/ApplicationCommand.h>

#include <WHartStack/CommInterfaceOverIP/UDPClientIP.h> //udp impl
#include <WHartStack/CommInterfaceOverIP/UDPServerIP.h> //udp impl
#include <WHartStack/CommInterfaceOverIP/MessageIP.h>
#include <Shared/UtilsSolo.h>

#include <nlib/detail/bytes_print.h>

#include <boost/bind.hpp> //for binding callbacks

namespace hart7 {
namespace gateway {

#define REJOIN_TIMER 30000

GatewayStack::GatewayStack(GatewayConfig & config_, nlib::socket::ServiceSocket::Ptr & p_service, nlib::timer::TimerFactory::Ptr & p_timerFactory) :
	subapp::WHartSubApplicationData(commonData), config(config_), service(p_service), timerFactory(p_timerFactory),
	transport(commonData), network(commonData), datalink(commonData),
	mng(commonData, *this, transport, network, datalink, service, timerFactory)
{
	transport.upper = this;
	transport.lower = &network;

	network.upper = &transport;
	network.lower = &datalink;

	datalink.upper = &network;

	this->lower = &transport;
	m_nCurrentJoinReqHandle = 0;
	m_nLastConfirmed = 0;
	ResetStack();
}

GatewayStack::~GatewayStack()
{
}

WHartHandle GatewayStack::TransmitRequest(const WHartAddress & dest, WHartPriority priority,
		WHartTransportType transportType, WHartServiceID serviceID, const CHartCmdWrapperList & req,
		WHartSessionKey::SessionKeyCode sessionCode)
{
	// binarize all requests
	uint8_t buffer[SUBAPP_MAX_COMMANDS_BUFFER_SIZE];
	int writtenBytes = RawWHart_WriteCmdList(buffer, sizeof(buffer), req);

	//LOG_INFO_APP ("GatewayStack::TransmitResponse: success=" << (writtenBytes>0) << " cmds=" << CHartCmdWrapperListShortDetails(res));

	if (writtenBytes <= 0)
	{
		LOG_ERROR_APP("GatewayStack::TransmitRequest: dest=" << dest <<  " cmds=" << req);
		return 0;
	}


	WHartHandle handle = GenerateNewHandle();
	m_nLastConfirmed = 0;
	WHartPayload apdu(buffer, writtenBytes);
	//downgrade to debug when the HartServerIntegration is finished
	LOG_INFO_APP("TransmitRequest: handle=" << handle << ", destAddress=" << dest << ", priority=" << priority << " " << ToString(transportType)
	             /*<< ", APDU=" << GetHex(buffer, writtenBytes, ' ' )*/);
    LOG_DEBUG_APP("TransmitRequest: handle=" << handle << ", destAddress=" << dest << ", priority=" << priority << " " << ToString(transportType)
    			<< ", cmds=" << req << ", APDU=" << GetHex(buffer, writtenBytes, ' ' ) );
	lower->TransmitRequest(handle, dest, priority, serviceID, transportType, apdu, sessionCode);

	if (m_nLastConfirmed)
	{
	    handle = 0;
		m_nLastConfirmed = 0;
	}
	return handle;

}

void GatewayStack::TransmitResponse(WHartHandle handle, WHartServiceID serviceID,
		const CHartCmdWrapperList & res, WHartSessionKey::SessionKeyCode sessionCode)
{
	//binarize all requests
	uint8_t buffer[SUBAPP_MAX_COMMANDS_BUFFER_SIZE];
	int writtenBytes = RawWHart_WriteCmdList(buffer,sizeof(buffer),res);


	if (writtenBytes < 0)
	{	LOG_ERROR_APP ("GatewayStack::TransmitResponse: ERROR" << res);
		return;
	}
	LOG_INFO_APP ("GatewayStack::TransmitResponse:  cmds=" << CHartCmdWrapperListShortDetails(res) << ", serviceID=" << serviceID);
    LOG_DEBUG_APP ("GatewayStack::TransmitResponse:  cmds=" << res << ", serviceID=" << serviceID);
	lower->TransmitResponse(handle, serviceID, WHartPayload(buffer, writtenBytes), sessionCode);
}

void GatewayStack::Start()
{
	WHartAddress nm(NetworkManager_UniqueID());

	StartNMSession(config.NetworkManagerHost(), config.NetworkManagerPort(), config.m_nNmClientListenPort );

	StartAPsServer(config.ListenAccessPointPort(), config.AccessPointClientMinPort(), config.AccessPointClientMaxPort());
	service->Run();
}

void GatewayStack::PrepareShutdown()
{
    LOG_WARN_APP("GatewayStack::PrepareShutdown");

    TransmitConfirmCallBack = NULL;
    TransmitIndicateCallBack = NULL;

    NotifyWriteSessionCallback = NULL;
    NotifyRejoinCallback = NULL;

    // close NM Session
    LOG_WARN_APP("GatewayStack::PrepareShutdown - close NM Session");
    clientNM->NewSessionSucceeded = NULL;
    clientNM->NewSessionFailed = NULL;
    clientNM->SessionClosed = NULL;
    clientNM->Close();

    // close AP session
    LOG_WARN_APP("GatewayStack::PrepareShutdown - stop AP Server");
    serverAPs->NewSession = NULL;
    serverAPs->SessionClosed = NULL;
    serverAPs->Close();
    sleep(1);
}

int GatewayStack::ExtractWHartCmds(bool p_bIsResponse, const WHartPayload & apdu, CHartCmdWrapperList & p_oConfList, uint8_t p_u8DeviceStatus)
{
	int nRetCode = RawWHart_ReadCmdList (p_bIsResponse, apdu.data, apdu.dataLen, p_oConfList, p_u8DeviceStatus);

	if (nRetCode <= 0 )
	{
		LOG_ERROR_APP("GatewayStack::ExtractWHartCmds ERROR cmds=" << p_oConfList );
	}
	else
	{
		LOG_DEBUG_APP("GatewayStack::ExtractWHartCmds SUCCESS cmds=" << CHartCmdWrapperListShortDetails(p_oConfList) );
	}


	return nRetCode;
}


void GatewayStack::TransmitConfirm(WHartHandle requestHandle, const WHartLocalStatus & localStatus, DeviceIndicatedStatus status, const WHartPayload & apdu)
{
	LOG_DEBUG_APP("GatewayStack confirm a message with locaStaus=" << localStatus.status << ", apdu=" << apdu);

	m_nLastConfirmed = requestHandle;
	if (!TransmitConfirmCallBack)
	{
		//so far nothing to do if no callback specified
		return;
	}

	CHartCmdWrapperList oConfList;
	if (localStatus.status < WHartLocalStatus::whartlsError_Start)
	{
		if (!ExtractWHartCmds( true, apdu, oConfList, status.deviceStatus))
		{
		    LOG_WARN_APP("Extract WHart Cmds fail; TransmitConfirm - locaStaus=" << localStatus.status
		                 << ", status=" << status << ", apdu=" << apdu);
		    return;
		}
	}

	if (m_nCurrentJoinReqHandle && m_nCurrentJoinReqHandle == requestHandle)
	{
		m_nCurrentJoinReqHandle = 0;
		LOG_INFO_APP("GatewayStack::TransmitConfirm for JOIN cmds=" << CHartCmdWrapperListShortDetails(oConfList) );
		return;
	}

	TransmitConfirmCallBack(requestHandle, localStatus, status, oConfList);
}

void GatewayStack::TryExecCmd(const WHartAddress & src, CHartCmdWrapper * p_pCmd, CHartCmdWrapper * p_pRsp)
{
	LOG_DEBUG_APP("GatewayStack::TryExecCmd: src=" << src << " cmdId=" << p_pCmd->GetCmdId() << " data=" << GetHex(p_pCmd->GetRawData(), p_pCmd->GetRawDataLen(),' '));
	if (!(src == NetworkManager_UniqueID()) && !(src == NetworkManager_Nickname()) )
	{
		LOG_ERROR_APP("GatewayStack::TryExecCmd: src=" << src << " not NM");
		p_pRsp->LoadRaw(p_pCmd->GetCmdId(), 0, NULL, RCS_E16_AccessRestricted);
		return;
	}

	uint8_t u8Res = p_pCmd->GetParsedFromRaw();
	if (u8Res != RCS_N00_Success)
	{
		p_pRsp->LoadRaw(p_pCmd->GetCmdId(), 0, NULL, u8Res);
		return;
	}

	switch(p_pCmd->GetCmdId())
	{
	case CMDID_C801_DeleteService:
		{
			C801_DeleteService_Req& req973 = *(C801_DeleteService_Req*)  p_pCmd->GetParsedData();

			mng.DeleteService(req973);

			//LOG_ERROR_APP("CMDID_C801_DeleteService: -- received; should not happen ");

			//TODO: notify upper layer about new service
			//TODO see if this must be removed...
			//					services[req973.m_unPeerNickname] = req973.m_ucServiceID;

			C801_DeleteService_Resp resp973;
			resp973.m_ucServiceId = req973.m_ucServiceId;
			resp973.m_ucReason = req973.m_ucReason;
			resp973.m_ucNoOfServiceEntriesRemaining = 10;

			p_pRsp->LoadParsed(p_pCmd->GetCmdId(), sizeof(resp973), &resp973, RCS_N00_Success );
		}
		break;
	case CMDID_C961_WriteNetworkKey:
		{
			const C961_WriteNetworkKey_Req& req961 = *(const C961_WriteNetworkKey_Req*) p_pCmd->GetParsedData();
			C961_WriteNetworkKey_Resp resp961;
			memset(&resp961,0,sizeof(resp961));

			COPY_FIXED_ARRAY(resp961.m_aKeyValue, req961.m_aKeyValue);

			//handle c961
			memcpy (resp961.m_tExecutionTime, req961.m_tExecutionTime, sizeof(req961.m_tExecutionTime));
			COPY_FIXED_ARRAY(resp961.m_aKeyValue, req961.m_aKeyValue);
			p_pRsp->LoadParsed(p_pCmd->GetCmdId(), sizeof(resp961), &resp961, RCS_N00_Success );
		}
		break;
	case CMDID_C962_WriteDeviceNicknameAddress:
		{	//handle c962
			const C962_WriteDeviceNicknameAddress_Req& req962 =	*(const C962_WriteDeviceNicknameAddress_Req*)p_pCmd->GetParsedData();

			C962_WriteDeviceNicknameAddress_Resp resp962;
			int nRspCode = RCS_E01_Undefined1; //should be invalid nickname
			if (req962.m_unNickname != Gateway_Nickname())
			{
				LOG_ERROR_APP("Invalid nickname for GW! " <<std::hex << req962.m_unNickname);
				p_pRsp->LoadParsed(p_pCmd->GetCmdId(), 0, NULL, RCM_E65_InvalidNickname );
				break;
			}

			commonData.myNickname = req962.m_unNickname;
			nRspCode = RCS_N00_Success;

			resp962.m_unNickname = commonData.myNickname;

			p_pRsp->LoadParsed(p_pCmd->GetCmdId(), sizeof(resp962), &resp962, nRspCode );
		}
		break;
	case CMDID_C963_WriteSession:
		{
			const C963_WriteSession_Req* pReq963 = (const C963_WriteSession_Req*) p_pCmd->GetParsedData();

			//handle c963
			LOG_INFO_APP("C963_WriteSession: peer=" << GetHex(pReq963->m_aPeerUniqueID, sizeof(pReq963->m_aPeerUniqueID))
				<< " nick=" <<std::hex << pReq963->m_unPeerNickname <<" key=" << nlib::detail::BytesToString((uint8_t*) pReq963->m_aKeyValue, 16));

			C963_WriteSession_Resp resp963;

			RESET_WHART7_TIME40_RAW(resp963.m_tExecutionTime);

			network.WriteSession(pReq963, &resp963);

			//if ( memcmp(&NetworkManager_UniqueID(), req963.m_aPeerUniqueID, sizeof(req963.m_aPeerUniqueID) ) == 0 )
			if (NetworkManager_Nickname() == pReq963->m_unPeerNickname)
			{	state = stOperational;
				commonData.isJoined = true;
				LOG_INFO_APP("Gateway is JOINED ");
			}

			p_pRsp->LoadParsed(p_pCmd->GetCmdId(), sizeof(resp963), &resp963, RCS_N00_Success );
		}
	    break;
	case CMDID_C965_WriteSuperframe:
		{
			C965_WriteSuperframe_Resp resp965;
			memset(&resp965, 0, sizeof(resp965));

			//WHartTime40 time = { 0, 0 }; //TODO-generate time

			RESET_WHART7_TIME40_RAW (resp965.m_tExecutionTime);// = time;
			resp965.m_ucRemainingSuperframesNo = 100;
			resp965.m_ucSuperframeID = 0;
			resp965.m_ucSuperframeMode = 0;
			resp965.m_unSuperframeSlotsNo = 100;

			p_pRsp->LoadParsed(p_pCmd->GetCmdId(), sizeof(resp965), &resp965, RCS_N00_Success );
		}
	    break;
	case CMDID_C969_WriteGraphNeighbourPair:
		{
			C969_WriteGraphNeighbourPair_Req& req969 = *(C969_WriteGraphNeighbourPair_Req*) p_pCmd->GetParsedData();

			mng.AddGraph(req969);

			C969_WriteGraphNeighbourPair_Resp resp969;
			memset(&resp969, 0, sizeof(resp969));
			resp969.m_unGraphID = req969.m_unGraphID;
			resp969.m_unNeighborNickname = req969.m_unNeighborNickname;
			resp969.m_ucRemainingConnectionsNo = 5;

			p_pRsp->LoadParsed(p_pCmd->GetCmdId(), sizeof(req969), &req969, RCS_N00_Success );
		}
		break;

	case CMDID_C970_DeleteGraphConnection:
		{
			C970_DeleteGraphConnection_Req& req969 = *(C970_DeleteGraphConnection_Req*) p_pCmd->GetParsedData();
			mng.DeleteGraph(req969);

			C970_DeleteGraphConnection_Resp resp969;
			memset(&resp969, 0, sizeof(resp969));
			resp969.m_unGraphID = req969.m_unGraphID;
			resp969.m_unNeighborNickname = req969.m_unNeighborNickname;
			resp969.m_ucRemainingConnectionsNo = 5;

			p_pRsp->LoadParsed(p_pCmd->GetCmdId(), sizeof(req969), &req969, RCS_N00_Success );
		}
		break;

	case CMDID_C973_WriteService:

		{
			C973_WriteService_Req& req973 =
				*(C973_WriteService_Req*) p_pCmd->GetParsedData();

			mng.WriteService(req973);
			//services[req973.m_unPeerNickname] = req973.m_ucServiceID;
			/// TODO: notify upper layer about new service

			C973_WriteService_Resp resp973;
			resp973.m_unPeerNickname = req973.m_unPeerNickname;
			resp973.m_ucServiceID = req973.m_ucServiceID;
			resp973.m_ucRequestFlags = req973.m_ucRequestFlags;
			resp973.m_eApplicationDomain = req973.m_eApplicationDomain;
			resp973.m_ucRouteID = req973.m_ucRouteID;
			resp973.m_tPeriod = req973.m_tPeriod;
			resp973.m_ucRemainingServicesNo = 10;

			p_pRsp->LoadParsed(p_pCmd->GetCmdId(), sizeof(resp973), &resp973, RCS_N00_Success );
		}
		break;
	case CMDID_C974_WriteRoute:
		{
			C974_WriteRoute_Req& req974 = *(C974_WriteRoute_Req*)p_pCmd->GetParsedData();
			mng.WriteRoute(req974);

			C974_WriteRoute_Resp resp974;

			//memset(&resp974, 0, sizeof(resp974));
			resp974.m_ucRouteID = req974.m_ucRouteID;
			resp974.m_unPeerNickname = req974.m_unPeerNickname;
			resp974.m_unGraphID = req974.m_unGraphID;
			resp974.m_ucRemainingRoutesNo = 100;

			p_pRsp->LoadParsed(p_pCmd->GetCmdId(), sizeof(resp974), &resp974, RCS_N00_Success );
		}
		break;
	case CMDID_C975_DeleteRoute:
		{
			C975_DeleteRoute_Req& req975 =	*(C975_DeleteRoute_Req*) p_pCmd->GetParsedData();

				C975_DeleteRoute_Resp resp975;
				resp975.m_ucRouteID = req975.m_ucRouteID;

				p_pRsp->LoadParsed(p_pCmd->GetCmdId(), sizeof(resp975), &resp975, RCS_N00_Success );
		}
		break;
	case CMDID_C976_WriteSourceRoute:
		{
			C976_WriteSourceRoute_Req& req976 =	*(C976_WriteSourceRoute_Req*) p_pCmd->GetParsedData();

			mng.WriteSourceRoute(req976);

			C976_WriteSourceRoute_Resp resp976;
			memset(&resp976, 0, sizeof(resp976));
			resp976.m_ucRouteID = req976.m_ucRouteID;
			resp976.m_ucHopsNo = req976.m_ucHopsNo;
			resp976.m_ucRemainingSourceRoutesNo = 1;
			memcpy(resp976.m_aNicknameHopEntries, req976.m_aNicknameHopEntries, 16);

			p_pRsp->LoadParsed(p_pCmd->GetCmdId(), sizeof(resp976), &resp976, RCS_N00_Success );
		}
		break;
	case CMDID_C977_DeleteSourceRoute:
		{
			C977_DeleteSourceRoute_Req& req977 =*(C977_DeleteSourceRoute_Req*) p_pCmd->GetParsedData();

			C977_DeleteSourceRoute_Resp resp977;
			resp977.m_ucRouteID = req977.m_ucRouteID;

			p_pRsp->LoadParsed(p_pCmd->GetCmdId(), sizeof(resp977), &resp977, RCS_N00_Success );
		}
		break;

	default:
		LOG_INFO_APP("GatewayStack::TryExecCmd: cmdId " << p_pCmd->GetCmdId() << " not implemented");
		p_pRsp->LoadRaw(p_pCmd->GetCmdId(),0,NULL, RCS_E64_CommandNotImplemented);
	    break;
	}
}


void GatewayStack::TransmitIndicate(WHartHandle handle, const WHartAddress & src, WHartPriority priority, DeviceIndicatedStatus status,
					  WHartTransportType transportType, const WHartPayload & apdu, WHartSessionKey::SessionKeyCode sessionCode)
{
    LOG_DEBUG_APP("TransmitIndicate src=" << src << ", apdu=" << apdu);

	CHartCmdWrapperList oIndList;
	bool bIsResponse = IsResponseType(transportType);

	if (!ExtractWHartCmds( bIsResponse, apdu, oIndList, status.deviceStatus))
	{
        LOG_WARN_APP("Extract WHart Cmds fail; TransmitIndicate src=" << src << ", trType=" << ToString(transportType) << ", status=[" << status << "], apdu=" << apdu);
	    return;
	}

	LOG_INFO_APP("TransmitIndicate src=" << src << ", trType=" << ToString(transportType) << ", status=[" << status << "], cmds=" << CHartCmdWrapperListShortDetails(oIndList));

	if (TransmitIndicateCallBack)
	{
		TransmitIndicateCallBack(handle, src, priority, status, transportType, oIndList);
	}
}

void GatewayStack::ResetStack()
{
	mng.ResetStack();
	if (serverAPs)
	{
	    ((transport::UDPServerIP*)serverAPs.get())->CloseAllSessions();
	}

	commonData.myUniqueID = Gateway_UniqueID();
	commonData.myNickname = Gateway_Nickname();
	state = stIdle;

	//add nm join key
	{
		C963_WriteSession_Req reqNMJoinKey;
		{
			// boost::uint8_t joinKey[] = { 0x00, 0x00, 0x12, 0x34, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00 };
			memcpy(reqNMJoinKey.m_aKeyValue, config.m_u8AppJoinKey, sizeof(config.m_u8AppJoinKey));

			reqNMJoinKey.m_ulPeerNonceCounterValue = 0; //TODO-generate nonce
			reqNMJoinKey.m_unPeerNickname = NetworkManager_Nickname();
			reqNMJoinKey.m_eSessionType = WHartSessionKey::joinKeyed;
			reqNMJoinKey.m_ucReserved = 0;

			memcpy ( reqNMJoinKey.m_aPeerUniqueID, NetworkManager_UniqueID().bytes, sizeof(reqNMJoinKey.m_aPeerUniqueID) );

			memset (reqNMJoinKey.m_tExecutionTime, 0, sizeof(reqNMJoinKey.m_tExecutionTime));
		}
		mng.WriteKey(reqNMJoinKey);
	}

	//add nm route
	{
		C974_WriteRoute_Req reqNMRoute;
		{
			reqNMRoute.m_unPeerNickname = NetworkManager_Nickname();
			reqNMRoute.m_ucRouteID = 0;
			reqNMRoute.m_unGraphID = 1;
		}
		mng.WriteRoute(reqNMRoute);
	}

	//add graph connection for NM
	{
		C969_WriteGraphNeighbourPair_Req reqNMGraph;
		{
			reqNMGraph.m_unGraphID = 1;
			reqNMGraph.m_unNeighborNickname = NetworkManager_Nickname();
		}
		mng.AddGraph(reqNMGraph);
	}

	//add graph connection for TR
	{
		C969_WriteGraphNeighbourPair_Req reqTRGraph;
		{
			reqTRGraph.m_unGraphID = 0;
			reqTRGraph.m_unNeighborNickname = 0x70F8;
		}
		mng.AddGraph(reqTRGraph);
	}

}

void GatewayStack::SendJoinRequest()
{
	// complete c000 response
	C000_ReadUniqueIdentifier_Resp c000;

	FillGwIdentityResponse (&c000, config);

	CHartCmdWrapper::Ptr pCmd000(new CHartCmdWrapper);
	pCmd000->LoadParsed(CMDID_C000_ReadUniqueIdentifier, sizeof(C000_ReadUniqueIdentifier_Resp), &c000, RCS_N00_Success);


	// complete c020 response
	C020_ReadLongTag_Resp c020;

	memcpy(c020.longTag, config.m_szLongTag, sizeof(c020.longTag));

	CHartCmdWrapper::Ptr pCmd020(new CHartCmdWrapper);
	pCmd020->LoadParsed(CMDID_C020_ReadLongTag,  sizeof(C020_ReadLongTag_Resp),&c020, RCS_N00_Success);

	C787_ReportNeighborSignalLevels_Resp c787;
	memset(&c787, 0, sizeof(c787));
	c787.m_ucNeighborTableIndex = 0;
	c787.m_ucNoOfNeighborEntriesRead = 1;
	c787.m_ucTotalNoOfNeighbors = 1;
	c787.m_aNeighbors[0].RSLindB = 123;
	c787.m_aNeighbors[0].nickname = NetworkManager_Nickname();

	CHartCmdWrapper::Ptr pCmd787(new CHartCmdWrapper);
	pCmd787->LoadParsed(CMDID_C787_ReportNeighborSignalLevels, sizeof(C787_ReportNeighborSignalLevels_Resp),&c787, RCS_N00_Success);

	CHartCmdWrapperList oJoinList;

	oJoinList.push_back(pCmd000);
	oJoinList.push_back(pCmd020);
	oJoinList.push_back(pCmd787);

	WHartAddress nm(NetworkManager_UniqueID());

	m_nCurrentJoinReqHandle = TransmitRequest(nm, whartpEvent, wharttPublishNotify, 0, oJoinList, WHartSessionKey::joinKeyed);

	joinTimer = timerFactory->CreateDeadlineTimer(REJOIN_TIMER);
	joinTimer->Elapsed = boost::bind(&GatewayStack::ProcessJoinStatus, this);
}

void GatewayStack::ProcessJoinStatus()
{
    if (state == stOperational)
        return;

    LOG_INFO_APP("Join flow timeouted. Resending join request.");
    SendJoinRequest();
}


void GatewayStack::StartNMSession(const std::string & host, int port, int my_port)
{
	clientNM.reset(new transport::UDPClientIP(nlib::socket::Address(host, port), my_port, service, timerFactory));
	clientNM->NewSessionSucceeded = boost::bind(&GatewayStack::NMSessionSucceeded, this);
	clientNM->NewSessionFailed = boost::bind(&GatewayStack::NMSessionFailed, this, _1);
	clientNM->SessionClosed = boost::bind(&GatewayStack::NMSessionClosed, this);


	((transport::UDPClientIP*)clientNM.get())->SetInactivityCloseTime(5 * config.m_u8GranularityKeepAlive * 1000);
	((transport::UDPClientIP*)clientNM.get())->SetGranularityKeepAlive(config.m_u8GranularityKeepAlive * 1000);

	clientNM->Connect();

	LOG_INFO_APP("start session to NM on host=" << host << ", port=" << port);
}

void GatewayStack::NMSessionSucceeded()
{
	LOG_INFO_APP("session to NM succeeded.");

	datalink.CreateNeighborSession(NetworkManager_UniqueID(), NetworkManager_Nickname());
	datalink.SetNeighborSession(NetworkManager_UniqueID(), clientNM->ActiveSession());

	SendJoinRequest();
	state = stJoining;
}

void GatewayStack::NMSessionFailed(const std::string & reason)
{
	//retry a new connection after a time
	LOG_ERROR_APP("session to NM failed! Reason=" << reason);
	clientNM->Connect();
}

void GatewayStack::NMSessionReset()
{
	// for Close - Reconnect procedure fired by CMD 42
	LOG_INFO_APP("Session to NM is being reset ..");
	ResetStack();
	clientNM->Close();
}

void GatewayStack::NMSessionClosed()
{
	state = stIdle;
	LOG_INFO_APP("NMSessionClosed::ResetStack");
	ResetStack();
	if (NotifyRejoinCallback)
	{
		NotifyRejoinCallback();
	}

	LOG_WARN_APP("Session to NM closed. reconnect start ...");
	datalink.ResetNeighborSession(clientNM->ActiveSession());

	clientNM->Connect();
	LOG_DEBUG_APP("GatewayStack::NMSessionClosed: end");
}

void GatewayStack::StartAPsServer(int listenPort, int minPort, int maxPort)
{
	LOG_INFO_APP("Start listen for access points on port=" << listenPort);

	serverAPs.reset(new stack::transport::UDPServerIP(service, timerFactory, listenPort, minPort, maxPort));
	serverAPs->NewSession = boost::bind(&GatewayStack::NewAPSession, this, _1, _2);
	LOG_INFO_APP("NewSession");
	serverAPs->SessionClosed = boost::bind(&GatewayStack::APSessionClosed, this, _1);
	LOG_INFO_APP("SessionClosed");
	serverAPs->Listen();
	LOG_INFO_APP("Listen");
}

void GatewayStack::NewAPSession(const std::string & host, const stack::transport::SessionIP::Ptr session)
{
	LOG_INFO_APP("NewAPSession: established from host=" << host);
	session->ReceiveMessage = boost::bind(&GatewayStack::GetAPSessionIdentity, this, session, _1,
				boost::function1<void, const stack::transport::MessageIP&>());
}

void GatewayStack::APSessionClosed(const stack::transport::SessionIP::Ptr session)
{
	LOG_WARN_APP("APSessionClosed: session=" << *session);
	datalink.ResetNeighborSession(session);
	session->ReceiveMessage = NULL;
}

void GatewayStack::GetAPSessionIdentity(const stack::transport::SessionIP::Ptr session,
		const stack::transport::MessageIP & packet, boost::function1<void, const stack::transport::MessageIP&> callback)
{
	class Detector: public stack::transport::IMessageIPVisitor
	{
		LOG_DEF_APP("hart7.gateway.GatewayStack.Detector");
	private:
		WHartAddress detectedID;
		bool success;

	private:
		virtual void Visit(stack::transport::WirelessNPDU_Request& req)
		{
			LOG_INFO("WirelessNPDU_Request:" << req);
			if (req.srcAddress.type == WHartAddress::whartaUniqueID)
			{
				detectedID = WHartAddress(req.srcAddress.address.uniqueID);
				success = true;
			}
			else if (req.srcAddress.type == WHartAddress::whartaNickname)
			{
				detectedID = WHartAddress(req.srcAddress.address.nickname);
				success = true;
			}
		}

		virtual void Visit(stack::transport::WirelessNPDU_ACK& ack)
		{
			detectedID = No_Nickname();
			success = true;
		}

	public:
		bool Do(const stack::transport::MessageIP& packet, WHartAddress& address)
		{
			success = false;
			const_cast<stack::transport::MessageIP&> (packet).Accept(*this);

			address = detectedID;
			return success;
		}
	};

	WHartAddress detectedAddress;
	if (Detector().Do(packet, detectedAddress))
	{
		if (detectedAddress == No_Nickname())	// this is an ACK packet, so forward to callback
		{
			if (callback)
				callback(packet);
		}
		else
		{
			LOG_INFO("Detected address=" << detectedAddress << " for session=" << *session);
			if (detectedAddress.type == WHartAddress::whartaUniqueID)
			{
				datalink.CreateNeighborSession(detectedAddress.address.uniqueID, No_Nickname()); //no nick name for now
				datalink.SetNeighborSession(detectedAddress.address.uniqueID, session);

				// [andy] hijack the callback for receive message
				boost::function1<void, const stack::transport::MessageIP&> dllCallback = session->ReceiveMessage;
				session->ReceiveMessage = boost::bind(&GatewayStack::GetAPSessionIdentity, this, session, _1,
				//			boost::function1<void, const MessageIP&>());
						dllCallback);

				if (dllCallback)	// hand packet to DLL
					dllCallback(packet);

				return;
			}
			else if (detectedAddress.type == WHartAddress::whartaNickname)
			{
				// this call will set the ReceiveMessage back towards the DLL.
				datalink.UpdateNeighborSessionNickname(session, detectedAddress.address.nickname);
			}

			//now we should forward the packet to the corresponding DataLink previously registered
			//to handle new packets, see: datalink.SetNeighborSession
			if (callback)
				callback(packet);
		}
	}
	else
	{
		LOG_ERROR("Invalid packet received! packet=" << packet << " for session=" << *session);
	}
}

} // namespace gateway
} // namespace hart7
