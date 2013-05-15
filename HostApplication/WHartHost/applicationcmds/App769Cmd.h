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

#ifndef APP769CMD_H_
#define APP769CMD_H_


#include <WHartHost/applicationcmds/AbstractAppCommand.h>
#include <WHartHost/model/Device.h>


namespace hart7 {
namespace hostapp {

/*
 * Command Pattern
 */
class IAppCommandVisitor;
class App769Cmd;
typedef boost::shared_ptr<App769Cmd> App769CmdPtr;

class App769Cmd: public AbstractAppCommand
{
public:
	virtual bool Accept(IAppCommandVisitor& visitor);

protected:
	void DumpToStream(std::ostream& p_rStream) const;

public:
	App769Cmd(MAC& mac);

public:
	MAC GetDeviceMac();

private:
	MAC m_mac;

};

}
}

#endif
