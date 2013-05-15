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
 * SessionIP.cpp
 *
 *  Created on: Sep 2, 2010
 *      Author: andy
 */

#include <WHartStack/CommInterfaceOverIP/SessionIP.h>

namespace hart7 {
namespace stack {
namespace transport {

std::ostream& operator<<(std::ostream& output, SessionIP& session)
{
    session.AcceptStream(output);
    return output;
}

}
}
}
