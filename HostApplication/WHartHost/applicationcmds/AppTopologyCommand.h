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

#ifndef APPTOPOLOGYCOMMAND_H_
#define APPTOPOLOGYCOMMAND_H_

#include <WHartHost/applicationcmds/AbstractAppCommand.h>
#include <WHartHost/model/reports/TopologyReport.h>



namespace hart7 {
namespace hostapp {

class IAppCommandVisitor;

class AppTopologyCommand;
typedef boost::shared_ptr<AppTopologyCommand> AppTopologyCommandPtr;


/**
 * @brief Represents a topology command from application perspective
 * Command Pattern
 */
class AppTopologyCommand : public AbstractAppCommand
{

//default
public:
	AppTopologyCommand();
	virtual bool Accept(IAppCommandVisitor& visitor);

protected:
	void DumpToStream(std::ostream& p_rStream) const;


//for request/response
public:
	enum TopologyState
	{
		GetUniqueIDs_state = 0,
		GetNicknames_state,
		GetNeighbours_state,
		GetNeighbsPerGraphID_state,
		GetDeviceDetails_state,
		GetDeviceStatistics_state,
		GetBatteryLife_state
		
	};
	TopologyState m_state;

	//for GetNicknames_state, GetDeviceDetails_state, GetBatteryLife_state, GetNeighbours_state, GetNeighbsPerGraphID_state, GetDeviceStatistics_state
	int m_sumOfAllOffsets;


//for response
public:
	TopologyReport		m_Report;

};



} //namespace hostapp
} //namsepace hart7

#endif /*APPTOPOLOGYCOMMAND_H_*/
