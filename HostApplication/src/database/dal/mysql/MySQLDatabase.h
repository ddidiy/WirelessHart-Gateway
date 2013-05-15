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
 * MySQLDatabase.h
 *
 *  Created on: Nov 25, 2008
 *      Author: nicu.dascalu
 */

#ifndef MYSQLDATABASE_H_
#define MYSQLDATABASE_H_

#include <nlib/dbxx/Connection.h>
#include <nlib/dbxx/Command.h>

#include <nlib/dbxx/mysql/MySQLDriver.h>


namespace hart7 {
namespace hostapp {

typedef nlib::dbxx::Connection<nlib::dbxx::mysql::ConnectionDriver> MySQLConnection;
typedef nlib::dbxx::Transaction<nlib::dbxx::mysql::ConnectionDriver> MySQLTransaction;
typedef nlib::dbxx::Command<nlib::dbxx::mysql::CommandDriver> MySQLCommand;
typedef nlib::dbxx::ResultSet<nlib::dbxx::mysql::ResultSetDriver> MySQLResultSet;

} //namespace hostapp
} //namespace hart7


#endif /* MYSQLDATABASE_H_ */
