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

#ifndef ISA100TIME_H_
#define ISA100TIME_H_

#include "Common/NETypes.h"

namespace NE {
namespace Common {

/**
 * Time difference is indicated in milliseconds. An optional day count is also
 * available for situations where millisecond differences alone do not suffice.
 */
class TimeDifference {

    private:

        /**
         * The MSB is 1 if the <code>local_days</code> is also used.
         * The second
         */
        Uint32 local_milliseconds;

        /**
         * The number of days. This one is used if the MSB of the <code>local_milliseconds</code>
         * is set to 1.
         */
        Uint16 local_days;

    public:

        /**
         * Constructs an instance of TimeDifference only with the milliseconds
         * field. The number of days is not set.
         * The most significant bit is set to 0.
         */
        TimeDifference(Uint32 milliseconds_) {
            // the most significant bit must be 0;
            local_milliseconds = milliseconds_ & 0x7FFFFFFF;
        }

        /**
         * Constructs an instance of TimeDifference with the milliseconds field
         * and days field.
         * The most significant bit is set to 1.
         */
        TimeDifference(Uint32 milliseconds_, Uint16 days) {
            // the most significant bit must be 1;
            local_milliseconds = milliseconds_ | 0x80000000;
            local_days = days;
        }

        /**
         * Returns the number of milliseconds, stripping off the most
         * significant 4 bits.
         */
        Uint32 miliseconds() const {
            return local_milliseconds & 0x0FFFFFFFF;
        }

        /**
         * Returns the number of milliseconds, WITHOUT stripping off any of the bits.
         * TODO: find a better name for this method
         */
        Uint32 millisecondsMember() const {
            return local_milliseconds;
        }

        /**
         * Returns the number of days.
         */
        Uint16 days() const {
            return local_days;
        }

        /**
         * Returns <code>true</code> if it contains the number of days also.
         */
        bool isUsingDays() const {
            return local_milliseconds & 0x80000000;
        }

};

} // namespace Common

} // namespace Isa100


#endif /*ISA100TIME_H_*/
