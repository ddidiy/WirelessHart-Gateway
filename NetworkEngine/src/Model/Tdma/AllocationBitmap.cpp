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
 * AllocationBitmap.cpp
 *
 *  Created on: Sep 11, 2009
 *      Author: ioanpocol
 */

#include "AllocationBitmap.h"
#include <iomanip>

using namespace NE::Model::Tdma;

AllocationBitmap::AllocationBitmap() {
    for (int i = 0; i < 8; i++) {
        index[i] = 0;
    }
}

AllocationBitmap::~AllocationBitmap() {
}

Uint8 AllocationBitmap::getTimeslotIndex() {
    return timeslotIndex;
}

PublishPeriod::PublishPeriodEnum AllocationBitmap::getPublishPeriod() {
    return publishPeriod;
}

int AllocationBitmap::GetMaxTix(PublishPeriod::PublishPeriodEnum publishPeriod)
{
    int tix = 0;
    switch (publishPeriod)
    {
        case PublishPeriod::P_250_MS:   tix =1; break;
        case PublishPeriod::P_500_MS:   tix =2; break;
        case PublishPeriod::P_1_S:      tix =4; break;
        case PublishPeriod::P_2_S:      tix =8; break;
        case PublishPeriod::P_4_S:      tix =16; break;
        case PublishPeriod::P_8_S:      tix =32; break;
        case PublishPeriod::P_16_S:     tix =64; break;
        case PublishPeriod::P_32_S:     tix =128; break;
        case PublishPeriod::P_64_S:     tix =256; break;
        case PublishPeriod::P_ALL:      tix =1; break;
        default: break;
    }

    return tix;
}

void AllocationBitmap::initMax(PublishPeriod::PublishPeriodEnum publishPeriod)
{
    initAllocationBitmap(GetMaxTix(publishPeriod) - 1, publishPeriod);
}

void AllocationBitmap::initAllocationBitmap(Uint8 timeslotIndex_, PublishPeriod::PublishPeriodEnum publishPeriod_) {
    timeslotIndex = timeslotIndex_;
    publishPeriod = publishPeriod_;

    Uint32 value = 0;
    Uint8 count = publishPeriod / 8;

    if (count == 0) {
        // for P_16_S = 4, P_32_S = 2, P_64_S = 1,  P_ALL = 0
        value = 0x80000000 >> (timeslotIndex % 32);
        for (int i = 0; i < 8; i++) {
            if (i % (8 / publishPeriod) == (timeslotIndex / 32)) {
                index[i] = value;
            } else {
                index[i] = 0;
            }
        }
    } else {
        if (publishPeriod == PublishPeriod::P_250_MS) {
            value = 0xFFFFFFFF;
        } else if (publishPeriod == PublishPeriod::P_500_MS) {
            value = 0xAAAAAAAA;
        } else if (publishPeriod == PublishPeriod::P_1_S) {
            value = 0x88888888;
        } else if (publishPeriod == PublishPeriod::P_2_S) {
            value = 0x80808080;
        } else if (publishPeriod == PublishPeriod::P_4_S) {
            value = 0x80008000;
        } else if (publishPeriod == PublishPeriod::P_8_S) {
            value = 0x80000000;
        }

        value = value >> timeslotIndex;

        for (int i = 0; i < 8; i++) {
            index[i] = value;
        }
    }
}

bool AllocationBitmap::selectNextPosition() {
    LOG_DEBUG("selectNextPosition: (timeslotIndex + 1):" << (int) (timeslotIndex + 1) << " < (256 / publishPeriod):"
                << (int) (256 / publishPeriod));

    if (timeslotIndex + 1 < 256 / publishPeriod) {
        timeslotIndex += 1;
        initAllocationBitmap(timeslotIndex, publishPeriod);
        LOG_DEBUG("selectNextPosition: true");
        return true;
    } else {
        LOG_DEBUG("selectNextPosition: false");
        return false;
    }

}

bool AllocationBitmap::selectPreviousPosition() {
    LOG_DEBUG("selectPreviousPosition: (timeslotIndex - 1):" << (int) (timeslotIndex - 1) << " >= 0");

    if (timeslotIndex - 1 >= 0) {
        timeslotIndex -= 1;
        initAllocationBitmap(timeslotIndex, publishPeriod);
        LOG_DEBUG("selectPreviousPosition: true");
        return true;
    } else {
        LOG_DEBUG("selectPreviousPosition: false");
        return false;
    }

}

bool AllocationBitmap::isFreePosition(const AllocationBitmap& allocationBitmap) {
    for (int i = 0; i < 8; i++) {
        if ((index[i] & allocationBitmap.index[i]) != 0) {
            return false;
        }
    }
    return true;
}

void AllocationBitmap::operator+(const AllocationBitmap& allocationBitmap) {
    for (int i = 0; i < 8; i++) {
        if ((index[i] & allocationBitmap.index[i]) != 0) {
//            LOG_ERROR("AllocationBitmap -op+ index=" << (int) i << ", crt=" << std::hex << index[i] << ", new="
//                        << allocationBitmap.index[i] << std::dec);
//            LOG_ERROR("AllocationBitmap -op+ crtTi=" << (int) timeslotIndex << ", newTi="
//                        << (int) allocationBitmap.timeslotIndex << ", newPp=" << (int) publishPeriod << ", oldPp="
//                        << (int) allocationBitmap.publishPeriod);

        }

        index[i] |= allocationBitmap.index[i];
    }
}

void AllocationBitmap::operator|(const AllocationBitmap& allocationBitmap) {
    for (int i = 0; i < 8; i++) {
        index[i] |= allocationBitmap.index[i];
    }
}

void AllocationBitmap::operator-(const AllocationBitmap& allocationBitmap) {
    for (int i = 0; i < 8; i++) {
        if ((index[i] & allocationBitmap.index[i]) != allocationBitmap.index[i]) {
            LOG_DEBUG("AllocationBitmap -op- index=" << (int) i << ", crt=" << std::hex << index[i] << ", new="
                        << allocationBitmap.index[i] << std::dec);
            LOG_DEBUG("AllocationBitmap -op- crtTi=" << (int) timeslotIndex << ", newTi="
                        << (int) allocationBitmap.timeslotIndex << ", newPp=" << (int) publishPeriod << ", oldPp="
                        << (int) allocationBitmap.publishPeriod);
        }

        index[i] &= ~allocationBitmap.index[i];
    }
}

bool AllocationBitmap::operator==(const AllocationBitmap& allocationBitmap) {
    for (int i = 0; i < 8; i++) {
        if (index[i] != allocationBitmap.index[i]) {
            return false;
        }
    }
    return true;
}

void AllocationBitmap::toString(std::ostream& stream, bool newLine) {
    for (int i = 0; i < 8; i++) {
        stream << std::hex << std::setw(8) << std::setfill('0') << index[i] << std::dec;
    }

    if (newLine)
        stream << std::endl;
}

int AllocationBitmap::GetNumberOfAllocatedSlots()
{
    int count = 0;
    Uint32 temp;
    for (int i = 0; i < 8; i++)
    {
        //see The Aggregate Magic Algorithms (UK University of Kentucky) - Population count algorithm
        temp = index[i] - ( (index[i] >> 1) & 0x55555555 );
        temp = (temp & 0x33333333) + ( (temp >> 2) & 0x33333333 );
        temp = (temp & 0x0F0F0F0F)  + ( (temp >> 4) & 0x0F0F0F0F );
        temp = temp + (temp >> 8);
        temp = ( temp + (temp >> 16) );
        count += temp & 0x3F;
    }
    return count;
}
