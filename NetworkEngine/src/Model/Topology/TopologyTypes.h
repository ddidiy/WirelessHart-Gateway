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

#ifndef ROUTINGGRAPHTYPES_H_
#define ROUTINGGRAPHTYPES_H_

#include "Common/NETypes.h"
#include <map>

namespace NE {

namespace Model {

namespace Topology {

namespace EvaluatePathPriority {

enum EvaluatePathPriority {
    NotSet = 0,
    InboundDiscovery = 1,
    OutboundDiscovery = 2,
    InboundAttach = 3,
    OutboundAttach = 4,
    InboundService = 5,
    OutboundService = 6,
    InboundAffected = 7,
    OutboundAffected = 8
};

inline std::string toString(EvaluatePathPriority evaluatePathPriority) {

    switch (evaluatePathPriority) {
        case EvaluatePathPriority::NotSet:
            return "Evaluate NotSet";
        case EvaluatePathPriority::InboundDiscovery:
            return "Evaluate InboundDiscovery";
        case EvaluatePathPriority::OutboundDiscovery:
            return "Evaluate OutboundDiscovery";
        case EvaluatePathPriority::InboundAttach:
            return "Evaluate InboundAttach";
        case EvaluatePathPriority::OutboundAttach:
            return "Evaluate OutboundAttach";
        case EvaluatePathPriority::InboundService:
            return "Evaluate InboundService";
        case EvaluatePathPriority::OutboundService:
            return "Evaluate OutboundService";
        case EvaluatePathPriority::InboundAffected:
            return "Evaluate InboundAffected";
        case EvaluatePathPriority::OutboundAffected:
            return "Evaluate OutboundAffected";
        default:
            return "Unknown";
    }
}
}

namespace RoutingTypes {
/**
 * The routing type.
 */
enum RoutingTypes_Enum {
    SOURCE_ROUTING = 0, GRAPH_ROUTING = 1, BROADCAST_ROUTING = 2, GRAPH_ROUTING_WITH_MIN_PATH_SELECTION = 3
};

inline std::string toString(RoutingTypes_Enum routeType) {

    switch (routeType) {
        case RoutingTypes::SOURCE_ROUTING:
            return "srcRtng";
        case RoutingTypes::GRAPH_ROUTING:
            return "grphRtng";
        case RoutingTypes::BROADCAST_ROUTING:
            return "brdctRtng";
        default:
            return "Unknown";
    }
}
}

namespace NodeType {
enum NodeTypeEnum {
    NOT_SET = 0, MANAGER = 1, GATEWAY = 2, BACKBONE = 3, NODE = 4
};
}

//the length is 21
static const Uint16 distribution[] = {0, 1, 2, 3, 4, 5, 6, 7, 8, 9, 10, 11, 12, 13, 14, 15, 64, 0x80, 0x00FF, 0x0FFF, 0xFFFF};
static const Uint16 distributionForBackbone[] = {0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0x00FF, 0xFFFF};


}

}

}

#endif /* ROUTINGGRAPHTYPES_H_ */
