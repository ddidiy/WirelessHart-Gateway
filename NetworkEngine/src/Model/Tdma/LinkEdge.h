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
 * LinkEdge.h
 *
 *  Created on: Mar 17, 2009
 *      Author: ioanpocol
 */

#ifndef LINKEDGE_H_
#define LINKEDGE_H_

#include "Common/NEAddress.h"

namespace NE {

namespace Model {

namespace Tdma {

class LinkEdge;
typedef std::list<LinkEdge> LinkEdgesList;

class LinkEdge {

        Address32 source;

        Address32 destination;

        bool retry;

    public:
        /**
         *
         */
        LinkEdge(Address32 source, Address32 destination, bool retry);

        /**
         *
         */
        Address32 getSource();

        /**
         *
         */
        Address32 getDestination();

        /**
         *
         */
        bool isRetry();

        /**
         *
         */
        std::string toString();

        /**
         *
         */
        std::string toString(std::ostringstream& stream);
};

}
}
}
#endif /* LINKEDGE_H_ */
