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
 * PathIdGenerator.h
 *
 *  Created on: Jul 24, 2009
 *      Author: Radu Pop
 */

#ifndef PATHIDGENERATOR_H_
#define PATHIDGENERATOR_H_
#include <set>
#include "Common/NETypes.h"

namespace NE {

namespace Model {

namespace Topology {

class PathIdGenerator {

    private:

        static std::set<Uint16> allocatedIDs;

        static Uint16 lastId;

    public:

        static Uint16 generatePathId();

        static void clearPathId(Uint16 routeId);

        static Uint16 getLastPathId();
};

}

}

}

#endif /* PATHIDGENERATOR_H_ */
