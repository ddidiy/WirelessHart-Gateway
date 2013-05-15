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

#ifndef gw_util_h__
#define gw_util_h__

#include <ApplicationLayer/Model/UniversalCommands.h>
#include "GatewayConfig.h"


namespace hart7 {
namespace gateway {


/**
 * Generates a C000_response with the data for the Gateway.
 */
void FillGwIdentityResponse(C000_ReadUniqueIdentifier_Resp * p_pResp, GatewayConfig & config);


class CBinSearchCmdPredicate
{
public:
	template <class T>
	bool operator ()(const T & p_rEntry, uint16_t p_nCmdId)
	{
		return p_rEntry.cmdId < p_nCmdId;
	}
};

} // namespace gateway
} // namespace hart7

#endif // gw_util_h__
