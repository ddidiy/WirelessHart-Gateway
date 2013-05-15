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
// C++ Interface: CmdProcessorWirelessHART
//
// Created on : Nov 2, 2009
//
// Author: Catrina Mihailescu <catrina@r2d2>, (C) 2009
//
//
// Processes WirelessHART commands.
//

#ifndef CMDPROCESSORWIRELESSHART_H_
#define CMDPROCESSORWIRELESSHART_H_

#include <WHartGateway/CmdProcessor.h>
#include "LogicalDevice.h"
#include "HostAppSession.h"
#include "GwRequest.h"



namespace hart7
{
namespace gateway
{

class Gateway;

class CmdProcessorWirelessHART : public  CmdProcessor
{

public:
	CmdProcessorWirelessHART(Gateway& p_pGateway);

public:
	bool ExecLocalCmdWirelessHART(CHartCmdWrapper::Ptr & p_pCmd, CHartCmdWrapper::Ptr & p_pRsp, CHostAppSession::Ptr & p_pHostSession);
	bool ExecCmdNmOrGwDualCmds(CHartCmdWrapper::Ptr & p_pCmd, CHartCmdWrapper::Ptr & p_pRsp, CHostAppSession::Ptr & p_pHostSession);

};


}

}


#endif //CMDPROCESSORWIRELESSHART_H
