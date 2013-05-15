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

/*
 * ReportDispatcher.cpp
 *
 *  Created on: Apr 26, 2010
 *      Author: radu
 */

#include "ReportDispatcher.h"

namespace hart7 {

namespace nmanager {

ReportDispatcher::ReportDispatcher()
{
}

ReportDispatcher::~ReportDispatcher()
{
}

void ReportDispatcher::registerReportListener(ReportListenerPointer listener)
{
    LOG_TRACE("registerReportListener(listener)");

    this->reportsListeners.push_back(listener);
}

void ReportDispatcher::dispatchReport779(const WHartAddress& source,
                                         const C779_ReportDeviceHealth_Resp& reportDeviceHealth)
{
    LOG_TRACE("callReport779Listeners()");

    std::list<ReportListenerPointer>::iterator itRep = reportsListeners.begin();
    for (; itRep != reportsListeners.end(); ++itRep)
    {
        (*itRep)->newReport779(source, reportDeviceHealth);
    }
}

void ReportDispatcher::dispatchReport780(const WHartAddress& source,
                                         const C780_ReportNeighborHealthList_Resp& neighborHealthList)
{
    LOG_TRACE("dispatchReport780()");

    std::list<ReportListenerPointer>::iterator itRep = reportsListeners.begin();
    for (; itRep != reportsListeners.end(); ++itRep)
    {
        (*itRep)->newReport780(source, neighborHealthList);
    }
}

void ReportDispatcher::dispatchReport787(const WHartAddress& source,
                                         const C787_ReportNeighborSignalLevels_Resp& neighborSignalLevels)
{
    LOG_TRACE("dispatchReport787()");

    std::list<ReportListenerPointer>::iterator itRep = reportsListeners.begin();
    for (; itRep != reportsListeners.end(); ++itRep)
    {
        (*itRep)->newReport787(source, neighborSignalLevels);
    }
}

}

}
