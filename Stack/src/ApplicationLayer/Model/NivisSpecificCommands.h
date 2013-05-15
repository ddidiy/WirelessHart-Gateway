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

#ifndef NIVISSPECIFICCOMMANDS_H
#define NIVISSPECIFICCOMMANDS_H

#include "CommonTables.h"

enum
{
    CMDID_C64765_NivisMetaCommand = 64765
};

enum
{
    C64765_ReqSize = 10     // ... (variable size)
};

enum
{
    C64765_RespSize = 10    // ... (variable size)

};

#define C64765_NivisMetaCommand_ReqSize            C832_ReqSize

#define C64765_NivisMetaCommand_RespSize           C832_RespSize


/******************** CMD 64765 *************************/

typedef struct
{
    uint16_t			Nickname;  // F980 for NM, F981 for GW
    _device_address_t	DeviceUniqueId;
    uint8_t			    CommandSize;    // !!! this will no be serializable
    uint8_t				Command[256];
}C64765_NivisMetaCommand_Req;


typedef struct
{
    uint16_t			Nickname;  // F980 for NM, F981 for GW
	_device_address_t	DeviceUniqueId;
	uint8_t				CommandSize;     // !!! this will no be serializable
	uint8_t				Command[256];
}C64765_NivisMetaCommand_Resp;


enum
{
    C64765_ResponseCode_Success = 0,
    C64765_ResponseCode_Error_Parsing_Inside_Command = 1,
    C64765_ResponseCode_Command_Not_Treated = 4
};

#endif /* NIVISSPECIFICCOMMANDS_H */
