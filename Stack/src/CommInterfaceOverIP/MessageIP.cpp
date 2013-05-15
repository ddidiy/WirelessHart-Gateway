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
 * MessageIP.cpp
 *
 *  Created on: Dec 18, 2008
 *      Author: nicu.dascalu
 */

#include <WHartStack/CommInterfaceOverIP/MessageIP.h>
#include <nlib/detail/bytes_print.h>
#include <nlib/sformat.h>
#include <stdio.h>

namespace hart7 {
namespace stack {
namespace transport {

std::ostream& operator<<(std::ostream& output, const MessageIP& message)
{
    message.AcceptStream(output);
    return output;
}


void SessionInitiate_Request::Accept(IMessageIPVisitor& visitor)
{
	visitor.Visit(*this);
}

void SessionInitiate_Request::AcceptStream(std::ostream& stream) const
{
	stream << "SessionInitiate_Request [TransactionID=" << transactionID << " Status=" << (int)status << " MasterType=" << (int)masterType
        << " InactivityCloseTime=" << inactivityCloseTime << " BypassIOCache=" << (bypassIOCache ? "true" : "false") << "]";
}

void SessionInitiate_Response::Accept(IMessageIPVisitor& visitor)
{
	visitor.Visit(*this);
}

void SessionInitiate_Response::AcceptStream(std::ostream& stream) const
{
	stream << "SessionInitiate_Response [TransactionID=" << transactionID << " Status=" << (int) status << " MasterType=" << (int) masterType
        << " InactivityCloseTime=" << inactivityCloseTime << " BypassIOCache=" << (bypassIOCache ? "true" : "false") << "]";
}

void SessionClose_Request::Accept(IMessageIPVisitor& visitor)
{
	visitor.Visit(*this);
}

void SessionClose_Request::AcceptStream(std::ostream& stream) const
{
	stream << "SessionClose_Request [TransactionID=" << transactionID << " Status=" << (int)status << " BypassIOCache="
        << (bypassIOCache ? "true" : "false") << "]";
}

void SessionClose_Response::Accept(IMessageIPVisitor& visitor)
{
	visitor.Visit(*this);
}

void SessionClose_Response::AcceptStream(std::ostream& stream) const
{
	stream << "SessionClose_Response [TransactionID=" << transactionID << " Status=" << (int)status << " BypassIOCache="
        << (bypassIOCache ? "true" : "false") << "]";
}

void KeepAlive_Request::Accept(IMessageIPVisitor& visitor)
{
	visitor.Visit(*this);
}

void KeepAlive_Request::AcceptStream(std::ostream& stream) const
{
	stream << "KeepAlive_Request [TransactionID=" << transactionID << " Status=" << (int)status << " BypassIOCache="
        << (bypassIOCache ? "true" : "false") << "]";
}

void KeepAlive_Response::Accept(IMessageIPVisitor& visitor)
{
	visitor.Visit(*this);
}

void KeepAlive_Response::AcceptStream(std::ostream& stream) const
{
	stream << "KeepAlive_Response [TransactionID=" << transactionID << " Status=" << (int)status << " BypassIOCache="
        << (bypassIOCache ? "true" : "false") << "]";
}

void HARTWiredPDU_Request::Accept(IMessageIPVisitor& visitor)
{
	visitor.Visit(*this);
}

void HARTWiredPDU_Request::AcceptStream(std::ostream& stream) const
{
	stream << "HARTWiredPDU_Request [TransactionID=" << transactionID << " Status=" << (int)status << " BypassIOCache="
        << (bypassIOCache ? "true" : "false") << "]";
}

void HARTWiredPDU_Response::Accept(IMessageIPVisitor& visitor)
{
	visitor.Visit(*this);
}

void HARTWiredPDU_Response::AcceptStream(std::ostream& stream) const
{
	stream << "HARTWiredPDU_Response [TransactionID=" << transactionID << " Status=" << (int)status << " BypassIOCache="
        << (bypassIOCache ? "true" : "false") << "]";
}

void HARTWiredPDU_Notify::Accept(IMessageIPVisitor& visitor)
{
	visitor.Visit(*this);
}

void HARTWiredPDU_Notify::AcceptStream(std::ostream& stream) const
{
	stream << "HARTWiredPDU_Notify [TransactionID=" << transactionID << " Status=" << (int)status << " BypassIOCache="
        << (bypassIOCache ? "true" : "false") << "]";
}

void HARTDirectPDU_Request::Accept(IMessageIPVisitor& visitor)
{
	visitor.Visit(*this);
}

void HARTDirectPDU_Request::AcceptStream(std::ostream& stream) const
{
	stream << "HARTDirectPDU_Request [TransactionID=" << transactionID << " Status=" << (int)status << " BypassIOCache="
        << (bypassIOCache ? "true" : "false") << "]";
}

void HARTDirectPDU_Response::Accept(IMessageIPVisitor& visitor)
{
	visitor.Visit(*this);
}

void HARTDirectPDU_Response::AcceptStream(std::ostream& stream) const
{
	stream << "HARTDirectPDU_Response [TransactionID=" << transactionID << " Status=" << (int)status << " BypassIOCache="
        << (bypassIOCache ? "true" : "false") << "]";
}

void HARTDirectPDU_Notify::Accept(IMessageIPVisitor& visitor)
{
	visitor.Visit(*this);
}

void HARTDirectPDU_Notify::AcceptStream(std::ostream& stream) const
{
	stream << "HARTDirectPDU_Notify [TransactionID=" << transactionID << " Status=" << (int)status << " BypassIOCache="
        << (bypassIOCache ? "true" : "false") << "]";
}

void WirelessNPDU_Request::Accept(IMessageIPVisitor& visitor)
{
	visitor.Visit(*this);
}

void WirelessNPDU_Request::AcceptStream(std::ostream& stream) const
{
	stream << "WirelessNPDU_Request [TransactionID=" << transactionID << " Status=" << (int) status << " Priority=" << (int)priority << " Address="
        << srcAddress << " NPDU="<< nlib::detail::BytesToString(npdu.data(), npdu.size()) << " BypassIOCache=" << (bypassIOCache ? "true" : "false") <<  "]";
}

void WirelessNPDU_ACK::Accept(IMessageIPVisitor& visitor)
{
	visitor.Visit(*this);
}

void WirelessNPDU_ACK::AcceptStream(std::ostream& stream) const
{
	stream << "WirelessNPDU_ACK [TransactionID=" << transactionID << " Status=" << (int) status << " BypassIOCache=" << (bypassIOCache ? "true" : "false") << "]";
}

} // namesapce transport
} // namespace stack
} // namespace hart7
