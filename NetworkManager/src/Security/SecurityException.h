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

#ifndef SECURITYEXCEPTION_H_
#define SECURITYEXCEPTION_H_

#include "Common/NEException.h"
#include "Common/NETypes.h"

using namespace NE::Common;

namespace hart7 {

namespace security {

/**
 * This exception is thrown when the processing of request/confirmation APDUs fails.
 * @author Ioan-Vasile Pocol
 * @version 1.0
 */
class SecurityException: public NEException
{

    public:

        SecurityException(const char* message) :
            NEException(message)
        {
            cause = 0;
        }

        SecurityException(const stdString& message) :
            NEException(message)
        {
            cause = 0;
        }

        SecurityException(Uint8 cause, const char* message) :
            NEException(message)
        {
            this->cause = cause;
        }

        SecurityException(Uint8 cause, const stdString& message) :
            NEException(message)
        {
            this->cause = cause;
        }

        Uint8 Cause()
        {
            return cause;
        }

    private:

        Uint8 cause;

};

}
}

#endif /*SECURITYEXCEPTION_H_*/
