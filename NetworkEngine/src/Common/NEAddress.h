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

#ifndef ISA100ADDRESS_H_
#define ISA100ADDRESS_H_

#include "Common/NETypes.h"
#include "Misc/Marshall/Stream.h"
#include <iomanip>

using namespace NE::Misc::Marshall;

namespace NE {
namespace Common {

class ToStr {

    private:

        Address32 address32;

        Uint16 width;

    public:

        ToStr(Address32 address32_) {
            address32 = address32_;
            width = 4;
        }

        ToStr(Address32 address32_, Uint16 width_) {
            address32 = address32_;
            width = width_;
        }

        /**
         * Print the address on 4 chars.
         */
        friend std::ostream& operator<<(std::ostream& output, const ToStr& toStr) {
            output << std::uppercase << std::setw(toStr.width) << std::hex
                        << (long long) toStr.address32 << std::setfill(' ') << std::dec;
            return output;
        }
};

/**
 * 64 bits address used as EUI-64 address.
 */
struct Address64 {

    public:

        /** the length of the address in octets */
        static const Uint8 LENGTH = 8;

        /** the length of the address represented as string */
        static const Uint8 STRING_LENGTH = 64 / 4 + 64 / 8 - 1;

        /** the length of the address represented as string without ':' separators */
        static const Uint8 SHORT_STRING_LENGTH = 64 / 4;

        /** the value of the address */
        Uint8 value[8];

        std::string address64String;

        /**
         * Initializes the address to 0x0000:0x0000:0x0000:0x0000
         */
        Address64();

        /**
         *
         */
        void marshall(OutputStream& stream);

        /**
         *
         */
        void unmarshall(InputStream& stream);

        /**
         * Initializes the address from a binary format.
         * @param the binary value of the address
         */
        void loadBinary(Uint8 tmpValue[LENGTH]);

        /**
         * Load the address from format FFFF:FFFF:FFFF:FFFF.
         * @param tmpValue the string value of the address
         * @throws Isa100::Common::InvalidArgumentException if the length
         * of tmpValue is not Address64::STRING_LENGTH.
         */
        void loadString(const std::string& tmpValue);

        /**
         * Load the address from format FFFFFFFFFFFFFFFF.
         * @param tmpValue
         */
        void loadShortString(const std::string& tmpValue);

        static Address64 createFromString(const std::string& tmpValue);
        static Address64 createFromShortString(const std::string& tmpValue);

        /**
         * Check if the address match the input format.
         * Example: FFFF:FFFF:FFFF:XXXX match the address FFFF:FFFF:FFFF:FFFF
         *          FFFF:FFFF:FFDD:XXXX doesn't match the address FFFF:FFFF:FFFF:FFFF
         */
        bool match(const std::string& format) const;

        /**
         * Less operator required by map container.
         * @param compare the address compare with
         */
        bool operator<(const Address64 &compare) const;

        /**
         * Equal operator.
         * @param compare the address compare with
         */
        bool operator==(const Address64 &address) const;

        /**
         * Formats the value in format hexa
         */
        void createAddressString();

        /**
         * returns the hexa formated value
         */
        const std::string& toString() const;
};

}
}
#endif /*ISA100ADDRESS_H_*/

