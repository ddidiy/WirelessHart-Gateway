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

/**
 * nicu.dascalu
 * THIS IS JUST an IMPLEMENTATION code of parsing/composing  genric HART command using functions tables.
 */

#ifndef APPLICATION_COMMAND_H
#define APPLICATION_COMMAND_H

#include <WHartStack/WHartTypes.h>
#include "ApplicationLayer/Model/CommonTables.h"
#include "Model/CommonResponseCode.h"
#include "Model/CommandOperations.h"


#ifdef __cplusplus
extern "C" {
#endif

	typedef int (*GetParsedLenFctType)();
	typedef int (*GetBinLenFctType)();

	typedef enum
	{
		pcecSuccess = 0,
		pcecNotEnoughBytes = 1,
		pcecNotImplemented = 2
	} ParseComposeErrorCodes;
/**
 * Holds a pair of parser & composer functions for cmdId
 * 	request parser + response composer
 *  request composer + response parser
 */
typedef struct
{
	uint16_t cmdId;
	ParseFunction fnParser;
	ExecuteFunction fnExecute;
	ComposeFunction fnComposer;
	GetParsedLenFctType fnGetParsedLen;
	GetBinLenFctType fnGetBinLen;

} ParseExecuteComposerEntry;

typedef struct
{
	/*[out] will be filled with parsed command id*/
	uint16_t parsedCmdId;
	/*[in,out] preallocated memory that will contain parsed command*/
	void* parsedCommand;
	/*[out] will be filled with response code if is response*/
	uint8_t parsedResponseCode;

	/*[in] the size of preallocated memory that will contain parsed command.*/
	_command_size_t parsedCommandSize;
	/*[out] the size of used memory that will contain parsed command. */
	_command_size_t parsedCommandEffectiveSize;

	/*[in] holds input binary command data*/
	const void* inputBuffer;
	/*[in] holds the size of input binary command data*/
	_command_size_t inputBufferSize;
	/*[out] tells how many bytes have been consumed from the @buffer*/
	_command_size_t consumedBufferBytes;

	/*[in] holds the table that contains all supported command functions (asc ordered by cmdid)*/
	const ParseExecuteComposerEntry* table;
	/*[in] holds the size of the table*/
	uint16_t tableSize;
	/*[out] will be filled with the index of command in @table*/
	int16_t tableIndex;

} ParseWirelessCommandData;

/**
 * Parses the Application request Command from the buffer, and put the result into commandData->parsedCommand.
 *
 * @returns ParseComposeErrorCodes
 */
ParseComposeErrorCodes ParseWirelessRequestCommmand(ParseWirelessCommandData* commandData);

/**
 * Parses the Application request Command from the buffer, and put the result into commandData->parsedCommand.
 *
 * @returns ParseComposeErrorCodes
 */
ParseComposeErrorCodes ParseWirelessResponseCommmand(ParseWirelessCommandData* commandData);

typedef struct
{
	/*[in] contains the current command id*/
	uint16_t cmdId;
	/*[in] contains the data of composed command*/
	void* composingCommand;
	/*[in] for response command only holds response code that should be written*/
	uint8_t responseCode;

	/*[out] will be filled with composed binary data of @composing command*/
	const void* outputBuffer;
	/*[in] holds the size of @outputBuffer*/
	_command_size_t outputBufferSize;
	/*[out] will be filled with no of bytes written in @outputBuffer*/
	_command_size_t writenBufferBytes;

	/*[in] holds the table that holds all supported command functions (asc ordered by cmdid)*/
	const ParseExecuteComposerEntry* table;
	/*[in] holds the size of the table*/
	uint16_t tableSize;
	/*[out] will be filled with the index of command in @table*/
	uint16_t tableIndex;

}ComposeWirelessCommandData;

/**
 *
 */
int16_t FindEntry(const ParseExecuteComposerEntry* sortedList, uint16_t listSize, const uint16_t cmdId);

ParseComposeErrorCodes ComposeWirelessRequestCommmand(ComposeWirelessCommandData* commandData);

ParseComposeErrorCodes ComposeWirelessResponseCommmand(ComposeWirelessCommandData* commandData);



#ifdef __cplusplus
}
#endif


#endif /* APPLICATION_COMMAND_H */
