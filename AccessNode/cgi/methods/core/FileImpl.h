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

#ifndef _FILE_IMPL_H_
#define _FILE_IMPL_H_

#include <sys/types.h>
#include <sys/stat.h>
#include <fcntl.h>
#include <glob.h>

#include "../../../Shared/h.h"

class CFileImpl {
public:
	CFileImpl() : m_fname(NULL) {}
	CFileImpl(const char *p_kszFName) : m_fname(NULL)
	{
		bind(p_kszFName);
	}
	CFileImpl& bind(const char *p_kszFName)
	{
		free( m_fname ) ;
		if ( ! p_kszFName )
		{
			m_fname = strdup("");
			return *this ;
		}
		m_fname = strdup(p_kszFName);
		return *this ;
	}
	~CFileImpl()
	{
		free( m_fname ) ;
	}
	bool create()
	{
		int rv ;
		rv = open( m_fname , O_CREAT, 0644 );
		if ( -1 == rv ) return false ;
		return true ;
	}
	/// Tests whether the file or directory denoted by this
	/// abstract pathname exists.
	bool exists(glob_t* globbuf=NULL  )
	{
		if ( NULL == globbuf )
		{
			if ( ! access( m_fname, R_OK ) ) return true ;
			return false ;
		}
		int rv = glob( m_fname, GLOB_NOSORT , NULL, globbuf);
		if ( 0 != rv ) return false;
		return true ;
	}
	/// Deletes the file or directory denoted by this abstract pathname.
	/// If this pathname denotes a directory, then the directory must be
	/// empty in order to be deleted.
	bool remove( )
	{
		if ( !m_fname ) return false ;
		int rv = unlink(m_fname) ;
		if ( -1 == rv )
		{
			PERR("Unable to remove file[%s]", m_fname );
			return false ;
		}
		return true ;
	}
	time_t creationTime()
	{
		if ( !m_fname ) return -1 ;
		struct stat buf ;
		int rv = stat( m_fname, &buf );
		if ( -1 == rv ) return 0 ;
		return buf.st_ctime ;
	}
	int size()
	{
		struct stat buf ;
		int rv = stat( m_fname, &buf );
		if ( -1 == rv ) return -1 ;
		return buf.st_size ;
	}
protected:
	char* m_fname ;
} ;


#endif	/* _FILE_IMPL_H_*/
