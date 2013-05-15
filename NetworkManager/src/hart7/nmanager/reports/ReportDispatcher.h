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
 * ReportDispatcher.h
 *
 *  Created on: Apr 26, 2010
 *      Author: radu
 */

#ifndef REPORTDISPATCHER_H_
#define REPORTDISPATCHER_H_

#include "../AllNetworkManagerCommands.h"
#include <WHartStack/WHartStack.h>
#include "ReportListener.h"
#include <nlib/log.h>
#include <list>

namespace hart7 {

namespace nmanager {

using namespace hart7::stack;

/**
 * Used to register classes that want to be informed about reports received from the field.
 *
 * @author Radu Pop
 */
class ReportDispatcher
{
        LOG_DEF("h7.n.r.ReportDispatcher")
        ;

    private:
        // TODO should we keep the reports cached until the GW requests them ?

        // we could have more than 1 listener for an alarm source
        std::list<ReportListenerPointer> reportsListeners;

    public:

        ReportDispatcher();

        virtual ~ReportDispatcher();

        /**
         * Register a listener for report.
         */
        void registerReportListener(ReportListenerPointer listener);

        /**
         * When a new 779 report appears, dispatches the report to the interesting parties.
         */
        void dispatchReport779(const WHartAddress& source,
                               const C779_ReportDeviceHealth_Resp& reportDeviceHealth);

        /**
         * When a new 780 report appears, dispatches the report to the interesting parties.
         */
        void dispatchReport780(const WHartAddress& source,
                                                 const C780_ReportNeighborHealthList_Resp& neighborHealthList);

        /**
         * When a new 787 report appears, dispatches the report to the interesting parties.
         */
        void dispatchReport787(const WHartAddress& source,
                                                 const C787_ReportNeighborSignalLevels_Resp& neighborSignalLevels);

};

}

}

#endif /* REPORTDISPATCHER_H_ */
