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

#ifndef Isa100TYPES_H
#define Isa100TYPES_H

#include <string>
#include <ctime>
#include <exception>
#include <sstream>
#include <string.h>
#include <ios>
#include <boost/shared_ptr.hpp>
#include <queue>
#include <list>
#include <set>

namespace NetworkEngineEventType {
/**
 * Enumeration with possible event types.
 */
enum NetworkEngineEventTypeEnum {
    NONE = 0, //
    JOIN_REQUEST = 1,
    MAKE_ROUTER = 2,
    REMOVE_DEVICES = 3,
    REMOVE_DEVICES_UDP = 4,
    TERMINATE_SERVICE = 5,
    EVALUATE_NEXT_ROUTE = 6
};

inline std::string getNetworkEngineEventTypeDescription(NetworkEngineEventTypeEnum event) {

    if (NetworkEngineEventType::NONE == event) {
        return "NONE";
    } else if (NetworkEngineEventType::JOIN_REQUEST == event) {
        return "JOIN_REQUEST";
    } else if (NetworkEngineEventType::MAKE_ROUTER == event) {
        return "MAKE_ROUTER";
    } else if (NetworkEngineEventType::REMOVE_DEVICES == event) {
        return "REMOVE_DEVICES";
    } else if (NetworkEngineEventType::REMOVE_DEVICES_UDP == event) {
        return "REMOVE_DEVICES_UDP";
    } else if (NetworkEngineEventType::TERMINATE_SERVICE == event) {
        return "TERMINATE_SERVICE";
    } else if (NetworkEngineEventType::EVALUATE_NEXT_ROUTE == event) {
        return "EVALUATE_NEXT_ROUTE";
    }

    return "UNKNOWN";
}
}

namespace Status {
/**
 * Enumeration with possible service statuses.
 */
enum StatusEnum {
    NEW = 1, //
    CHANGED = 2,
    PENDING = 3, // pending ... => ACTIVE or DELETED
    ACTIVE = 4, // after commit =>  (NEW, RECOVERED => ACTIVE; REMOVED => DELETED; NEW => PENDING)
    REMOVED = 5, // used before commit
    RECOVERED = 6, // recovered entity
    CANDIDATE = 7,
    NOT_PRESENT = 8, // as default saved entity status (first save status)
    DELETED = 9,
    CANDIDATE_FOR_REMOVAL = 10
// used after commit - in case of roll back for undo
};

inline std::string getStatusDescription(StatusEnum status) {

    if (Status::NEW == status) {
        return "NEW";
    } else if (Status::CHANGED == status) {
        return "CHANGED";
    } else if (Status::PENDING == status) {
        return "PENDING";
    } else if (Status::ACTIVE == status) {
        return "ACTIVE";
    } else if (Status::REMOVED == status) {
        return "REMOVED";
    } else if (Status::RECOVERED == status) {
        return "RECOVERED";
    } else if (Status::CANDIDATE == status) {
        return "CANDIDATE";
    } else if (Status::NOT_PRESENT == status) {
        return "NOT_PRESENT";
    } else if (Status::DELETED == status) {
        return "DELETED";
    }

    return "UNKNOWN";
}

}

/*
 * The address32 is composed from subnetId and nickname
 * The first bit in nickname is the alias flag
 */
typedef unsigned int Address32;
typedef std::list<Address32> AddressList;
typedef std::set<Address32> AddressSet;
typedef std::queue<Address32> AddressQueue;
typedef std::set<unsigned short> GraphIdSet;

#define NODE_PROBABILITY 100

#define BROADCAST_ADDRESS 0xFFFF
#define MANAGER_ADDRESS 0xF980
#define GATEWAY_ADDRESS 0xF981

#define MANAGEMENT_ROUTE 0x0
#define MANAGEMENT_SERVICE 0x80

#define INBOUND_AP_GRAPH_ID 0x0
#define INBOUND_GW_GRAPH_ID 0x102 //for test only is set to 0; the right value is 0x102
#define OUTBOUND_GW_GRAPH_ID 0x103

#define ETHERNET_TRAFFIC 5000

#define MAX_15BITS_VALUE 0x7FFF

typedef unsigned char Byte;
typedef std::basic_string<unsigned char> Bytes;
typedef boost::shared_ptr<Bytes> BytesPointer;

typedef unsigned char Uint8;
typedef unsigned short Uint16;
typedef unsigned int Uint32;
typedef unsigned long long Uint64;

typedef signed char Int8;
typedef signed short Int16;
typedef signed int Int32;

typedef double Double;
typedef float Float;
typedef std::string VisibleString;

typedef std::_Ios_Openmode std__ios_flags;

typedef std::basic_istringstream<Byte> stdIstringstreamBytes;
typedef boost::shared_ptr<stdIstringstreamBytes> stdIstringstreamBytesPointer;

typedef std::ios_base::openmode stdOpenmode;

typedef std::basic_ostringstream<char> stdOstringstream;
typedef std::basic_ostringstream<Byte> stdOstringstreamBytes;
typedef boost::shared_ptr<stdOstringstreamBytes> stdOstringstreamBytesPointer;

typedef std::streampos stdStreampos;
typedef std::streamsize stdStreamsize;

typedef std::exception stdException;
typedef std::string stdString;

typedef time_t DateTime;
typedef unsigned int ServiceId;

namespace Time {

inline void toString(time_t time, std::string &timeString) {
    char *str = ctime(&time);
    str[strlen(str) - 1] = 0;
    timeString = std::string(str);
}
}

namespace Type {

inline void toString(Uint32 value, std::string &addressString) {
    std::ostringstream stream;
    stream << (long long) value;
    addressString = stream.str();
}

inline void toHexString(Uint32 value, std::string &addressString) {
    std::ostringstream stream;
    stream << std::hex << (long long) value << std::dec;
    addressString = stream.str();
}
}

inline std::string boolToString(bool value) {
    return (value == 0) ? "false" : "true";
}

#include "NETime.h"

#endif
