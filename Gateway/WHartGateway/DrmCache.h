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

#ifndef DrmCache_h__
#define DrmCache_h__

#include <list>
#include <map>

#include <WHartStack/WHartTypes.h>
#include <WHartStack/WHartStack.h>
#include <WHartStack/util/WHartCmdWrapper.h>

#include "Shared/SimpleTimer.h"

namespace hart7 {
namespace gateway {

using namespace stack;





class CGwRequest;


///////////////////////////////////////////////////////////////////////
/// @brief Delayed Response Mechanism cache.
///////////////////////////////////////////////////////////////////////
class CDrmCache
{
public:
	static const int m_sCmdNotInCache = -1;
	class CDrmCacheEntry
	{
	public:
		int						m_nRevTime;
		CHartCmdWrapper::Ptr	m_pCmd;
		CHartCmdWrapper::Ptr	m_pRsp;
	};

	typedef std::list<CDrmCacheEntry> CDrmCacheEntryList;
	typedef std::map<int, CDrmCacheEntryList> CDrmCacheCmdMap;
	typedef std::map<WHartUniqueID, CDrmCacheCmdMap, CompareUniqueID> CDrmCacheUniqueIDMap;


public:
	CDrmCache(): m_nEntryLifetime(3600) {	m_oRefreshTimer.SetTimer(60 * 1000); }

	////////////////////////////////////////////////////////////////////////////////
	/// @brief	ResponseAdd adds a response to DRM cache
	/// @param	[in] p_pCmdMap	- the map with commands for a device
	/// @param	[in] p_pCmd		- command
	/// @param	[int] p_pRsp	- response  for command
	////////////////////////////////////////////////////////////////////////////////
	void ResponseAdd(CDrmCacheCmdMap & p_pCmdMap, CHartCmdWrapper::Ptr p_pCmd, CHartCmdWrapper::Ptr p_pRsp);


	////////////////////////////////////////////////////////////////////////////////
	/// @brief	ResponseAdd adds a req commands and responses to DRM cache
	/// @param	[in] p_pCmdMap	- the map with commands for a device
	/// @param	[in] p_pReq		- req commands
	/// @param	[in] p_pRspList	- responses  for req commands
	////////////////////////////////////////////////////////////////////////////////
	void ResponseAdd(const CGwRequest * p_pReq, const CHartCmdWrapperList & p_pRspList);

	////////////////////////////////////////////////////////////////////////////////
	/// @brief	ResponsePop returns a response if in cache and deletes from cache.
	/// @retval	m_sCmdNotInCache(-1) if not in cache, RCS_N00_SUCCESS if in cache a valid response (including error response)
	/// @retval	DR_INIT, DR_BUSY, DR_* -> command pending response
	/// @param	[in] p_pCmdMap	- the map with commands for a device
	/// @param	[in] p_pCmd		- cmd to get rsp for
	/// @param	[out] p_prRsp	- the response to cmd
	////////////////////////////////////////////////////////////////////////////////
	int ResponsePop(CDrmCacheCmdMap & p_pCmdMap, CHartCmdWrapper::Ptr & p_pCmd, CHartCmdWrapper::Ptr & p_prRsp);

	////////////////////////////////////////////////////////////////////////////////
	/// @brief	ResponsePop for req commands try to get responses. It will delete the responses.
	/// @retval	m_sCmdNotInCache(-1) if not in cache, RCS_N00_SUCCESS if in cache a valid response (including error response)
	/// @retval	DR_INIT, DR_BUSY, DR_* -> command pending response
	/// @param	[in/out] CGwRequest -
	////////////////////////////////////////////////////////////////////////////////
	int ResponsePop(CGwRequest *  p_pReq);

	////////////////////////////////////////////////////////////////////////////////
	/// @brief	Refresh cleanup older requests
	////////////////////////////////////////////////////////////////////////////////
	void Refresh();


	void SetEntryLifetime(int p_nLife) { m_nEntryLifetime = p_nLife; }
private:
	int						m_nEntryLifetime;

	CDrmCacheUniqueIDMap	m_oCacheMap;

	CSimpleTimer			m_oRefreshTimer;
};


} // namespace gateway
} // namespace hart7


#endif // DrmCache_h__
