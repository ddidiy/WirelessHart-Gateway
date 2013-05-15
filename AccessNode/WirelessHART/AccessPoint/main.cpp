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

//////////////////////////////////////////////////////////////////////////////
/// @author	Marius Negreanu
/// @date	Thu Dec 18 13:40:22 2008 UTC
/// $Id: main.cpp,v 1.12.30.1 2013/05/15 19:19:17 owlman Exp $
//////////////////////////////////////////////////////////////////////////////


#include "App.h"


/// @mainpage WirelessHART AccessPoint
/// @section OverViewSec Overview.
/// This document provides a comprehensive architectural overview of the
/// WirelessHART AccessPoint, using a number of different
/// architectural views to depict different aspects of the system.
/// It is intended to capture and convey the significant architectural
/// decisions which have been made.
/// @section DataFlowSec Internal data flow
/// <pre>
///
///
///
///
///
///+-----------------+
///| Gateway         | (UpLink)
///+-^----------_----+
///  |          |[2.1]
///  |          |
///  |[1.3]     |
///+-^----------_------+
///| . Router   .[2.2] |
///| .          .      |
///| .          .      |
///| .[1.2]     .      |
///+-^----------_------+
///  |          |[2.3]
///  |          |
///  |          |
///  |          |
///  | [1.1]    |
///+-^----------.------+
///| Transceiver       | (DownLink)
///+-------------------+
///
/// </pre>
/// <b>Legend:</b><br/>
/// [1.*] Up Data Flow, from Transceiver up to Gateway.<br/>
/// [2.*] Down Data Flow, from Gateway down to Transceiver.<br/><br/>
///
/// [1.1] \ref TTYNtwkHdr Tty network message request.<br/>
/// [1.2] \ref CEventLoop::convertDownToUp Convert a Tty Network message
///			to a CHartUpLink::WirelessRequest.<br/>
/// [1.3] \ref HartPDUHdr A CHartUpLink::WirelessRequest message.<br/><br/>
///
/// [2.1] \ref HartPDUHdr A CHartUpLink::WirelessRequest response message.<br/>
/// [2.2] \ref CEventLoop::convertUpToDown \ref TTYNtwkHdr \ref CHartUpLink::WirelessRequest
///			Convert a CHartUpLink::WirelessRequest to a Tty Network message.<br/>
/// [2.3] \ref TTYNtwkAckOutAck Tty Network Acknowledge.<br/><br/>
///
/// <b>Observation:</b><br/>
///	TTYNtwkHdr::m_nHandle and HartPDUHdr::m_ui16TransactId will be between 0 and \ref QHANDLE_MAX.<br/><br/>
///
/// <b>File Naming convention.</b><br/>
/// The files implementing a piece of protocol are named
/// using the schema ProtocolDirectionPurpose.<br/>
/// Ex: <b>HartUpMatcher.h</b><br/>
///
/// At this moment I identified three purposes, listed in the coupling order:<br/><ul>
/// <li>Headers. (Ex: HartUpHdr.h)</li>
/// <li>Packet type matcher.(Uses headers)(Ex: HartUpMatcher.h)</li>
/// <li>Link layers.(Uses Headers and Matchers)(Ex: HartUpLink.h)</li></ul>
/// @defgroup AccessPoint WirelessHART AccessPoint
/// @{ @}


int main( int argc, char** argv)
{
	CWHAccessPointApp oApp;

	if ( !oApp.Init() )
		exit(EXIT_FAILURE );
	LOG("[$Revision: 1.12.30.1 $] started");

	oApp.Run();
	oApp.Close();
	exit( EXIT_SUCCESS );
}
