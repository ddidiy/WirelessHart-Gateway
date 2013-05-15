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

#ifndef CONVERT_H_
#define CONVERT_H_

#include <iomanip>
#include "Common/NETypes.h"
#include "boost/algorithm/string/split.hpp"
#include "boost/algorithm/string/classification.hpp"

using namespace boost;
using namespace std;
using namespace NE::Common;

namespace NE {
namespace Misc {
namespace Convert {

inline std::string bytes2string(const std::basic_string<unsigned char>& bytes) {
    std::ostringstream stream;
    for (std::basic_string<unsigned char>::size_type i = 0; i < bytes.size(); i++) {
        stream << std::uppercase << std::hex << std::setw(2) << std::setfill('0') << (int) bytes[i] << std::dec;
        if (i < bytes.size() - 1)
            stream << " ";
    }
    return stream.str();
}

/**
 * Transform only the first <code>length</code> bytes into a string.
 */
inline std::string bytes2string(const std::basic_string<unsigned char>& bytes, size_t length) {
    std::ostringstream stream;
    size_t noBytes = std::min(bytes.size(), length);

    for (std::basic_string<unsigned char>::size_type i = 0; i < noBytes; i++) {
        stream << std::uppercase << std::hex << std::setw(2) << std::setfill('0') << (int) bytes[i] << std::dec;
        if (i < bytes.size() - 1) {
            stream << " ";
        }
    }

    if (length < bytes.size()) {
        stream << "...";
    }
    return stream.str();
}

typedef vector<string> split_vector_type;
inline Bytes string2bytes(std::string text) {
    split_vector_type splitVec;
    split(splitVec, text, is_any_of(" "));

    Bytes bytes;
    for (split_vector_type::const_iterator it = splitVec.begin(); it != splitVec.end(); it++) {
        int b;
        std::istringstream(*it) >> std::hex >> b;
        bytes.push_back((Uint8) b);
    }
    return bytes;
}

inline std::string array2string(const Uint8* value, Uint16 length) {
    std::ostringstream stream;
    for (Uint16 i = 0; i < length; i++) {
        stream << std::uppercase << std::hex << std::setw(2) << std::setfill('0') << (int) value[i] << std::dec;
        if (i < length - 1)
            stream << " ";
    }
    return stream.str();
}

inline
bool compare(BytesPointer sir1, BytesPointer sir2, int length) {
    if (sir1 == NULL) {
        return false;
    }
    for (int i = 0; i < length; i++) {
        if (sir1->at(i) != sir2->at(i)) {
            return false;
        }
    }

    return true;
}

inline
bool compareBytes(Bytes& sir1, Bytes& sir2) {
    if (sir1.size() != sir2.size()) {
        return false;
    }
    for (Bytes::size_type i = 0; i < sir1.size(); i++) {
        if (sir1.at(i) != sir2.at(i)) {
            return false;
        }
    }

    return true;
}

} //namespace Convert
} //namespace Misc
} //namespace Isa100

#endif /*CONVERT_H_*/
