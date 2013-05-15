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
 * C178_PublishedDynamicData.c
 *
 *  Created on: Feb 15, 2011
 *      Author: Sorin.Bidian
 */
#include "C178_PublishedDynamicData.h"
#include "../UniversalCommands/C009_ReadDeviceVariablesWithStatus.h"
#include "../UniversalCommands/C048_ReadAdditionalDeviceStatus.h"

uint8_t Compose_C178_PublishedDynamicData_Req(C178_PublishedDynamicData_Req* request, ComposerContext* context,
		BinaryStream* toStream)
{
	context->cmdId = CMDID_C178_PublishedDynamicData;

	STREAM_WRITE_UINT8(toStream, request->Header_STX);
	STREAM_WRITE_BYTES(toStream, &request->Address, 5);
	STREAM_WRITE_UINT8(toStream, request->Command);
	STREAM_WRITE_UINT8(toStream, request->ByteCount);
	STREAM_WRITE_UINT8(toStream, request->Parity);

	return RCS_N00_Success;
}

/**
 * @Debinarize
 */
uint8_t Parse_C178_PublishedDynamicData_Req(C178_PublishedDynamicData_Req* request, ParserContext* context,
		BinaryStream* fromStream)
{
#ifdef _DEBUG
// 	if (2 > fromStream->remainingBytes)
// 		return RCS_E05_TooFewDataBytesReceived;
#endif

	STREAM_READ_UINT8(fromStream, &request->Header_STX);
	STREAM_READ_BYTES(fromStream, &request->Address, 5);
	STREAM_READ_UINT8(fromStream, &request->Command);
	STREAM_READ_UINT8(fromStream, &request->ByteCount);
	STREAM_READ_UINT8(fromStream, &request->Parity);

	return RCS_N00_Success;
}

/**
 *
 */
uint8_t Compose_C178_PublishedDynamicData_Resp(C178_PublishedDynamicData_Resp* response, ComposerContext* context,
		BinaryStream* toStream)
{
	context->cmdId = CMDID_C178_PublishedDynamicData;

//	STREAM_WRITE_UINT8(toStream, response->Header_ACK);
//	STREAM_WRITE_BYTES(toStream, &response->Address, 5);
//	STREAM_WRITE_UINT8(toStream, response->Command);
//	STREAM_WRITE_UINT8(toStream, response->ByteCount);
//	STREAM_WRITE_UINT8(toStream, response->ResponseCode);
//	STREAM_WRITE_UINT8(toStream, response->DeviceStatus);
	STREAM_WRITE_UINT8(toStream, response->NumberOfCommandsInStructure);


//    if (response->NumberOfCommandsInStructure > 0)
//    {
//        for(int i = 0; i < response->NumberOfCommandsInStructure; i++)
//        {
//            if (response->c009_ID == CMDID_C009_ReadDeviceVariablesWithStatus)
//            {
//                STREAM_WRITE_UINT16(toStream, response->c009_ID);
//                Compose_C009_ReadDeviceVariablesWithStatus_Resp(&response->c009, context, toStream);
//            }
//            else if (response->c048_ID == CMDID_C048_ReadAdditionalDeviceStatus)
//            {
//                STREAM_WRITE_UINT16(toStream, response->c048_ID);
//                Compose_C048_ReadAdditionalDeviceStatus_Resp(&response->c048, context, toStream);
//            }
//        }
//    }

	if (response->NumberOfCommandsInStructure > 0) {

        // 1. FirstCommand - cmdID=9
        STREAM_WRITE_UINT16(toStream, response->FirstCommandNumber);
        STREAM_WRITE_UINT8(toStream, response->FirstCommandByteCount);
        STREAM_WRITE_UINT8(toStream, response->FirstCommandResponseCode);
        STREAM_WRITE_UINT8(toStream, response->FirstExtendedDeviceStatus);

        // Primary
        STREAM_WRITE_UINT8(toStream, response->VarCodes_Primary);
        STREAM_WRITE_UINT8(toStream, response->VarClassification_Primary);
        STREAM_WRITE_UINT8(toStream, response->UnitCodes_Primary);
        STREAM_WRITE_FLOAT(toStream, response->VarValue_Primary);
        STREAM_WRITE_UINT8(toStream, response->VarStatus_Primary);

        // Secondary
        STREAM_WRITE_UINT8(toStream, response->VarCodes_Secondary);
        STREAM_WRITE_UINT8(toStream, response->VarClassification_Secondary);
        STREAM_WRITE_UINT8(toStream, response->UnitCodes_Secondary);
        STREAM_WRITE_FLOAT(toStream, response->VarValue_Secondary);
        STREAM_WRITE_UINT8(toStream, response->VarStatus_Secondary);

        // Tertiary
        STREAM_WRITE_UINT8(toStream, response->VarCodes_Tertiary);
        STREAM_WRITE_UINT8(toStream, response->VarClassification_Tertiary);
        STREAM_WRITE_UINT8(toStream, response->UnitCodes_Tertiary);
        STREAM_WRITE_FLOAT(toStream, response->VarValue_Tertiary);
        STREAM_WRITE_UINT8(toStream, response->VarStatus_Tertiary);

        // Quaternary
        STREAM_WRITE_UINT8(toStream, response->VarCodes_Quaternary);
        STREAM_WRITE_UINT8(toStream, response->VarClassification_Quaternary);
        STREAM_WRITE_UINT8(toStream, response->UnitCodes_Quaternary);
        STREAM_WRITE_FLOAT(toStream, response->VarValue_Quaternary);
        STREAM_WRITE_UINT8(toStream, response->VarStatus_Quaternary);

        // Percent
        STREAM_WRITE_UINT8(toStream, response->VarCodes_Percent);
        STREAM_WRITE_UINT8(toStream, response->VarClassification_Percent);
        STREAM_WRITE_UINT8(toStream, response->UnitCodes_Percent);
        STREAM_WRITE_FLOAT(toStream, response->VarValue_Percent);
        STREAM_WRITE_UINT8(toStream, response->VarStatus_Percent);

        // LoopC
        STREAM_WRITE_UINT8(toStream, response->VarCodes_LoopC);
        STREAM_WRITE_UINT8(toStream, response->VarClassification_LoopC);
        STREAM_WRITE_UINT8(toStream, response->UnitCodes_LoopC);
        STREAM_WRITE_FLOAT(toStream, response->VarValue_LoopC);
        STREAM_WRITE_UINT8(toStream, response->VarStatus_LoopC);

        STREAM_WRITE_UINT32(toStream, response->Time);


        if (response->NumberOfCommandsInStructure > 1) {

            // 2. SecondCommand cmdId=48
            STREAM_WRITE_UINT16(toStream, response->SecondCommandNumber);
            STREAM_WRITE_UINT8(toStream, response->SecondCommandByteCount);
            STREAM_WRITE_UINT8(toStream, response->SecondCommandResponseCode);
            STREAM_WRITE_UINT8(toStream, response->AdditionalStatus0);
            STREAM_WRITE_UINT8(toStream, response->AdditionalStatus1);
            STREAM_WRITE_UINT8(toStream, response->AdditionalStatus2);
            STREAM_WRITE_UINT8(toStream, response->AdditionalStatus3);
            STREAM_WRITE_UINT8(toStream, response->AdditionalStatus4);
            STREAM_WRITE_UINT8(toStream, response->AdditionalStatus5);
            STREAM_WRITE_UINT8(toStream, response->SecondExtendedDeviceStatus);
            STREAM_WRITE_UINT8(toStream, response->DevOperMode);
            STREAM_WRITE_UINT8(toStream, response->StandardStatus0);
            STREAM_WRITE_UINT8(toStream, response->StandardStatus1);
            STREAM_WRITE_UINT8(toStream, response->AnalogChannelSaturated);
            STREAM_WRITE_UINT8(toStream, response->StandardStatus2);
            STREAM_WRITE_UINT8(toStream, response->StandardStatus3);
        }

	}

	STREAM_WRITE_UINT8(toStream, response->Parity);

	return RCS_N00_Success;
}

/**
 *
 *///   STREAM_READ_UINT8(fromStream, &response->Header_ACK);
//  STREAM_READ_BYTES(fromStream, &response->Address, 5);
//  STREAM_READ_UINT8(fromStream, &response->Command);
//  STREAM_READ_UINT8(fromStream, &response->ByteCount);
//  STREAM_READ_UINT8(fromStream, &response->ResponseCode);
//  STREAM_READ_UINT8(fromStream, &response->DeviceStatus);

uint8_t Parse_C178_PublishedDynamicData_Resp(C178_PublishedDynamicData_Resp* response, ParserContext* context,
		BinaryStream* fromStream)
{
#ifdef _DEBUG
// 	if (2 > fromStream->remainingBytes)
// 		return RCS_E05_TooFewDataBytesReceived;
#endif


//	STREAM_READ_UINT8(fromStream, &response->Header_ACK);
//	STREAM_READ_BYTES(fromStream, &response->Address, 5);
//	STREAM_READ_UINT8(fromStream, &response->Command);
//	STREAM_READ_UINT8(fromStream, &response->ByteCount);
//	STREAM_READ_UINT8(fromStream, &response->ResponseCode);
//	STREAM_READ_UINT8(fromStream, &response->DeviceStatus);
	STREAM_READ_UINT8(fromStream, &response->NumberOfCommandsInStructure);


//	if (response->NumberOfCommandsInStructure > 0)
//	{
//        uint16_t cmd_ID = 0xFFFF;
//	    for(int i = 0; i < response->NumberOfCommandsInStructure; i++)
//	    {
//	        STREAM_READ_UINT16(fromStream, &cmd_ID);
//
//	        if (cmd_ID == CMDID_C009_ReadDeviceVariablesWithStatus)
//	        {
//	            response->c009_ID = cmd_ID;
//	            Parse_C009_ReadDeviceVariablesWithStatus_Resp(&response->c009, context, fromStream);
//	        }
//	        else if (cmd_ID == CMDID_C048_ReadAdditionalDeviceStatus)
//	        {
//	            response->c048_ID = cmd_ID;
//	            Parse_C048_ReadAdditionalDeviceStatus_Resp(&response->c048, context, fromStream);
//	        }
//	    }
//	}

	// 1. FirstCommand - cmdID=9
    STREAM_READ_UINT16(fromStream, &response->FirstCommandNumber);
    STREAM_READ_UINT8(fromStream, &response->FirstCommandByteCount);

//    if (fromStream->remainingBytes < response->FirstCommandByteCount)
//        return RCS_E05_TooFewDataBytesReceived;

    STREAM_READ_UINT8(fromStream, &response->FirstCommandResponseCode);
    STREAM_READ_UINT8(fromStream, &response->FirstExtendedDeviceStatus);

    // Primary
    STREAM_READ_UINT8(fromStream, &response->VarCodes_Primary);
    STREAM_READ_UINT8(fromStream, &response->VarClassification_Primary);
    STREAM_READ_UINT8(fromStream, &response->UnitCodes_Primary);
    STREAM_READ_FLOAT(fromStream, &response->VarValue_Primary);
    STREAM_READ_UINT8(fromStream, &response->VarStatus_Primary);

    // Secondary
    STREAM_READ_UINT8(fromStream, &response->VarCodes_Secondary);
    STREAM_READ_UINT8(fromStream, &response->VarClassification_Secondary);
    STREAM_READ_UINT8(fromStream, &response->UnitCodes_Secondary);
    STREAM_READ_FLOAT(fromStream, &response->VarValue_Secondary);
    STREAM_READ_UINT8(fromStream, &response->VarStatus_Secondary);

    // Tertiary
    STREAM_READ_UINT8(fromStream, &response->VarCodes_Tertiary);
    STREAM_READ_UINT8(fromStream, &response->VarClassification_Tertiary);
    STREAM_READ_UINT8(fromStream, &response->UnitCodes_Tertiary);
    STREAM_READ_FLOAT(fromStream, &response->VarValue_Tertiary);
    STREAM_READ_UINT8(fromStream, &response->VarStatus_Tertiary);

    // Quaternary
    STREAM_READ_UINT8(fromStream, &response->VarCodes_Quaternary);
    STREAM_READ_UINT8(fromStream, &response->VarClassification_Quaternary);
    STREAM_READ_UINT8(fromStream, &response->UnitCodes_Quaternary);
    STREAM_READ_FLOAT(fromStream, &response->VarValue_Quaternary);
    STREAM_READ_UINT8(fromStream, &response->VarStatus_Quaternary);

    // Percent
    STREAM_READ_UINT8(fromStream, &response->VarCodes_Percent);
    STREAM_READ_UINT8(fromStream, &response->VarClassification_Percent);
    STREAM_READ_UINT8(fromStream, &response->UnitCodes_Percent);
    STREAM_READ_FLOAT(fromStream, &response->VarValue_Percent);
    STREAM_READ_UINT8(fromStream, &response->VarStatus_Percent);

    // LoopC
    STREAM_READ_UINT8(fromStream, &response->VarCodes_LoopC);
    STREAM_READ_UINT8(fromStream, &response->VarClassification_LoopC);
    STREAM_READ_UINT8(fromStream, &response->UnitCodes_LoopC);
    STREAM_READ_FLOAT(fromStream, &response->VarValue_LoopC);
    STREAM_READ_UINT8(fromStream, &response->VarStatus_LoopC);

    STREAM_READ_UINT32(fromStream, &response->Time);


    // 2. SecondCommand cmdId=48
    STREAM_READ_UINT16(fromStream, &response->SecondCommandNumber);
    STREAM_READ_UINT8(fromStream, &response->SecondCommandByteCount);

//    if (fromStream->remainingBytes < response->SecondCommandByteCount)
//        return RCS_E05_TooFewDataBytesReceived;

    STREAM_READ_UINT8(fromStream, &response->SecondCommandResponseCode);

    STREAM_READ_UINT8(fromStream, &response->AdditionalStatus0);
    STREAM_READ_UINT8(fromStream, &response->AdditionalStatus1);
    STREAM_READ_UINT8(fromStream, &response->AdditionalStatus2);
    STREAM_READ_UINT8(fromStream, &response->AdditionalStatus3);
    STREAM_READ_UINT8(fromStream, &response->AdditionalStatus4);
    STREAM_READ_UINT8(fromStream, &response->AdditionalStatus5);
    STREAM_READ_UINT8(fromStream, &response->SecondExtendedDeviceStatus);
    STREAM_READ_UINT8(fromStream, &response->DevOperMode);
    STREAM_READ_UINT8(fromStream, &response->StandardStatus0);
    STREAM_READ_UINT8(fromStream, &response->StandardStatus1);
    STREAM_READ_UINT8(fromStream, &response->AnalogChannelSaturated);
    STREAM_READ_UINT8(fromStream, &response->StandardStatus2);
    STREAM_READ_UINT8(fromStream, &response->StandardStatus3);

    if (fromStream->remainingBytes >= sizeof(response->Parity))
	{
	    STREAM_READ_UINT8(fromStream, &response->Parity);
	}

	return RCS_N00_Success;
}

