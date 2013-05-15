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

#include <stdio.h>
#include "GatewayProcessor.h"
#include "Table.h"




namespace hart7 {
namespace gateway {


#define WHART_PARSED_REQ_MAX_SIZE		512	/*bytes*/
static unsigned char WHCommandBuff[WHART_PARSED_REQ_MAX_SIZE];


int SerializeWHCommand(const WHartUniqueID &whAddr, const stack::WHartCommand &whCmd,
					   stack::transport::HARTDirectPDU_Request &req)
{

	int tableIndex = FindEntry ( g_composeReqParseResp, g_nComposeReqParseRespSize, whCmd.commandID);
	ComposeFunction composer = (tableIndex < 0) ? 0 : g_composeReqParseResp[tableIndex].fnComposer;

	if (!composer)
	{
		LOG_ERROR_APP("[GatewayIO]: Req -> no composer function for wh_cmdNo=" << (int)whCmd.commandID);
		return -1;
	}

	uint8_t tmp[255];
	BinaryStream stream;
	STREAM_INIT(&stream, tmp, sizeof(tmp));

	ComposerContext context;
	context.cmdId = whCmd.commandID;

	uint8_t u8Res = (*composer)(whCmd.command, &context, &stream);

	if (u8Res != RCS_N00_Success)
	{
		LOG_ERROR_APP("[GatewayIO]: Req -> composer error="<<(int)u8Res << "for wh_cmdNo=" <<(int)whCmd.commandID);
		return -1;
	}

	LOG_DEBUG_APP("[GatewayIO]: Req -> composing request successfully for cmd_id=" << (int)whCmd.commandID);

	unsigned short uwNo = ntohs(whCmd.commandID);
	unsigned char  ucLen = stream.nextByte - tmp;

	req.hartPDU.insert(req.hartPDU.end(), whAddr.bytes, whAddr.bytes + sizeof(whAddr.bytes));
	req.hartPDU.insert(req.hartPDU.end(), (unsigned char*)(&uwNo), ((unsigned char*)(&uwNo)) + sizeof(uwNo));
	req.hartPDU.insert(req.hartPDU.end(), &ucLen, &ucLen + sizeof(ucLen) );
	req.hartPDU.insert(req.hartPDU.end(), tmp, tmp + ucLen);

	//for logging only
	if (LOG_DEBUG_ENABLED_APP())
	{
		std::string strMsg;
		char szNo[100];

		//address
		strMsg += " address=0x";
		{
			std::ostringstream strBytes;
			strBytes << std::hex << std::uppercase << std::setfill('0');

			for (int i = 0; i < (int)sizeof(whAddr.bytes); i++)
				strBytes << std::setw(2) << (unsigned int)whAddr.bytes[i] << ' ';

			strMsg += strBytes.str();
		}

		//cmdNo
		strMsg += " cmdNo=";
		sprintf(szNo, "%d", whCmd.commandID);
		strMsg += szNo;

		//ByteCount
		strMsg += " dataLength=";
		sprintf(szNo, "%d", ucLen);
		strMsg += szNo;

		//databytes
		strMsg += " dataBytes=0x";
		if (ucLen > 0)
		{
			std::ostringstream strBytes;
			strBytes << std::hex << std::uppercase << std::setfill('0');

			for (int i = 0 ; i < ucLen; i++)
				strBytes << std::setw(2) << (int)(((char*)tmp)[i]) << ' ';

			strMsg += strBytes.str();
		}
		else
		{
			strMsg += "'no bytes'";
		}

		LOG_DEBUG_APP("[GatewayIO]: Req -> message_composed = " << strMsg.c_str());
	}

	return 0;
}

// 0  - ok
// <0 - not ok
static int UnserializeWHCommand(const stack::transport::MessageIP& packet, stack::WHartCommand &whCmd, bool p_bIsNotify=false);


bool MessageChecker::IsChangeNotificationMsg(const stack::transport::MessageIP& packet, WHartUniqueID& addr/*[out]*/,
											 unsigned short &notificationMask/*[out]*/,
											 hostapp::AppNoBurstRspNotification::CMD_Type &cmdType/*[out]*/,
											 bool &isMetaPacket/*out*/)
{
	m_pNotificationMask = &notificationMask;
	m_pAddr = &addr;
	m_cmdType = &cmdType;
	m_isChangeNotification = false;
	m_pIsMetaPacket = &isMetaPacket;
	const_cast<stack::transport::MessageIP&> (packet).Accept(*this);
	return m_isChangeNotification;
}

void MessageChecker::Visit(stack::transport::HARTDirectPDU_Notify& notif)
{

	//
	unsigned short cmdNoOffset = 0;
	unsigned short devAddrOffset = 0;
	unsigned short dataBuffOffset = 0;
	unsigned short dataLenOffset = 0;
	*m_pIsMetaPacket = false;

	//packet_type
	unsigned short whCmdNo = 0;
	if ((whCmdNo = ntohs(*((unsigned short*)&notif.hartPDU[META_WHART_CMD_NO_OFFSET]))) == CMDID_C64765_NivisMetaCommand)
	{
		cmdNoOffset = META_INNER_WHART_CMD_NO_OFFSET;
		devAddrOffset = META_INNER_WHART_DEV_ADDRESS_OFFSET;
		dataBuffOffset = META_INNER_WHART_RESP_DATA_BUFFER_OFFSET;
		dataLenOffset = META_INNER_WHART_DATA_LENGTH_OFFSET;
		*m_pIsMetaPacket = true;

		//LOG_INFO_APP("TRACKING_MNG-> cmd=" << whCmdNo);
	}
	else
	{
		cmdNoOffset = WHART_CMD_NO_OFFSET;
		devAddrOffset = WHART_DEV_ADDRESS_OFFSET;
		dataBuffOffset = WHART_RESP_DATA_BUFFER_OFFSET;
		dataLenOffset = WHART_DATA_LENGTH_OFFSET;
	}



	 whCmdNo = ntohs(*((unsigned short*)&notif.hartPDU[cmdNoOffset]));
	 m_isChangeNotification = (whCmdNo == CMDID_C839_ChangeNotification);

	 //LOG_INFO_APP("TRACKING_MNG-> inner, cmd=" << whCmdNo);

	if (!m_isChangeNotification)
	{
		memcpy(m_pAddr->bytes, &notif.hartPDU[devAddrOffset], sizeof(m_pAddr->bytes));
		//anticipate
		m_isChangeNotification = true;
		*m_pNotificationMask = NotificationMaskCodesMask_ResponseReceived;

		switch (whCmdNo)
		{
		case CMDID_C000_ReadUniqueIdentifier:
			*m_cmdType = hostapp::AppNoBurstRspNotification::CMD_Report_000;
			break;
		case CMDID_C020_ReadLongTag:
			*m_cmdType = hostapp::AppNoBurstRspNotification::CMD_Report_020;
			break;
		case CMDID_C769_ReadJoinStatus:
			*m_cmdType = hostapp::AppNoBurstRspNotification::CMD_Report_769;
			break;
		case CMDID_C779_ReportDeviceHealth:
			*m_cmdType = hostapp::AppNoBurstRspNotification::CMD_Report_779;
			break;
		case CMDID_C780_ReportNeighborHealthList:
			*m_cmdType = hostapp::AppNoBurstRspNotification::CMD_Report_780;
			break;
		case CMDID_C783_ReadSuperframeList:
			*m_cmdType = hostapp::AppNoBurstRspNotification::CMD_Report_783;
			break;
		case CMDID_C784_ReadLinkList:
			*m_cmdType = hostapp::AppNoBurstRspNotification::CMD_Report_784;
			break;
		case CMDID_C785_ReadGraphList:
			*m_cmdType = hostapp::AppNoBurstRspNotification::CMD_Report_785;
			break;
		case CMDID_C787_ReportNeighborSignalLevels:
			*m_cmdType = hostapp::AppNoBurstRspNotification::CMD_Report_787;
			break;
		case CMDID_C788_AlarmPathDown:
			*m_cmdType = hostapp::AppNoBurstRspNotification::CMD_Alert_788;
			break;
		case CMDID_C789_AlarmSourceRouteFailed:
			*m_cmdType = hostapp::AppNoBurstRspNotification::CMD_Alert_789;
			break;
		case CMDID_C790_AlarmGraphRouteFailed:
			*m_cmdType = hostapp::AppNoBurstRspNotification::CMD_Alert_790;
			break;
		case CMDID_C791_AlarmTransportLayerFailed:
			*m_cmdType = hostapp::AppNoBurstRspNotification::CMD_Alert_791;
			break;
		case CMDID_C800_ReadServiceList:
			*m_cmdType = hostapp::AppNoBurstRspNotification::CMD_Report_800;
			break;
		case CMDID_C801_DeleteService:
			*m_cmdType = hostapp::AppNoBurstRspNotification::CMD_Report_801;
			break;
		case CMDID_C802_ReadRouteList:
			*m_cmdType = hostapp::AppNoBurstRspNotification::CMD_Report_802;
			break;
		case CMDID_C803_ReadSourceRoute:
			*m_cmdType = hostapp::AppNoBurstRspNotification::CMD_Report_803;
			break;
		case CMDID_C814_ReadDeviceListEntries:
			*m_cmdType = hostapp::AppNoBurstRspNotification::CMD_Report_814;
			break;
		case CMDID_C818_WriteChannelBlacklist:
			*m_cmdType = hostapp::AppNoBurstRspNotification::CMD_Report_818;
			break;
		case CMDID_C832_ReadNetworkDeviceIdentity:
			*m_cmdType = hostapp::AppNoBurstRspNotification::CMD_Report_832;
			break;
		case CMDID_C833_ReadNetworkDeviceNeighbourHealth:
			*m_cmdType = hostapp::AppNoBurstRspNotification::CMD_Report_833;
			break;
		case CMDID_C834_ReadNetworkTopologyInformation:
			*m_cmdType = hostapp::AppNoBurstRspNotification::CMD_Report_834;
			break;
		case CMDID_C965_WriteSuperframe:
			*m_cmdType = hostapp::AppNoBurstRspNotification::CMD_Report_965;
			break;
		case CMDID_C966_DeleteSuperframe:
			*m_cmdType = hostapp::AppNoBurstRspNotification::CMD_Report_966;
			break;
		case CMDID_C967_WriteLink:
			*m_cmdType = hostapp::AppNoBurstRspNotification::CMD_Report_967;
			break;
		case CMDID_C968_DeleteLink:
			*m_cmdType = hostapp::AppNoBurstRspNotification::CMD_Report_968;
			break;
		case CMDID_C969_WriteGraphNeighbourPair:
			*m_cmdType = hostapp::AppNoBurstRspNotification::CMD_Report_969;
			break;
		case CMDID_C970_DeleteGraphConnection:
			*m_cmdType = hostapp::AppNoBurstRspNotification::CMD_Report_970;
			break;
		case CMDID_C971_WriteNeighbourPropertyFlag:
			*m_cmdType = hostapp::AppNoBurstRspNotification::CMD_Report_971;
			break;
		case CMDID_C973_WriteService:
			*m_cmdType = hostapp::AppNoBurstRspNotification::CMD_Report_973;
			break;
		case CMDID_C974_WriteRoute:
			*m_cmdType = hostapp::AppNoBurstRspNotification::CMD_Report_974;
			break;
		case CMDID_C975_DeleteRoute:
			*m_cmdType = hostapp::AppNoBurstRspNotification::CMD_Report_975;
			break;
		case CMDID_C976_WriteSourceRoute:
			*m_cmdType = hostapp::AppNoBurstRspNotification::CMD_Report_976;
			break;
		case CMDID_C977_DeleteSourceRoute:
			*m_cmdType = hostapp::AppNoBurstRspNotification::CMD_Report_977;
			break;
		default:
			m_isChangeNotification = false;
			*m_pNotificationMask = 0;
		}
		return;
	}

	memcpy(m_pAddr->bytes, &notif.hartPDU[dataBuffOffset], sizeof(m_pAddr->bytes));
	*m_pNotificationMask = 0;
	if (!(notif.hartPDU[dataLenOffset] > (sizeof(m_pAddr->bytes) + sizeof(char))))
	{
		LOG_ERROR_APP("[NOTIFICATION_PACKET]: invalid notification packet");
		return;
	}

	unsigned char cmdsNo = notif.hartPDU[dataBuffOffset + sizeof(m_pAddr->bytes)];

	if (notif.hartPDU[dataLenOffset] != ( WHART_RESPONSE_CODE_SIZE + WHART_DEVICE_STATUS_SIZE + sizeof(m_pAddr->bytes) + sizeof(char) + cmdsNo*sizeof(short)))
	{
		LOG_ERROR_APP("[NOTIFICATION_PACKET]: invalid notification packet");
		return;
	}

	for (int i = 0; i < cmdsNo; ++i)
	{
		unsigned short cmdNo = ntohs(*((unsigned short*)&notif.hartPDU[dataBuffOffset + sizeof(m_pAddr->bytes) + sizeof(char) + i*sizeof(short)]));

		switch(cmdNo)
		{
		case CMDID_C769_ReadJoinStatus:
		case CMDID_C778_ReadBatteryLife:
		case CMDID_C785_ReadGraphList:
		case CMDID_C814_ReadDeviceListEntries:
		case CMDID_C832_ReadNetworkDeviceIdentity:
		case CMDID_C833_ReadNetworkDeviceNeighbourHealth:
		case CMDID_C834_ReadNetworkTopologyInformation:
			*m_pNotificationMask = (*m_pNotificationMask) | NotificationMaskCodesMask_NetworkTopology;
			break;
		case CMDID_C001_ReadPrimaryVariable:
		case CMDID_C002_ReadLoopCurrentAndPercentOfRange:
		case CMDID_C003_ReadDynamicVariablesAndLoopCurrent:
		case CMDID_C009_ReadDeviceVariablesWithStatus:
		case CMDID_C033_ReadDeviceVariables:
		case CMDID_C178_PublishedDynamicData:
			*m_pNotificationMask = (*m_pNotificationMask) | NotificationMaskCodesMask_BurstMode;
			break;

		case CMDID_C779_ReportDeviceHealth:
		case CMDID_C780_ReportNeighborHealthList:
		case CMDID_C783_ReadSuperframeList:
		case CMDID_C784_ReadLinkList:
		case CMDID_C787_ReportNeighborSignalLevels:
		case CMDID_C788_AlarmPathDown:
		case CMDID_C789_AlarmSourceRouteFailed:
		case CMDID_C790_AlarmGraphRouteFailed:
		case CMDID_C791_AlarmTransportLayerFailed:
		case CMDID_C800_ReadServiceList:
		case CMDID_C802_ReadRouteList:
		case CMDID_C803_ReadSourceRoute:
			*m_pNotificationMask = (*m_pNotificationMask) | NotificationMaskCodesMask_NetworkSchedule;
			break;

		case CMDID_C000_ReadUniqueIdentifier:
		case CMDID_C020_ReadLongTag:
			*m_pNotificationMask = (*m_pNotificationMask) | NotificationMaskCodesMask_DeviceConfiguration;
			break;
		default:
			break;
		}
	}
}


int MsgUnserializer::Unserialize(const stack::transport::MessageIP& packet, stack::CHartCmdWrapper &parser,
					stack::WHartCommand &whCmd /*[in/out]*/)
{
	m_pParser = &parser;
	m_pWHCmd = &whCmd;
	const_cast<stack::transport::MessageIP&> (packet).Accept(*this);
	return m_error;
}

////////
static int UnserializeWHCommand(const stack::transport::MessageIP& packet, stack::WHartCommand &whCmd,
								bool p_bIsNotify/* = false*/)
{
	std::basic_string<boost::uint8_t>& hartPDU = ((p_bIsNotify) ? ((stack::transport::HARTDirectPDU_Notify&)packet).hartPDU :
		((stack::transport::HARTDirectPDU_Response&)packet).hartPDU);
	const char* msgType = (!p_bIsNotify) ? "'response'" : "'notification'";

	whCmd.commandID = htons(*((unsigned short*)&hartPDU[WHART_CMD_NO_OFFSET]));
	if (IsResponseCodeError(hartPDU[WHART_RESPONSE_CODE_OFFSET]))
	{
		whCmd.responseCode = hartPDU[WHART_RESPONSE_CODE_OFFSET];
		return 0;
	}

	int tableIndex = FindEntry ( g_composeReqParseResp, g_nComposeReqParseRespSize, whCmd.commandID);
	ParseFunction parser = 0;
	GetParsedLenFctType getParsedLen = 0;
	GetBinLenFctType getBinLen = 0;

	if (tableIndex >= 0 )
	{
		const ParseExecuteComposerEntry& tableEntry = g_composeReqParseResp[tableIndex];
		parser = tableEntry.fnParser;
		getParsedLen = tableEntry.fnGetParsedLen;
		getBinLen = tableEntry.fnGetBinLen;
		if( parser == 0 || getParsedLen == 0 || getBinLen == 0)
		{	LOG_ERROR_APP("[GatewayIO]: Resp -> " << msgType << " undefined: parser =" << parser << " getParsedLen =" << getParsedLen << " getBinLen =" << getParsedLen);
			return -1;
		}
	}
	else
	{	LOG_ERROR_APP("[GatewayIO]: Resp -> " << msgType << "Cmd not implemented");
		return -1;
	}

	BinaryStream stream;
	STREAM_INIT(&stream, (const uint8_t*) &hartPDU[WHART_RESP_DATA_BUFFER_OFFSET], hartPDU[WHART_DATA_LENGTH_OFFSET]-WHART_RESPONSE_CODE_SIZE-WHART_DEVICE_STATUS_SIZE);

	ParserContext context;
	context.allocatedCommandSize = 512;
	context.maxNeededParsedSize = 512;

	if (stream.remainingBytes < getBinLen())
	{	LOG_ERROR_APP("[GatewayIO]: Resp -> " << msgType << " Too few data bytes received.");
		return -1; //too few bytes
	}

	context.maxNeededParsedSize = getParsedLen();
	uint8_t u8Res = (*parser)(WHCommandBuff, &context, &stream);
	if (u8Res != RCS_N00_Success)
	{
		LOG_ERROR_APP("[GatewayIO]: Resp -> " << msgType << " parser error="<<(int)u8Res);
		return -1;
	}

	whCmd.command = WHCommandBuff;
	whCmd.responseCode = hartPDU[WHART_RESPONSE_CODE_OFFSET];

	LOG_DEBUG_APP("[GatewayIO]: Resp -> " << msgType << " parsing successfully for cmd_id=" << (int)whCmd.commandID << "with data_len=" << (int)hartPDU[WHART_DATA_LENGTH_OFFSET]);

	//for logging only
	if (LOG_DEBUG_ENABLED_APP())
	{
		std::string strMsg;
		char szNo[10];

		//status
		strMsg = "wh_resp: comm_status=";
		sprintf(szNo, "%d", packet.status);
		strMsg += szNo;

		//transac
		strMsg += " transacID=";
		sprintf(szNo, "%d", packet.transactionID);
		strMsg += szNo;

		//address
		strMsg += " address=0x";
		{
			std::ostringstream strBytes;
			strBytes << std::hex << std::uppercase << std::setfill('0');

			for (int i = 0; i < WHART_DEV_ADDRESS_SIZE; i++)
				strBytes << std::setw(2) << (unsigned int)hartPDU[WHART_DEV_ADDRESS_OFFSET + i] << ' ';

			strMsg += strBytes.str();
		}

		//cmdNo
		strMsg += " cmdNo=";
		sprintf(szNo, "%d", htons(*((unsigned short*)&hartPDU[WHART_CMD_NO_OFFSET])));
		strMsg += szNo;

		//ByteCount
		strMsg += " dataLength=";
		sprintf(szNo, "%d", hartPDU[WHART_DATA_LENGTH_OFFSET]);
		strMsg += szNo;

		//response code
		strMsg += " respCode=";
		sprintf(szNo, "%d", hartPDU[WHART_RESPONSE_CODE_OFFSET]);
		strMsg += szNo;

		if (!IsResponseCodeError(hartPDU[WHART_RESPONSE_CODE_OFFSET]))
		{
			//databytes
			strMsg += " dataBytes=0x";
			{
				std::ostringstream strBytes;
				strBytes << std::hex << std::uppercase << std::setfill('0');

				for (int i = WHART_RESP_DATA_BUFFER_OFFSET; i < (int)hartPDU.size(); i++)
					strBytes << std::setw(2) << (unsigned int)hartPDU[i] << ' ';

				strMsg += strBytes.str();
			}
		}

		LOG_DEBUG_APP("[GatewayIO]: Resp -> " << msgType << " message_received = " << strMsg.c_str());
	}

	return 0;
}

void MsgUnserializer::UnserializeHARTDirectPDUMessage(stack::transport::MessageIP& msg, bool p_bIsNotify/* = false*/)
{
	std::basic_string<boost::uint8_t>& hartPDU = ((p_bIsNotify) ? ((stack::transport::HARTDirectPDU_Notify&)msg).hartPDU :
																  ((stack::transport::HARTDirectPDU_Response&)msg).hartPDU);

	const char* msgType = (!p_bIsNotify) ? "response" : "notification";

	WHartUniqueID uniqueID;
	m_error = 0;
	uint8_t errorCodeResponse = 0;
	if (m_pParser->LoadHartDirect(true, hartPDU.c_str(), hartPDU.size(), uniqueID, errorCodeResponse ) < 0)
	{
		LOG_ERROR_APP("[GatewayIO]: Resp -> invalid " << msgType << " data when loading it for parsing");
		m_error = -1;
		return;
	}

	LOG_DEBUG_APP("[GatewayIO]: Resp -> loading  " << msgType << "  data successfully with cmd_id=" << m_pParser->GetCmdId());

	if (m_pParser->GetParsedFromRaw() > 0)
	{
		LOG_ERROR_APP("[GatewayIO]: Resp -> error parsing  " << msgType << "  data");
		m_error = -1;
		return;
	}

	LOG_DEBUG_APP("[GatewayIO]: Resp -> parsing successfully for cmd_id=" << m_pParser->GetCmdId());
	m_pWHCmd->commandID = m_pParser->GetCmdId();
	m_pWHCmd->responseCode = m_pParser->GetResponseCode();
	int len = m_pParser->GetParsedDataLen();
	memcpy(WHCommandBuff,m_pParser->GetParsedData(), len);
	m_pWHCmd->command = WHCommandBuff;

	//for logging only
	if (LOG_DEBUG_ENABLED_APP())
	{
		std::string strMsg;
		char szNo[10];

		//status
		strMsg = "wh_resp: comm_status=";
		sprintf(szNo, "%d", msg.status);
		strMsg += szNo;

		//transac
		strMsg += " transacID=";
		sprintf(szNo, "%d", msg.transactionID);
		strMsg += szNo;

		//address
		strMsg += " address=0x";
		{
			std::ostringstream strBytes;
			strBytes << std::hex << std::uppercase << std::setfill('0');

			for (int i = 0; i < WHART_DEV_ADDRESS_SIZE; i++)
				strBytes << std::setw(2) << (unsigned char)hartPDU[WHART_DEV_ADDRESS_OFFSET + i] << ' ';

			strMsg += strBytes.str();
		}

		//cmdNo
		strMsg += " cmdNo=";
		sprintf(szNo, "%d", htons(*((unsigned short*)&hartPDU[WHART_CMD_NO_OFFSET])));
		strMsg += szNo;

		//ByteCount
		strMsg += " dataLength=";
		sprintf(szNo, "%d", hartPDU[WHART_DATA_LENGTH_OFFSET]);
		strMsg += szNo;

		//response code
		strMsg += " respCode=";
		sprintf(szNo, "%d", hartPDU[WHART_RESPONSE_CODE_OFFSET]);
		strMsg += szNo;

		if (!IsResponseCodeError(hartPDU[WHART_RESPONSE_CODE_OFFSET]))
		{
			//databytes
			strMsg += " dataBytes=0x";
			{
				std::ostringstream strBytes;
				strBytes << std::hex << std::uppercase << std::setfill('0');

				for (int i = WHART_RESP_DATA_BUFFER_OFFSET; i < (int)hartPDU.size(); i++)
					strBytes << std::setw(2) << (unsigned char)hartPDU[i] << ' ';

				strMsg += strBytes.str();
			}
		}

		LOG_DEBUG_APP("[GatewayIO]: Resp -> message_received = " << strMsg.c_str());
	}
}


void MsgUnserializer::Visit(stack::transport::HARTDirectPDU_Notify& notif)
{
	m_error = UnserializeWHCommand(notif, *m_pWHCmd, true);
}

void MsgUnserializer::Visit(stack::transport::HARTDirectPDU_Response& res)
{
	m_error = UnserializeWHCommand(res, *m_pWHCmd);
}



int MsgSerializer::Serialize(const WHartUniqueID &addr, const stack::WHartCommand &whCmd, stack::CHartCmdWrapper &parser,
					stack::transport::MessageIP& packet/*[in/out]*/)
{
	m_pParser = &parser;
	m_whCmd = whCmd;
	m_addr = addr;
	const_cast<stack::transport::MessageIP&> (packet).Accept(*this);
	return m_error;
}


void MsgSerializer::Visit(stack::transport::HARTDirectPDU_Request& req)
{
	m_error = 0;

	//compose data
	m_pParser->LoadParsed(m_whCmd.commandID, WHART_PARSED_REQ_MAX_SIZE, m_whCmd.command, -1/*no response code*/);

	if (m_pParser->GetRawFromParsed() > 0)
	{
		LOG_ERROR_APP("[GatewayIO]: Req -> error when composing request for cmd_id = " << m_whCmd.commandID);
		m_error = -1;
		return;
	}

	LOG_DEBUG_APP("[GatewayIO]: Req -> composing request successfully for cmd_id=" << m_pParser->GetCmdId());

	unsigned short uwNo = ntohs(m_whCmd.commandID);
	unsigned char  ucNo = m_pParser->GetRawDataLen();

	req.hartPDU.insert(req.hartPDU.end(), m_addr.bytes, m_addr.bytes + sizeof(m_addr.bytes));
	req.hartPDU.insert(req.hartPDU.end(), (unsigned char*)(&uwNo), ((unsigned char*)(&uwNo)) + sizeof(uwNo));
	req.hartPDU.insert(req.hartPDU.end(), &ucNo, &ucNo + sizeof(ucNo) );
	req.hartPDU.insert(req.hartPDU.end(), (unsigned char*)(m_pParser->GetRawData()), (unsigned char*)(m_pParser->GetRawData()) + ucNo);

	//for logging only
	if (LOG_DEBUG_ENABLED_APP())
	{
		std::string strMsg;
		char szNo[100];

		//address
		strMsg += " address=0x";
		{
			std::ostringstream strBytes;
			strBytes << std::hex << std::uppercase << std::setfill('0');

			for (int i = 0; i < (int)sizeof(m_addr.bytes); i++)
				strBytes << std::setw(2) << (int)m_addr.bytes[i] << ' ';

			strMsg += strBytes.str();
		}

		//cmdNo
		strMsg += " cmdNo=";
		sprintf(szNo, "%d", m_whCmd.commandID);
		strMsg += szNo;

		//ByteCount
		strMsg += " dataLength=";
		sprintf(szNo, "%d", m_pParser->GetRawDataLen());
		strMsg += szNo;

		//databytes
		strMsg += " dataBytes=0x";
		if (m_pParser->GetRawDataLen() > 0)
		{
			std::ostringstream strBytes;
			strBytes << std::hex << std::uppercase << std::setfill('0');

			for (int i = 0 ; i < m_pParser->GetRawDataLen(); i++)
				strBytes << std::setw(2) << (int)(((char*)m_pParser->GetRawData())[i]) << ' ';

			strMsg += strBytes.str();
		}
		else
		{
			strMsg += "'no bytes'";
		}

		LOG_DEBUG_APP("[GatewayIO]: Req -> message_composed = " << strMsg.c_str());
	}
}



}
}
