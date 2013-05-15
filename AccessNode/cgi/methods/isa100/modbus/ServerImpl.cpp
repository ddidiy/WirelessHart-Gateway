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



#include <ctype.h>
#include <strings.h>

#include "Shared/Utils.h"
#include "methods/core/ConfigImpl.h"
#include "ServerImpl.h"


namespace CIsa100 {
namespace Modbus  {


#define CONF_MODBUS NIVIS_PROFILE"modbus_gw.ini"

////////////////////////////////////////////////////////////////////////////////
/// @author Marcel Ionescu
/// @brief Extract an integer
/// @param p_szSrc comma-separated row starting with an integer
/// @param p_szDst Filled with extracted integer. It is allocated by user and have at least 10 bytes reserved
/// @retval false if the string cannot be extracted - may also be used to validate the input
/// @note Does null-terminate the string
////////////////////////////////////////////////////////////////////////////////
static bool sExtractInteger( const char * p_szSrc, size_t p_dwSize, char * p_pDst)
{	size_t nChr = 0;
	for( int i = 0; p_szSrc[i] && (p_szSrc[i] != ',') && (nChr < p_dwSize); ++i )	/// stop parsing at comma
	{	if( isdigit( p_szSrc[i] ))
		{	if( p_pDst )
				p_pDst[ nChr ] = p_szSrc[ i ];
			++nChr;
		}
	}
	p_pDst[ nChr ] = 0;
	return true;
}

////////////////////////////////////////////////////////////////////////////////
/// @author Marcel Ionescu
/// @brief Update a row into MODBUS configuration file
/// @param p_szRowValue The row value.
///	HOST format: <UnitId>, <EUI64>
///	Register format: <start_address>, <word_count>,<TSAPID>,<ObjId>,<AttrId>,<Idx1>,<Idx2>,<MethId>
/// @param p_szSection - HOSTS / INPUT_REGISTERS / HOLDING_REGISTERS
/// @retval false cannot set the row
/// @retval true row updated ok
/// @remarks for HOST primary key is <UnitId>
/// @remarks for REGISTER primary key is <start_address>
/// @remarks If row was already there, updates it's info, otherwise add the device
/// @remarks The configuration file allows definition of multiple register maps to
/// 	accomodate multiple vendors whose sensors may export different variables in the
/// 	same <TSAPID>,<ObjId>,<AttrId>,<Idx1>,<Idx2>,<MethId>
////////////////////////////////////////////////////////////////////////////////
bool ServerImpl::setRow( const char * p_szRowValue, const char * p_szSection )
{	CConfigImpl oConfig;
	if( ( !strcasecmp( p_szSection, "INPUT_REGISTERS") ) || ( !strcasecmp( p_szSection, "HOLDING_REGISTERS") ) )
	{
		return oConfig.setConfigRow( CONF_MODBUS, p_szSection, "REGISTER", p_szRowValue, &sExtractInteger, 10 );
	}

	LOG("ERROR modbus.setRow: invalid section [%s]", p_szSection);
	return false;
}

////////////////////////////////////////////////////////////////////////////////
/// @brief   Delete a line from the MODBUS provisioning file.
/// @param p_szRowValue The row value.
///	HOST format: <UnitId>, <EUI64>. Only the primary key <UnitId> is important
///	Register format: <start_address>, <word_count>,<TSAPID>,<ObjId>,<AttrId>,<Idx1>,<Idx2>,<MethId>
///		For Register, only the primary key <start_address> is important
/// @param p_szSection - HOSTS / INPUT_REGISTERS / HOLDING_REGISTERS
/// @remarks The configuration file allows definition of multiple register maps to
/// 	accomodate multiple vendors whose sensors may export different variables in the
/// 	same <TSAPID>,<ObjId>,<AttrId>,<Idx1>,<Idx2>,<MethId>
////////////////////////////////////////////////////////////////////////////////
bool ServerImpl::delRow( const char * p_szRowValue, const char * p_szSection )
{	CConfigImpl oConfig;
	if( ( !strcasecmp( p_szSection, "INPUT_REGISTERS") ) || ( !strcasecmp( p_szSection, "HOLDING_REGISTERS") ) )
	{
		return oConfig.delConfigRow( CONF_MODBUS, p_szSection, "REGISTER", p_szRowValue, &sExtractInteger, 10);
	}

	return true;
}


}	// namespace Modbus
}	// namespace CIsa100
