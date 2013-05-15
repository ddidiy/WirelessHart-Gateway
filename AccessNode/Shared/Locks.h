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


/// @addtogroup libshared
/// @{
//if write,read are safe for concurrent reading and writing there is not need for flock()
#define NEED_LOCK

#ifdef NEED_LOCK
    #define FLOCK( p_nFd, p_nLock ) flock(p_nFd,p_nLock)
    #define FCNTL( p_nFd, p_nLock, p_nStart, p_nLen )\
    {\
	    struct flock lock={0};	\
	    lock.l_type=p_nLock;	\
	    lock.l_whence=SEEK_SET;	\
	    lock.l_start=p_nStart;	\
	    lock.l_len=p_nLen;		\
	    lock.l_pid = getpid();	\
	    fcntl(p_nFd, F_SETLKW, &lock);\
    };
#else
    #define FLOCK( p_nFd, p_nLock ) 	
    #define FCNTL( p_nFd, p_nLock, p_nStart, p_nLen ) 
/// @}
#endif	//NEED_LOCK
