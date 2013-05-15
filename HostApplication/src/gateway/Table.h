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


#ifndef TABLE_H
#define TABLE_H


#include <ApplicationLayer/Binarization/UniversalCommands.h>
#include <ApplicationLayer/Binarization/CommonPracticeCommands.h>
#include <ApplicationLayer/Binarization/WirelessNetworkManagerCommands.h>
#include <ApplicationLayer/Binarization/GatewayCommands.h>
#include <ApplicationLayer/Binarization/WirelessApplicationCommands.h>

#include <ApplicationLayer/Binarization/DataLinkLayerCommands.h>
#include <ApplicationLayer/Binarization/PhysicalLayerCommands.h>
#include <ApplicationLayer/Binarization/NetworkLayerCommands.h>

#include <ApplicationLayer/ApplicationCommand.h>

#define CREATE_COMPOSER_REQ_PARSER_RESP_ENTRY(CommandName)\
  {CMDID_##CommandName, (ParseFunction)&Parse_##CommandName##_Resp, NULL, (ComposeFunction)&Compose_##CommandName##_Req,  GetLengthParsed_##CommandName##_Resp, GetLengthBin_##CommandName##_Resp}

#define CREATE_PARSER_REQ_COMPOSER_RESP_ENTRY(CommandName)\
  {CMDID_##CommandName, (ParseFunction)&Parse_##CommandName##_Req, NULL, (ComposeFunction)&Compose_##CommandName##_Resp, GetLengthParsed_##CommandName##_Req, GetLengthBin_##CommandName##_Req}


#define CONCAT(CommandName,Suffix)\
	CommandName##Suffix

#define CREATE_CMD_RESP_BIN_LEN(CommandName)\
      inline int GetLengthBin_##CommandName##_Resp() { return CONCAT(CommandName,_RespSize); }

#define CREATE_CMD_RESP_PARSED_LEN(CommandName)\
      inline int GetLengthParsed_##CommandName##_Resp() { return sizeof(CONCAT(CommandName,_Resp)); }

#define CREATE_CMD_REQ_BIN_LEN(CommandName)\
      inline int GetLengthBin_##CommandName##_Req() { return CONCAT(CommandName,_ReqSize); }

#define CREATE_CMD_REQ_PARSED_LEN(CommandName)\
      inline int GetLengthParsed_##CommandName##_Req() { return sizeof(CONCAT(CommandName,_Req)); }


namespace hart7 {
namespace gateway {

extern const ParseExecuteComposerEntry g_parseReqComposeResp[];
extern const ParseExecuteComposerEntry g_composeReqParseResp[]; 

extern const int g_nComposeReqParseRespSize;
extern const int g_nParseReqComposeRespSize;
} // namespace gateway
} // namespace hart7



#endif /* TABLE_H */
