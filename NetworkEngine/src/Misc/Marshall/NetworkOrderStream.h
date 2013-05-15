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

#ifndef NETWORKORDERSTREAM_H_
#define NETWORKORDERSTREAM_H_

#include "Common/NETypes.h"
#include "Common/logging.h"
#include "Misc/Marshall/Stream.h"

using namespace NE::Common;

namespace NE {
namespace Misc {
namespace Marshall {

class NetworkOrderStream: public InputStream, public OutputStream {
    LOG_DEF("I.M.M.NetworkOrderStream")

    private:

    const BytesPointer input;

    public:

        stdIstringstreamBytes istream;
        stdOstringstreamBytes ostream;

    public:

        NetworkOrderStream() {
        }

        NetworkOrderStream(const Bytes& input_) :
            input(new Bytes(input_)), istream(input_) {
        }

        NetworkOrderStream(const BytesPointer& input_) :
            input(input_), istream(*input_) {
        }

        virtual ~NetworkOrderStream() {
        }

        virtual void read(bool& value);
        virtual void read(Int8& value);
        virtual void read(Int16& value);
        virtual void read(Int32& value);
        virtual void read(Uint8& value);
        virtual void read(Uint16& value);
        virtual void read(Uint32& value);
        virtual void read(Float& value);
        virtual void read(Double& value);
        virtual void read(DateTime& value);

        /**
         * Reads a TimeDifference from the stream.
         * First reads the number of milliseconds member from the stream
         * and checks whether the the number of days is used; if yes
         * then the number of days is also read from the stream.
         */
        virtual void read(TimeDifference& value);

        /**
         * Read count bytes from stream and place them in value. The previous content
         * of value will be cleared.
         */
        virtual void read(Bytes& value, stdStreamsize count);

        virtual stdStreamsize remainingBytes() const;
        virtual void peek(stdStreampos position, Uint8& value);

        virtual void write(const bool& value);
        virtual void write(const Int8& value);
        virtual void write(const Int16& value);
        virtual void write(const Int32& value);
        virtual void write(const Uint8& value);
        virtual void write(const Uint16& value);
        virtual void write(const Uint32& value);
        virtual void write(const Float& value);
        virtual void write(const Double& value);
        virtual void write(const DateTime& value);

        /**
         * Writes an instance of TimeDifference object.
         * If the number of days from the object is used then
         * the number of milliseconds and the number of days are written to
         * the stream in this order.If the number of days it is not used, then
         * only the number of milliseconds is written on the stream.
         */
        virtual void write(const TimeDifference& value);
        virtual void write(const Bytes& value);

    private:
        void readBytes(Byte* buff, stdStreamsize count);
        /**
         * Checks to see if the stream contains at least <code>count</code>
         * bytes to read. If there are not enough bytes throws an exception.
         */
        bool existEnoughBytes(stdStreamsize count) const;

};

} // namespace Marshall
} // namespace Misc
} // namespace Isa100

#endif /*NETWORKORDERSTREAM_H_*/
