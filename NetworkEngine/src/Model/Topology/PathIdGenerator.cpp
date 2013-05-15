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
 * RouteIdGenerator.cpp
 *
 *  Created on: Mar 25, 2009
 *      Author: Catalin Pop
 */

#include "PathIdGenerator.h"
#include "Common/NEException.h"

namespace NE {

namespace Model {

namespace Topology {

std::set<Uint16> PathIdGenerator::allocatedIDs;

Uint16 PathIdGenerator::lastId = 0x103; //can start from 0 because first time will be incremented

Uint16 PathIdGenerator::generatePathId() {
    int numberOfFullSearchLoops = 0; //theoretically needed in case all route IDs are occupied to avoid forever loop
    ++lastId;

    if (lastId > MAX_15BITS_VALUE) {
        lastId = 0x103;
    }

    if (lastId == 0) {
        ++lastId;
    }

    while (allocatedIDs.find(lastId) != allocatedIDs.end()) {
        ++lastId;
        if (lastId > MAX_15BITS_VALUE) {
            lastId = 0x103;
            ++numberOfFullSearchLoops;
        }

        if (numberOfFullSearchLoops == 2) {
            // means that a full search all possible routes ID was made 1 time
            throw NE::Common::NEException(
                        "RouteIdGenerator::generateRouteID() : All route ID between 0 and 32767(0x7FFF) are occupied. NO MORE IDs AVAILABLE!");
        }
    }

    allocatedIDs.insert(lastId);
    return lastId;
}

void PathIdGenerator::clearPathId(Uint16 pathId) {
    allocatedIDs.erase(pathId);
}

Uint16 PathIdGenerator::getLastPathId() {
    return lastId;
}

}

}

}
