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

#ifndef Isa100EXCEPTION_H
#define Isa100EXCEPTION_H

#include <exception>
#include <string>
#include <stdio.h>
#include <stdarg.h>

namespace NE {
namespace Common {

/**
 * Define for throwing an exception with a message created by concatenation. This define logs the exception then throw.
 * Example of usage:
 * THROW_EX(Isa100Exception, "An exception from object=" << objectId << " in case=" << caseType);
 */
#define THROW_EX(ExceptionClass, msg)\
    std::ostringstream stream;\
    stream << msg;\
    LOG_ERROR(stream.str());\
    throw ExceptionClass(stream.str())

#define THROW_EX2(ExceptionClass, msg)\
    std::ostringstream stream;\
    stream << msg;\
    throw ExceptionClass(stream.str())

/**
 * Base exception class of all execeptions thrown by code.
 */
class NEException: public std::exception {

    public:

        NEException(const std::string& message) :
            msg(message) {
        }

        NEException(const char* fmt, ...) :
            msg(fmt) {
            const int buffSize = 1024 * 1024;
            char buffer[buffSize];
            va_list args;
            va_start( args, fmt );
            vsnprintf(buffer, buffSize, fmt, args);
            va_end( args );

            msg = buffer;
        }

        virtual ~NEException() throw() {
        }

    protected:

        std::string msg;

    public:

        virtual const char* what() const throw() {
            return msg.c_str();
        }

        virtual std::string getMsg() {
            return msg;
        }
}; //end class Isa100Exception

/**
 * Thrown in methods when a invalid argument(parameter) is passed to method call.
 */
class InvalidArgumentException: public NEException {

    public:

        InvalidArgumentException(const std::string& message) :
            NEException(message) {
        }

        InvalidArgumentException(const char* message) :
            NEException(message) {
        }
};

} //end namespace Common
} // end namespace Isa100

#endif
