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
 * GwCmdsDescriptor.h
 *
 *  Created on: October 22, 2009
 *      Author: gabriel.ghervase
 */

#ifndef GwCmdsDescriptor_h__
#define GwCmdsDescriptor_h__

#include <string>

#include <ApplicationLayer/Model/UniversalCommands.h>
#include <ApplicationLayer/Model/CommonPracticeCommands.h>
#include <ApplicationLayer/Model/WirelessNetworkManagerCommands.h>
#include <ApplicationLayer/Model/GatewayCommands.h>
#include <ApplicationLayer/Model/WirelessApplicationCommands.h>

#include <ApplicationLayer/Model/DataLinkLayerCommands.h>
#include <ApplicationLayer/Model/PhysicalLayerCommands.h>
#include <ApplicationLayer/Model/NetworkLayerCommands.h>


#include <ApplicationLayer/Binarization/UniversalCommands.h>
#include <ApplicationLayer/Binarization/CommonPracticeCommands.h>
#include <ApplicationLayer/Binarization/WirelessNetworkManagerCommands.h>
#include <ApplicationLayer/Binarization/GatewayCommands.h>
#include <ApplicationLayer/Binarization/WirelessApplicationCommands.h>

#include <ApplicationLayer/Binarization/DataLinkLayerCommands.h>
#include <ApplicationLayer/Binarization/PhysicalLayerCommands.h>
#include <ApplicationLayer/Binarization/NetworkLayerCommands.h>

#include <ApplicationLayer/ApplicationCommand.h>

#define CREATE_CMD_DESCRIPTOR_ENTRY(CommandName)\
  {CMDID_##CommandName, (char*)#CommandName }

/**
 * Holds string descriptors for each command.
 */


namespace hart7 {
namespace gateway {



struct GwDescriptorEntry
{
	uint16_t cmdId;
	char* m_pszCmdName;
	
};

extern const GwDescriptorEntry g_pCommandDescriptorEntries[];
extern const int g_nCommandDescriptorEntriesSize;

const GwDescriptorEntry& GetCmdDescriptor( uint16_t p_nCmdId );

} // namespace gateway
} // namespace hart7


#endif /* GwCmdsDescriptor_h__ */
