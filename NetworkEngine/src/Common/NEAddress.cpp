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

#include "NEAddress.h"
#include "Common/NEException.h"
#include <iomanip>


namespace NE {
namespace Common {


/*
 * 64 bits address used as EUI-64 address.
 */
Address64::Address64() {
    for (int i = 0; i < LENGTH; i++) {
        value[i] = 0;
    }
    address64String = " 00-00-00-00-00-00-00-00";
}

void Address64::marshall(OutputStream& stream) {
    for(Uint8 i = 0; i < LENGTH; i++) {
        stream.write(value[i]);
    }
}

void Address64::unmarshall(InputStream& stream) {
    for(Uint8 i = 0; i < LENGTH; i++) {
        stream.read(value[i]);
    }
    createAddressString ();
}

void Address64::loadBinary(Uint8 tmpValue[LENGTH]) {
    for (int i = 0; i < LENGTH; i++) {
        value[i] = tmpValue[i];
    }
    createAddressString ();
}

void Address64::loadString(const std::string& tmpValue) {
    if (tmpValue.size() != STRING_LENGTH) {
        std::ostringstream msg;
        msg << "Address64: invalid address on loadString(";
        msg << tmpValue;
        msg << ")";
        throw InvalidArgumentException(msg.str());
    }
    Uint8 offset;
    Uint8 tmp = 0;
    Uint8 count = 0;

    for (int i = 0; i < STRING_LENGTH; i++) {
        offset = 0;

        if ((tmpValue[i] == '-') || (tmpValue[i] == ':') || (i == STRING_LENGTH - 1)) {

            if (i == STRING_LENGTH - 1){
                i++;
            }

            if ( (i == 0) || ( (i+1) % 3 != 0 ) ) {
                std::ostringstream msg;
                msg << "Address64: invalid address on loadString(";
                msg << tmpValue;
                msg << ")";
                throw InvalidArgumentException(msg.str());
            }

            if (tmpValue[i-2] >= 'A'&& tmpValue[i-2] <= 'F') {
                offset = 'A' - 10;
            } else if (tmpValue[i-2] >= 'a'&& tmpValue[i-2] <= 'f') {
                offset = 'a' - 10;
            } else if (tmpValue[i-2] >= '0'&& tmpValue[i-2] <= '9') {
                offset = '0';
            }
            value[count] = (tmpValue[i-2] - offset)*16;

            if (tmpValue[i-1] >= 'A'&& tmpValue[i-1] <= 'F') {
                offset = 'A' - 10;
            } else if (tmpValue[i-1] >= 'a'&& tmpValue[i-1] <= 'f') {
                offset = 'a' - 10;
            } else if (tmpValue[i-1] >= '0'&& tmpValue[i-1] <= '9') {
                offset = '0';
            }

            value[count] += tmpValue[i-1] - offset;
            count++;
            continue;
        }


    }
    createAddressString ();
}

void Address64::loadShortString(const std::string& tmpValue) {
    if (tmpValue.size() != SHORT_STRING_LENGTH) {
        std::ostringstream msg;
        msg << "Address64: invalid address on loadString(";
        msg << tmpValue;
        msg << ")";
        throw InvalidArgumentException(msg.str());
    }

    Uint8 offset;
    Uint8 tmp = 0;

    for (int i = 0; i < SHORT_STRING_LENGTH; i++) {
        offset = 0;

        if (tmpValue[i] >= 'A'&& tmpValue[i] <= 'F') {
            offset = 'A' - 10;
        } else if (tmpValue[i] >= 'a'&& tmpValue[i] <= 'f') {
            offset = 'a' - 10;
        } else if (tmpValue[i] >= '0'&& tmpValue[i] <= '9') {
            offset = '0';
        }

        if (i % 2 == 1) {
            value[i/2] = tmp*16 + tmpValue[i] - offset;
        } else {
            tmp = tmpValue[i] - offset;
        }
    }
    createAddressString ();
}

Address64 Address64::createFromString(const std::string& tmpValue) {
    Address64 address;
    address.loadString(tmpValue);
    return address;
}

Address64 Address64::createFromShortString(const std::string& tmpValue) {
    Address64 address;
    address.loadShortString(tmpValue);
    return address;
}

// FFFF:FFFF:FFDD:XXXX doesn't match the address FFFF:FFFF:FFFF:FFFF
bool Address64::match(const std::string& format) const {
    bool isMatched = true;
    try {
       if (format.size() != STRING_LENGTH) {
           isMatched = false;
       } else {

           std::string addressAsString = toString();
           for (int i = 0; i < STRING_LENGTH; i++) {

                if (format[i] == '-') {
                    continue;
                } else {
                    if ( (format[i] >= 'A' && format[i] <= 'F')
                            || (format[i] >= 'a' && format[i] <= 'f')
                            || (format[i] >= '0' && format[i] <= '9')) {
                        if (addressAsString[i] != format[i]) {
                            //LOG_DEBUG("NO MATCH! position=" << i << ", " << addressAsString << ", " << format);
                            isMatched = false;
                            break;
                        }
                    }
                }
           }
       }
    } catch(NEException& ex) {
        isMatched = false;
    }
    return isMatched;
}

bool Address64::operator<(const Address64 &compare) const {
    for (int i = 0; i < LENGTH; i++) {
        if (value[i] != compare.value[i]) {
            return value[i] < compare.value[i];
        }
    }

    return false;
}

bool Address64::operator==(const Address64 &address) const {
    Uint8 i = 0;
    while (i < LENGTH && value[i] == address.value[i]) {
        i++;
    }
    return (i >= LENGTH);
}

void Address64::createAddressString ()
{
    std::ostringstream stream;
    for (Uint8 i = 0; i < LENGTH; i++) {
        stream << std::uppercase << std::hex << std::setw(2) << std::setfill('0') << (int)value[i] << std::dec;
        if (i != LENGTH - 1) {
            stream << ':';
        }
    }
    address64String = stream.str();
}
const std::string& Address64::toString() const {
    return address64String;
}

}
}

