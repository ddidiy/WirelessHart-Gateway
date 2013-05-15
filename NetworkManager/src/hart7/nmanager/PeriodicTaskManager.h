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
 * PeriodicTaskManager.h
 *
 *  Created on: Aug 25, 2009
 *      Author: Andy
 */

#ifndef PERIODICTASKMANAGER_H_
#define PERIODICTASKMANAGER_H_

#include "IPeriodicTask.h"
#include <vector>
#include <stdint.h>

namespace hart7 {
namespace nmanager {

/**
 * Responsible with calling periodic tasks. Responsible also with managing tasks that need to be called
 * at certain times in the future.
 */
class PeriodicTaskManager
{
    public:

        void PeriodicTask(uint32_t timeElapsed);

        void RegisterPeriodicTask(IPeriodicTask& periodicTask);

        void UnregisterPeriodicTask(IPeriodicTask& periodicTask);

        /**
         * Placeholder to be able to fit a IPeriodicTask& in a vector.
         */
        struct IPeriodicTaskPlaceholder
        {
            public:

                IPeriodicTaskPlaceholder(IPeriodicTask& periodicTask_) :
                    periodicTask(periodicTask_)
                {
                }

                IPeriodicTaskPlaceholder(const IPeriodicTaskPlaceholder& other) :
                    periodicTask(other.periodicTask)
                {
                }

                friend bool operator==(IPeriodicTaskPlaceholder& left, IPeriodicTaskPlaceholder& right)
                {
                    return &(left.periodicTask) == &(right.periodicTask);
                }

                IPeriodicTaskPlaceholder& operator=(const IPeriodicTaskPlaceholder& periodicTask_)
                {
                    periodicTask = periodicTask_.periodicTask;
                    return *this;
                }

                IPeriodicTask& periodicTask;
        };
        typedef std::vector<IPeriodicTaskPlaceholder> PeriodicTaskList;

    private:

        PeriodicTaskList periodicTasks;
};

}
}

#endif /* PERIODICTASKMANAGER_H_ */
