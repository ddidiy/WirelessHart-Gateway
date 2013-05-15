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
 * LinkEdge.cpp
 *
 *  Created on: Mar 17, 2009
 *      Author: ioanpocol
 */

#include "LinkEdge.h"

using namespace NE::Model::Tdma;

LinkEdge::LinkEdge(Address32 source_, Address32 destination_, bool retry_) :
    source(source_), destination(destination_), retry(retry_) {

}

Address32 LinkEdge::getSource() {
    return source;
}

Address32 LinkEdge::getDestination() {
    return destination;
}

bool LinkEdge::isRetry() {
    return retry;
}

std::string LinkEdge::toString() {
    std::ostringstream str;

    return toString(str);
}

std::string LinkEdge::toString(std::ostringstream& stream) {
    stream << "(" << ToStr(source) << ", " << ToStr(destination) << ")";

    return stream.str();
}
