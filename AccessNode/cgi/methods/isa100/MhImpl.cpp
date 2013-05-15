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

#include <sys/stat.h>
#include "../../../Shared/IniParser.h"
#include "methods/core/ConfigImpl.h"
#include "MhImpl.h"

#define CONF_MHA_PUBLISHERS NIVIS_PROFILE"Monitor_Host_Publishers.conf"

#if defined( RELEASE_ISA )
////////////////////////////////////////////////////////////////////////////////
/// @author Marcel Ionescu
/// @brief Update a publisher info in MH provisioning file
/// @param p_szEUI64 EUI64 
/// @param p_szConcentrator <CO_TSAP_ID>,<CO_ID>,<Data_Period>,<Data_Phase>,<Data_StaleLimit>,< ContentVersion>
/// @param p_pVectorChannels array of channels, each with format <TSAP_ID>,<ObjID>,<AttrID>,<Index1>, <Index2>, <format>, <name>, <unit_of_measurement>
/// @retval false cannot set the publisher
/// @retval true publisher updated ok
/// @remarks If publisher was already there, updates it's info, otherwise create the publisher
/// update/create variables CONCENTRATOR in section <EUI64> (create the section if necessary)
/// for each CHANNEL from input
///		add/update the variable on the disk
/// delete extra CHANNEL variables from the disc 
////////////////////////////////////////////////////////////////////////////////
bool MhImpl::setPublisher( const char * p_szEUI64, const char * p_szConcentrator,
								const ChannelsVector * p_pVectorChannels )
{	unsigned idx = 0;
	CConfigImpl oConfig;
	oConfig.setVariable( CONF_MHA_PUBLISHERS, p_szEUI64, "CONCENTRATOR", p_szConcentrator);
	
	CConfigImpl::VarDetailVector * pVariables = oConfig.getGroupVariables( CONF_MHA_PUBLISHERS, p_szEUI64 );
	if( !pVariables )
		return false;

	ChannelsVector::const_iterator itChannels = p_pVectorChannels->begin();
	CConfigImpl::VarDetailVector::iterator it = pVariables->begin();
	for( ;(it != pVariables->end()) && ( itChannels != p_pVectorChannels->end()); ++it )
	{	/// Compute the index only for CHANNEL VARIABLES, skip other variable in the same section
		if( !strcasecmp("CHANNEL", (*it)->varName) )
		{	LOG("setPublisher: setting CHANNEL=[%s] at idx %u", (*itChannels), idx);
			oConfig.setVariable( CONF_MHA_PUBLISHERS, p_szEUI64, "CHANNEL", (*itChannels), 0, idx);
			++itChannels;
			++idx;
		}
	}
	/// Less variables on disk than user-provided. Add extra vars from user input
	if( (it == pVariables->end()) && (itChannels != p_pVectorChannels->end()) )
	{	LOG("setPublisher: LESS  channels on disk (%u) than on input (%d). Add from input", idx, p_pVectorChannels->size());
		for( ;itChannels != p_pVectorChannels->end(); ++itChannels )
		{	LOG("setPublisher: adding  CHANNEL=[%s] at idx %u", (*itChannels), idx);
			oConfig.setVariable( CONF_MHA_PUBLISHERS, p_szEUI64, "CHANNEL", (*itChannels), 0, idx);
			++idx;
		}
	} /// More variables on disk than user-provided. Delete the surplus from disk
	else if( (it != pVariables->end()) && (itChannels == p_pVectorChannels->end()) )
	{	CIniParser oIniParser;
		LOG("setPublisher: MORE  channels on disk than on input (%u). Delete extra", idx);
		if ( oIniParser.Load( CONF_MHA_PUBLISHERS, "r+", true ))
		{	for( ;it != pVariables->end(); ++it )
			{	if( !strcasecmp("CHANNEL", (*it)->varName) )
				{	/// No need to increment index, deleting decrements the real number instead
					oIniParser.DeleteVar( p_szEUI64, "CHANNEL", 0, idx);
				}
			}
		}
		else
		{
			LOG("setPublisher: Unable to open MHA config file [%s]", CONF_MHA_PUBLISHERS);
		}
	}	/// else the number is equal
	else
	{
		LOG("setPublisher: EQUAL number of channels on disk (%u) and input", idx);
	}

	/// cleanup. Have it separated by the main loop for clarity - the speed penalty is small
	for( CConfigImpl::VarDetailVector::iterator itVariables = pVariables->begin(); itVariables != pVariables->end(); ++itVariables )
	{
		delete (*itVariables)->varName ;
		delete (*itVariables)->varValue ;
		delete (*itVariables);
	}
	delete pVariables;

	return true;
}

#elif defined( RELEASE_WHART )

#define BURST_KEY_SIZE (7+1)

////////////////////////////////////////////////////////////////////////////////
/// @author Florin Muresan
/// @brief Extract BurstMessage key.
///	@brief BURST=<command number>, <burst message>, <update period>, <maximum update period>
///	@brief BurstMessageKey=<command number>, <burst message> where:
///	@brief 		<command number> is integer from set {1, 2, 3, 9, 33}
///	@brief 		<burst message> is integer from set {1, 2, 3, 9, 33}
/// @param p_szSrc comma-separated row starting with an integer
/// @param p_szDst Filled with extracted integer. It is allocated by user and have at least <BURST_KEY_SIZE> bytes reserved
/// @retval false if the string cannot be extracted - may also be used to validate the input
/// @note Does null-terminate the string
////////////////////////////////////////////////////////////////////////////////
static bool sExtractBurstKey( const char * p_szSrc, size_t p_dwSize, char * p_pDst) {
	if (!p_szSrc || *p_szSrc == 0) {
		return false;
	}
	size_t nCommasCount = 0;
	for( size_t i = 0; p_szSrc[i] && (i < p_dwSize); ++i )	{
		if( p_pDst ) {
			if ( p_szSrc[i] == ',' ) {
				nCommasCount++;
			}
			if (nCommasCount == 2) {
				p_pDst[i] = p_szSrc[i];
				p_pDst[i+1] = 0;
				break;
			}
			p_pDst[i] = p_szSrc[i];
		}
	}
	return true;
}

bool cleanExtraVariables( const char * p_szEUI64 )
{
    CIniParser oIniParser;
    if (!oIniParser.Load( CONF_MHA_PUBLISHERS, "r+", true ))
        return false;

    bool result = false;
    CConfigImpl oConfig;
    CConfigImpl::VarDetailVector * pVariables = oConfig.getGroupVariables( CONF_MHA_PUBLISHERS, p_szEUI64 );
    if (pVariables)
    {
        for (CConfigImpl::VarDetailVector::iterator it = pVariables->begin(); (it != pVariables->end()); ++it )
        {
            if (strcasecmp("BURST", (*it)->varName) &&
                strcasecmp("VARIABLE", (*it)->varName) &&
                strcasecmp("TRIGGER", (*it)->varName)) {
                result = oIniParser.DeleteVar( p_szEUI64, (*it)->varName, 0 );
                LOG("delExtraVariables: delete %s=%s, %s", (*it)->varName, (*it)->varValue, (result ? "successful" : "failure"));
            }
        }
    }

    /// cleanup. Have it separated by the main loop for clarity - the speed penalty is small
    if (pVariables)
    {
        for( CConfigImpl::VarDetailVector::iterator itVar = pVariables->begin(); itVar != pVariables->end(); ++itVar )
        {
            delete (*itVar)->varName ;
            delete (*itVar)->varValue ;
            delete (*itVar);
        }
        delete pVariables;
    }

    return true;
}

////////////////////////////////////////////////////////////////////////////////
/// @author Florin Muresan
/// @brief Update a publisher info in MH provisioning file for WirelessHART
/// @param p_szEUI64 EUI64
/// @param p_szBurstMessage, in format: <command number>, <burst message>, <update period>, <maximum update period>
/// @param p_pVectorVariables array of variables, each in format: <command number>, <burst message>, <device variable code>, <name>, <device variable slot> <device variable classification>, <units code>
/// @param p_szTrigger, in format: <command number>, <burst message>, <burst trigger mode selection>, <device variable classification>, <units code>, <trigger level>
/// @retval false cannot set the publisher
/// @retval true publisher updated ok
/// @remarks If publisher was already there, updates it's info, otherwise create the publisher
/// update/create variables VARIABLE in section <EUI64> (create the section if necessary)
/// for each CHANNEL from input
///		add/update the variable on the disk
/// delete extra CHANNEL variables from the disc
////////////////////////////////////////////////////////////////////////////////
bool MhImpl::setPublisher( const char * p_szEUI64, const char * p_szBurstMessage,
						   const ChannelsVector * p_pVectorVariables,
						   const char * p_szTrigger) {
	CConfigImpl oConfig;
	bool result = false;

	/** Get Burst key */
	char burstKey[BURST_KEY_SIZE];
	if (!sExtractBurstKey( p_szBurstMessage, BURST_KEY_SIZE, burstKey ) ) {
		LOG("setPublisher: GET Burst key from message [%s], fails", burstKey, p_szBurstMessage);
		return false;
	}

	/** Add / edit Burst Message */
	result = oConfig.setConfigRow( CONF_MHA_PUBLISHERS, p_szEUI64, "BURST", p_szBurstMessage, &sExtractBurstKey, BURST_KEY_SIZE );
	LOG("setPublisher: SET Burst=[%s] %s", p_szBurstMessage, (result ? "successful" : "failure"));

	/** Add / edit / delete Trigger */
	if (p_szTrigger && p_szTrigger[0] != 0) {
		result = oConfig.setConfigRow( CONF_MHA_PUBLISHERS, p_szEUI64, "TRIGGER", p_szTrigger, &sExtractBurstKey, BURST_KEY_SIZE );
		LOG("setPublisher: SET Trigger=[%s] %s", p_szTrigger, (result ? "successful" : "failure"));
	} else {
		result = oConfig.delConfigRow( CONF_MHA_PUBLISHERS, p_szEUI64, "TRIGGER", burstKey, &sExtractBurstKey, BURST_KEY_SIZE );
		LOG("setPublisher: DEL Trigger=[%s] %s", burstKey, (result ? "successful" : "failure"));
	}

	/** Add / edit Variables */
	CConfigImpl::VarDetailVector * pVariables = oConfig.getGroupVariables( CONF_MHA_PUBLISHERS, p_szEUI64 );
	if( !pVariables )
		return false;

	unsigned idx = 0;
	char valKey[BURST_KEY_SIZE];
	ChannelsVector::const_iterator itVariables = p_pVectorVariables->begin();
	CConfigImpl::VarDetailVector::iterator it = pVariables->begin();
	for( ;(it != pVariables->end()) && ( itVariables != p_pVectorVariables->end()); ++it ) {
		/// Compute the index only for VARIABLE, skip other variable in the same section
		if ( !strcasecmp("VARIABLE", (*it)->varName) ) {
			if ( sExtractBurstKey( (*it)->varValue, BURST_KEY_SIZE, valKey ) && !strcasecmp( valKey, burstKey ) ) {
				LOG("setPublisher: EDIT Variable=[%s] at idx %u", (*itVariables), idx);
				oConfig.setVariable( CONF_MHA_PUBLISHERS, p_szEUI64, "VARIABLE", (*itVariables), 0, idx);
				++itVariables;
			}
			++idx;
		}
	}
	/// Less variables on disk than user-provided. Add extra vars from user input
	if( (it == pVariables->end()) && (itVariables != p_pVectorVariables->end()) ) {
		for( ;itVariables != p_pVectorVariables->end(); ++itVariables ) {
			if ( sExtractBurstKey( (*itVariables), BURST_KEY_SIZE, valKey ) && !strcasecmp( valKey, burstKey ) ) {
				LOG("setPublisher: ADD  Variable=[%s] at idx %u", (*itVariables), idx);
				oConfig.setVariable( CONF_MHA_PUBLISHERS, p_szEUI64, "VARIABLE", (*itVariables), 0, idx);
			}
			++idx;
		}
	} /// More variables on disk than user-provided. Delete the surplus from disk
	else if( (it != pVariables->end()) && (itVariables == p_pVectorVariables->end()) ) {
		CIniParser oIniParser;
		//LOG("setPublisher: MORE Variables on disk than on input (%u). Delete extra", idx);
		if ( oIniParser.Load( CONF_MHA_PUBLISHERS, "r+", true )) {
			for( ;it != pVariables->end(); ++it ) {
				if( !strcasecmp("VARIABLE", (*it)->varName) ) {
					if (sExtractBurstKey( (*it)->varValue, BURST_KEY_SIZE, valKey ) && !strcasecmp( valKey, burstKey ) ) {
						/// No need to increment index, deleting decrements the real number instead
						oIniParser.DeleteVar( p_szEUI64, "VARIABLE", 0, idx );
						LOG("setPublisher: DEL Variable=[%s] at idx %u", (*it)->varValue, idx);
					}
				}
			}
		} else {
			LOG("setPublisher: Unable to open MHA config file [%s]", CONF_MHA_PUBLISHERS);
		}
	} else {	/// else the number is equal
		LOG("setPublisher: EQUAL number of variables on disk (%u) and input", idx);
	}

	cleanExtraVariables( p_szEUI64 );

	/// cleanup. Have it separated by the main loop for clarity - the speed penalty is small
	for( CConfigImpl::VarDetailVector::iterator itVar = pVariables->begin(); itVar != pVariables->end(); ++itVar ) {
		delete (*itVar)->varName ;
		delete (*itVar)->varValue ;
		delete (*itVar);
	}
	delete pVariables;

	return true;
}
#endif

#if defined( RELEASE_ISA )
////////////////////////////////////////////////////////////////////////////////
/// @brief   Delete a publisher line from the MH provisioning file.
/// @param   p_szEUI64 EUI64
/// @retval  false cannot erase the publisher
/// @retval  true  publisher erased ok
////////////////////////////////////////////////////////////////////////////////
bool MhImpl::delPublisher( const char * p_szEUI64)
{	CIniParser oIniParser;
	if ( !oIniParser.Load( CONF_MHA_PUBLISHERS, "r+", true ))
	{
		LOG("delPublisher: Unable to open file " CONF_MHA_PUBLISHERS);
		return false ;
	}
	return oIniParser.DeleteGroup( p_szEUI64 );
}

#elif defined( RELEASE_WHART )
////////////////////////////////////////////////////////////////////////////////
/// @author Florin Muresan
/// @brief   Delete a publisher line from the MH provisioning file.
/// @param   p_szEUI64 EUI64
/// @param   p_szBurst Burst message
/// @retval  false cannot erase the publisher or some variables
/// @retval  true  publisher erased ok
////////////////////////////////////////////////////////////////////////////////
bool MhImpl::delPublisher( const char * p_szEUI64, const char * p_szBurst) {
	CConfigImpl oConfig;
	bool result = false, resAll = false;

	resAll = oConfig.delConfigRow( CONF_MHA_PUBLISHERS, p_szEUI64, "BURST", p_szBurst, &sExtractBurstKey, BURST_KEY_SIZE );
	LOG("delPublisher: delete BURST=[%s] %s", p_szBurst, (resAll ? "successful" : "failure"));

	char burstKey[BURST_KEY_SIZE], varKey[BURST_KEY_SIZE];
	sExtractBurstKey(p_szBurst, BURST_KEY_SIZE, burstKey);
	CConfigImpl::VarDetailVector * pVariables = oConfig.getGroupVariables( CONF_MHA_PUBLISHERS, p_szEUI64 );
	if ( pVariables ) {
		for( CConfigImpl::VarDetailVector::iterator it = pVariables->begin(); (it != pVariables->end()); ++it ) {
			/// delete all variables assigned to selected burst
			if( !strcasecmp("VARIABLE", (*it)->varName) ) {
				sExtractBurstKey((*it)->varValue, BURST_KEY_SIZE, varKey);
				if(!strcasecmp( burstKey, varKey) ) {	/// value match
					//result = oConfig.delVarInNthSection( CONF_MHA_PUBLISHERS, p_szEUI64, varKey, "VARIABLE", (*it)->varValue, &sExtractBurstKey, BURST_KEY_SIZE );
					result = oConfig.delConfigRow( CONF_MHA_PUBLISHERS, p_szEUI64, "VARIABLE", (*it)->varValue, &sExtractBurstKey, BURST_KEY_SIZE);
					LOG("delPublisher: delete VARIABLE=[%s] %s", (*it)->varValue, (result ? "successful" : "failure"));
				}
			}
		}
	}

	/// delete trigger assigned to selected burst
	result = oConfig.delConfigRow( CONF_MHA_PUBLISHERS, p_szEUI64, "TRIGGER", burstKey, &sExtractBurstKey, BURST_KEY_SIZE );
	LOG("delPublisher: delete TRIGGER=[%s] %s", burstKey, (result ? "successful" : "failure"));

    cleanExtraVariables( p_szEUI64 );

	/// delete group if is empty
	pVariables = oConfig.getGroupVariables( CONF_MHA_PUBLISHERS, p_szEUI64 );
	if( pVariables ) {
		CConfigImpl::VarDetailVector::iterator it = pVariables->begin();
		if (it == pVariables->end()) {
			CIniParser oIniParser;
			if ( !oIniParser.Load( CONF_MHA_PUBLISHERS, "r+", true ))	{
				LOG("delPublisher: Unable to open file " CONF_MHA_PUBLISHERS);
				return false ;
			}
			//result = oIniParser.DeleteGroup( p_szEUI64 );
			result = oIniParser.SetVar(p_szEUI64, "AUTO_DETECT", "DELETED", 0, true);
			LOG("delPublisher: delete GROUP=[%s] %s", p_szEUI64, (result ? "successful" : "failure"));
		}
	}

	/// cleanup. Have it separated by the main loop for clarity - the speed penalty is small
    if ( pVariables ) {
        for( CConfigImpl::VarDetailVector::iterator itVariables = pVariables->begin(); itVariables != pVariables->end(); ++itVariables ) {
            delete (*itVariables)->varName ;
            delete (*itVariables)->varValue ;
            delete (*itVariables);
        }
        delete pVariables;
    }

	return resAll;
}
#endif
