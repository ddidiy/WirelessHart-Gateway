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

#ifndef APPPREADVALUECMD_H_
#define APPPREADVALUECMD_H_


#include <WHartHost/applicationcmds/AbstractAppCommand.h>
#include <WHartHost/model/DeviceReading.h>


namespace hart7 {
namespace hostapp {

/**
 * @brief Represents a topology command from application perspective
 * Command Pattern
 */
class IAppCommandVisitor;
class AppReadValueCmd;

typedef boost::shared_ptr<AppReadValueCmd> AppReadValueCmdPtr;

class AppReadValueCmd: public AbstractAppCommand
{
public:
	AppReadValueCmd ( uint16_t p_nCmdNo, uint16_t p_nDeviceId,
					  DeviceReading::ReadingType p_enumReadingType,
					  const PublishChannelSetT &p_rList,
					  bool bypassIOCache = false);

	virtual bool Accept(IAppCommandVisitor& visitor);

protected:
	void DumpToStream(std::ostream& p_rStream) const;

public:
	uint16_t GetCmdNo();
	int GetDeviceId();
	DeviceReading::ReadingType GetReadingType();
	PublishChannelSetT& GetChannelList();
	bool GetBypassIOCache();

private:
	uint16_t m_nCmdNo;
	int m_nDeviceId;
	DeviceReading::ReadingType m_enumReadingType;
	PublishChannelSetT m_oList;
	bool m_bypassIOCache;
};

}
}

#endif
