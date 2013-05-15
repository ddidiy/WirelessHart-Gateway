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

#include "NetworkOrderStream.h"
#include "Common/NETypes.h"

#include <arpa/inet.h>

namespace NE {
namespace Misc {
namespace Marshall {

void NetworkOrderStream::read(bool& value) {
    if (existEnoughBytes(1)) {
        istream.read((Byte*) &value, sizeof(value));
    }
}

void NetworkOrderStream::read(Int8& value) {
    if (existEnoughBytes(sizeof(value))) {
        istream.read((Byte*) &value, sizeof(value));
    }
}

void NetworkOrderStream::read(Int16& value) {
    if (existEnoughBytes(sizeof(value))) {
        //        Int8 val;
        //        read(val);
        //        value = val;
        //        read(val);
        //        value = (value << 8) + val;
        Uint16 val;
        read(val);
        value = (Int16) val;
    }
}

void NetworkOrderStream::read(Int32& value) {
    if (existEnoughBytes(sizeof(value))) {
        //        Int8 val;
        //        read(val);
        //        value = val;
        //        read(val);
        //        value = (value << 8) + val;
        //        read(val);
        //        value = (value << 8) + val;
        //        read(val);
        //        value = (value << 8) + val;
        Uint32 valTemp;
        read(valTemp);
        value = (Int32) valTemp;
    }
}

void NetworkOrderStream::read(Uint8& value) {
    if (existEnoughBytes(sizeof(value))) {
        istream.read((Byte*) &value, sizeof(value));
    }
}

void NetworkOrderStream::read(Uint16& value) {
    if (existEnoughBytes(sizeof(value))) {
        Uint8 val;
        read(val);
        value = val;
        read(val);
        value = (value << 8) + val;
    }
}

void NetworkOrderStream::read(Uint32& value) {
    if (existEnoughBytes(sizeof(value))) {
        Uint8 val;
        read(val);
        value = val;
        read(val);
        value = (value << 8) + val;
        read(val);
        value = (value << 8) + val;
        read(val);
        value = (value << 8) + val;
    }
}

void NetworkOrderStream::read(Float& value) {
    if (existEnoughBytes(sizeof(value))) {
        Float networkValue;
        stdOstringstreamBytes ostrm;
        stdOstringstreamBytes ostrmReverse;

        istream.read((Byte*) &networkValue, sizeof(networkValue));

        ostrm.write((Byte*) &networkValue, sizeof(networkValue));

        for (size_t i = 0; i < ostrm.str().size(); i++) {
            ostrmReverse.write((Byte*) &ostrm.str()[ostrm.str().size() - i - 1], sizeof(Byte));
        }

        stdIstringstreamBytes istrm(ostrmReverse.str());
        istrm.read((Byte*) &value, sizeof(value));
    }
}

void NetworkOrderStream::read(Double& value) {
    if (existEnoughBytes(sizeof(value))) {
        Double networkValue;
        stdOstringstreamBytes ostrm;
        stdOstringstreamBytes ostrmReverse;

        istream.read((Byte*) &networkValue, sizeof(networkValue));

        ostrm.write((Byte*) &networkValue, sizeof(networkValue));

        for (size_t i = 0; i < ostrm.str().size(); i++) {
            ostrmReverse.write((Byte*) &ostrm.str()[ostrm.str().size() - i - 1], sizeof(Byte));
        }

        stdIstringstreamBytes istrm(ostrmReverse.str());
        istrm.read((Byte*) &value, sizeof(value));
    }
}

/**
 * Reads a TimeDifference from the stream.
 * First reads the number of milliseconds member from the stream
 * and checks to see if the the number the days is used, and if it is
 * used tben the number of days is also read from the stream.
 */
void NetworkOrderStream::read(TimeDifference& value) {
    Uint32 milliseconds;
    Uint16 noDays;

    if (existEnoughBytes(sizeof(milliseconds))) {
        read(milliseconds);
    }

    if (milliseconds & 0x80000000) {
        if (existEnoughBytes(sizeof(noDays))) {
            read(noDays);
        }
        value = TimeDifference(milliseconds, noDays);
    } else {
        value = TimeDifference(milliseconds);
    }
}

void NetworkOrderStream::read(Bytes& value, stdStreamsize count) {
    if (remainingBytes() < count) {
        LOG_ERROR("Not enough bytes to read!");
        throw StreamException("Not enough bytes to read!");
    }

    stdStreamsize i = 0;
    value.clear();
    while (i++ < count) {
        Byte b;
        istream.read(&b, 1);
        value.push_back(b);
    }
}

stdStreamsize NetworkOrderStream::remainingBytes() const {
    return istream.rdbuf()->in_avail();
}

void NetworkOrderStream::peek(stdStreampos position, Uint8& value) {
    if (position >= (stdStreampos) input->size()) {
        LOG_ERROR("Peeked byte is out of range!");
        throw StreamException("Peeked byte is out of range!");
    }

    value = *(input->data() + position);
}

void NetworkOrderStream::write(const bool& value) {
    ostream.write((Byte*) &value, sizeof(value));
}

void NetworkOrderStream::write(const Int8& value) {
    ostream.write((Byte*) &value, sizeof(value));
}

void NetworkOrderStream::write(const Int16& value) {
    write((Uint8) ((value & 0xFF00) >> 8));
    write((Uint8) (value & 0x00FF));
}

void NetworkOrderStream::write(const Int32& value) {
    write((Uint8) ((value & 0xFF000000) >> 24));
    write((Uint8) ((value & 0x00FF0000) >> 16));
    write((Uint8) ((value & 0x0000FF00) >> 8));
    write((Uint8) (value & 0x000000FF));
}

void NetworkOrderStream::write(const Uint8& value) {
    ostream.write((Byte*) &value, sizeof(value));
}

void NetworkOrderStream::write(const Uint16& value) {
    write((Uint8) ((value & 0xFF00) >> 8));
    write((Uint8) (value & 0x00FF));
}

void NetworkOrderStream::write(const Uint32& value) {
    write((Uint8) ((value & 0xFF000000) >> 24));
    write((Uint8) ((value & 0x00FF0000) >> 16));
    write((Uint8) ((value & 0x0000FF00) >> 8));
    write((Uint8) (value & 0x000000FF));
}

void NetworkOrderStream::write(const Float& value) {
    stdOstringstreamBytes ostrm;
    stdOstringstreamBytes ostrmReverse;

    ostrm.write((Byte*) &value, sizeof(value));

    for (size_t i = 0; i < ostrm.str().size(); i++) {
        ostrmReverse.write((Byte*) &ostrm.str()[ostrm.str().size() - i - 1], sizeof(Byte));
    }

    ostream << (ostrmReverse.str());
}

void NetworkOrderStream::write(const Double& value) {
    stdOstringstreamBytes ostrm;
    stdOstringstreamBytes ostrmReverse;

    ostrm.write((Byte*) &value, sizeof(value));

    for (size_t i = 0; i < ostrm.str().size(); i++) {
        ostrmReverse.write((Byte*) &ostrm.str()[ostrm.str().size() - i - 1], sizeof(Byte));
    }

    ostream << (ostrmReverse.str());
}

void NetworkOrderStream::write(const DateTime& value) {
    /* TODO: de implementat */
}

/**
 * Writes an instance of TimeDifference object.
 * If the number of days from the object is used then
 * the number of milliseconds and the number of days are written to
 * the stream in this order.If the number of days it is not used, then
 * only the number of milliseconds is written on the stream.
 */
void NetworkOrderStream::write(const TimeDifference& value) {
    write(value.millisecondsMember());
    if (value.isUsingDays()) {
        write(value.days());
    }
}

void NetworkOrderStream::write(const Bytes& value) {
    for (Bytes::size_type i = 0; i < value.size(); i++) {
        ostream.write((Byte*) &value[i], sizeof(value[i]));
    }
}

void NetworkOrderStream::readBytes(Byte* buff, stdStreamsize count) {
    if (remainingBytes() < count) {
        LOG_ERROR("Not enough bytes to read!");
        throw StreamException("Not enough bytes to read!");
    }

    istream.read(buff, count);
}

void NetworkOrderStream::read(DateTime& value) {
    /* TO DO - implement */
}

bool NetworkOrderStream::existEnoughBytes(stdStreamsize count) const {
    if (remainingBytes() < count) {
        std::ostringstream stream;
        stream << "Not enough bytes to read! required=" << (int) count << ", remaining=" << (int) remainingBytes();
        LOG_ERROR(stream.str());
        throw StreamException(stream.str());
    }

    return true;
}

}
}
}
