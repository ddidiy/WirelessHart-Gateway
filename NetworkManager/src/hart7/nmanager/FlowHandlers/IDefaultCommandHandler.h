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
 * IDefaultCommandHandler.h
 *
 *  Created on: Sep 16, 2009
 *      Author: andrei.petrut
 */

#ifndef IDEFAULTCOMMANDHANDLER_H_
#define IDEFAULTCOMMANDHANDLER_H_

#include <WHartStack/WHartStack.h>
#include <boost/shared_array.hpp>
#include <boost/function.hpp>

namespace hart7 {
namespace nmanager {

/**
 * Holds a WHartCommand, and also allocates memory for the internal command.
 */
struct WHartCommandWrapper
{
        stack::WHartCommand command;
        boost::shared_array<uint8_t> commandBuffer;

        WHartCommandWrapper()
        {
            commandBuffer.reset(new uint8_t[300]);
        }
};

/**
 * Will be called for each request before passing the request to the command processor.
 * Should send response.
 */
class IDefaultCommandHandler
{
    public:

        typedef boost::shared_ptr<IDefaultCommandHandler> Ptr;

        virtual ~IDefaultCommandHandler()
        {
        }

        // Handle request
        virtual bool HandleCommand(const stack::WHartAddress& src, const stack::WHartCommand& request,
                                   WHartCommandWrapper& response) = 0;

        // Will be checked for after all handlers are called. Returning true will erase the handler.
        virtual bool CommandHandlerFinished() = 0;
};

typedef boost::function1<void, IDefaultCommandHandler::Ptr> RegisterDefaultHandlerCallback;

}
}

#endif /* IDEFAULTCOMMANDHANDLER_H_ */
