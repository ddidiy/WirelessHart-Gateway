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
 * $Id: SqldalUnmarshaller.h,v 1.11.32.1 2013/05/15 19:19:18 owlman Exp $
 */


#ifndef _SQL_DAL_UNMARSHALLER_H_
#define _SQL_DAL_UNMARSHALLER_H_

#include "methods/methods.h"
#include "SqldalImpl.h"
//////////////////////////////////////////////////////////////////////////////
/// @class CSqldalUnmarshaller
/// @ingroup LibUnmarshaller
//////////////////////////////////////////////////////////////////////////////
class CSqldalUnmarshaller : public MethodUnmarshaller
{
public:
	static MethodUnmarshaller* GetInstance()
	{
		if ( !m_pInstance )
			m_pInstance = new CSqldalUnmarshaller();
		return m_pInstance ;
	}
public:
	int open( RPCCommand& cmd ) ;
	int lastInsertedRowId( RPCCommand& cmd ) ;
	int rowsAffected( RPCCommand& cmd ) ;
	int execute( RPCCommand& cmd ) ;
	int getCsv( RPCCommand& cmd ) ;
	int close( RPCCommand& cmd ) ;
private:
	static CSqldalUnmarshaller* m_pInstance ;
	CSqldalImpl m_oSqlDal ;
} ;

#endif	/* _SQL_DAL_UNMARSHALLER_H_ */
