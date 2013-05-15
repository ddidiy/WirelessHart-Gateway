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
// C++ Interface: CmdProcessorCommonPractice
//
// Created on Nov 3, 2009
//
// Author: Catrina Mihailescu <catrina@r2d2>, (C) 2009
//
//
// Processes Common Practice Commands
//

#ifndef CMDPROCESSORCOMMONPRACTICE_H_
#define CMDPROCESSORCOMMONPRACTICE_H_

#include <WHartGateway/CmdProcessor.h>
#include "LogicalDevice.h"
#include "HostAppSession.h"
#include "GwRequest.h"

namespace hart7
{
namespace gateway
{

class Gateway;

class CmdProcessorCommonPractice : public CmdProcessor
{

public:
	CmdProcessorCommonPractice(Gateway& _gateway);
public:
	bool ExecLocalCmdCommonPractice(CHartCmdWrapper::Ptr & p_pCmd, CHartCmdWrapper::Ptr & p_pRsp, CHostAppSession::Ptr & p_pHostSession);

};

}

}

#endif //CMDPROCESSORCOMMONPRACTICE_H
