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

#ifndef MESSAGE_IP_SERIALIZER_H
#define MESSAGE_IP_SERIALIZER_H

#include <WHartStack/CommInterfaceOverIP/MessageIP.h>


#include <nlib/log.h>
#include <nlib/exception.h>
#include <loki/Function.h>

#include <deque>
#include <string>


namespace hart7 {
namespace stack {
namespace transport {

class InvalidPacketException: public nlib::Exception
{
};

class Packet
{
public:
	static const boost::uint16_t PACKETHEADER_SIZE = 8; // plus command code
	static const boost::uint8_t PACKETHEADER_VERSION = 1;

	enum MessageTypeFlags
	{
		mtfDefault = 0,
		mtfBypassIOCache = 1
	};

	enum _MessageType
	{
		mtRequest = 0,
		mtResponse = 1,
		mtPublishNotification = 2
	};

	enum MessageIdentifier
	{
		miSessionInitiate = 0,
		miSessionClose = 1,
		miKeepAlive = 2,
		miHartWiredPDU = 3,
		miHartDirectPDU = 4,
		miWirelessPDU = 10 // for communication between NM<->GW, GW<->AP
	};

	struct MessageType
	{
		uint8_t type; //bits 0-3
		uint8_t flags; //bits 4-7
	};

	Packet()
	{
		version = PACKETHEADER_VERSION;
		messageType.flags = 0;
		messageType.type = mtRequest;

		messageID = miSessionInitiate;
		status = 0;
		transactionID = 0;
	}

	std::string ToString() const;

	uint8_t version;
	MessageType messageType;
	MessageIdentifier messageID;
	uint8_t status;
	uint16_t transactionID;
	std::basic_string<boost::uint8_t> data;
};



class MessageIPSerializer
{
	LOG_DEF("h7.s.t.MessageIPSerializer");
public:
	typedef std::basic_string<boost::uint8_t> BufferOut;

private:
	class PacketCreator: public IMessageIPVisitor
	{
	public:
		void Create(const MessageIP& message, Packet& packet);

		// IMessageIPVisitor
	private:
		void Visit(SessionInitiate_Request& req);
		void Visit(SessionInitiate_Response& res);

		void Visit(SessionClose_Request& req);
		void Visit(SessionClose_Response& res);

		void Visit(KeepAlive_Request& req);
		void Visit(KeepAlive_Response& res);

		void Visit(HARTWiredPDU_Request& req);
		void Visit(HARTWiredPDU_Response& res);
		void Visit(HARTWiredPDU_Notify& res);

		void Visit(HARTDirectPDU_Request& req);
		void Visit(HARTDirectPDU_Response& res);
		void Visit(HARTDirectPDU_Notify& res);

		void Visit(WirelessNPDU_Request& req);
		void Visit(WirelessNPDU_ACK& res);

	private:
		Packet* packet;
	};

public:
	void Serialize(const MessageIP& message, BufferOut& out);

};



class MessageIPUnserializer
{
    LOG_DEF("h7.s.t.MessageIPUnserializer")
	;
public:
	MessageIPUnserializer();

	// for streamed bytes
	void ParseBytes(const boost::uint8_t* bytes, std::size_t size);
	Loki::Function<void(const Packet&)> PacketParsed;
	Loki::Function<void(const MessageIP&)> MessageUnserialized;

	void UnserializePacket(const Packet& packet);

	//for datagram bytes
	MessageIP::Ptr ParseBytesComplete(const boost::uint8_t* bytes, std::size_t size);

	/*
	 *@throws InvalidPacketException
	 */
	void Reset();

	MessageIP::Ptr ParsePacket(const Packet& packet);
	MessageIP::Ptr CreateMessage(const Packet& packet);


private:
	Packet currentPacket;
	int currentPacketDataSize;

	enum PacketParserState
	{
		ppsWaitingForHeader,
		ppsWaitingForData
	} currentState;

	typedef std::deque<boost::uint8_t> BufferInput;
	BufferInput buffer;
};


} //namspace transport
} //namespace stack
} //namespace hart7

#endif /*MESSAGE_IP_SERIALIZER_H*/
