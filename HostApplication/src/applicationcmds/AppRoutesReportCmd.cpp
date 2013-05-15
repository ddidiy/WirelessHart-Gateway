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

#include <WHartHost/applicationcmds/AppRoutesReportCmd.h>
#include <WHartHost/applicationcmds/IAppCommandVisitor.h>


namespace hart7 {
namespace hostapp {


AppRoutesReportCmd::AppRoutesReportCmd(int p_nNbOfNeededReports):
		m_enumState(GetRouteLists_State), m_nNbOfNeededRouteListReports(p_nNbOfNeededReports),
		m_nNbOfNeededSourceRoutesReports(0),m_noOfDevices(p_nNbOfNeededReports)
{
}

bool AppRoutesReportCmd::Accept(IAppCommandVisitor& visitor)
{
	visitor.Visit(*this);
	return true;
}


void AppRoutesReportCmd::DumpToStream(std::ostream& p_rStream) const
{
	p_rStream << "AppRoutesReportCmd[CommandID=" << dbCommand.commandID 
		<< " Status=" << (int)dbCommand.commandStatus 
		<< " DevicesNo=" << m_noOfDevices << "]";
}

RoutesListT& AppRoutesReportCmd::GetRoutes()
{
	return m_Routes;
}
//
void AppRoutesReportCmd::RegisterRouteListReport() 
{
	if (--m_nNbOfNeededRouteListReports == 0)
	{	m_enumState = GetSourceRoutes_State;
	}
}

SourceRoutesListT& AppRoutesReportCmd::GetSourceRoutes()
{
	return m_SourceRoutes;
}
void AppRoutesReportCmd::RegisterSourceRouteReport()
{ 
	--m_nNbOfNeededSourceRoutesReports; 
}
void AppRoutesReportCmd::RegisterRoute(DevicePtr& p_rDevice, int p_nRouteId)
{
	m_oRouteIdsPerDevice[p_rDevice].push_back(p_nRouteId);
	++m_nNbOfNeededSourceRoutesReports;
}
bool AppRoutesReportCmd::ReceivedAllRouteListReports()
{
	return m_nNbOfNeededRouteListReports <= 0;
}
bool AppRoutesReportCmd::ReceivedAllSourceRouteReports() 
{
	return (m_enumState == GetSourceRoutes_State) && (m_nNbOfNeededSourceRoutesReports <= 0); 
}

}
}
