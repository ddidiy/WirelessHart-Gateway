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
 * AllocationBitmap.h
 *
 *  Created on: Sep 11, 2009
 *      Author: ioanpocol
 */

#ifndef ALLOCATIONBITMAP_H_
#define ALLOCATIONBITMAP_H_

#include "TdmaTypes.h"
#include "Common/logging.h"

namespace NE {

namespace Model {

namespace Tdma {

class AllocationBitmap {

        LOG_DEF("I.M.G.AllocationBitmap");

        Uint32 index[8];

        Uint8 timeslotIndex;

        PublishPeriod::PublishPeriodEnum publishPeriod;

    public:

        /**
         *
         */
        AllocationBitmap();

        /**
         *
         */
        virtual ~AllocationBitmap();

        /**
         *
         */
        Uint8 getTimeslotIndex();

        /**
         *
         */
        PublishPeriod::PublishPeriodEnum getPublishPeriod();

        /**
         * Init the index for the maximum possible value for the specified publish period.
         * @param publishPeriod
         */

        void initMax(PublishPeriod::PublishPeriodEnum publishPeriod);


        int GetMaxTix(PublishPeriod::PublishPeriodEnum publishPeriod);

        /**
         * Init the index for the requested parameters, from timeslotIndex,
         * @param timeslotIndex - 0 to 119
         * @param publishTimeslots - 1 to 120
         */
        void initAllocationBitmap(Uint8 timeslotIndex, PublishPeriod::PublishPeriodEnum publishTimeslots);

        /**
         * Go to the next position; return false if no other position is available
         * @return true if there is a next position
         */
        bool selectNextPosition();

        /**
         * Go to the previous position; return false if no other position is available
         * @return true if there is a previous position
         */
        bool selectPreviousPosition();

        /**
         * Return true if the current position is free for the selected request
         */
        bool isFreePosition(const AllocationBitmap& allocationBitmap);

        /**
         * op1 OR op2 operator for the index
         * Add the allocation to the index
         */
        void operator+(const AllocationBitmap& allocationBitmap);

        /**
         * op1 OR op2 operator for the index; don't check if the data is already set
         */
        void operator|(const AllocationBitmap& allocationBitmap);

        /**
         * op1 AND !op2 operations
         * Remove the allocation from the index
         */
        void operator-(const AllocationBitmap& allocationBitmap);

        /**
         * op1 == op2 operations
         * Check if the allocations are the same
         */
        bool operator==(const AllocationBitmap& allocationBitmap);

        /**
         *
         */
        void toString(std::ostream& stream, bool newLine = true);

        int GetNumberOfAllocatedSlots();

};

}
}
}

#endif /* ALLOCATIONBITMAP_H_ */
