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

#ifndef COMMAND_OPERATIONS_H
#define COMMAND_OPERATIONS_H

#include <WHartStack/util/Binarization.h>
#include "CommonTables.h"

typedef enum
{
	CD_REQUEST = 1,
	CD_RESPONSE = 2
} CommandDirection;

enum
{
	WIRELESS_CMD = 0,
	WIRED_CMD = 1
};

//typedef struct
//{
//	_command_size_t allocatedCommandSize;
//	int				maxNeededParsedSize;
//} ParserContext;

typedef uint8_t (*ParseFunction)(void* /*command*/, ParserContext* /*context*/, BinaryStream* /*fromStream*/);


//typedef struct
//{
//  uint16_t cmdId;
//} ComposerContext;

typedef uint8_t (*ComposeFunction)(void* /*command*/, ComposerContext* /*context*/, BinaryStream* /*toStream*/);


typedef struct
{
  uint16_t cmdId;
  uint8_t priority;
  uint8_t transportType;
  //SHORT_ADDR srcAddress;
  uint8_t control;
  uint8_t remainingBytes;
} ExecuteContext;

typedef uint8_t (*ExecuteFunction)(void* /*request*/, void* /*response*/, ExecuteContext* /*context*/);


#endif /*COMMAND_OPERATIONS_H*/
