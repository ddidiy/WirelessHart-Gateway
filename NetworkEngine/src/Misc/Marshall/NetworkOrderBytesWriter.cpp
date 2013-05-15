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

#include "NetworkOrderBytesWriter.h"

using namespace NE::Common;

namespace NE {
namespace Misc {
namespace Marshall {

NetworkOrderBytesWriter::NetworkOrderBytesWriter() {
}

NetworkOrderBytesWriter::~NetworkOrderBytesWriter() {
}

void NetworkOrderBytesWriter::write(BytesPointer destination, Uint8 value) {
    destination->push_back(value);
}

void NetworkOrderBytesWriter::write(BytesPointer destination, Uint16 value) {
    destination->push_back((Uint8) ((value & 0xFF00) >> 8));
    destination->push_back((Uint8) (value & 0x00FF));
}

void NetworkOrderBytesWriter::write(BytesPointer destination, Uint32 value) {
    destination->push_back((Uint8) ((value & 0xFF000000) >> 24));
    destination->push_back((Uint8) ((value & 0x00FF0000) >> 16));
    destination->push_back((Uint8) ((value & 0x0000FF00) >> 8));
    destination->push_back((Uint8) (value & 0x000000FF));
}

void NetworkOrderBytesWriter::write(BytesPointer destination, const Uint8* value, Uint16 count) {
    for (int i = 0; i < count; i++) {
        destination->push_back(value[i]);
    }
}

void NetworkOrderBytesWriter::write(BytesPointer destination, const BytesPointer value) {
    NetworkOrderBytesWriter::write(destination, value, 0, value->size());
}

void NetworkOrderBytesWriter::write(BytesPointer destination, const Bytes& value) {
    for (Uint16 i = 0; i < value.size(); i++) {
        destination->push_back(value[i]);
    }
}

void NetworkOrderBytesWriter::write(BytesPointer destination, const Bytes& value, int start, int count) {
    for (int i = start; i < start + count; i++) {
        destination->push_back(value[i]);
    }
}

void NetworkOrderBytesWriter::write(BytesPointer destination, const BytesPointer value, int start, int count) {
    for (int i = start; i < start + count; i++) {
        destination->push_back(value->at(i));
    }
}
}
}
}
