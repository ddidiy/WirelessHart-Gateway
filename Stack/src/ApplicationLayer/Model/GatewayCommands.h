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

#ifndef GATEWAYCOMMANDS_H
#define GATEWAYCOMMANDS_H

#include "CommonTables.h"
#include "UniversalCommands.h"

// computed for max packet size = 128
//TODO replace defines
#define MaxNeighbours 7
#define MaxGraphIds 57
#define MaxBurstCommands 20
#define MaxChangeNotifications 61


enum
{
	CMDID_C832_ReadNetworkDeviceIdentity            = 832,
	CMDID_C833_ReadNetworkDeviceNeighbourHealth     = 833,
	CMDID_C834_ReadNetworkTopologyInformation       = 834,
	CMDID_C835_ReadBurstMessageList                 = 835,
	CMDID_C836_FlushCacheResponses                  = 836,
	CMDID_C837_WriteUpdateNotificationBitMask       = 837,
	CMDID_C838_ReadUpdateNotificationBitMask        = 838,
	CMDID_C839_ChangeNotification                   = 839,
	CMDID_C840_ReadDeviceStatistics                 = 840,
	CMDID_C841_ReadNetworkDeviceIdentityByNickname  = 841,
	CMDID_C842_WriteDeviceSchedulingFlags           = 842,
	CMDID_C843_ReadDeviceSchedulingFlags            = 843,
	CMDID_C844_ReadNetworkConstraints               = 844,
	CMDID_C845_WriteNetworkConstraints              = 845,

	//Emerson specific command
	CMDID_C178_PublishedDynamicData					= 178
};

enum
{
  C832_ReqSize =  5,
  C833_ReqSize =  7,
  C834_ReqSize =  7,
  C835_ReqSize =  5,
  C836_ReqSize =  5,
  C837_ReqSize =  7,
  C838_ReqSize =  5,
  C839_ReqSize =  5,
  C840_ReqSize =  5,
  C841_ReqSize =  2,
  C842_ReqSize =  6,
  C843_ReqSize =  5,
  C844_ReqSize =  0,
  C845_ReqSize =  2,
  C178_ReqSize =  9
};

enum
{
  C832_RespSize = 39,
  C833_RespSize = 22,
  C834_RespSize = 15, // or 17, 19, ... (variable size)
  C835_RespSize = 12, // or 18, 24, ... (variable size)
  C836_RespSize =  5,
  C837_RespSize =  7,
  C838_RespSize =  7,
  C839_RespSize =  8, // or 10, 12, ... 26 (variable size)
  C840_RespSize = 57,
  C841_RespSize = 39,
  C842_RespSize =  6,
  C843_RespSize =  6,
  C844_RespSize =  2,
  C845_RespSize =  2,
  C178_RespSize = 1 // Number of Commands in structure

};


#define C832_ReadNetworkDeviceIdentity_ReqSize            C832_ReqSize
#define C833_ReadNetworkDeviceNeighbourHealth_ReqSize     C833_ReqSize
#define C834_ReadNetworkTopologyInformation_ReqSize       C834_ReqSize
#define C835_ReadBurstMessageList_ReqSize                 C835_ReqSize
#define C836_FlushCacheResponses_ReqSize                  C836_ReqSize
#define C837_WriteUpdateNotificationBitMask_ReqSize       C837_ReqSize
#define C838_ReadUpdateNotificationBitMask_ReqSize        C838_ReqSize
#define C839_ChangeNotification_ReqSize                   C839_ReqSize
#define C840_ReadDeviceStatistics_ReqSize                 C840_ReqSize
#define C841_ReadNetworkDeviceIdentityByNickname_ReqSize  C841_ReqSize
#define C842_WriteDeviceSchedulingFlags_ReqSize           C842_ReqSize
#define C843_ReadDeviceSchedulingFlags_ReqSize            C843_ReqSize
#define C844_ReadNetworkConstraints_ReqSize               C844_ReqSize
#define C845_WriteNetworkConstraints_ReqSize              C845_ReqSize
#define C178_PublishedDynamicData_ReqSize             	  C178_ReqSize


#define C832_ReadNetworkDeviceIdentity_RespSize           C832_RespSize
#define C833_ReadNetworkDeviceNeighbourHealth_RespSize    C833_RespSize
#define C834_ReadNetworkTopologyInformation_RespSize      C834_RespSize
#define C835_ReadBurstMessageList_RespSize                C835_RespSize
#define C836_FlushCacheResponses_RespSize                 C836_RespSize
#define C837_WriteUpdateNotificationBitMask_RespSize      C837_RespSize
#define C838_ReadUpdateNotificationBitMask_RespSize       C838_RespSize
#define C839_ChangeNotification_RespSize                  C839_RespSize
#define C840_ReadDeviceStatistics_RespSize                C840_RespSize
#define C841_ReadNetworkDeviceIdentityByNickname_RespSize C841_RespSize
#define C842_WriteDeviceSchedulingFlags_RespSize          C842_RespSize
#define C843_ReadDeviceSchedulingFlags_RespSize           C843_RespSize
#define C844_ReadNetworkConstraints_RespSize              C844_RespSize
#define C845_WriteNetworkConstraints_RespSize             C845_RespSize
#define C178_PublishedDynamicData_RespSize             	  C178_RespSize


/******************** CMD 832 *************************/

typedef struct
{
	_device_address_t DeviceUniqueID;
}C832_ReadNetworkDeviceIdentity_Req;

typedef struct
{
	_device_address_t DeviceUniqueID;
	uint16_t Nickname;
	char LongTag[32];
}C832_ReadNetworkDeviceIdentity_Resp;

enum
{
	C832_ResponseCode_Success = 0,
	C832_ResponseCode_InvalidSelection = 2,
	C832_ResponseCode_TooFewBytes = 5
};


/******************** CMD 833 *************************/

typedef struct
{
	_device_address_t UniqueID;
	uint8_t NeighbourIndex;
	uint8_t NeighbourEntriesToRead;
}C833_ReadNetworkDeviceNeighbourHealth_Req;


	typedef struct
	{
		uint16_t NeighbourNickname;
		int8_t NeighbourRSL;
		uint32_t TransmittedPacketCount;
		uint32_t TransmittedPacketWithNoACKCount;
		uint32_t ReceivedPacketCount;
	} NeighbourDescription;


typedef struct
{
	_device_address_t UniqueID;
	uint8_t NeighbourIndex;
	uint8_t NeighbourCount;
	NeighbourDescription Neighbours[MaxNeighbours];
}C833_ReadNetworkDeviceNeighbourHealth_Resp;

enum
{
	C833_ResponseCode_Success = 0,
	C833_ResponseCode_InvalidSelection = 2,
	C833_ResponseCode_TooFewBytes = 5,
	C833_W08 = RCM_W08_SetToNearestPossibleValue,
	C833_E65 = RCM_E65_InvalidNeighborTableIndex,
};


/******************** CMD 834 *************************/

typedef struct
{
	_device_address_t DeviceLongAddress;
	uint16_t GraphIndexNo;
}C834_ReadNetworkTopologyInformation_Req;

typedef struct
{
	_device_address_t DeviceLongAddress;
	uint16_t GraphIndexNo;
	uint16_t TotalGraphsNo;
	uint16_t IndexGraphId;
	uint16_t NeighboursNo;
	uint16_t Neighbour[MaxGraphIds];
}C834_ReadNetworkTopologyInformation_Resp;

enum
{
	C834_ResponseCode_Success = 0,
	C834_ResponseCode_InvalidSelection = 2,
	C834_ResponseCode_TooFewBytes = 5,
	C834_E65 = RCM_E65_EntryNotFound,
};


/******************** CMD 835 *************************/

typedef struct
{
	_device_address_t UniqueID;
}C835_ReadBurstMessageList_Req;

	typedef struct
	{
		uint16_t CommandNoBeingBurst;
		uint32_t ReceivedBurstPacketsNo;
	} BurstCommandDesc;

typedef struct
{
	_device_address_t UniqueID;
	uint8_t DifferentBurstCommandsNo;
	BurstCommandDesc BurstCommands[MaxBurstCommands];

}C835_ReadBurstMessageList_Resp;

enum
{
	C835_ResponseCode_Success = 0 //TODO
};


/******************** CMD 836 *************************/

typedef struct
{
	_device_address_t UniqueID;
}C836_FlushCacheResponses_Req;

typedef struct
{
	_device_address_t UniqueID;
}C836_FlushCacheResponses_Resp;

enum
{
	C836_ResponseCode_Success = 0 // TODO
};


/******************** CMD 837 *************************/

typedef struct
{
	_device_address_t UniqueID;
	uint16_t ChangeNotificationFlags;
}C837_WriteUpdateNotificationBitMask_Req;

typedef struct
{
	_device_address_t UniqueID;
	uint16_t ChangeNotificationFlags;
}C837_WriteUpdateNotificationBitMask_Resp;

enum
{
	C837_NOO = RCS_N00_Success,
	C837_E05 = RCS_E05_TooFewDataBytesReceived,
	C837_E16 = RCS_E16_AccessRestricted,
	C837_E65 = RCM_E65_UnknownUID,
	C837_E66 = RCM_E66_UnknownNotificationFlag
};


/******************** CMD 838 *************************/

typedef struct
{
	_device_address_t UniqueID;
}C838_ReadUpdateNotificationBitMask_Req;

typedef struct
{
	_device_address_t UniqueID;
	uint16_t ChangeNotificationFlags;
}C838_ReadUpdateNotificationBitMask_Resp;

enum
{
	C838_N00 = RCS_N00_Success,
	C838_E05 = RCS_E05_TooFewDataBytesReceived,
	C838_E65 = RCM_E65_UnknownUID,
};


/******************** CMD 839 *************************/

typedef struct
{
	_device_address_t DeviceAddress;
}C839_ChangeNotification_Req;

typedef struct
{
	_device_address_t DeviceAddress;
	uint8_t ChangeNotificationNo;
	uint16_t ChangeNotifications[MaxChangeNotifications];
}C839_ChangeNotification_Resp;

enum
{
	C839_N00 = RCS_N00_Success,
	C839_E05 = RCS_E05_TooFewDataBytesReceived,
	C839_E65 = RCM_E65_UnknownUID,
};


/******************** CMD 840 *************************/

typedef struct
{
	_device_address_t UniqueID;
}C840_ReadDeviceStatistics_Req;

typedef struct
{
 _device_address_t UniqueID;
 uint16_t ActiveGraphsNo;
 uint16_t ActiveFramesNo;
 uint16_t ActiveLinksNo;
 uint8_t NeighboursNo;
 _time_t AverageLatency;
 uint16_t JoinCount;
 _date_t LastJoinTime;
 uint32_t LastJoinTimeToday;	// last join time in 1/32 ms from 12:00AM today

 uint32_t DeviceGeneratedPackagesNo;
 uint32_t DeviceTerminatedPackagesNo;
 uint32_t DataLinkLayerMicFailuresNo;
 uint32_t NetworkLayerMicFailuresNo;
 uint32_t CRCErrorsNo;
 uint32_t NonceCounterValuesNotReceivedByDeviceNo; // Number of Nonce Counter Values not received by the device. This value is incremented for each zero in Nonce Counter History that underflows.
 uint32_t NonceCounterValuesNotReceivedFromDeviceNo; // Number of Nonce Counter Values not received from the device
 _time_t StandardDeviationOfLatency;
}C840_ReadDeviceStatistics_Resp;

enum
{
	C840_ResponseCode_Success = 0,
	C840_E02 = RCS_E02_InvalidSelection,
	C840_E05 = RCS_E05_TooFewDataBytesReceived,
};


/******************** CMD 841 *************************/

typedef struct
{
	uint16_t DeviceNickname;
}C841_ReadNetworkDeviceIdentityByNickname_Req;


typedef struct
{
	uint16_t Nickname;
	_device_address_t DeviceUniqueID;
	char LongTag[32];
}C841_ReadNetworkDeviceIdentityByNickname_Resp;


enum
{
	C841_ResponseCode_Success = 0 // TODO
};


/******************** CMD 842 *************************/

typedef struct
{
	_device_address_t UniqueID;
	uint8_t SchedulingFlags;
}C842_WriteDeviceSchedulingFlags_Req;

typedef struct
{
	_device_address_t UniqueID;
	uint8_t SchedulingFlags;
}C842_WriteDeviceSchedulingFlags_Resp;

enum
{
	C842_ResponseCode_Success = 0 // TODO
};


/******************** CMD 843 *************************/

typedef struct
{
	_device_address_t UniqueID;
}C843_ReadDeviceSchedulingFlags_Req;

typedef struct
{
	_device_address_t UniqueID;
	uint8_t SchedulingFlags;
}C843_ReadDeviceSchedulingFlags_Resp;

enum
{
	C843_ResponseCode_Success = 0 // TODO
};


/******************** CMD 844 *************************/

typedef EmptyCommand_Req
 C844_ReadNetworkConstraints_Req;

typedef struct
{
	uint8_t NetworkFlags;
	uint8_t ReqRespPairMessagesPerTenSeconds;
}C844_ReadNetworkConstraints_Resp;

enum
{
	C844_ResponseCode_Success = 0 // TODO
};


/******************** CMD 845 *************************/

typedef struct
{
	uint8_t NetworkFlags;
	uint8_t ReqRespPairMessagesPerTenSeconds;
}C845_WriteNetworkConstraints_Req;

typedef struct
{
	uint8_t NetworkFlags;
	uint8_t ReqRespPairMessagesPerTenSeconds;
}C845_WriteNetworkConstraints_Resp;

enum
{
	C845_ResponseCode_Success = 0 // TODO
};

/******************** CMD 178 *************************/
typedef struct
{
	uint8_t Header_STX; 				//(0x82) header
	_device_address_t Address;
	uint8_t Command;			//178(0xB2)
	uint8_t ByteCount;			//0
	uint8_t Parity;				//tU8
}C178_PublishedDynamicData_Req;

typedef struct
{
	uint8_t Header_ACK;				//(0x86) header
	_device_address_t Address;
	uint8_t Command;			//178(0xB2)
	uint8_t ByteCount;			//77(0x4D)
	uint8_t ResponseCode;
	uint8_t DeviceStatus;
	uint8_t NumberOfCommandsInStructure;	//2

//	//
//	uint16_t c009_ID;		//9
//	C009_ReadDeviceVariablesWithStatus_Resp c009;
//
//	//
//	uint16_t c048_ID;		//48(0x0030)
//	C048_ReadAdditionalDeviceStatus_Resp c048;

    //
    uint16_t FirstCommandNumber;        //9
    uint8_t FirstCommandByteCount;      //54(0x36)
    uint8_t FirstCommandResponseCode;   //command 9 codes
    uint8_t FirstExtendedDeviceStatus;

    uint8_t VarCodes_Primary;               //PV: Device Variable Code
    uint8_t VarClassification_Primary;      //PV: Device Variable Classification
    uint8_t UnitCodes_Primary;              //PV: Device Variable Units Code
    float VarValue_Primary;                 //PV: Device Variable Value
    uint8_t VarStatus_Primary;              //PV: Device Variable Status

    uint8_t VarCodes_Secondary;             //SV: Device Variable Code
    uint8_t VarClassification_Secondary;    //SV: Device Variable Classification
    uint8_t UnitCodes_Secondary;            //SV: Device Variable Units Code
    float VarValue_Secondary;               //SV: Device Variable Value
    uint8_t VarStatus_Secondary;            //SV: Device Variable Status

    uint8_t VarCodes_Tertiary;              //TV: Device Variable Code
    uint8_t VarClassification_Tertiary;     //TV: Device Variable Classification
    uint8_t UnitCodes_Tertiary;             //TV: Device Variable Units Code
    float VarValue_Tertiary;                //TV: Device Variable Value
    uint8_t VarStatus_Tertiary;             //TV: Device Variable Status

    uint8_t VarCodes_Quaternary;            //QV: Device Variable Code
    uint8_t VarClassification_Quaternary;   //QV: Device Variable Classification
    uint8_t UnitCodes_Quaternary;           //QV: Device Variable Units Code
    float VarValue_Quaternary;              //QV: Device Variable Value
    uint8_t VarStatus_Quaternary;           //QV: Device Variable Status

    uint8_t VarCodes_Percent;               //244; PV % of Range: Device Variable Code
    uint8_t VarClassification_Percent;      //PV % of Range: Device Variable Classification
    uint8_t UnitCodes_Percent;              //PV % of Range: Device Variable Units Code
    float VarValue_Percent;                 //PV % of Range: Device Variable Value
    uint8_t VarStatus_Percent;              //PV % of Range: Device Variable Status

    uint8_t VarCodes_LoopC;                 //245; PV Loop Current: Device Variable Code
    uint8_t VarClassification_LoopC;        //PV Loop Current: Device Variable Classification
    uint8_t UnitCodes_LoopC;                //PV Loop Current: Device Variable Units Code
    float VarValue_LoopC;                   //PV Loop Current: Device Variable Value
    uint8_t VarStatus_LoopC;                //PV Loop Current: Device Variable Status

    uint32_t Time;                          //PV: Time Stamp


    //
    uint16_t SecondCommandNumber;       //48(0x0030)
    uint8_t SecondCommandByteCount;     //13(0x0D)
    uint8_t SecondCommandResponseCode;  //command 48 codes
    uint8_t AdditionalStatus0;          //Transmitter Status 0
    uint8_t AdditionalStatus1;          //Transmitter Status 1
    uint8_t AdditionalStatus2;          //Transmitter Status 2
    uint8_t AdditionalStatus3;          //Transmitter Status 3
    uint8_t AdditionalStatus4;          //Transmitter Status 4
    uint8_t AdditionalStatus5;          //Transmitter Status 5
    uint8_t SecondExtendedDeviceStatus; //Extended Device Status
    uint8_t DevOperMode;                //Device Operating Mode
    uint8_t StandardStatus0;            //Standardize Status 0
    uint8_t StandardStatus1;            //Standardize Status 1
    uint8_t AnalogChannelSaturated;     //0
    uint8_t StandardStatus2;            //Standardize Status 2
    uint8_t StandardStatus3;            //Standardize Status 3

	//
	uint8_t Parity;

} C178_PublishedDynamicData_Resp;

enum
{
    C178_NOO = RCS_N00_Success,             //No Command-Specific Errors
    C178_E16 = RCS_E16_AccessRestricted,    //Access Restricted (Forward compatibility only)
    C178_E32 = RCS_E32_Busy
};

#endif /*GATEWAYCOMMANDS_H*/
