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

#include "GatewayRequestsHandler.h"
#include "Misc/Convert/Convert.h"
#include "Model/Device.h"
#include "Model/Tdma/TdmaTypes.h"
#include "../operations/WHOperationQueue.h"
#include "../../util/ManagerUtils.h"
#include "IDefaultCommandHandler.h"
#include <ApplicationLayer/ApplicationCommand.h>
#include <WHartStack/WHartNetworkData.h>
#include <hart7/util/NMLog.h>
#include <hart7/util/CommandsToString.h>
#include <time.h>

namespace hart7 {

namespace nmanager {

GatewayRequestsHandler::GatewayRequestsHandler(hart7::nmanager::CommonData& commonData_) :
    commonData(commonData_), gatewayWrappedRequestsHandler(commonData_)
{
}

GatewayRequestsHandler::~GatewayRequestsHandler()
{
}

}

}
