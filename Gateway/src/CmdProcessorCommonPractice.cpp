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

//
// C++ Implementation: CmdProcessorCommonPractice
//
// Description:
//
//
// Author: Catrina Mihailescu <catrina@r2d2>, (C) 2009
//
//
//
#include <WHartGateway/CmdProcessorCommonPractice.h>

#include <WHartStack/util/WHartCmdWrapper.h>
#include <WHartStack/WHartSubApplicationData.h>

#include <WHartGateway/AllGatewayCommands.h>
#include <WHartGateway/GwUtil.h>

#include <boost/foreach.hpp>
#include <boost/bind.hpp> //for binding callbacks
#include <Shared/UtilsSolo.h>

#include <WHartGateway/GatewayConfig.h>
#include <WHartGateway/GatewayStack.h>
#include <WHartStack/util/WHartCmdWrapper.h>
#include <WHartGateway/Gateway.h>
#include <WHartGateway/LogicalDevice.h>
#include <WHartGateway/HostAppSession.h>
#include <WHartGateway/GwRequest.h>


using namespace boost;

namespace hart7 {
namespace gateway{

CmdProcessorCommonPractice::CmdProcessorCommonPractice(Gateway & _gateway):
			CmdProcessor(_gateway)
{
}

bool CmdProcessorCommonPractice::ExecLocalCmdCommonPractice(CHartCmdWrapper::Ptr & p_pCmd, CHartCmdWrapper::Ptr & p_pRsp, CHostAppSession::Ptr & p_pHostSession)
{
	bool bRet = false;

	switch (p_pCmd->GetCmdId())
	{
		case CMDID_C041_PerformSelfTest:
		{
            bRet = true;
            C041_PerformSelfTest_Resp stResp;

            p_pRsp.reset(new CHartCmdWrapper);
            p_pRsp->LoadParsed(p_pCmd->GetCmdId(),sizeof(stResp), &stResp, RCS_N00_Success);
		}
		break;

		case CMDID_C042_PerformDeviceReset:
		{
            bRet = true;

            C042_PerformDeviceReset_Resp stResp;

             // set Reset Flag
            gateway.config.m_bResetFlag = true;
            gateway.Reset_GW();
            gateway.stack.NMSessionReset();
            LOG_INFO_APP("ADDED -- finished procedure for cmd 42!!");
            p_pRsp.reset(new CHartCmdWrapper);
            p_pRsp->LoadParsed(p_pCmd->GetCmdId(), sizeof(stResp), &stResp, RCS_N00_Success);

            // unset Reset Flag
            gateway.config.m_bResetFlag = false;
		}
		break;

		case CMDID_C059_WriteNumberOfResponsePreambles:
		{
			bRet = true;
			C059_WriteNumberOfResponsePreambles_Req * pReq = (C059_WriteNumberOfResponsePreambles_Req*) p_pCmd->GetParsedData();
			p_pRsp.reset(new CHartCmdWrapper);

			if (pReq == 0)
			{
			    p_pRsp->LoadParsed(p_pCmd->GetCmdId(), 0, NULL, RCS_E06_DeviceSpecificCommandError);
			}
			else if (pReq->noOfPreambles < (uint8_t)5)
			{
			    p_pRsp->LoadParsed(p_pCmd->GetCmdId(), 0, NULL, RCS_E04_PassedParameterTooSmall);
			}
			else if (pReq->noOfPreambles > (uint8_t) 20)
			{
			    p_pRsp->LoadParsed(p_pCmd->GetCmdId(), 0, NULL, RCS_E03_PassedParameterTooLarge);
			}
			else
			{
				/// write to config file
				if(!gateway.config.WriteMinRespPreamblesNo(pReq->noOfPreambles))
				{	p_pRsp->LoadParsed(p_pCmd->GetCmdId(), 0, NULL, RCS_E06_DeviceSpecificCommandError);
				}
				else
				{	C059_WriteNumberOfResponsePreambles_Resp stResp;
					stResp.noOfPreambles = pReq->noOfPreambles;
					gateway.config.m_u8MinRespPreamblesNo = pReq->noOfPreambles;
					p_pRsp->LoadParsed(p_pCmd->GetCmdId(), sizeof(stResp), &stResp, RCS_N00_Success);
				}
			}
		}
		break;

        case CMDID_C074_ReadIOSystemCapabilities:
        {
            bRet = true;
            p_pRsp.reset(new CHartCmdWrapper);

            C074_ReadIOSystemCapabilities_Resp	resp;
            memset(&resp, 0, sizeof(C074_ReadIOSystemCapabilities_Resp));

            resp.noOfIOCards = 1;
            resp.noOfChannelsPerIOCard = 1;
            resp.noOfSubdevicesPerChannel = 50;

            //ap and nm are not include in the list
            if (gateway.m_oDevices.GetDeviceUniqueIDMap().size() > 2 )
            {
                resp.noOfDevicesDetected = gateway.m_oDevices.GetDeviceUniqueIDMap().size() - 2;
            }
            else
            {
                resp.noOfDevicesDetected = 0;
            }

            resp.noOfDelayedResponsesSupported = 2;
            resp.masterMode = 1;
            resp.sendRetryCount = 3;

            p_pRsp->LoadParsed(p_pCmd->GetCmdId(), sizeof(C074_ReadIOSystemCapabilities_Resp), &resp, RCS_N00_Success );

            break;
        }

        case CMDID_C075_PollSubDevice:
        {
            bRet = true;
            p_pRsp.reset(new CHartCmdWrapper);

            C075_PollSubDevice_Req* pReq = (C075_PollSubDevice_Req*) p_pCmd->GetParsedData();
            C075_PollSubDevice_Resp resp;

            int rc = gateway.m_oDevices.Fill_C075_PollSubDevice(pReq, &resp);
            p_pRsp->LoadParsed(p_pCmd->GetCmdId(), sizeof(C075_PollSubDevice_Resp), &resp, rc);

            break;
        }

        case CMDID_C084_ReadSubDeviceIdentitySummary:
        {
            bRet = true;
            p_pRsp.reset(new CHartCmdWrapper);

            C084_ReadSubDeviceIdentitySummary_Req* pReq = (C084_ReadSubDeviceIdentitySummary_Req*) p_pCmd->GetParsedData();
            C084_ReadSubDeviceIdentitySummary_Resp resp;

            int rc = gateway.m_oDevices.Fill_C084_ReadSubDeviceIdentitySummary(pReq, &resp);
            p_pRsp->LoadParsed(p_pCmd->GetCmdId(), sizeof(C084_ReadSubDeviceIdentitySummary_Resp), &resp, rc );

            break;
        }

        case CMDID_C085_ReadIOChannelStatistics:
        {
            bRet = true;
            p_pRsp.reset(new CHartCmdWrapper);

            C085_ReadIOChannelStatistics_Req* pReq = (C085_ReadIOChannelStatistics_Req*) p_pCmd->GetParsedData();
            C085_ReadIOChannelStatistics_Resp resp;

            int rc = gateway.m_oDevices.Fill_C085_ReadIOChannelStatistics(pReq, &resp);
            p_pRsp->LoadParsed(p_pCmd->GetCmdId(), sizeof(C085_ReadIOChannelStatistics_Resp), &resp, rc );
            break;
        }

        case CMDID_C086_ReadSubDeviceStatistics:
        {
            bRet = true;
            p_pRsp.reset(new CHartCmdWrapper);

            C086_ReadSubDeviceStatistics_Req* pReq = (C086_ReadSubDeviceStatistics_Req*) p_pCmd->GetParsedData();
            C086_ReadSubDeviceStatistics_Resp resp;

            int rc = gateway.m_oDevices.Fill_C086_ReadSubDeviceStatistics(pReq, &resp);
            p_pRsp->LoadParsed(p_pCmd->GetCmdId(), sizeof(C086_ReadSubDeviceStatistics_Resp), &resp, rc );
            break;
        }

        case CMDID_C087_WriteIOSystemMasterMode:
        {
            bRet = true;
            p_pRsp.reset(new CHartCmdWrapper);

            C087_WriteIOSystemMasterMode_Req* pReq = (C087_WriteIOSystemMasterMode_Req*) p_pCmd->GetParsedData();
            C087_WriteIOSystemMasterMode_Resp resp;
            resp.masterMode = 1;

            int rc;
            if (pReq->masterMode != 1/*primary master*/)
            {	rc = C087_E16;/*RCS_E16_AccessRestricted*/
            }
            else
            {	rc = RCS_N00_Success;
            }

            p_pRsp->LoadParsed(p_pCmd->GetCmdId(), sizeof(C087_WriteIOSystemMasterMode_Resp), &resp, rc);

            break;
        }

        case CMDID_C088_WriteIOSystemRetryCount:
        {
            bRet = true;
            p_pRsp.reset(new CHartCmdWrapper);

            C088_WriteIOSystemRetryCount_Req* pReq = (C088_WriteIOSystemRetryCount_Req*) p_pCmd->GetParsedData();

            C088_WriteIOSystemRetryCount_Resp resp;
            resp.retryCount = gateway.config.GwReqMaxRetryNo();

            int rc;
            if (pReq == 0)
            {
                rc = C088_E06/* = RCS_E06_DeviceSpecificCommandError*/;
            }
            else if (pReq->retryCount < 2)
            {
                rc = C088_E04 /*= RCS_E04_PassedParameterTooSmall*/;
            }
            else if (pReq->retryCount > 5)
            {
                rc = C088_E03/* = RCS_E03_PassedParameterTooLarge*/;
            }
            else
            {
                rc = RCS_N00_Success;
                gateway.config.WriteGwReqMaxRetryNo(pReq->retryCount);
                gateway.config.m_nGwReqMaxRetryNo = pReq->retryCount;
                resp.retryCount = pReq->retryCount;
            }

            p_pRsp->LoadParsed(p_pCmd->GetCmdId(), sizeof(C088_WriteIOSystemRetryCount_Resp), &resp, rc);
            break;
        }

        case CMDID_C089_SetRealTimeClock:
        {
            bRet = true;
            p_pRsp.reset(new CHartCmdWrapper);

            C089_SetRealTimeClock_Req* pReq = (C089_SetRealTimeClock_Req*) p_pCmd->GetParsedData();
            C089_SetRealTimeClock_Resp stResp;

            memcpy(&stResp.timeSetCode , &pReq->timeSetCode, sizeof(stResp.timeSetCode));
            memcpy(&stResp.date.day, &pReq->date.day, sizeof(stResp.date.day));

            memcpy(&stResp.date.month, &pReq->date.month, sizeof(stResp.date.month));
            memcpy(&stResp.date.year, &pReq->date.year, sizeof(stResp.date.year));
            memcpy(&stResp.time, &pReq->time, sizeof(stResp.time));

            p_pRsp->LoadParsed(p_pCmd->GetCmdId(), sizeof(stResp), &stResp, RCS_E07_InWriteProtectMode);

        }
        break;

        case CMDID_C106_FlushDelayedResponses:
        {
            bRet = true;
            C106_FlushDelayedResponses_Resp stResp;
            p_pRsp.reset(new CHartCmdWrapper);
            p_pRsp->LoadParsed(p_pCmd->GetCmdId(), sizeof(stResp), &stResp,RCS_N00_Success );

            }
            break;

        case CMDID_C109_BurstModeControl:
        {
            C109_BurstModeControl_Req* pReq = (C109_BurstModeControl_Req*) p_pCmd->GetParsedData();
            C109_BurstModeControl_Resp stResp;

            stResp.burstModeCode = pReq->burstModeCode;
            stResp.burstMessage = pReq->burstMessage;

            bRet = true;

            p_pRsp.reset(new CHartCmdWrapper);
            p_pRsp->LoadParsed(p_pCmd->GetCmdId(), sizeof(stResp), &stResp,RCS_N00_Success );
        }
        break;

        default:
        {
            LOG_WARN_APP("ExecLocalCmd: cmdId="<<p_pCmd->GetCmdId()<<" CommandNotImplemented ");
            p_pRsp.reset(new CHartCmdWrapper);
            p_pRsp->LoadParsed (p_pCmd->GetCmdId(), 0, 0, RCS_E64_CommandNotImplemented);
            bRet = true;
        }
        break;
	}
	return bRet;
}

}
}
