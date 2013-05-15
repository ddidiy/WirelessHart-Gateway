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
 * ChannelTimeslotAllocation.h
 *
 *  Created on: Mar 17, 2009
 *      Author: ioanpocol
 */

#ifndef CHANNELTIMESLOTALLOCATION_H_
#define CHANNELTIMESLOTALLOCATION_H_

#include "AllocationBitmap.h"

namespace NE {

namespace Model {

namespace Tdma {

class ChannelTimeslotAllocation {

        AllocationBitmap allocationBitmap;

    public:

        /**
         *
         */
        ChannelTimeslotAllocation();

        /**
         *
         */
        virtual ~ChannelTimeslotAllocation();

        /**
         *
         */
        bool isFreePosition(const AllocationBitmap& allocationBitmap);

        /**
         *
         */
        void allocate(const AllocationBitmap& allocationBitmap);

        /**
         *
         */
        void allocateDiscovery(const AllocationBitmap& allocationBitmap);

        /**
         *
         */
        void deallocate(const AllocationBitmap& allocationBitmap);

        /**
         *
         */
        void toString(std::ostream& stream);

        AllocationBitmap& getAllocationBitmap() { return allocationBitmap; }

        friend std::ostream& operator<<(std::ostream& output, ChannelTimeslotAllocation& bmp);
};

std::ostream& operator<<(std::ostream& output, ChannelTimeslotAllocation& bmp);
}
}
}


#endif /* CHANNELTIMESLOTALLOCATION_H_ */
