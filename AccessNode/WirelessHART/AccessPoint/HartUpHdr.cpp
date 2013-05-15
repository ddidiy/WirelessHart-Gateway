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

// $Id: HartUpHdr.cpp,v 1.6.32.1 2013/05/15 19:19:17 owlman Exp $

//////////////////////////////////////////////////////////////////////////////
/// @author	Marius Negreanu
/// @date	Thu Dec 18 13:40:22 2008 UTC
//////////////////////////////////////////////////////////////////////////////

#include "HartUpHdr.h"

/// SessionInit Message and StatusClass.
struct HartStatus StatusSessionInit_st[] ={
	{ 0,true ,"Success"},
	{ 2,false,"Invalid Selection"},
	{ 5,false,"Too Few Data Bytes"},
	{ 8,true ,"Set no nearest possible value"},
	{15,false,"All available sessions in use"},
	{16,false,"Access Restricted, session allready set"},
	{ 0,false, NULL}
};
struct HartStatusAssoc StatusSessionInitAssoc_st= { "SESSION_INIT", StatusSessionInit_st} ;

/// SessionClose Message and StatusClass.
struct HartStatus StatusSessionClose_st[] ={
	{ 0,true,"Success"},
	{ 0,false,NULL}
};
struct HartStatusAssoc StatusSessionCloseAssoc_st= { "SESSION_CLOSE", StatusSessionClose_st} ;

/// KeepAlive Message and StatusClass.
struct HartStatus StatusKeepAlive_st[] ={
	{ 0,true,"Success"},
	{ 0,false,NULL}
};
struct HartStatusAssoc StatusKeepAliveAssoc_st= { "KEEP_ALIVE", StatusKeepAlive_st} ;

/// WirelessRequest Message and StatusClass.
struct HartStatus WirelessRequest_st[] ={
	{ 0,true ,"Success"},
	{ 5,false,"Too Few Data Bytes"},
	{35,false,"DR Dead"},
	{61,false,"No Buffers"},
	{62,false,"Priority Too Low"},
	{ 0,false,NULL}
};
struct HartStatusAssoc WirelessRequestAssoc_st= { "WREQUEST", WirelessRequest_st} ;
