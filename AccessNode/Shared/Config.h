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

/***************************************************************************
                          Config.h  -  description
                             -------------------
    begin                : Oct 22 2003
    email                : ion.ticus@nivis.com
 @addtogroup libshared
 @{
***************************************************************************/

#ifndef CONFIG_H
#define CONFIG_H

#include "IniParser.h"
#include "Common.h"
#include "Socket.h"
#include <arpa/inet.h>
//#include "list"

//lint -library

enum
{
	NO_RF_MEMBER,
	RF_MASTER_AN,
	RF_SLAVE_AN
} ;

//using namespace std;
class CConfig: protected CIniParser
{
public:
	CConfig();
	virtual ~CConfig();


    int Init( const char * p_szModule, const char * p_szFileName = INI_FILE);
	void ReadAndSetLogLevel();

	//TAKE CARE: the resulting group name is composed of
	//	p_szGroup and m_szModule with a underscore in between
	const char * FindGroup(const char * p_szGroup, bool p_bGetNext = false, bool p_bLogErrors = true );

	int GetVar(const char * p_szVarName, int * p_pnValue, int p_nPos = 0, bool p_bLogErrors = true  )
			{ return CIniParser::GetVar( (const char*)NULL, p_szVarName, p_pnValue, p_nPos, p_bLogErrors ); }

	int GetVar(const char * p_szVarName, float * p_pfValue, int p_nPos = 0, bool p_bLogErrors = true )
			{ return CIniParser::GetVar( (const char*)NULL, p_szVarName, p_pfValue, p_nPos, p_bLogErrors ); }

	int GetVar(const char * p_szVarName, char * p_szValue, int p_nValueSize, int p_nPos = 0, bool p_bLogErrors = true )
			{ return CIniParser::GetVar( (const char*)NULL, p_szVarName, p_szValue, p_nValueSize, p_nPos, p_bLogErrors ); }

	int GetVar(const char * p_szVarName, unsigned char * p_pValue, int p_nValueSize, int p_nPos = 0, bool p_bLogErrors = true )
			{ return CIniParser::GetVar( (const char*)NULL, p_szVarName, p_pValue, p_nValueSize, p_nPos, p_bLogErrors ); }

	int GetVar(const char * p_szVarName, net_address * p_pNetAddress, int p_nPos = 0, bool p_bLogErrors = true )
			{ return CIniParser::GetVar( (const char*)NULL, p_szVarName, p_pNetAddress, p_nPos, p_bLogErrors ); }

/*	int GetVar(const char * p_szVarName, GetVarList * p_pValuesList, int p_nPos = 0 , bool p_bLogErrors = true )
			{ return CIniParser::GetVar( (const char*)NULL, p_szVarName, p_pValuesList, p_nPos, p_bLogErrors);}*/
	int GetVar(const char * p_szVarName, TIPv6Address * p_pNetAddress, int p_nPos = 0, bool p_bLogErrors = true )
			{ return CIniParser::GetVar( (const char*)NULL, p_szVarName, p_pNetAddress, p_nPos, p_bLogErrors ); }

	int GetVarAsBaudRate( const char * p_szVarName, int * p_pnValue, int p_nPos = 0 );
	static int GetBaudRate( const char * p_szBaudRate );

	int GetVarAsNetAddressExt( const char * p_szVarName, CNetAddressExt& p_roNetAddrExt, int p_nPos = 0 );
	static int GetNetAddressExt( const char * p_pszNetAddrExt, CNetAddressExt& p_roNetAddrExt);


	int IsRfNetMember() const
	{
		return NO_RF_MEMBER != m_nIsMasterAN;
	}


	int IsMasterRFAN() const
	{
		return RF_MASTER_AN == m_nIsMasterAN;
	}


	union
	{
		rf_address          m_stAddress;
		unsigned int		m_nAnId;
	};
	unsigned short      m_shAppId;
	int					m_nIsMasterAN;
	char				m_szMeshInterface[12];
	char*				m_ppModulesToWatch[32];

private:
	char                m_szModule[32];

};

#define READ_DEFAULT_VARIABLE_INT( _name_, _v_, _defVal_ )			\
	if( !GetVar( _name_, &_v_, 0, false) )							\
	{	_v_ = _defVal_;												\
		LOG( "Assign default value for %s = %d", _name_, _v_ );	\
	}else{\
		LOG( "Assign configured value for %s = %d", _name_, _v_ );\
	}

#define READ_DEFAULT_VARIABLE_FLOAT(_name_, _v_, _defVal_ )			\
	if( !GetVar( _name_, &_v_, 0, false) )							\
	{	_v_ = _defVal_;												\
		LOG( "Assign default value for %s = %f", _name_, _v_ );	\
	}else{\
		LOG( "Assign configured value for %s = %f", _name_, _v_ );\
	}

#define READ_DEFAULT_VARIABLE_STRING( _name_, _v_, _defVal_ )		\
	if( !GetVar( _name_, _v_, sizeof( _v_ ), 0, false ) )			\
	{	strncpy( _v_, _defVal_, sizeof( _v_ ));				\
		_v_ [ sizeof( _v_ )-1 ] = 0;					\
		LOG( "Assign default value for %s = %s", _name_, _v_  );	\
	}else{\
		LOG( "Assign configured value for %s = %s", _name_, _v_ );\
	}

#define READ_DEFAULT_VARIABLE_YES_NO( _name_, _v_, _defVal_ )					\
	{	char _temp_[ 16 ];														\
		READ_DEFAULT_VARIABLE_STRING( _name_, _temp_, _defVal_  )				\
		_v_ = (		!strcasecmp( _temp_, "Y") || !strcasecmp( _temp_, "YES")	\
				||	!strcasecmp( _temp_, "1") || !strcasecmp( _temp_, "TRUE"));	\
	}

#define READ_MANDATORY_VARIABLE( _name_, _v_ )	\
	{	if( GetVar( _name_, &_v_ ) < 0 )		\
			return false;						\
	}

#define READ_MANDATORY_VARIABLE_STRING( _name_, _v_ )	\
	{	if( GetVar( _name_, _v_, sizeof( _v_ )) < 0 )	\
			return false;								\
	}

#define READ_MANDATORY_VARIABLE_HEX( _name_, _v_ )						\
	{	if( GetVar( _name_, (unsigned char*)&_v_, sizeof( _v_ ) ) < 0 )	\
			return false;												\
	}

#define READ_DEFAULT_VARIABLE_HEX( _name_, _v_ , _defVal_ )             \
    {   if( GetVar( _name_, (unsigned char*)&_v_, sizeof( _v_ ) ) )    \
        {   memcpy( _v_, _defVal_, sizeof( _defVal_ ));                 \
            LOG( "Assign default value for %s = %s",                    \
                _name_, GetHex(_v_, sizeof(_v_))  );                    \
        }                                                               \
        else                                                            \
        {   LOG( "Assign configured value for %s = %s",                 \
                _name_, GetHex(_v_, sizeof(_v_))  );                    \
        }                                                               \
    }

#define READ_DEFAULT_VARIABLE_BAUD_RATE( _name_, _v_, _defVal_ )		\
	if( !GetVarAsBaudRate( _name_, &_v_, 0) )				\
	{	_v_ = _defVal_;							\
		LOG( "Assign default value for %s = %d", _name_, _v_ );		\
	}else{									\
		LOG( "Assign configured value for %s = %d", _name_, _v_ );	\
	}

#define READ_DEFAULT_VARIABLE_NET_ADDR_EXT(_name_, _v_, _defVal_ )		\
	if( !GetVarAsNetAddressExt( _name_, _v_) )				\
	{	GetNetAddressExt( _defVal_,  _v_);				\
		LOG( "Assign default value for %s = %s", _name_,  _defVal_);	\
	}else{									\
		char _conf_addr_[128];						\
		inet_ntop(_v_.GetFamily(), _v_.GetIPv6(), _conf_addr_, 128);	\
		LOG( "Assign configured value for %s = %s, %hu",		\
			_name_ , _conf_addr_, _v_.GetPort());			\
	}



/// @}
#endif // CONFIG_H
