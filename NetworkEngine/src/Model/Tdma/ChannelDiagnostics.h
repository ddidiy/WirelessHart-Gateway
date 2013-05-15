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
 * ChannelDiagnostics.h
 *
 *  Created on: Jun 29, 2009
 *      Author: Andy
 */

#ifndef CHANNELDIAGNOSTICS_H_
#define CHANNELDIAGNOSTICS_H_

namespace NE {
namespace Model {
namespace Tdma {

class ChannelDiagnostics {

    public:

        struct Diag {

            public:

                /**
                 * Percentage of time transmissions on channel that did not receive an ACK.
                 */
                Uint8 noAck;

                /**
                 * Percentage of time transmissions on channel aborted due to CCA.
                 */
                Int8 ccaBackoff;
        };

        std::map<Uint8, Diag> Channels;
};

}
}
}

#endif /* CHANNELDIAGNOSTICS_H_ */
