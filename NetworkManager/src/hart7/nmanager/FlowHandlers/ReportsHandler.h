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
 * ReportsHandler.h
 *
 *  Created on: Oct 2, 2009
 *      Author: andrei.petrut
 */

#ifndef REPORTSHANDLER_H_
#define REPORTSHANDLER_H_

#include "../AllNetworkManagerCommands.h"
#include "../CommonData.h"
#include "../INodeVisibleVerifier.h"

#include <WHartStack/WHartStack.h>
#include <nlib/log.h>

using namespace hart7::stack;

/**
 * Responsible with handling the health reports from the devices.
 */
class ReportsHandler
{
        LOG_DEF("h7.n.ReportsHandler")
        ;

    public:

        ReportsHandler(hart7::nmanager::CommonData& commonData_) :
            commonData(commonData_)
        {
        }

        void Handle(const WHartAddress& source, const C779_ReportDeviceHealth_Resp& deviceHealth);

        void Handle(const WHartAddress& source, const C780_ReportNeighborHealthList_Resp& neighborHealth);

        void Handle(const WHartAddress& source, const C787_ReportNeighborSignalLevels_Resp& neighborSignalLevels);

    private:

        hart7::nmanager::CommonData& commonData;

        // represents the visible edges to be processed;  Only used when the visible check is deactivated
        std::set<hart7::nmanager::VisibleEdge> visibleEdges;

};

#endif /* REPORTSHANDLER_H_ */
