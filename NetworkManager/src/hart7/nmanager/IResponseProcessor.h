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

/*
 * IResponseProcessor.h
 *
 *  Created on: May 25, 2009
 *      Author: andrei.petrut
 */

#ifndef IRESPONSEPROCESSOR_H_
#define IRESPONSEPROCESSOR_H_

#include <boost/shared_ptr.hpp>
#include <WHartStack/WHartStack.h>

namespace hart7 {
namespace nmanager {

using namespace hart7::stack;

/**
 * Interface for response processing.
 */
class IResponseProcessor
{
    public:

        typedef boost::weak_ptr<IResponseProcessor> WeakPtr;

        virtual ~IResponseProcessor()
        {
        }

        /**
         * Process a confirm from the stack.
         */
        virtual void ProcessConfirm(WHartHandle requestHandle, const WHartLocalStatus& localStatus,
                                    const WHartCommandList& list) = 0;

        /**
         * Process an indicate from the stack.
         */
        virtual void ProcessIndicate(WHartHandle handle, const WHartAddress& src, WHartPriority priority,
                                     WHartTransportType transportType, const WHartCommandList& list) = 0;
};

}
}

#endif /* IRESPONSEPROCESSOR_H_ */
