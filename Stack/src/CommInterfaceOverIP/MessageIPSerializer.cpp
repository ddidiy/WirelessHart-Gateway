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
 * MessageIPSerializer.cpp
 *
 *  Created on: Nov 10, 2008
 *      Author: Andy
 */
#include <WHartStack/CommInterfaceOverIP/MessageIPSerializer.h>

#include <nlib/binary/adapter.h>
#include <nlib/binary/stdint.h>
#include <nlib/iterator.h>
#include <nlib/detail/bytes_print.h>

#include <boost/format.hpp>
#include <boost/bind.hpp>
#include <iterator> //for back_insert_iterator


namespace hart7 {
namespace stack {
namespace transport {

using nlib::binary::BigEndian;
using nlib::binary::BinaryWriter;

typedef std::back_insert_iterator < MessageIPSerializer::BufferOut> BackInsertIterator;


std::string Packet::ToString() const
{
	return boost::str(boost::format("Packet[version=%1%, type=%2%, id=%3%, status=%4%, tid=%5%, data=%6%]")//
	    % (int) version % (int) messageType.type % (int) messageID % (int) status % (int) transactionID
	    % nlib::detail::BytesToString(data.data(), data.size()));
}

void MessageIPSerializer::Serialize(const MessageIP& message, BufferOut& out)
{
	Packet packet;
	PacketCreator creator;
	creator.Create(message, packet);

	BinaryWriter<BackInsertIterator> writer(std::back_inserter(out));
	writer.Write<BigEndian> (packet.version);
	uint8_t messageType = (packet.messageType.type & 0x0F) | ((packet.messageType.flags & 0x0F) << 4);
	writer.Write<BigEndian> (messageType);
	writer.Write<BigEndian> ((boost::uint8_t) packet.messageID);
	writer.Write<BigEndian> (packet.status);
	writer.Write<BigEndian> (packet.transactionID);

	std::size_t dataSize = packet.data.size() + Packet::PACKETHEADER_SIZE;
	writer.Write<BigEndian> ((boost::uint16_t) dataSize);

	//TODO:[andy]- see if we can do it somehow by not copying the data around that much...
	writer.Write<BigEndian> (packet.data.begin(), packet.data.end());

	//LOG_DEBUG("Serialize: message=" << message.ToString()
	//		<< " in bytes=" << nlib::detail::BytesToString(out.data(), out.size()));
}

void MessageIPSerializer::PacketCreator::Create(const MessageIP& message, Packet& packet_)
{
	packet = &packet_;
	packet->version = 1;
	packet->transactionID = message.transactionID;
	packet->data.reserve(128);	// assure we have enough space to avoid reallocation.

	const_cast<MessageIP&> (message).Accept(*this);
}

void MessageIPSerializer::PacketCreator::Visit(SessionInitiate_Request& req)
{
	packet->messageType.type = Packet::mtRequest;
	packet->messageType.flags = req.bypassIOCache ? Packet::mtfBypassIOCache : Packet::mtfDefault;
	packet->messageID = Packet::miSessionInitiate;
	packet->status = 0;

	BinaryWriter<BackInsertIterator> writer(std::back_inserter(packet->data));
	writer.Write<BigEndian> (req.masterType);
	writer.Write<BigEndian> (req.inactivityCloseTime);
}

void MessageIPSerializer::PacketCreator::Visit(SessionInitiate_Response& res)
{
	packet->messageType.type = Packet::mtResponse;
	packet->messageType.flags = (res.bypassIOCache ? Packet::mtfBypassIOCache : Packet::mtfDefault) & 0x01;
	packet->messageID = Packet::miSessionInitiate;
	packet->status = res.status;

	BinaryWriter<BackInsertIterator> writer(std::back_inserter(packet->data));
	writer.Write<BigEndian> (res.masterType);
	writer.Write<BigEndian> (res.inactivityCloseTime);
}

void MessageIPSerializer::PacketCreator::Visit(SessionClose_Request& req)
{
	packet->messageType.type = Packet::mtRequest;
	packet->messageType.flags = req.bypassIOCache ? Packet::mtfBypassIOCache : Packet::mtfDefault;
	packet->messageID = Packet::miSessionClose;
	packet->status = 0;
}

void MessageIPSerializer::PacketCreator::Visit(SessionClose_Response& res)
{
	packet->messageType.type = Packet::mtResponse;
	packet->messageType.flags = res.bypassIOCache ? Packet::mtfBypassIOCache : Packet::mtfDefault;
	packet->messageID = Packet::miSessionClose;
	packet->status = res.status;
}

void MessageIPSerializer::PacketCreator::Visit(KeepAlive_Request& req)
{
	packet->messageType.type = Packet::mtRequest;
	packet->messageType.flags = req.bypassIOCache ? Packet::mtfBypassIOCache : Packet::mtfDefault;
	packet->messageID = Packet::miKeepAlive;
	packet->status = 0;
}

void MessageIPSerializer::PacketCreator::Visit(KeepAlive_Response& res)
{
	packet->messageType.type = Packet::mtResponse;
	packet->messageType.flags = res.bypassIOCache ? Packet::mtfBypassIOCache : Packet::mtfDefault;
	packet->messageID = Packet::miKeepAlive;
	packet->status = res.status;
}

void MessageIPSerializer::PacketCreator::Visit(HARTWiredPDU_Request& req)
{
	packet->messageType.type = Packet::mtRequest;
	packet->messageType.flags = req.bypassIOCache ? Packet::mtfBypassIOCache : Packet::mtfDefault;
	packet->messageID = Packet::miHartWiredPDU;
	packet->status = 0;

	BinaryWriter<BackInsertIterator> writer(std::back_inserter(packet->data));
	writer.Write<BigEndian> (req.hartPDU.begin(), req.hartPDU.end());
}

void MessageIPSerializer::PacketCreator::Visit(HARTWiredPDU_Response& res)
{
	packet->messageType.type = Packet::mtResponse;
	packet->messageType.flags = res.bypassIOCache ? Packet::mtfBypassIOCache : Packet::mtfDefault;
	packet->messageID = Packet::miHartWiredPDU;
	packet->status = res.status;

	BinaryWriter<BackInsertIterator> writer(std::back_inserter(packet->data));
	writer.Write<BigEndian> (res.hartPDU.begin(), res.hartPDU.end());
}

void MessageIPSerializer::PacketCreator::Visit(HARTWiredPDU_Notify& res)
{
	packet->messageType.type = Packet::mtPublishNotification;
	packet->messageType.flags = res.bypassIOCache ? Packet::mtfBypassIOCache : Packet::mtfDefault;
	packet->messageID = Packet::miHartWiredPDU;
	packet->status = res.status;

	BinaryWriter<BackInsertIterator> writer(std::back_inserter(packet->data));
	writer.Write<BigEndian> (res.hartPDU.begin(), res.hartPDU.end());
}

void MessageIPSerializer::PacketCreator::Visit(HARTDirectPDU_Request& req)
{
	packet->messageType.type = Packet::mtRequest;
	packet->messageType.flags = req.bypassIOCache ? Packet::mtfBypassIOCache : Packet::mtfDefault;
	packet->messageID = Packet::miHartDirectPDU;
	packet->status = 0;

	BinaryWriter<BackInsertIterator> writer(std::back_inserter(packet->data));
	writer.Write<BigEndian> (req.hartPDU.begin(), req.hartPDU.end());
}

void MessageIPSerializer::PacketCreator::Visit(HARTDirectPDU_Response& res)
{
	packet->messageType.type = Packet::mtResponse;
	packet->messageType.flags = res.bypassIOCache ? Packet::mtfBypassIOCache : Packet::mtfDefault;
	packet->messageID = Packet::miHartDirectPDU;
	packet->status = res.status;

	BinaryWriter<BackInsertIterator> writer(std::back_inserter(packet->data));
	writer.Write<BigEndian> (res.hartPDU.begin(), res.hartPDU.end());
}

void MessageIPSerializer::PacketCreator::Visit(HARTDirectPDU_Notify& res)
{
	packet->messageType.type = Packet::mtPublishNotification;
	packet->messageType.flags = res.bypassIOCache ? Packet::mtfBypassIOCache : Packet::mtfDefault;
	packet->messageID = Packet::miHartDirectPDU;
	packet->status = res.status;

	BinaryWriter<BackInsertIterator> writer(std::back_inserter(packet->data));
	writer.Write<BigEndian> (res.hartPDU.begin(), res.hartPDU.end());
}

void MessageIPSerializer::PacketCreator::Visit(WirelessNPDU_Request& req)
{
	packet->messageType.type = Packet::mtRequest;
	packet->messageType.flags = req.bypassIOCache ? Packet::mtfBypassIOCache : Packet::mtfDefault;
	packet->messageID = Packet::miWirelessPDU;
	packet->status = 0;

	//create first byte:
	//1. byte with b2 set if the source address is long,
	// and b1b0 are the priorities (00-Alarm, 01-Normal, 10-Data, 11-Command)
	boost::uint8_t byte = (boost::uint8_t) (0x3 & req.priority);
	switch (req.srcAddress.type)
	{
	case WHartAddress::whartaNickname:
		//do nothing
		break;

	case WHartAddress::whartaUniqueID:
		byte = (boost::uint8_t) (byte | 0x4);
		break;

	default:
		assert(false && "not supported");
	}

	BinaryWriter<BackInsertIterator> writer(std::back_inserter(packet->data));
	writer.Write<BigEndian> (byte);

	if (req.srcAddress.type == WHartAddress::whartaNickname)
	{
		writer.Write<BigEndian> (req.srcAddress.address.nickname);
	}
	else
	{
		writer.Write<BigEndian> ((boost::uint8_t) 0x00);
		writer.Write<BigEndian> ((boost::uint8_t) 0x1B);
		writer.Write<BigEndian> ((boost::uint8_t) 0x1E);
		for (int i = 0; i < (int) sizeof(req.srcAddress.address.uniqueID.bytes); i++)
			writer.Write<BigEndian> (req.srcAddress.address.uniqueID.bytes[i]);
	}
	writer.Write<BigEndian> (req.npdu.begin(), req.npdu.end());
}

void MessageIPSerializer::PacketCreator::Visit(WirelessNPDU_ACK& res)
{
	packet->messageType.type = Packet::mtResponse;
	packet->messageType.flags = res.bypassIOCache ? Packet::mtfBypassIOCache : Packet::mtfDefault;
	packet->messageID = Packet::miWirelessPDU;
	packet->status = res.status;
}

using nlib::binary::BigEndian;
using nlib::binary::BinaryReader;

typedef std::basic_string<boost::uint8_t>::const_iterator DataReadIterator;


MessageIPUnserializer::MessageIPUnserializer()
{
	PacketParsed = boost::bind(&MessageIPUnserializer::UnserializePacket, this, _1);
}

void MessageIPUnserializer::ParseBytes(const boost::uint8_t* bytes, std::size_t size)
{
//	LOG_DEBUG("Parsing received bytes...");
	buffer.insert(buffer.end(), bytes, bytes + size);

	typedef nlib::front_extract_iterator<BufferInput> FrontExtractIterator;
	FrontExtractIterator iterator(buffer);
	BinaryReader<FrontExtractIterator> br(iterator);

	while (true)
	{
		if (currentState == ppsWaitingForHeader)
		{
			if (buffer.size() >= Packet::PACKETHEADER_SIZE)
			{
				//parse packet header
				currentPacket.version = br.Read<BigEndian, boost::uint8_t> ();
				uint8_t messageType = br.Read<BigEndian, boost::uint8_t> ();
				currentPacket.messageType.type = messageType & 0x0F;
				currentPacket.messageType.flags = (messageType & 0xF0) >> 4;
				currentPacket.messageID = (Packet::MessageIdentifier) br.Read<BigEndian, boost::uint16_t> ();
				currentPacket.status = br.Read<BigEndian, boost::uint8_t> ();
				currentPacket.transactionID = br.Read<BigEndian, boost::uint16_t> ();
				int dataSize = br.Read<BigEndian, boost::uint16_t> () - Packet::PACKETHEADER_SIZE;
				currentPacket.data.resize(dataSize);

				currentState = ppsWaitingForData;
			}
			else
			{
				break; // not enough bytes
			}
		}
		if (currentState == ppsWaitingForData)
		{
			if (buffer.size() >= currentPacket.data.size())
			{
				//read data bytes
				br.Read<BigEndian> (currentPacket.data.begin(), currentPacket.data.end());

				if (PacketParsed)
					PacketParsed(currentPacket);
				currentState = ppsWaitingForHeader;
			}
			else
			{
				break; // not enough bytes.
			}
		}
	}
}

void MessageIPUnserializer::UnserializePacket(const Packet& packet)
{
	MessageIP::Ptr message = ParsePacket(packet);

	if (MessageUnserialized)
		MessageUnserialized(*message);
}

MessageIP::Ptr MessageIPUnserializer::ParseBytesComplete(const boost::uint8_t* bytes, std::size_t size)
{
//	LOG_DEBUG("ParseBytesComplete size=" << size
//			<< " bytes=<" << nlib::detail::BytesToString(bytes, size) << ">");

	buffer.clear();
	buffer.insert(buffer.end(), bytes, bytes + size);
	typedef nlib::front_extract_iterator<BufferInput> FrontExtractIterator;
	FrontExtractIterator iterator(buffer);
	BinaryReader<FrontExtractIterator> br(iterator);

	if (buffer.size() < Packet::PACKETHEADER_SIZE)
	{
		LOG_WARN("Not enough header bytes.");
		return MessageIP::Ptr();
	}

	Packet packet;
	//parse packet header
	packet.version = br.Read<BigEndian, boost::uint8_t> ();
	uint8_t messageType = br.Read<BigEndian, boost::uint8_t> ();
	packet.messageType.type = messageType & 0x0F;
	packet.messageType.flags = (messageType & 0xF0) >> 4;
	packet.messageID = (Packet::MessageIdentifier) br.Read<BigEndian, boost::uint8_t> ();
	packet.status = br.Read<BigEndian, boost::uint8_t> ();
	packet.transactionID = br.Read<BigEndian, boost::uint16_t> ();
	int dataSize = br.Read<BigEndian, boost::uint16_t> () - Packet::PACKETHEADER_SIZE;
	if (dataSize < 0 || dataSize > 65535)
	{
		LOG_ERROR("ParseBytesComplete dataSize=" << dataSize << " is invalid...");
		return MessageIP::Ptr();
	}
	packet.data.resize(dataSize);

	if (buffer.size() < packet.data.size())
	{
		LOG_ERROR("Not enough content size.");
		return MessageIP::Ptr();
	}
	else if (buffer.size() > packet.data.size())
	{
		LOG_WARN("There are unused bytes in packet. ususedCount=" << buffer.size()- packet.data.size());
	}

	//read data bytes
	br.Read<BigEndian> (packet.data.begin(), packet.data.end());

//	LOG_DEBUG("ParseBytesComplete in packet=" << packet.ToString());

	return ParsePacket(packet);
}

MessageIP::Ptr MessageIPUnserializer::ParsePacket(const Packet& packet)
{
	class PacketParser: public IMessageIPVisitor
	{
	private:
		const Packet& packet;
		BinaryReader<DataReadIterator> br;

	public:
		PacketParser(const Packet& packet_) :
			packet(packet_), br(packet_.data.begin(), packet_.data.end())
		{
		}

		void Do(MessageIP& message)
		{
			message.Accept(*this);
		}

		//IMessageIPVisitor
	private:
		void Visit(SessionInitiate_Request& req)
		{
			req.masterType = br.Read<BigEndian, boost::uint8_t> ();
			req.inactivityCloseTime = br.Read<BigEndian, boost::uint32_t> ();
		}

		void Visit(SessionInitiate_Response& res)
		{
			res.masterType = br.Read<BigEndian, boost::uint8_t> ();
			res.inactivityCloseTime = br.Read<BigEndian, boost::uint32_t> ();
		}

		void Visit(SessionClose_Request& req)
		{
			//nothing to parse (no payload)
		}

		void Visit(SessionClose_Response& res)
		{
			//nothing to parse (no payload)
		}

		void Visit(KeepAlive_Request& req)
		{
		}

		void Visit(KeepAlive_Response& res)
		{
		}

		void Visit(HARTWiredPDU_Request& req)
		{
			req.hartPDU = packet.data;
		}

		void Visit(HARTWiredPDU_Response& res)
		{
			res.hartPDU = packet.data;
		}

		void Visit(HARTWiredPDU_Notify& res)
		{
			res.hartPDU = packet.data;
		}

		void Visit(HARTDirectPDU_Request& req)
		{
			req.hartPDU = packet.data;
		}

		void Visit(HARTDirectPDU_Response& res)
		{
			res.hartPDU = packet.data;
		}

		void Visit(HARTDirectPDU_Notify& res)
		{
			res.hartPDU = packet.data;
		}

		void Visit(WirelessNPDU_Request& req)
		{
			boost::uint8_t byte = br.Read<BigEndian, boost::uint8_t> ();
			req.priority = (WHartPriority) (byte & 0x03);
			//read address
			if (byte & 0x04)
			{
				///skip next 3 bytes
				for (int i = 0; i < 3; i++)
					br.Read<BigEndian, boost::uint8_t> ();

				WHartUniqueID uid;
				for (int i = 0; i < (int) sizeof(uid.bytes); i++)
					uid.bytes[i] = br.Read<BigEndian, boost::uint8_t> ();

				req.srcAddress = uid;
				req.npdu.resize(packet.data.size() - (1 + 3 + sizeof(uid.bytes)));
			}
			else
			{
				WHartShortAddress nickname = br.Read<BigEndian, boost::uint16_t> ();

				req.srcAddress = nickname;
				req.npdu.resize(packet.data.size() - (1 + sizeof(nickname)));
			}

			br.Read<BigEndian> (req.npdu.begin(), req.npdu.end());
		}

		void Visit(WirelessNPDU_ACK& res)
		{
			//nothing to parse (no payload)
		}
	};

	MessageIP::Ptr message = CreateMessage(packet);
	if (!message)
	{
		THROW_EXCEPTION0(InvalidPacketException);
	}

	try
	{
		message->status = packet.status;
		message->transactionID = packet.transactionID;
		message->bypassIOCache = ((packet.messageType.flags & Packet::mtfBypassIOCache) != 0) ? true : false;

		PacketParser(packet).Do(*message);

		message->parsedStatus = true;
//		LOG_INFO("ParsePacket:"
//				<< " packet =" << packet.ToString()
//				<< " in message=" << *message);
	}
	catch (nlib::binary::BinaryException& ex)
	{
		message->parsedStatus = false;
		LOG_ERROR("Populate message=" << *message
				<< " for packet=" << packet.ToString() << " failed!"
				<< " Error=" << ex.what());
	}
	return message;
}

MessageIP::Ptr MessageIPUnserializer::CreateMessage(const Packet& packet)
{
	switch (packet.messageID)
	{
	case Packet::miSessionInitiate:
		if (packet.messageType.type == Packet::mtRequest)
		{
			return MessageIP::Ptr(new SessionInitiate_Request());
		}
		else if (packet.messageType.type == Packet::mtResponse)
		{
			return MessageIP::Ptr(new SessionInitiate_Response());
		}
		break;
	case Packet::miSessionClose:
		if (packet.messageType.type == Packet::mtRequest)
		{
			return MessageIP::Ptr(new SessionClose_Request());
		}
		else if (packet.messageType.type == Packet::mtResponse)
		{
			return MessageIP::Ptr(new SessionClose_Response());
		}
		break;
	case Packet::miKeepAlive:
		if (packet.messageType.type == Packet::mtRequest)
		{
			return MessageIP::Ptr(new KeepAlive_Request());
		}
		else if (packet.messageType.type == Packet::mtResponse)
		{
			return MessageIP::Ptr(new KeepAlive_Response());
		}
		break;
	case Packet::miHartDirectPDU:
		if (packet.messageType.type == Packet::mtRequest)
		{
			return MessageIP::Ptr(new HARTDirectPDU_Request());
		}
		else if (packet.messageType.type == Packet::mtResponse)
		{
			return MessageIP::Ptr(new HARTDirectPDU_Response());
		}
		else if (packet.messageType.type == Packet::mtPublishNotification)
		{
			return MessageIP::Ptr(new HARTDirectPDU_Notify());
		}
		break;
	case Packet::miHartWiredPDU:
		if (packet.messageType.type == Packet::mtRequest)
		{
			return MessageIP::Ptr(new HARTWiredPDU_Request());
		}
		else if (packet.messageType.type == Packet::mtResponse)
		{
			return MessageIP::Ptr(new HARTWiredPDU_Response());
		}
		else if (packet.messageType.type == Packet::mtPublishNotification)
		{
			return MessageIP::Ptr(new HARTWiredPDU_Notify());
		}
		break;
	case Packet::miWirelessPDU:
		if (packet.messageType.type == Packet::mtRequest)
		{
			return MessageIP::Ptr(new WirelessNPDU_Request());
		}
		else if (packet.messageType.type == Packet::mtResponse)
		{
			return MessageIP::Ptr(new WirelessNPDU_ACK());
		}
		break;

	default:
		LOG_DEBUG("Invalid packet.messageID=" << packet.messageID)
		;
		break;
	}

	return MessageIP::Ptr();
}


} //namespace transport
} //namespace stack
} //namespace hart7
