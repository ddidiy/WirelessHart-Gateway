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

#include "ManagerUtils.h"

namespace hart7 {

namespace util {

void ManagerUtils::determineElementsToReturn(uint16_t listSize, uint8_t requestedIndex, uint8_t requestedCount,
                                             uint8_t& returnedIndex, uint8_t& returnedCount)
{

    uint16_t requestedIndex16 = requestedIndex;
    uint16_t returnedIndex16 = returnedIndex;

    determineElementsToReturn(listSize, requestedIndex16, requestedCount, returnedIndex16, returnedCount);

    returnedIndex = (uint8_t) returnedIndex16;
}

void ManagerUtils::determineElementsToReturn(uint16_t listSize, uint16_t requestedIndex, uint8_t requestedCount,
                                             uint16_t& returnedIndex, uint8_t& returnedCount)
{
    if (listSize == 0)
    {
        returnedIndex = 0;
        returnedCount = 0;

        return;
    }

    // if the requestedIndex is outside list boundaries returned the last element
    if (requestedIndex >= listSize)
    {
        returnedIndex = listSize - 1;
        returnedCount = 1;
        return;
    }

    returnedIndex = requestedIndex;
    returnedCount = std::min(requestedCount, (uint8_t) (listSize - requestedIndex));
}

}

}
