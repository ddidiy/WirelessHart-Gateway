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

#ifndef MESSAGE_IP_H
#define MESSAGE_IP_H

#include <WHartStack/WHartStack.h> //for WirelessNPDU request/response

#include <boost/cstdint.hpp> //for standard integer types
#include <boost/shared_ptr.hpp>
#include <string>

namespace hart7 {
namespace stack {
namespace transport {

class IMessageIPVisitor;

class MessageIP
{
public:
	typedef boost::shared_ptr<MessageIP> Ptr;

	MessageIP()
	{
		transactionID = 0xFFFF;
	}

	virtual ~MessageIP()
	{
	}
	virtual void Accept(IMessageIPVisitor& visitor) = 0;
	virtual void AcceptStream(std::ostream& stream) const = 0;

	friend std::ostream& operator<<(std::ostream& output, const MessageIP& message);

public:
	bool bypassIOCache;
	boost::uint16_t transactionID;
	boost::uint8_t status;
	bool parsedStatus;
};
std::ostream& operator<<(std::ostream& output, const MessageIP& message);

//#SessionInitiate
class SessionInitiate_Request: public MessageIP
{
	//MessageIP
public:
    virtual void AcceptStream(std::ostream& stream) const;

private:
	void Accept(IMessageIPVisitor& visitor);

public:
	boost::uint8_t masterType;
	boost::uint32_t inactivityCloseTime;
};

class SessionInitiate_Response: public MessageIP
{
	//MessageIP
private:
	void Accept(IMessageIPVisitor& visitor);
public:
    void AcceptStream(std::ostream& stream) const;
	enum ResponseCodes
	{
		rcSuccess = 0,
		rcErr_InvalidMasterType = 2,//2 Error Invalid Selection (Invalid Master Type)
		rcErr_TooFewDataBytesReceived = 5,//5 Error Too Few Data Bytes Received
		rcWarn_InactivityTimerValue = 8,//8 Warning Set to Nearest Possible Value (Inactivity timer value)
		rcErr_AllAvailableSessionsInUse = 15,//15 Error All available sessions in use
		rcErr_AccessRestricted = 16	//16 Error Access Restricted, Session already established
	};

	uint8_t masterType;
	uint32_t inactivityCloseTime;
};

//#SessionClose

class SessionClose_Request: public MessageIP
{
	//MessageIP
public:
    virtual void AcceptStream(std::ostream& stream) const;
private:
	void Accept(IMessageIPVisitor& visitor);
};

class SessionClose_Response: public MessageIP
{
public:
	enum ResponseCodes
	{
		rcSuccess = 0
	};
	//MessageIP
public:
    virtual void AcceptStream(std::ostream& stream) const;
private:
	void Accept(IMessageIPVisitor& visitor);
};

//#KeepAlive
class KeepAlive_Request: public MessageIP
{
	//MessageIP
public:
    virtual void AcceptStream(std::ostream& stream) const;
private:
	void Accept(IMessageIPVisitor& visitor);
};

class KeepAlive_Response: public MessageIP
{
public:
	enum ResponseCodes
	{
		rcSuccess = 0
	};
	//MessageIP
public:
    virtual void AcceptStream(std::ostream& stream) const;
private:
	void Accept(IMessageIPVisitor& visitor);
};

//#HART Wired PDU
class HARTWiredPDU_Request: public MessageIP
{
public:
	std::basic_string<boost::uint8_t> hartPDU;
	//MessageIP
public:
    virtual void AcceptStream(std::ostream& stream) const;
private:
	void Accept(IMessageIPVisitor& visitor);
};

class HARTWiredPDU_Response: public MessageIP
{
public:
	enum ResponseCodes
	{
		rcSuccess = 0,//0 Success No error occurred
		rcErr_TooFewDataBytes = 5,//5 Error Too Few Data Bytes Received
		rcErr_AccessRestricted = 16,//16 Error Access Restricted (Server resources exhausted)
		rcErr_DRdead = 35 //35 Error DR Dead (device not connected or no response)
	};

	std::basic_string<boost::uint8_t> hartPDU;
	//MessageIP
public:
    virtual void AcceptStream(std::ostream& stream) const;
private:
	void Accept(IMessageIPVisitor& visitor);
};

class HARTWiredPDU_Notify: public MessageIP
{
public:
	enum ResponseCodes
	{
		rcSuccess = 0,//0 Success No error occurred
		rcErr_TooFewDataBytes = 5,//5 Error Too Few Data Bytes Received
		rcErr_AccessRestricted = 16,//16 Error Access Restricted (Server resources exhausted)
		rcErr_DRdead = 35 //35 Error DR Dead (device not connected or no response)
	};

	std::basic_string<boost::uint8_t> hartPDU;
	//MessageIP
public:
    virtual void AcceptStream(std::ostream& stream) const;
private:
	void Accept(IMessageIPVisitor& visitor);
};


//#HARTDirectPDU

class HARTDirectPDU_Request: public MessageIP
{
public:
	std::basic_string<boost::uint8_t> hartPDU;
	//MessageIP
public:
    virtual void AcceptStream(std::ostream& stream) const;
private:
	void Accept(IMessageIPVisitor& visitor);
};

class HARTDirectPDU_Response: public MessageIP
{
public:
	enum ResponseCodes
	{
		rcSuccess = 0,//0 Success No error occurred
		rcErr_TooFewDataBytes = 5,//5 Error Too Few Data Bytes Received
		rcErr_AccessRestricted = 16,//16 Error Access Restricted (Server resources exhausted)
		rcErr_DRdead = 35 //35 Error DR Dead (device not connected or no response)
	};

	std::basic_string<boost::uint8_t> hartPDU;
	//MessageIP
public:
    virtual void AcceptStream(std::ostream& stream) const;
private:
	void Accept(IMessageIPVisitor& visitor);
};

class HARTDirectPDU_Notify: public MessageIP
{
public:
	enum ResponseCodes
	{
		rcSuccess = 0,//0 Success No error occurred
		rcErr_TooFewDataBytes = 5,//5 Error Too Few Data Bytes Received
		rcErr_AccessRestricted = 16,//16 Error Access Restricted (Server resources exhausted)
		rcErr_DRdead = 35 //35 Error DR Dead (device not connected or no response)
	};

	std::basic_string<boost::uint8_t> hartPDU;
	//MessageIP
public:
    virtual void AcceptStream(std::ostream& stream) const;
private:
	void Accept(IMessageIPVisitor& visitor);
};

/**
 * Transports a wireless network pdu payload between GW, NM, (AP ??).
 */
class WirelessNPDU_Request: public MessageIP
{
public:
	WHartPriority priority;
	WHartAddress srcAddress;
	std::basic_string<boost::uint8_t> npdu;
	//MessageIP
public:
    virtual void AcceptStream(std::ostream& stream) const;
private:
	void Accept(IMessageIPVisitor& visitor);
};

/**
 * Transports an ACK for a request.
 */
class WirelessNPDU_ACK: public MessageIP
{
public:
	enum ResponseCodes
	{
		rcSuccess = 0,//0 Success No error occurred
		rcErr_TooFewDataBytes = 5,//5 Error Too Few Data Bytes Received
		rcErr_DRdead = 35,//35 Error DR Dead (device not connected or no response)
		rcErr_NoBuffers = 61,
		rcErr_NoAlarmEventBuffers = 62,
		rcErr_PriorityTooLow = 63
	};
	//MessageIP
public:
    virtual void AcceptStream(std::ostream& stream) const;
private:
	void Accept(IMessageIPVisitor& visitor);
};

/**
 * The Visitor from Visitor Pattern.
 */
class IMessageIPVisitor
{
public:
	virtual ~IMessageIPVisitor()
	{
	}

	virtual void Visit(SessionInitiate_Request& req) {}
	virtual void Visit(SessionInitiate_Response& res) {}

	virtual void Visit(SessionClose_Request& req) {}
	virtual void Visit(SessionClose_Response& res) {}

	virtual void Visit(KeepAlive_Request& req)  {}
	virtual void Visit(KeepAlive_Response& res)  {}

	virtual void Visit(HARTWiredPDU_Request& req) {}
	virtual void Visit(HARTWiredPDU_Response& res) {}
	virtual void Visit(HARTWiredPDU_Notify& res) {}

	virtual void Visit(HARTDirectPDU_Request& req) {}
	virtual void Visit(HARTDirectPDU_Response& res) {}
	virtual void Visit(HARTDirectPDU_Notify& res) {}

	virtual void Visit(WirelessNPDU_Request& req) {}
	virtual void Visit(WirelessNPDU_ACK& res)  {}
};


} // namesapce transport
} // namespace stack
} // namespace hart7

#endif /*MESSAGE_IP_H*/
