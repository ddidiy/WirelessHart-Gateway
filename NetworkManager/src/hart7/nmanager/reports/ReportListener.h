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
 * ReportListener.h
 *
 *  Created on: Apr 26, 2010
 *      Author: radu
 */

#ifndef REPORTLISTENER_H_
#define REPORTLISTENER_H_

#include <stdint.h>
#include "boost/shared_ptr.hpp"
#include <WHartStack/WHartStack.h>

using namespace hart7::stack;

namespace hart7 {

namespace nmanager {

/**
 * This should be implemented by classes who want to be notified when a report is received.
 * @author Radu Pop
 */
class ReportListener
{

    public:

        virtual ~ReportListener()
        {
        }

        virtual void newReport779(const WHartAddress& source,
                                  const C779_ReportDeviceHealth_Resp& reportDeviceHealth) = 0;

        virtual void newReport780(const WHartAddress& source,
                                  const C780_ReportNeighborHealthList_Resp& neighborHealthList) = 0;

        virtual void newReport787(const WHartAddress& source,
                                  const C787_ReportNeighborSignalLevels_Resp& neighborSignalLevels) = 0;

        virtual std::string toStringReportListener() = 0;
};

typedef boost::shared_ptr<ReportListener> ReportListenerPointer;

}

}

#endif /* REPORTLISTENER_H_ */
