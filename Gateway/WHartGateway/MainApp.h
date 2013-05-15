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
 * MainApp.h
 *
 *  Created on: Nov 13, 2008
 *      Author: ovidiu.rauca
 */

#ifndef WHART_GATEWAY_MAINAPP_H_
#define WHART_GATEWAY_MAINAPP_H_

#include <nlib/log.h>


namespace hart7 {
namespace gateway {

/**
 * Main GW application loop.
 */
class MainApp
{
	LOG_DEF("hart7.gateway.MainApp");

public:
	MainApp();
	virtual ~MainApp();

	void Run(int argc, char* argv[]);

};

} // namespace gateway
} // namespace hart7


#endif /* WHART_GATEWAY_MAINAPP_H_ */
