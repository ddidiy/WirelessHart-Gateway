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

#ifndef WHartCmdWrapper_h__
#define WHartCmdWrapper_h__

#include <boost/shared_ptr.hpp>
#include <boost/utility.hpp>
#include <WHartStack/WHartTypes.h>
#include <WHartStack/WHartStack.h>

#include <ApplicationLayer/ApplicationCommand.h>
#include <ApplicationLayer/Binarization/NivisSpecificCommands.h>
#include <ApplicationLayer/Model/CommonPracticeCommands.h>

#include <nlib/log.h>


#include <list>
#include <map>

#define CREATE_CMD_RESP_STRUCTURE(CommandText)\
  { CommandText##_Resp}



#define HART_DELIM_FRAME_TYPE_MASK		0x07
#define HART_DELIM_FRAME_TYPE_BURST		0x01
#define HART_DELIM_FRAME_TYPE_M2D		0x02
#define HART_DELIM_FRAME_TYPE_D2M		0x06

#define HART_DELIM_ADDR_TYPE_MASK		0x80
#define HART_DELIM_ADDR_TYPE_LONG		0x80
#define HART_DELIM_ADDR_TYPE_SHORT		0x00

#define HART_LONG_ADDR_BURST			0x40


namespace hart7 {
namespace stack {

using namespace stack;

/**
 * Network Management Specification - spec085r1.1-depr.pdf pg. 25/100 - Fig.4
 * Common Practice Command Specification - spec151r9.1-depr.pdf pg. 100/172
 */
struct TSubDevice
{
	uint8_t			m_nCard;
	uint8_t			m_nChannel;
	uint8_t			m_nDevicePollingAddress;
	uint8_t			m_nTransmitPreambleCount;
	uint8_t 		m_nDelimiter;
};

/**
 *
 */
struct TMetaCmdsInfo
{
	TMetaCmdsInfo()		{ memset (this, 0, sizeof(TMetaCmdsInfo)); }

	WHartUniqueID		m_oMetaCmdUniqueId;
	uint16_t			m_u16MetaCmdId;
	uint8_t				m_u8MetaCmdDeviceStatus;
	int16_t				m_i16MetaCmdResponseCode;

	WHartUniqueID		m_oInnerCmdUniqueId;

	union
	{
		struct
		{
			uint16_t			m_u16ExtDeviceType; // 0, gw or nm
		} m_oCmdNivisMeta;

		TSubDevice m_oCmdSubDevice;
	};
};


class CHartCmdWrapper  : boost::noncopyable
{
public:
	LOG_DEF("hart7.gateway.CHartCmdWrapper");

public:
	typedef boost::shared_ptr<CHartCmdWrapper> Ptr;

	enum {CmdType_Unk = 0, CmdType_WHart, CmdType_Hart};
	static const uint16_t g_sCmdIdInvalid = 0xffff;
	static const int16_t g_sRspCodeNotRsp = -1;

	enum {Not_a_Nivis_Meta_Cmd = 0};

private:
	uint8_t computeCheckByte(const uint8_t* p_pData, int len);

public:
	CHartCmdWrapper();
	~CHartCmdWrapper();

	void	ReleaseAll();
	void	ReleaseParsedData();

	int		LoadRawWHart(bool p_bIsResponse, const void * p_pData, int p_nMaxLen, uint8_t & errorCodeResponse, bool p_bReadDeviceStatus = false) ;
	int		GetRawWHart(void * p_pData, int p_nMaxLen, bool p_bWriteDeviceStatus = false) ;

	int		LoadHartDirect( bool p_bIsResponse, const void * p_pData, int p_nMaxLen, WHartUniqueID & p_oDeviceUniqueID, uint8_t & errorCodeResponse);
	int		GetHartDirect(void * p_pData, int p_nMaxLen, const WHartUniqueID & p_oDeviceUniqueID);// { return 0; }

	int		LoadHartWired( bool p_bIsResponse, const void * p_pData, int p_nMaxLen, WHartUniqueID & p_oDeviceUniqueID, uint8_t & errorCodeResponse);
	int		GetHartWired(void * p_pData, int p_nMaxLen, const WHartUniqueID & p_oDeviceUniqueID, uint8_t p_nDelimiter);


	void	LoadRaw(uint16_t p_u16CmdId, int p_nLen, void * p_pData, int p_nResponseCode = g_sRspCodeNotRsp, int p_nStatus = 0) ;
	void	LoadParsed(uint16_t p_u16CmdId, int p_nLen, void * p_pData, int p_nResponseCode = g_sRspCodeNotRsp, int p_nStatus = 0) ;

	int	ExtractDirectCommonHeader(bool p_bIsResponse, BinaryStream * p_pStream, uint16_t * p_pu16CmdId, int * p_pnLen,
	   	                       uint8_t & errorCodeResponse, int16_t * p_pi16ResponseCode, uint8_t * p_u8DeviceStatus = NULL );
	int	ExtractWiredCommonHeader(bool p_bIsResponse, BinaryStream * p_pStream, uint16_t * p_pu16CmdId, int * p_pnLen,
	   	                      uint8_t & errorCodeResponse, int16_t * p_pi16ResponseCode, uint8_t * p_u8DeviceStatus = NULL, bool p_bEmbeddedCommandHeader = false);

	int	WriteWiredCommonHeader(bool p_bIsResponse, BinaryStream & p_rStream, uint16_t p_u16CmdId, int p_nLen,  int16_t p_i16ResponseCode, uint8_t * p_pu8DeviceStatus = NULL);
	int	WriteDirectCommonHeader(bool p_bIsResponse, BinaryStream & p_rStream, uint16_t p_u16CmdId, int p_nLen,  int16_t p_i16ResponseCode, uint8_t * p_pu8DeviceStatus = NULL);

	int LoadMetaCmd77Header(bool p_bIsResponse, BinaryStream & stream, uint8_t & errorCodeResponse);
	bool IsBurstPublishAsRequest(uint16_t p_nCmdId, int p_nLen);

	void * GetRawData()
	{
		if (!HasRaw())
		{
		    GetRawFromParsed();
		}
		return m_pRawData;
	}
	int	GetRawDataLen()
	{
		if (!HasRaw())
		{
		    GetRawFromParsed();
		}
		return m_nRawLen;
	}

    bool IsFromAppHostClient()
    {
        return m_isFromAppHostClient;
    }
    void IsFromAppHostClient(bool isFromAppHostClient)
    {
        m_isFromAppHostClient = isFromAppHostClient;
    }

	void * GetParsedData()
	{
		if (!HasParsed())
		{
		    GetParsedFromRaw();
		}

		return m_pParsedData;
	}

	int	GetParsedDataLen()
	{
		if (!HasParsed())
		{
		    GetParsedFromRaw();
		}

		return m_nParseLen;
	}

	bool HasRaw() const { return m_bHasRaw; }
	bool HasParsed() const {return m_bHasParsed;}


	bool IsResponse() const { return m_i16ResponseCode != g_sRspCodeNotRsp;}

	uint16_t GetCmdId() const { return m_u16CmdId; }
	int	GetResponseCode() const { return m_i16ResponseCode; }
	void SetAsRequest () { m_i16ResponseCode = g_sRspCodeNotRsp; }
	void SetDeviceStatus (uint8_t p_u8DeviceStatus) { m_u8DeviceStatus = p_u8DeviceStatus; }
	void SetResponseCode (int16_t p_i16ResponseCode) { m_i16ResponseCode = p_i16ResponseCode; }

	int GetFrameFormat() {return m_nFormat;}
	void SetFrameFormat(int p_nFormat) { m_nFormat = p_nFormat; }

	void SetDeviceStatusFromTL (uint8_t p_u8DeviceStatus)
	{
		if (m_pMetaCmdsInfo)
		    m_pMetaCmdsInfo->m_u8MetaCmdDeviceStatus = p_u8DeviceStatus;
		else
		    m_u8DeviceStatus = p_u8DeviceStatus;
	}

	uint8_t GetDeviceStatus() const { return m_u8DeviceStatus; };

	uint8_t GetRawFromParsed();
	uint8_t GetParsedFromRaw();

	TMetaCmdsInfo *	MetaCmdInfoGet()		{	return m_pMetaCmdsInfo;		}
	void			MetaCmdInfoSet(TMetaCmdsInfo* p_pMetaCmdsInfo);
	void			MetaCmdInfoDel()		{  MetaCmdInfoSet(NULL); }

public:
	static void SetTable_ParseReqComposeResp( const ParseExecuteComposerEntry* p_pParseReqComposeResp, int p_nTableSize )
	{
		s_pParseReqComposeResp = p_pParseReqComposeResp;
		s_nParseReqComposeRespSize = p_nTableSize;
	}

	static void SetTable_ComposeReqParseResp( const ParseExecuteComposerEntry* p_pComposeReqParseResp, int p_nTableSize )
	{
		s_pComposeReqParseResp = p_pComposeReqParseResp;
		s_nComposeReqParseRespSize = p_nTableSize;
	}

private:
	uint8_t 	m_nDelimiter;
	uint8_t		m_nPolingAddress;

	uint16_t	m_u16CmdId;
	int16_t		m_i16ResponseCode;

	uint8_t		m_u8DeviceStatus; //only for HostApp cmds
	bool		m_bHasRaw;



	int			m_nRawLen;
	uint8_t*	m_pRawData;

	/**
	 * isFromAppHostClient is true for packets received from Host App Clients(HostApp, HCF Tester, ...);
	 */
	bool       m_isFromAppHostClient;

	bool		m_bHasParsed;
	int			m_nParseLen;
	uint8_t*	m_pParsedData;

	TMetaCmdsInfo*	m_pMetaCmdsInfo;

	int		m_nFormat;

private:
	static const ParseExecuteComposerEntry* s_pParseReqComposeResp;
	static const ParseExecuteComposerEntry* s_pComposeReqParseResp;

	static int s_nComposeReqParseRespSize;
	static int s_nParseReqComposeRespSize;

};

std::ostream& operator<< (std::ostream& out, const CHartCmdWrapper& p_rCmd );
std::ostream& operator<< (std::ostream& out, const CHartCmdWrapper* p_pCmd );


typedef std::list<CHartCmdWrapper::Ptr> CHartCmdWrapperList;




/**
 * Short Details for a given HartCmdWrapper.
 */
class CHartCmdWrapperShortDetails
{
public:
    CHartCmdWrapperShortDetails(const CHartCmdWrapper & hartCmdWrapper_) : hartCmdWrapper(hartCmdWrapper_) { }

    const CHartCmdWrapper & hartCmdWrapper;
};

std::ostream & operator<< (std::ostream & stream, const CHartCmdWrapperShortDetails & hartCmdWrapperShortDetails);




/**
 * Short Details for a given HartCmdWrapper List.
 */
class CHartCmdWrapperListShortDetails
{
public:
    CHartCmdWrapperListShortDetails(const CHartCmdWrapperList & hartCmdWrapperList_) : hartCmdWrapperList(hartCmdWrapperList_) { }

    const CHartCmdWrapperList & hartCmdWrapperList;
};

std::ostream & operator<< (std::ostream & stream, const CHartCmdWrapperListShortDetails & hartCmdWrapperListShortDetails);




int		RawWHart_ReadCmdList(bool p_bIsResponse, const void * p_pData, int p_nMaxLen, CHartCmdWrapperList & p_rCmdList,
   		                  uint8_t & errorCodeResponse, uint8_t p_u8DeviceStatus = 0) ;
int		RawWHart_WriteCmdList(void * p_pData, int p_nMaxLen, const CHartCmdWrapperList & p_rCmdList) ;

int		HartDirect_ReadCmdList(bool p_bIsResponse, const void * p_pData, int p_nMaxLen, bool isFromAppHostClient, WHartUniqueID & p_oDeviceUniqueID,
   		                       CHartCmdWrapperList & p_rCmdList, uint8_t & errorCodeResponse, uint8_t p_u8DeviceStatus = 0);
int		HartDirect_WriteCmdList(void * p_pData, int p_nMaxLen, const WHartUniqueID & p_oDeviceUniqueID, CHartCmdWrapperList & p_rCmdList  );

int		HartWired_ReadCmdList(bool p_bIsResponse, const void * p_pData, int p_nMaxLen, bool isFromAppHostClient, WHartUniqueID & p_oDeviceUniqueID,
   		                      CHartCmdWrapperList & p_rCmdList, uint8_t & errorCodeResponse, uint8_t p_u8DeviceStatus = 0);
int		HartWired_WriteCmdList(void * p_pData, int p_nMaxLen, const WHartUniqueID & p_oDeviceUniqueID, CHartCmdWrapperList & p_rCmdList, uint8_t p_nDelimiter  );

void	CmdList_MetaCmdInfoSet(CHartCmdWrapperList & p_rCmdList,TMetaCmdsInfo * p_pMetaCmdsInfo);
inline void CmdList_MetaCmdInfoDel(CHartCmdWrapperList & p_rCmdList) { CmdList_MetaCmdInfoSet(p_rCmdList, NULL); }
void CmdList_MetaCmdInfoSet_ToNM(CHartCmdWrapperList & p_rCmdList, WHartUniqueID & p_rInnerUniqueId );


std::ostream& operator<< (std::ostream & out, const CHartCmdWrapperList & p_rCmdList );

} // namespace stack
} // namespace hart7

#endif // WHartCmdWrapper_h__
