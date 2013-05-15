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

#ifndef ABSTRACTAPPCOMMAND_H_
#define ABSTRACTAPPCOMMAND_H_

#include <boost/shared_ptr.hpp>
#include <boost/cstdint.hpp> //used for inttypes

#include <iostream>

#include <WHartHost/model/DBCommand.h>


namespace hart7 {
namespace hostapp {


class AbstractAppCommand;
typedef boost::shared_ptr<AbstractAppCommand> AbstractAppCommandPtr;

class IAppCommandVisitor;

//inner data
class DBCommandsManager;
class DevicesManager;


/**
 * @brief The Visited class.
 */
class AbstractAppCommand
{

public:
	DBCommand  dbCommand;
	DBCommandsManager *pCommands;
	DevicesManager *pDevices;

public:
	AbstractAppCommand();
	virtual ~AbstractAppCommand();

	virtual bool Accept(IAppCommandVisitor& visitor) = 0;


protected:
	virtual void DumpToStream(std::ostream& p_rStream) const = 0;
	friend std::ostream& operator<< (std::ostream& p_rStream, const AbstractAppCommand& p_rCmd);
};


extern std::ostream& operator<< (std::ostream& p_rStream, const AbstractAppCommand& p_rCmd);


}
}

#endif /*ABSTRACTAPPCOMMAND_H_*/
