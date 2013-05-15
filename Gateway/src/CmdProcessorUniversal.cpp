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
// C++ Implementation: CmdProcessorUniversal
//
// Created on: Nov 2, 2009
//
// Author: Catrina Mihailescu <catrina@r2d2>, (C) 2009
//
//
//
#include<iostream>
#include <WHartGateway/CmdProcessorUniversal.h>
#include <WHartGateway/Gateway.h>

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
#include <time.h>
#include <sys/time.h>

using namespace boost;

namespace hart7 {
namespace gateway{

CmdProcessorUniversal::CmdProcessorUniversal(Gateway & _gateway):
	CmdProcessor(_gateway)
{

}
// returns	true	- the command was processed, a response (even error response) is prepared and it will be sent
//		MUST return false only if the command should be fwd

bool CmdProcessorUniversal::ExecLocalCmdUniversal(CHartCmdWrapper::Ptr & p_pCmd, CHartCmdWrapper::Ptr & p_pRsp, CHostAppSession::Ptr & p_pHostSession)
{
	bool bRet = false;

	switch (p_pCmd->GetCmdId())
	{
        case CMDID_C000_ReadUniqueIdentifier:
        {
            C000_ReadUniqueIdentifier_Resp stResp;

            FillGwIdentityResponse(&stResp, gateway.config);

            p_pRsp.reset(new CHartCmdWrapper);

            p_pRsp->LoadParsed(p_pCmd->GetCmdId(), sizeof(C000_ReadUniqueIdentifier_Resp), &stResp, RCS_N00_Success);

            bRet = true; // the command is not fwd
        }
        break;

        case CMDID_C001_ReadPrimaryVariable:
        {
            // empty request
            C001_ReadPrimaryVariable_Resp stResp;

            stResp.primaryVariableUnit = (UnitsCodes) DeviceVariableFamilyCodes_NotUsed; // code: 250 - Not used
            stResp.primaryVariable = 0;

            p_pRsp.reset(new CHartCmdWrapper);
            p_pRsp->LoadParsed(p_pCmd->GetCmdId(), sizeof(C001_ReadPrimaryVariable_Resp), &stResp, RCS_N00_Success );
            bRet = true;
        }
        break;

        case CMDID_C002_ReadLoopCurrentAndPercentOfRange:
        {
            //empty request
            C002_ReadLoopCurrentAndPercentOfRange_Resp stResp;

            stResp.primaryVariableLoopCurrent = 0;
            stResp.primaryVariablePercentOfRange = 0;

            p_pRsp.reset(new CHartCmdWrapper);
            p_pRsp->LoadParsed(p_pCmd->GetCmdId(), sizeof(C002_ReadLoopCurrentAndPercentOfRange_Resp), &stResp, RCS_N00_Success );
            bRet = true;
        }
        break;

        case CMDID_C003_ReadDynamicVariablesAndLoopCurrent:
        {
            //empty request
            C003_ReadDynamicVariablesAndLoopCurrent_Resp stResp;
            stResp.noOfVariables = 4;

            stResp.primaryVariableLoopCurrent = 0;

            stResp.variables[0].unitsCode = 250; // Code - Not used
            stResp.variables[0].variable = 0;

            stResp.variables[1].unitsCode = 250; //Code - Not used
            stResp.variables[1].variable = 0;

            stResp.variables[2].unitsCode = 250; //Code - Not used
            stResp.variables[2].variable = 0;

            stResp.variables[3].unitsCode = 250; //Code - Not used
            stResp.variables[3].variable = 0;

            p_pRsp.reset(new CHartCmdWrapper);
            p_pRsp->LoadParsed(p_pCmd->GetCmdId(), sizeof(C003_ReadDynamicVariablesAndLoopCurrent_Resp), &stResp, RCS_N00_Success );
            bRet = true;
        }
        break;

        case CMDID_C006_WritePollingAddress:
        {

            C006_WritePollingAddress_Req* stReq = (C006_WritePollingAddress_Req*) p_pCmd->GetParsedData();
            C006_WritePollingAddress_Resp stResp;

            if((stReq->pollingAddressDevice == 0) && (stReq->loopCurrentModeCode == LoopCurrentModeCodes_Disabled))
            {
                stResp.pollingAddressDevice = 0;
                stResp.loopCurrentModeCode = LoopCurrentModeCodes_Disabled;
                p_pRsp.reset(new CHartCmdWrapper);
                p_pRsp->LoadParsed(p_pCmd->GetCmdId(), sizeof(C006_WritePollingAddress_Resp), &stResp, RCS_N00_Success);
            }
            else
            {
                stResp.pollingAddressDevice = stReq->pollingAddressDevice;
                stResp.loopCurrentModeCode = stReq->loopCurrentModeCode;
                p_pRsp.reset(new CHartCmdWrapper);
                p_pRsp->LoadParsed(p_pCmd->GetCmdId(), sizeof(C006_WritePollingAddress_Resp), &stResp, RCS_E16_AccessRestricted);
            }

            bRet = true;
        }
        break;

        case CMDID_C007_ReadLoopConfiguration:
        {
            //empty request
            C007_ReadLoopConfiguration_Resp stResp;

            stResp.pollingAddressDevice = 0;
            stResp.loopCurrentModeCode = LoopCurrentModeCodes_Disabled;
            p_pRsp.reset(new CHartCmdWrapper);
            p_pRsp->LoadParsed(p_pCmd->GetCmdId(), sizeof(C007_ReadLoopConfiguration_Resp), &stResp, RCS_N00_Success);

            bRet = true;
        }
        break;

        case CMDID_C008_ReadDynamicVariableClassifications:
        {
            //empty request
            C008_ReadDynamicVariableClassifications_Resp stResp;

            stResp.primaryVariableClassification = DeviceVariableClassificationCodes_DeviceVariableNotClassified; // integer - 0
            stResp.secondaryVariableClassification = DeviceVariableClassificationCodes_DeviceVariableNotClassified;
            stResp.tertiaryVariableClassification = DeviceVariableClassificationCodes_DeviceVariableNotClassified;
            stResp.quaternaryVariableClassification = DeviceVariableClassificationCodes_DeviceVariableNotClassified;

            p_pRsp.reset(new CHartCmdWrapper);
            p_pRsp->LoadParsed(p_pCmd->GetCmdId(), sizeof(C008_ReadDynamicVariableClassifications_Resp), &stResp, RCS_N00_Success);

            bRet = true;
        }
        break;

        case CMDID_C009_ReadDeviceVariablesWithStatus:
        {
            C009_ReadDeviceVariablesWithStatus_Resp stResp;
            stResp.variablesSize = 4;

            stResp.extendedFieldDeviceStatus = gateway.config.m_u8FlagBits;

            stResp.slots[0].deviceVariableCode = DeviceVariableCodes_PrimaryVariable; //PV
            stResp.slots[0].deviceVariableClassificationCode = (DeviceVariableClassificationCodes) 0;	//set to "0" for not supported Device Vars
            stResp.slots[0].unitCode = (UnitsCodes) 250;				//Code -Not Used
            stResp.slots[0].deviceVariableValue = 2;//(float)127.00;			//0x7F
            stResp.slots[0].deviceVariableStatus = 48;				//0x30



            stResp.slots[1].deviceVariableCode = DeviceVariableCodes_SecondaryVariable; //SV
            stResp.slots[1].deviceVariableClassificationCode = (DeviceVariableClassificationCodes) 0;	//set to "0" for not supported Device Vars
            stResp.slots[1].unitCode = (UnitsCodes) 250;				//Code -Not Used
            stResp.slots[1].deviceVariableValue = 160;				//0xA0
            stResp.slots[1].deviceVariableStatus = 48;				//0x30

            stResp.slots[2].deviceVariableCode = DeviceVariableCodes_TertiaryVariable; //TV
            stResp.slots[2].deviceVariableClassificationCode = (DeviceVariableClassificationCodes) 0;	//set to "0" for not supported Device Vars
            stResp.slots[2].unitCode = (UnitsCodes) 250;				//Code -Not Used
            stResp.slots[2].deviceVariableValue = 0;				//0x00
            stResp.slots[2].deviceVariableStatus = 48;				//0x30


            stResp.slots[3].deviceVariableCode = DeviceVariableCodes_QuaternaryVariable; //QV
            stResp.slots[3].deviceVariableClassificationCode = (DeviceVariableClassificationCodes) 0;	//set to "0" for not supported Device Vars
            stResp.slots[3].unitCode = (UnitsCodes) 250;				//Code -Not Used
            stResp.slots[3].deviceVariableValue = 0;				//0x00
            stResp.slots[3].deviceVariableStatus = 48;				//0x30

            time_t rawtime = time(NULL) *1000; // in miliseconds
            stResp.slot0TimeStamp.u32 = rawtime;

            p_pRsp.reset(new CHartCmdWrapper);
            p_pRsp->LoadParsed(p_pCmd->GetCmdId(), sizeof(stResp), &stResp,RCM_W30_CommandResponseTruncated);

            bRet = true;
        }
        break;

        case CMDID_C011_ReadUniqueIdentifierAssociatedWithTag:
        {
            C011_ReadUniqueIdentifierAssociatedWithTag_Req* pReq = (C011_ReadUniqueIdentifierAssociatedWithTag_Req*)p_pCmd->GetParsedData();
            bRet = true;  // the command is not fwd

            if (memcmp(pReq->tag, gateway.config.m_szTag, sizeof(pReq->tag) ) != 0 )
            {
                break;
            }

            C000_ReadUniqueIdentifier_Resp stResp;

            FillGwIdentityResponse(&stResp, gateway.config);

            p_pRsp.reset(new CHartCmdWrapper);

            p_pRsp->LoadParsed(p_pCmd->GetCmdId(),sizeof(stResp),&stResp,RCS_N00_Success);

        }
        break;

        case CMDID_C012_ReadMessage:
        {
            C012_ReadMessage_Resp stResp;

            memcpy(stResp.message, gateway.config.m_szCmdUniversalMessage, sizeof(stResp.message));

            p_pRsp.reset(new CHartCmdWrapper);
            p_pRsp->LoadParsed(p_pCmd->GetCmdId(), sizeof(stResp), &stResp, RCS_N00_Success);

            bRet = true;  // the command is not fwd
        }
        break;

        case CMDID_C013_ReadTagDescriptorDate:
        {
            C013_ReadTagDescriptorDate_Resp stResp;

            memcpy( stResp.tag, gateway.config.m_szTag, sizeof(stResp.tag) );
            memcpy( stResp.descriptor, gateway.config.m_szMasterDescriptor, sizeof(stResp.descriptor) );
            stResp.dateCode = gateway.config.m_stMasterDate;

            p_pRsp.reset(new CHartCmdWrapper);
            p_pRsp->LoadParsed(p_pCmd->GetCmdId(),sizeof(stResp), &stResp, RCS_N00_Success);

            bRet = true;  // the command is not fwd
        }
        break;

        case CMDID_C014_ReadPrimaryVariableTransducerInformation:
        {
            // does not apply
            p_pRsp.reset(new CHartCmdWrapper);
            p_pRsp->LoadParsed(p_pCmd->GetCmdId(), 0, NULL, RCS_E16_AccessRestricted);

            bRet = true;  // the command is not fwd
        }
        break;

        case CMDID_C015_ReadDeviceInformation:
        {
            // does not apply
            p_pRsp.reset(new CHartCmdWrapper);
            p_pRsp->LoadParsed(p_pCmd->GetCmdId(), 0, NULL, RCS_E16_AccessRestricted);

            bRet = true;  // the command is not fwd
        }
        break;

        case CMDID_C016_ReadFinalAssemblyNumber:
        {
            C016_ReadFinalAssemblyNumber_Resp stResp;


            stResp.finalAssemblyNumber = gateway.config.m_u32FinalAssemblyNumber;

            p_pRsp.reset(new CHartCmdWrapper);
            p_pRsp->LoadParsed(p_pCmd->GetCmdId(), sizeof(stResp), &stResp, RCS_N00_Success);

            bRet = true;  // the command is not fwd
        }
        break;

        case CMDID_C017_WriteMessage:
        {
            bRet = true;  // the command is not fwd
            C017_WriteMessage_Req* pReq =  (C017_WriteMessage_Req*) p_pCmd->GetParsedData();

            p_pRsp.reset(new CHartCmdWrapper);

            memcpy(gateway.config.m_szCmdUniversalMessage, pReq->messageString, sizeof(pReq->messageString));

            /// write to config file
            if(!gateway.config.WriteCmdUniversalMessage(gateway.config.m_szCmdUniversalMessage))
            {
                p_pRsp->LoadParsed(p_pCmd->GetCmdId(), 0, NULL, RCS_E06_DeviceSpecificCommandError);
                break;
            }

            C017_WriteMessage_Resp stResp;
            memset(&stResp, 0, sizeof(C017_WriteMessage_Resp));

            memcpy(stResp.messageString, pReq->messageString, sizeof(stResp.messageString));

            p_pRsp->LoadParsed(p_pCmd->GetCmdId(), sizeof(stResp), &stResp, RCS_N00_Success);
        }
        break;

        case CMDID_C018_WriteTagDescriptorDate:
        {
            C018_WriteTagDescriptorDate_Req* pReq =  (C018_WriteTagDescriptorDate_Req*) p_pCmd->GetParsedData();

            memcpy(gateway.config.m_szTag, pReq->tag, sizeof(pReq->tag) );
            memcpy( gateway.config.m_szMasterDescriptor, pReq->descriptorUsedByTheMasterForRecordKeeping, sizeof(pReq->descriptorUsedByTheMasterForRecordKeeping) );
            gateway.config.m_stMasterDate = pReq->dateCodeUsedByTheMasterForRecordKeeping;

            C018_WriteTagDescriptorDate_Resp stResp;
            memcpy( stResp.tag,gateway.config.m_szTag, sizeof(stResp.tag) );
            memcpy( stResp.descriptorUsedByTheMasterForRecordKeeping, gateway.config.m_szMasterDescriptor, sizeof(stResp.descriptorUsedByTheMasterForRecordKeeping) );
            stResp.dateCodeUsedByTheMasterForRecordKeeping = gateway.config.m_stMasterDate;

            p_pRsp.reset(new CHartCmdWrapper);

            /// write to config file
            if((!gateway.config.WriteCmdTag(gateway.config.m_szTag))||(!gateway.config.WriteCmdMasterDescriptor(gateway.config.m_szMasterDescriptor))||(!gateway.config.WriteCmdMasterDate(gateway.config.m_stMasterDate)))
            {
                p_pRsp->LoadParsed(p_pCmd->GetCmdId(),sizeof(stResp),&stResp,RCS_E06_DeviceSpecificCommandError);
            }
            else
            {
                memcpy( stResp.tag, gateway.config.m_szTag, sizeof(stResp.tag) );
                memcpy( stResp.descriptorUsedByTheMasterForRecordKeeping, gateway.config.m_szMasterDescriptor, sizeof(stResp.descriptorUsedByTheMasterForRecordKeeping) );
                stResp.dateCodeUsedByTheMasterForRecordKeeping = gateway.config.m_stMasterDate;

                p_pRsp->LoadParsed(p_pCmd->GetCmdId(),sizeof(stResp), &stResp, RCS_N00_Success);
            }
            bRet = true;  // the command is not fwd
        }
        break;

        case CMDID_C019_WriteFinalAssemblyNumber:
        {
            C019_WriteFinalAssemblyNumber_Req* pReq =  (C019_WriteFinalAssemblyNumber_Req*) p_pCmd->GetParsedData();
            p_pRsp.reset(new CHartCmdWrapper);
            C019_WriteFinalAssemblyNumber_Resp stResp;
            stResp.finalAssemblyNumber = gateway.config.m_u32FinalAssemblyNumber;
            gateway.config.m_u32FinalAssemblyNumber = pReq->finalAssemblyNumber;
            /// write to config file
            if(!gateway.config.WriteCmdFinalAssemblyNumber(gateway.config.m_u32FinalAssemblyNumber))
                p_pRsp->LoadParsed(p_pCmd->GetCmdId(),sizeof(stResp),&stResp,RCS_E06_DeviceSpecificCommandError);
            else
            {
                stResp.finalAssemblyNumber = gateway.config.m_u32FinalAssemblyNumber;
                p_pRsp->LoadParsed(p_pCmd->GetCmdId(),sizeof(stResp),&stResp,RCS_N00_Success);
            }
            bRet = true;  // the command is not fwd
        }
        break;

        case CMDID_C020_ReadLongTag:
        {
            C020_ReadLongTag_Resp stResp;
            memcpy(stResp.longTag, gateway.config.m_szLongTag, sizeof(stResp.longTag) );

            p_pRsp.reset(new CHartCmdWrapper);
            p_pRsp->LoadParsed(p_pCmd->GetCmdId(), sizeof(stResp), &stResp, RCS_N00_Success);

            bRet = true;  // the command is not fwd
        }
        break;

        case CMDID_C021_ReadUniqueIdentifierAssociatedWithLongTag:
        {
            C021_ReadUniqueIdentifierAssociatedWithLongTag_Req * pReq = (C021_ReadUniqueIdentifierAssociatedWithLongTag_Req*)p_pCmd->GetParsedData();

            bRet = true;  // the command is not fwd

            if (memcmp(pReq->longTag, gateway.config.m_szLongTag, sizeof(pReq->longTag) ) != 0 )
            {
                break;
            }

            C000_ReadUniqueIdentifier_Resp stResp;
            FillGwIdentityResponse(&stResp, gateway.config);

            p_pRsp.reset(new CHartCmdWrapper);
            p_pRsp->LoadParsed(p_pCmd->GetCmdId(), sizeof(C000_ReadUniqueIdentifier_Resp), &stResp, RCS_N00_Success);
        }
        break;

        case CMDID_C022_WriteLongTag:
        {
            C022_WriteLongTag_Req* pReq =  (C022_WriteLongTag_Req*) p_pCmd->GetParsedData();
            C020_ReadLongTag_Resp stResp;
            p_pRsp.reset(new CHartCmdWrapper);
            memcpy( stResp.longTag, gateway.config.m_szLongTag, sizeof(stResp.longTag) );
            memcpy( gateway.config.m_szLongTag, pReq->longTag, sizeof(pReq->longTag) );
            /// write to config file
            if(!gateway.config.WriteCmdLongTag(gateway.config.m_szLongTag))
                p_pRsp->LoadParsed(p_pCmd->GetCmdId(), sizeof(stResp), &stResp, RCS_E06_DeviceSpecificCommandError);
            else
            {
                memcpy( stResp.longTag, gateway.config.m_szLongTag, sizeof(stResp.longTag) );
                p_pRsp->LoadParsed(p_pCmd->GetCmdId(),sizeof(stResp), &stResp, RCS_N00_Success);
            }
            bRet = true;  // the command is not fwd
        }
        break;

        case CMDID_C038_ResetConfigurationChangedFlag:
        {
            C038_ResetConfigurationChangedFlag_Req * pReq =(C038_ResetConfigurationChangedFlag_Req*) p_pCmd->GetParsedData();

            C038_ResetConfigurationChangedFlag_Resp ccfResp;
            ccfResp.configurationChangeCounter = gateway.config.m_u16ConfigChangeCounter;

            bool configurationChangeCounterMatch = (pReq->configurationChangeCounter == g_Cmd38ConfigurationChangedCounter_Any)
                    || (memcmp(&gateway.config.m_u16ConfigChangeCounter, &pReq->configurationChangeCounter, sizeof(pReq->configurationChangeCounter)) == 0);

            if (configurationChangeCounterMatch)
            {
                // reset Configuration Change Bit (6th bit of Device Status) & return Success
                p_pRsp.reset(new CHartCmdWrapper);
                uint8_t configChangeBitMask = (~FieldDeviceStatusesMask_ConfigurationChanged); // 10111111 <- reset the 6th bit
                gateway.config.DeviceStatus(gateway.config.DeviceStatus() & configChangeBitMask);

                if (!gateway.config.WriteDeviceStatus(gateway.config.DeviceStatus()))
                {
                    p_pRsp->LoadParsed(p_pCmd->GetCmdId(), sizeof(ccfResp), &ccfResp, RCS_E06_DeviceSpecificCommandError);
                }
                else
                {
                    p_pRsp->LoadParsed(p_pCmd->GetCmdId(), sizeof(C038_ResetConfigurationChangedFlag_Resp), &ccfResp, RCS_N00_Success);
                }
            }
            else
            {
                // return Configuration Change Counter Mismatch Code
                p_pRsp.reset(new CHartCmdWrapper);
                p_pRsp->LoadParsed(	p_pCmd->GetCmdId(), sizeof(C038_ResetConfigurationChangedFlag_Resp), &ccfResp, RCM_E09_ConfigurationChangeCounterMismatch);
            }
            bRet = true;
            break;
        }

        case CMDID_C048_ReadAdditionalDeviceStatus:
        {
            bRet = true;  // the command is not fwd

            uint8_t u8Res = p_pCmd->GetParsedFromRaw();
            if ( IsResponseCodeError(u8Res))
            {
                LOG_WARN_APP("ExecLocalCmd: cmdId=" << p_pCmd->GetCmdId() << " error on parsing res= " << (int)u8Res);
                p_pRsp.reset(new CHartCmdWrapper);
                p_pRsp->LoadParsed (p_pCmd->GetCmdId(), 0, 0, u8Res);
                return false;
            }

            C048_ReadAdditionalDeviceStatus_Resp rdasResp;
            C048_ReadAdditionalDeviceStatus_RespCodes response_type = (C048_ReadAdditionalDeviceStatus_RespCodes) RCS_N00_Success;

            // compose the response
            p_pRsp.reset(new CHartCmdWrapper);

            memcpy(rdasResp.deviceSpecificStatus1, gateway.config.m_u8deviceSpecificStatus1, sizeof(gateway.config.m_u8deviceSpecificStatus1));
            rdasResp.extendedDeviceStatus 		=  gateway.config.m_u8FlagBits;
            rdasResp.deviceOperatingMode 		=  gateway.config.m_u8deviceOperatingMode;
            rdasResp.standardizedStatus0		=  gateway.config.m_u8standardizedStatus0;
            rdasResp.standardizedStatus1 		=  gateway.config.m_u8standardizedStatus1;
            rdasResp.analogChannelSaturatedCode =  gateway.config.m_u8analogChannelSaturatedCode;
            rdasResp.standardizedStatus2 		=  gateway.config.m_u8standardizedStatus2;
            rdasResp.standardizedStatus3 		=  gateway.config.m_u8standardizedStatus3;
            rdasResp.analogChannelFixedCode 	=  gateway.config.m_u8analogChannelFixedCode;
            rdasResp.respLen = 25;
            memcpy(rdasResp.deviceSpecificStatus2, gateway.config.m_u8deviceSpecificStatus2, sizeof(gateway.config.m_u8deviceSpecificStatus2));

            if (p_pCmd->GetRawDataLen() > 0)
            {
                rdasResp.respLen = p_pCmd->GetRawDataLen();

                C048_ReadAdditionalDeviceStatus_Req * pReq = (C048_ReadAdditionalDeviceStatus_Req*) p_pCmd->GetParsedData();

                bool mismatch = (memcmp(&pReq->deviceSpecificStatus1,
                                    gateway.config.m_u8deviceSpecificStatus1,
                                    sizeof(gateway.config.m_u8deviceSpecificStatus1) ) != 0 )
                            ||	(memcmp(&pReq->extendedDeviceStatus,
                                    &gateway.config.m_u8FlagBits,
                                    sizeof(pReq->extendedDeviceStatus) ) != 0 )
                            ||	(memcmp(&pReq->deviceOperatingMode,
                                    &gateway.config.m_u8deviceOperatingMode,
                                    sizeof(gateway.config.m_u8deviceOperatingMode)) != 0)
                            ||	(memcmp(&pReq->standardizedStatus0,
                                    &gateway.config.m_u8standardizedStatus0,
                                    sizeof(gateway.config.m_u8standardizedStatus0)) != 0)
                            ||	(memcmp(&pReq->standardizedStatus1,
                                    &gateway.config.m_u8standardizedStatus1,
                                    sizeof(gateway.config.m_u8standardizedStatus1)) != 0)
                            ||	(memcmp(&pReq->analogChannelSaturatedCode,
                                    &gateway.config.m_u8analogChannelSaturatedCode,
                                    sizeof(gateway.config.m_u8analogChannelSaturatedCode)) != 0)
                            ||	(memcmp(&pReq->standardizedStatus2,
                                    &gateway.config.m_u8standardizedStatus2,
                                    sizeof(gateway.config.m_u8standardizedStatus2)) != 0)
                            ||	(memcmp(&pReq->standardizedStatus3,
                                    &gateway.config.m_u8standardizedStatus3,
                                    sizeof(gateway.config.m_u8standardizedStatus3)) != 0)
                            ||	(memcmp(&pReq->analogChannelFixedCode,
                                    &gateway.config.m_u8analogChannelFixedCode,
                                    sizeof(gateway.config.m_u8analogChannelFixedCode)) != 0)
                            ||	(memcmp(pReq->deviceSpecificStatus2,
                                    gateway.config.m_u8deviceSpecificStatus2,
                                    sizeof(gateway.config.m_u8deviceSpecificStatus2)) != 0);

                if	(mismatch)
                {	// device status mismatch
                    response_type = (C048_ReadAdditionalDeviceStatus_RespCodes) RCM_W14_StatusBytesMismatch;
                }
            }

            p_pRsp->LoadParsed(p_pCmd->GetCmdId(), sizeof(C048_ReadAdditionalDeviceStatus_Resp), &rdasResp, response_type);

            break;
        }

        default:
        {
            LOG_WARN_APP("ExecLocalCmd: cmdId=" << p_pCmd->GetCmdId() << " CommandNotImplemented ");
            p_pRsp.reset(new CHartCmdWrapper);
            p_pRsp->LoadParsed(p_pCmd->GetCmdId(), 0, 0, RCS_E64_CommandNotImplemented);
            bRet = true;

        }
        break;
/////////////////////////// end universal commands /////////////////////////////////////////
	}

	return bRet;
}




}

}
