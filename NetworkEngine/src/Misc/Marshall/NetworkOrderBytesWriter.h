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

#ifndef NETWORKORDERBYTESWRITER_H_
#define NETWORKORDERBYTESWRITER_H_

#include "Common/NETypes.h"

using namespace NE::Common;

namespace NE {
namespace Misc {
namespace Marshall {

class NetworkOrderBytesWriter {

    public:

        NetworkOrderBytesWriter();
        virtual ~NetworkOrderBytesWriter();

        static void write(BytesPointer destination, Uint8 value);
        static void write(BytesPointer destination, Uint16 value);
        static void write(BytesPointer destination, Uint32 value);
        static void write(BytesPointer destination, const Uint8* value, Uint16 count);
        static void write(BytesPointer destination, const BytesPointer value);
        static void write(BytesPointer destination, const Bytes& value);
        static void write(BytesPointer destination, const Bytes& value, int start, int count);
        static void write(BytesPointer destination, const BytesPointer value, int start, int count);
};
}
}
}
#endif /*NETWORKORDERBYTESWRITER_H_*/
