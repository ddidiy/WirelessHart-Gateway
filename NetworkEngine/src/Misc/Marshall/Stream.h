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

#ifndef STREAM_H_
#define STREAM_H_

#include <sstream>
#include "Common/NEException.h"
#include "Common/NETypes.h"

using namespace NE::Common;

namespace NE {
namespace Misc {
namespace Marshall {

/**
 * TODO: document uses of this exception
 */
class StreamException: public NEException {

    public:

        StreamException(const char* message) :
            NEException(message) {
        }

        StreamException(const stdString& message) :
            NEException(message) {
        }

        virtual ~StreamException() throw() {
        }
};

/**
 * Abstract an input stream.
 * @author Beniamin Tecar
 * @version 1.0
 */
class InputStream {

    public:

        virtual ~InputStream() {
        }

        virtual void read(bool& value) = 0;

        /**
         * Reads a boolean value, and advance the stream position.
         * @throw StreamException if not enough bytes available.
         */
        virtual void read(Int8& value) = 0;

        /**
         * Reads a short value, and advance the stream position.
         * @throw StreamException if not enough bytes available.
         */
        virtual void read(Int16& value) = 0;

        /**
         * @throw StreamException if not enough bytes available.
         * Reads an int value, and advance the stream position.
         */
        virtual void read(Int32& value) = 0;

        virtual void read(Uint8& value) = 0;
        virtual void read(Uint16& value) = 0;
        virtual void read(Uint32& value) = 0;
        virtual void read(Float& value) = 0;
        virtual void read(Double& value) = 0;

        /**
         * @throw StreamException if not enogh bytes available.
         * Reads max count byte to value, and advance the stream position.
         */
        virtual void read(Bytes& value, stdStreamsize count) = 0;

        /**
         * Read a byte from specified position, the stream position is not incemented.
         * @throw StreamException if the posion is out of range.
         */
        virtual void peek(stdStreampos position, Uint8& value) = 0;

        /**
         * Tells how many bytes are available in the stream.
         */
        virtual stdStreamsize remainingBytes() const = 0;
};

class OutputStream {

    public:

        virtual ~OutputStream() {
        }

        virtual void write(const bool& value) = 0;
        virtual void write(const Int8& value) = 0;
        virtual void write(const Int16& value) = 0;
        virtual void write(const Int32& value) = 0;
        virtual void write(const Uint8& value) = 0;
        virtual void write(const Uint16& value) = 0;
        virtual void write(const Uint32& value) = 0;
        virtual void write(const Float& value) = 0;
        virtual void write(const Double& value) = 0;
        virtual void write(const Bytes& value) = 0;
};
} // namespace Marshall
} //namespace Misc
} //namespace Isa100

#endif /*STREAM_H_*/
