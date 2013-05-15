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
 * PeriodicTaskManager.cpp
 *
 *  Created on: Aug 25, 2009
 *      Author: Andy
 */

#include "PeriodicTaskManager.h"

namespace hart7 {
namespace nmanager {

void PeriodicTaskManager::PeriodicTask(uint32_t timeElapsed)
{
    for (PeriodicTaskList::iterator it = periodicTasks.begin(); it != periodicTasks.end(); ++it)
    {
        it->periodicTask.PeriodicTask();
    }
}

void PeriodicTaskManager::RegisterPeriodicTask(IPeriodicTask& periodicTask)
{
    periodicTasks.push_back(IPeriodicTaskPlaceholder(periodicTask));
}

void PeriodicTaskManager::UnregisterPeriodicTask(IPeriodicTask& periodicTask)
{
    for (PeriodicTaskList::iterator it = periodicTasks.begin(); it != periodicTasks.end(); ++it)
    {
        if (&periodicTask == &(it->periodicTask))
        {
            periodicTasks.erase(it);
            break;
        }
    }
}

}
}
