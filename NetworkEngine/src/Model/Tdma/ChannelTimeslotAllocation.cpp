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
 * ChannelTimeslotAllocation.cpp
 *
 *  Created on: Mar 17, 2009
 *      Author: ioanpocol
 */

#include "ChannelTimeslotAllocation.h"

using namespace NE::Model::Tdma;

ChannelTimeslotAllocation::ChannelTimeslotAllocation() {
}

ChannelTimeslotAllocation::~ChannelTimeslotAllocation() {
}

bool ChannelTimeslotAllocation::isFreePosition(const AllocationBitmap& allocationBitmap_) {
    return allocationBitmap.isFreePosition(allocationBitmap_);
}

void ChannelTimeslotAllocation::allocate(const AllocationBitmap& allocationBitmap_) {
    allocationBitmap + allocationBitmap_;
}

void ChannelTimeslotAllocation::allocateDiscovery(const AllocationBitmap& allocationBitmap_) {
    allocationBitmap | allocationBitmap_;
}

void ChannelTimeslotAllocation::deallocate(const AllocationBitmap& allocationBitmap_) {
    allocationBitmap - allocationBitmap_;
}

void ChannelTimeslotAllocation::toString(std::ostream& stream) {
    allocationBitmap.toString(stream);
}


std::ostream& operator<<(std::ostream& output, ChannelTimeslotAllocation& bmp)
{
    bmp.toString(output);
    return output;
}
