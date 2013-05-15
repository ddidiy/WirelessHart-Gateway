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
 * ReportsProcessor.h
 *
 *  Created on: Jun 14, 2010
 *      Author: radu
 */

#ifndef REPORTSPROCESSOR_H_
#define REPORTSPROCESSOR_H_

#include "../CommonData.h"
#include "../operations/WHOperation.h"
#include "../operations/WHOperationQueue.h"
#include <nlib/log.h>
#include <WHartStack/WHartStack.h>

namespace hart7 {

namespace nmanager {

/**
 * Processes all the reports received from the devices.
 * @author Radu Pop
 */
class ReportsProcessor: public ReportListener
{
        LOG_DEF("h7.n.a.AlarmsProcessor")
        ;

    private:

        CommonData& commonData;

        operations::WHOperationQueue& operationsQueue;

        // these are used to forward reports to GW using the Nivis meta command (64765)
        NE::Model::Operations::EngineOperationsListPointer nivisMetaOperations;

        std::vector<operations::WHOperationPointer> nivisMetaWhOperations;

    private:

        void sendOperations();

    public:

        ReportsProcessor(CommonData& commonData, operations::WHOperationQueue& operationsQueue);

        virtual ~ReportsProcessor();

        /**
         * @see also ReportListener
         */
        void newReport779(const WHartAddress& source, const C779_ReportDeviceHealth_Resp& reportDeviceHealth);

        /**
         * @see also ReportListener
         */
        void newReport780(const WHartAddress& source, const C780_ReportNeighborHealthList_Resp& neighborHealthList);

        /**
         * @see also ReportListener
         */
        void newReport787(const WHartAddress& source, const C787_ReportNeighborSignalLevels_Resp& neighborSignalLevels);

        /**
         * @see also ReportListener
         */
        std::string toStringReportListener();
};

}

}

#endif /* REPORTSPROCESSOR_H_ */
