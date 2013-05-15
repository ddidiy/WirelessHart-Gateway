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

#ifndef __APPFIRMWARETRANSFERSCMD_H_
#define __APPFIRMWARETRANSFERSCMD_H_

#include <WHartHost/applicationcmds/AbstractAppCommand.h>
#include <WHartHost/database/DevicesManager.h>
#include <WHartHost/model/MMappedFile.h>

namespace hart7 {
namespace hostapp {

class IAppCommandVisitor;
class AppFirmwareTransfersCmd;

typedef boost::shared_ptr<AppFirmwareTransfersCmd> AppFirmwareTransfersCmdPtr;

class AppFirmwareTransfersCmd: public AbstractAppCommand
{

public:
	AppFirmwareTransfersCmd()
	{
	}

	virtual bool Accept(IAppCommandVisitor& visitor);

protected:
	void DumpToStream(std::ostream& p_rStream) const;

public:
	enum TransferStates
	{
		TransferOpen = 1,
		TransferInProgress = 2,
		TransferEnd = 3
	} m_currentTransferState;

	std::basic_string<char> m_fileName;
	boost::shared_ptr<MMappedFile> m_file;

	unsigned int m_currentOffset;
	unsigned int m_maxBlockSize;
	unsigned int m_currentBlockSize;
	unsigned int m_maxBlockCount;
	unsigned int m_currentBlockCount;

	unsigned short port;
	MAC m_deviceMAC;
	CMicroSec m_transferTime;
};

} //hostapp
} //hart7

#endif //__APPFIRMWARETRANSFERSCMD_H_

