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
 * TdmaTypes.h
 *
 *  Created on: Mar 17, 2009
 *      Author: ioanpocol
 */

#ifndef TDMATYPES_H_
#define TDMATYPES_H_

#include "Common/NETypes.h"
#include "Common/NEException.h"

namespace NE {

namespace Model {

namespace Tdma {

#define MAX_FREQUENCY 15
#define MAX_STAR_INDEX 25

namespace PublishPeriod {
/**
 * Enumeration with possible publish periods.
 */
enum PublishPeriodEnum {
    P_250_MS = 256, P_500_MS = 128, P_1_S = 64, P_2_S = 32, P_4_S = 16, P_8_S = 8, P_16_S = 4, P_32_S = 2, P_64_S = 1,  P_ALL = 0
};
}

namespace SuperframeId {
/**
 * Enumeration with possible publish periods.
 */
enum SuperframeIdEnum {
    //SID_250_MS = 0, SID_500_MS = 1, SID_1_S = 2, SID_2_S = 3, SID_4_S = 4, SID_8_S = 5, SID_16_S = 6, SID_32_S = 7, SID_64_S = 8, SID_ALL = 9
    SID_250_MS = 1, SID_500_MS = 2, SID_1_S = 3, SID_2_S = 4, SID_4_S = 5, SID_8_S = 6, SID_16_S = 7, SID_32_S = 8, SID_64_S = 9, SID_ALL = 10
    //SID_250_MS = 2, SID_500_MS = 3, SID_1_S = 4, SID_2_S = 5, SID_4_S = 6, SID_8_S = 7, SID_16_S = 8, SID_32_S = 9, SID_64_S = 10, SID_ALL = 11
    //SID_250_MS = 246, SID_500_MS = 247, SID_1_S = 248, SID_2_S = 249, SID_4_S = 250, SID_8_S = 251, SID_16_S = 252, SID_32_S = 253, SID_64_S = 254, SID_ALL = 255
};
}

namespace SuperframeLength {
/**
 * Enumeration with the number of slots for each superframe.
 */
enum SuperframeLengthEnum {
    SLENGTH_250_MS = 25,
    SLENGTH_500_MS = 50,
    SLENGTH_1_S = 100,
    SLENGTH_2_S = 200,
    SLENGTH_4_S = 400,
    SLENGTH_8_S = 800,
    SLENGTH_16_S = 1600,
    SLENGTH_32_S = 3200,
    SLENGTH_64_S = 6400,
    SLENGTH_ALL = 1
};
}

namespace LinkTypes {
/**
 * Enumeration of link type.
 */
enum LinkTypesEnum {
    ANY = -1, NORMAL = 0, DISCOVERY = 1, BROADCAST = 2, JOIN = 3
};
}

inline Uint8 getSuperframeId(PublishPeriod::PublishPeriodEnum publishPeriod) {

    Uint8 superframeId = 0;

    if (publishPeriod == PublishPeriod::P_250_MS) {
        superframeId = (Uint8) SuperframeId::SID_250_MS;
    } else if (publishPeriod == PublishPeriod::P_500_MS) {
        superframeId = (Uint8) SuperframeId::SID_500_MS;
    } else if (publishPeriod == PublishPeriod::P_1_S) {
        superframeId = (Uint8) SuperframeId::SID_1_S;
    } else if (publishPeriod == PublishPeriod::P_2_S) {
        superframeId = (Uint8) SuperframeId::SID_2_S;
    } else if (publishPeriod == PublishPeriod::P_4_S) {
        superframeId = (Uint8) SuperframeId::SID_4_S;
    } else if (publishPeriod == PublishPeriod::P_8_S) {
        superframeId = (Uint8) SuperframeId::SID_8_S;
    } else if (publishPeriod == PublishPeriod::P_16_S) {
        superframeId = (Uint8) SuperframeId::SID_16_S;
    } else if (publishPeriod == PublishPeriod::P_32_S) {
        superframeId = (Uint8) SuperframeId::SID_32_S;
    } else if (publishPeriod == PublishPeriod::P_64_S) {
        superframeId = (Uint8) SuperframeId::SID_64_S;
    } else if (publishPeriod == PublishPeriod::P_ALL) {
        superframeId = (Uint8) SuperframeId::SID_ALL;
    }else {
        std::ostringstream stream;
        stream << "Invalid publishPeriod in getSuperframeId() ";
        stream << (int) publishPeriod;
        throw NE::Common::NEException(stream.str());
    }

    return superframeId;
}

}
}
}

#endif /* TDMATYPES_H_ */
