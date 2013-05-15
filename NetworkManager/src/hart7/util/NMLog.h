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
 * NMLog.h
 *
 * Represents the log file used in NetworkManager project.
 * Logs operations, commands, etc.
 *
 *  Created on: Dec 15, 2008
 *      Author: Radu Pop
 */

#ifndef NMLog_H_
#define NMLog_H_

#include <nlib/log.h>
#include "../nmanager/operations/WHOperation.h"
#include <stdint.h>

namespace hart7 {

namespace util {

class NMLog
{

    public:

        NMLog();

        ~NMLog();

        /**
         * Logs the information from the command.
         */
        static void logCommand(uint16_t commandId, void* command, const WHartAddress& address);

        /**
         * Logs the information from the command response.
         */
        static void logCommandResponse(uint16_t commandId, uint16_t errorCode, void* command, const WHartAddress& address);

        /**
         * Logs the operation.
         */
        static void logOperation(hart7::nmanager::operations::WHOperationPointer wHOperation);

        /**
         * Logs the operation with a command response.
         */
        static void logOperationResponse(hart7::nmanager::operations::WHOperationPointer wHOperation);

        /**
         * Logs the device history for the devices in DeviceTable.
         */
        static void logDeviceHistory();
};

}
}

#endif /* NMLog_H_ */
