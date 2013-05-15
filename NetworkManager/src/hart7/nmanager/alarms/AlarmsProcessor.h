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
 * AlarmsProcessor.h
 *
 *  Created on: Jun 14, 2010
 *      Author: radu pop
 */

#ifndef ALARMSPROCESSOR_H_
#define ALARMSPROCESSOR_H_

#include "../CommonData.h"
#include "../operations/WHOperation.h"
#include "../operations/WHOperationQueue.h"
#include <nlib/log.h>
#include <WHartStack/WHartStack.h>

namespace hart7 {

namespace nmanager {

/**
 * Processes all the alarms received from the devices.
 * @author Radu Pop
 */
class AlarmsProcessor: public AlarmListener
{
        LOG_DEF("h7.n.a.AlarmsProcessor")
        ;

    private:

        CommonData& commonData;

        operations::WHOperationQueue& operationsQueue;

        // these are used to forward alarms to GW using the Nivis meta command (64765)
        NE::Model::Operations::EngineOperationsListPointer nivisMetaOperations;

    	std::vector<operations::WHOperationPointer> nivisMetaWhOperations;

    private :

	    void sendOperations();

    public:

	    /**
	     * Constructor.
	     */
        AlarmsProcessor(CommonData& commonData, operations::WHOperationQueue& operationsQueue);

        virtual ~AlarmsProcessor();

        /**
         * Processes a new 788 alarm.
         */
        void newAlarm788(uint16_t src, C788_AlarmPathDown_Resp& alarmPathDown);

        /**
         * Processes a new 789 alarm.
         */
        void newAlarm789(uint16_t src, C789_AlarmSourceRouteFailed_Resp& alarmSourceRouteFailed);

        /**
         * Processes a new 790 alarm.
         */
        void newAlarm790(uint16_t src, C790_AlarmGraphRouteFailed_Resp& alarmGraphRouteFailed);

        /**
         * Processes a new 791 alarm.
         */
        void newAlarm791(uint16_t src, C791_AlarmTransportLayerFailed_Resp& alarmTransportLayerFailed);

        /**
         * To string method.
         */
        std::string toStringAlarmListener();
};

}

}

#endif /* ALARMSPROCESSOR_H_ */
