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
// C++ Interface: AppGeneralCommand
//
// Description: 
//
//
// Author: Catrina Mihailescu <catrina@r2d2>, (C) 2010
//
//
//
#ifndef APPGENERALCMD_H_
#define APPGENERALCMD_H_


#include <WHartHost/applicationcmds/AbstractAppCommand.h>

namespace hart7 {
namespace hostapp {

class IAppCommandVisitor;

class AppGeneralCommand: public AbstractAppCommand
{
public:
	AppGeneralCommand (int p_nCmdNo, std::string p_szDataBytes, 
		int p_commandID, int p_deviceID, bool p_bypassIOCache = false) ;

	virtual bool Accept(IAppCommandVisitor& visitor);
	
protected:
	void DumpToStream(std::ostream& p_rStream) const;


public:
	int GetCmdNo();
	std::string GetDataBytes();
	int GetCommandID();
	int GetDeviceID();
	bool GetBypassIOCache();

private:
	int m_nCmdNo;
	std::string m_szDataBytes;
	int m_nCommandID;
	int m_nDeviceID;
	bool m_bypassIOCache;





};

}
}

#endif
