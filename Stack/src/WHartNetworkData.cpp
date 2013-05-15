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
 * WHartNetworkData.cpp
 *
 *  Created on: Dec 11, 2008
 *      Author: nicu.dascalu
 */
#include <WHartStack/WHartNetworkData.h>
#include <WHartStack/util/crypto/Ccm.h>

#include <nlib/detail/bytes_print.h>

namespace hart7 {
namespace stack {
namespace network {

const uint8_t CONTROLTYPE_1STSOURCEROUTE_MASK = 0x01;
const uint8_t CONTROLTYPE_2NDSOURCEROUTE_MASK = 0x02;
const uint8_t CONTROLTYPE_PROXYROUTE_MASK = 0x04;
const uint8_t CONTROLTYPE_SRCADDRESS_MASK = 0x40;
const uint8_t CONTROLTYPE_DESTADDRESS_MASK = 0x80;

const uint8_t SECURITYCONTROL_SECURITYTYPE_MASK = 0x0F;

const uint32_t CRYPT_BUFF_SIZE = 512;

const uint8_t NONCECOUNTERHISTORY_BIT_LENGTH = 32;

static
void StreamWriteNetAddress(BinaryStream* stream, const WHartAddress& value)
{
	switch (value.type)
	{
	case WHartAddress::whartaNickname:
	{
		STREAM_WRITE_UINT16(stream, value.address.nickname);
		break;
	}
	case WHartAddress::whartaUniqueID:
	{
		//compose EUI64 address, see TDMA Data Link Layer specification.pdf
		STREAM_WRITE_UINT24(stream, 0x001B1E);
		STREAM_WRITE_BYTES(stream, value.address.uniqueID.bytes, 5);
		break;
	}
	case WHartAddress::whartaProxy:
	{
		//compose EUI64 address, see TDMA Data Link Layer specification.pdf
		STREAM_WRITE_UINT24(stream, 0x001B1E);
		STREAM_WRITE_BYTES(stream, value.address.proxy.uniqueID.bytes, 5);
		break;
	}
    case WHartAddress::whartaProxyShort:
    {
        STREAM_WRITE_UINT16(stream, value.address.proxyShort.destNickname);
        break;
    }
	}
}

static
void StreamReadNetAddress(BinaryStream* stream, WHartAddress& value, bool isNickName)
{
	if (isNickName)
	{
		STREAM_READ_UINT16(stream, &value.address.nickname);
		value.type = WHartAddress::whartaNickname;
	} else
	{
		//compose EUI64 address, see TDMA Data Link Layer specification.pdf
		STREAM_SKIP(stream, 3); //0x001B1E, maybe should check if is equal
		STREAM_READ_BYTES(stream, value.address.uniqueID.bytes, 5);
		value.type = WHartAddress::whartaUniqueID;
	}
}

enum CryptoStatus
{
	whartcsSuccess = 0, whartcsFailed_OutOfMemory = 1, whartcsFailed_Authentication = 2,

	whartcsFailed_NotImpl = 0xFF
};

static
std::string CryptoStatusToString(CryptoStatus& status)
{
	switch (status)
	{
	case whartcsSuccess: return std::string("Success");
	case whartcsFailed_OutOfMemory: return std::string("Out of memory failure");
	case whartcsFailed_Authentication: return std::string("Authentication failed");
	case whartcsFailed_NotImpl: return std::string("Not implemented");
	default: return std::string("Unknown");
	}
}

static
uint32_t ReconstructNonceCounter(const Session& session, uint8_t nonceLSB)
{
	uint32_t nonceCounter = session.receiveNonceCounter & 0xFFFFFF00;

	if ((int)nonceLSB < (1 + (int)(session.receiveNonceCounter & 0xFF) - (int)sizeof(session.nonceCounterHistory) * 8))
	{
		nonceCounter += 0x0100;
	}
	nonceCounter += nonceLSB;

	return nonceCounter;
}

/**
 * encrypt the tpdu OR authenticates and decrypt
 * @spdu contains MIC + enciphered payload
 */
static
CryptoStatus EncryptDecrypt(const Session& session, uint32_t nonceCounterReconstructed, const WHartPayload& header, const WHartPayload& input,
		WHartPayload& output, const WHartAddress& src, const WHartAddress& dest, bool encrypt);
static
bool IsJoinRequestResponse(bool isJoinKey, const WHartAddress& src, const WHartAddress& dest);

WHartNetworkData::WHartNetworkData(CommonData& common_) :
	commonData(common_)
{
	Reset();
}

WHartNetworkData::~WHartNetworkData()
{
}

void WHartNetworkData::TransmitRequest(WHartHandle handle, const WHartAddress& dest, WHartPriority priority,
		WHartServiceID serviceID, const WHartPayload& tpdu, WHartSessionKey::SessionKeyCode sessionCode)
{
	if (dest == Broadcast_Nickname())
	{
		TransmitRequest_Broadcast(handle, priority, tpdu);
	} else
	{
		TransmitRequest_Unicast(handle, dest, priority, serviceID, tpdu, sessionCode);
	}
}

void WHartNetworkData::TransmitRequest_Unicast(WHartHandle handle, const WHartAddress& dest, WHartPriority priority,
		WHartServiceID serviceID, const WHartPayload& tpdu, WHartSessionKey::SessionKeyCode sessionCode)
{
//	LOG_DEBUG("TransmitRequest_Unicast handle=" << handle << ", dest=" << dest << ", serviceID=" << serviceID
//			<< ", TPDU=" << tpdu << ", sessionCode=" << sessionCode);
	//find session
	SessionTable::iterator session = FindSession(dest, sessionCode);
	if (session == sessionTable.end())
	{
		LOG_WARN("Session for Dest=" << dest << "with type=" << sessionCode << " not found!");
		upper->TransmitConfirm(handle, WHartLocalStatus(WHartLocalStatus::whartlsError_Start + 3));
		return;//invalid session
	}

	//get  route
	WHartLocalStatus error;
	RouteTable::iterator route = GetRoute(serviceID, dest, error);
	if (route == routeTable.end())
	{
		LOG_WARN("TransmitRequest_Unicast route not found!");
		upper->TransmitConfirm(handle, error);
		return;//route not found
	}

	uint8_t buffer[CommonData::MAX_PAYLOAD];
	BinaryStream stream;
	STREAM_INIT(&stream, buffer, (uint8_t) sizeof(buffer));

	uint8_t control = ((WHartAddress::whartaNickname == dest.type) || (WHartAddress::whartaProxyShort == dest.type)) ? 0 : CONTROLTYPE_DESTADDRESS_MASK;
	control |= (commonData.isJoined) ? 0 : CONTROLTYPE_SRCADDRESS_MASK;
	control |= ((WHartAddress::whartaProxy == dest.type) || (WHartAddress::whartaProxyShort == dest.type)) ? CONTROLTYPE_PROXYROUTE_MASK : 0;

	if (route->destinationType == Route::dtSourceRoute)
	{
		control |= CONTROLTYPE_1STSOURCEROUTE_MASK;
		if (route->sourceRoutePath.size() > 4)
		{
			control |= CONTROLTYPE_2NDSOURCEROUTE_MASK;
		}
	}

	WHartAddress src = commonData.isJoined ? WHartAddress(commonData.myNickname) : WHartAddress(commonData.myUniqueID);

	//write the network header
	WriteNetworkHeader(*route,
			dest,
			src,
			control,
			TTL, // ttl
			commonData.CurrentTime40().u32 & 0xFFFF,	// asn
			stream);


	//write security header without MIC
	STREAM_WRITE_UINT8(&stream, sessionCode & 0x0F);

	uint32_t nonceCounter = 0;
	if (sessionCode == WHartSessionKey::sessionKeyed)
	{
		session->transmitNonceCounter++;
		STREAM_WRITE_UINT8(&stream, session->transmitNonceCounter & 0xFF);
		nonceCounter = session->transmitNonceCounter;
	} else if (sessionCode == WHartSessionKey::joinKeyed)
	{
		bool isJoinResponse = IsJoinRequestResponse(true, src, dest);
		if (isJoinResponse)
		{
			LOG_DEBUG("Join response.");
			STREAM_WRITE_UINT32(&stream, session->receiveNonceCounter);
			nonceCounter = session->receiveNonceCounter;
		} else //isJoinRequest
		{
			session->transmitNonceCounter++;
			STREAM_WRITE_UINT32(&stream, session->transmitNonceCounter);
			nonceCounter = session->transmitNonceCounter;
		}
	}

	//call security
	uint8_t spduData[CommonData::MAX_PAYLOAD];
	WHartPayload spdu(spduData, sizeof(spduData));
	CryptoStatus encryptStatus = EncryptDecrypt(*session, nonceCounter, WHartPayload(buffer, stream.nextByte - buffer), tpdu, spdu,
			src, dest, true);

	if (encryptStatus != whartcsSuccess)
	{
		LOG_WARN("TransmitRequest_Unicast failed to encrypt Error=" << (int) encryptStatus);
		upper->TransmitConfirm(handle, WHartLocalStatus(WHartLocalStatus::whartlsError_Start + 4));
		return;//encrypt error
	}

	//write enciphered payload
	STREAM_WRITE_BYTES(&stream, spdu.data, spdu.dataLen);

	//forward to lower layer: dll
	lower->TransmitRequest(handle, GetDllDestinationAddress(*route), priority, 0, WHartPayload(buffer,
			stream.nextByte - buffer));

	route->numPktsTransmitted++;
}

void WHartNetworkData::WriteNetworkHeader(Route& route, const WHartAddress& dest, const WHartAddress& src, uint8_t control, uint8_t ttl, uint16_t asn, BinaryStream& stream)
{
	STREAM_WRITE_UINT8(&stream, control);
	STREAM_WRITE_UINT8(&stream, ttl);
	//write asn
	STREAM_WRITE_UINT16(&stream, asn);

	STREAM_WRITE_UINT16(&stream, route.graphID);
	StreamWriteNetAddress(&stream, dest);
	StreamWriteNetAddress(&stream, src);

	if (WHartAddress::whartaProxy == dest.type)
	{
		STREAM_WRITE_UINT16(&stream, dest.address.proxy.nickname);
	}

	if (WHartAddress::whartaProxyShort == dest.type)
	{
	    STREAM_WRITE_UINT16(&stream, dest.address.proxyShort.nickname);
	}

	if (route.destinationType == Route::dtSourceRoute)
	{
		int count = 0;
		for (std::vector<WHartShortAddress>::const_iterator it = route.sourceRoutePath.begin();
			it != route.sourceRoutePath.end(); ++it)
		{
			StreamWriteNetAddress(&stream, WHartAddress(*it));
			count++;
		}

		if (count < 8 && count != 4)	// prevent writing more than 8 addresses, do not write the rest of 4 if count == 4
		{
			// fill remaining bytes with 0XFF - segments are of 8 bytes ( 4 short addresses ) each,
			// must be filled with 0xFFFF
			for (int i = count % 4; i < 4; i++)
			{
				STREAM_WRITE_UINT16(&stream, 0xFFFF);
			}
		}
	}
}

WHartDllAddress WHartNetworkData::GetDllDestinationAddress(Route& route)
{
	bool isSourceRoute = route.destinationType == Route::dtSourceRoute;
	if (isSourceRoute)
	{
		if (route.sourceRoutePath.size() > 0)
		{
			std::vector<WHartShortAddress>::iterator currentPos = route.sourceRoutePath.begin();
			for (; currentPos != route.sourceRoutePath.end(); currentPos++)
			{
				if ((*currentPos) == commonData.myNickname)
				{
					break;
				}
			}

			if (currentPos != route.sourceRoutePath.end() && (++currentPos) != route.sourceRoutePath.end())
			{
				LOG_DEBUG("Source route found, forwarding to " << WHartAddress(*currentPos));
				return WHartDllAddress (*currentPos);
			}

			if (currentPos != route.sourceRoutePath.end() && (++currentPos) != route.sourceRoutePath.end())
			{
				LOG_DEBUG("Source route found, forwarding to " << WHartAddress(*currentPos));
				return WHartDllAddress (*currentPos);
			}
		}
	}

	// my address was not found in the source route or was last, or no source route at all
//	LOG_DEBUG("Forwarding on GraphID=" << std::hex << route.graphID);
	return WHartDllAddress(route.graphID, bool());
}

void WHartNetworkData::TransmitRequest_Broadcast(WHartHandle, WHartPriority priority, const WHartPayload& tpdu)
{

}

void WHartNetworkData::TransmitRequest_Join(const WHartAddress& through)
{

}

void WHartNetworkData::TransmitConfirm(WHartHandle handle, const WHartLocalStatus& localStatus)
{
//	LOG_DEBUG("TransmitConfirm handle=" << handle << " localStatus=" << localStatus.status);

	if (IsRoutedHandle(handle))
	{
		if (WHartLocalStatus::whartlsError_Start <= localStatus.status)
		{
			LOG_INFO("TransmitConfirm  handle=" << handle << " failed to route...");
			//todo [nicu.dascalu] - try another route, or add statistics
		}
		DeleteRoutedHandle(handle);
	} else
	{
		//just notify that the dll send or failed
//		LOG_INFO("transmitConfirm handle=" << handle << " forward to TL");
		upper->TransmitConfirm(handle, localStatus);
	}
}

void WHartNetworkData::TransmitIndicate(const WHartLocalStatus& localStatus, WHartPriority priority,
		const WHartDllAddress& dllSrc, WHartPayload& npdu)
{
//	LOG_DEBUG("TransmitIndicate: localStatus=" << localStatus.status << ", priority=" << priority << ", dllSrc="
//			<< dllSrc << ", NPDU=" << npdu);

	BinaryStream stream;
	STREAM_INIT(&stream, npdu.data, npdu.dataLen);

	//parse network header
	uint8_t controlByte;
	STREAM_READ_UINT8(&stream, &controlByte);
	uint8_t ttl;
	STREAM_READ_UINT8(&stream, &ttl);
	uint16_t asn;
	STREAM_READ_UINT16(&stream, &asn);

	//HACK:[andrei.petrut] - write ASN to common data so it can be used in the next packet...
	commonData.lastReadASN = asn;

	uint16_t graphID;
	STREAM_READ_UINT16(&stream, &graphID);
	WHartAddress dest;
	StreamReadNetAddress(&stream, dest, (controlByte & CONTROLTYPE_DESTADDRESS_MASK) == 0);
	WHartAddress src;
	StreamReadNetAddress(&stream, src, (controlByte & CONTROLTYPE_SRCADDRESS_MASK) == 0);

	//TODO:[andy] - implement isBroadcast
	bool isForMe = (dest == commonData.myNickname || dest == commonData.myUniqueID);
	bool isBroadcast = false;
	bool isSourceRoute = ((controlByte & CONTROLTYPE_1STSOURCEROUTE_MASK) != 0);

	WHartShortAddress proxy = 0;
	if (controlByte & CONTROLTYPE_PROXYROUTE_MASK)
	{
		STREAM_READ_UINT16(&stream, &proxy);
	}

	Route tempRoute;
	if (isSourceRoute)
	{
		tempRoute.destinationType = Route::dtSourceRoute;
		tempRoute.graphID = graphID;
		for (int i = 0; i < 4; i++)
		{
			WHartShortAddress addr;
			STREAM_READ_UINT16(&stream, &addr);
			if (addr != 0xFFFF)
			{
				tempRoute.sourceRoutePath.push_back(addr);
			}
		}
		if (controlByte & CONTROLTYPE_2NDSOURCEROUTE_MASK)
		{
			for (int i = 0; i < 4; i++)
			{
				WHartShortAddress addr;
				STREAM_READ_UINT16(&stream, &addr);
				if (addr != 0xFFFF)
				{
					tempRoute.sourceRoutePath.push_back(addr);
				}
			}
		}
	}

//	LOG_DEBUG("TransmitIndicate: isForMe=" << (int) isForMe << ", isBroadcast=" << (int) isBroadcast << ", isSourceRoute=" << (int)isSourceRoute << ", destAddress="
//			<< dest << ", TTL=" << ToStringHexa(ttl) << ", ASN=" << ToStringHexa(asn) << ", graphID="
//			<< ToStringHexa(graphID));

	if (isForMe || isBroadcast)
	{
		uint8_t securityControl; //contain session
		STREAM_READ_UINT8(&stream, &securityControl);

		WHartSessionKey::SessionKeyCode sessionKey = (WHartSessionKey::SessionKeyCode) (securityControl & 0x0F);
        uint8_t sessionType = securityControl & SECURITYCONTROL_SECURITYTYPE_MASK;

        //if join flow detected OnJoinCalback is called, in order to create session with the new device/ap if it is
        //provisioned correctly
        if ((sessionType == WHartSessionKey::joinKeyed) && (OnJoinCallback))
        {
            OnJoinCallback(src);
        }

		SessionTable::iterator session = FindSession(src, sessionKey);
		if (session == sessionTable.end())
		{
			LOG_WARN("Session for Dest=" << src << "with type=" << (int) sessionKey
					<< " not found! packet droped.");
			return;//invalid session
		}
		else
		{
//			LOG_DEBUG("Session used for packet is=" << sessionKey << " and dest=" << src << " and sessionID=" << (int)session->sessionID);
		}



		uint32_t counter;
		if (sessionType == WHartSessionKey::sessionKeyed)
		{
			uint8_t lsbNonce;
			STREAM_READ_UINT8(&stream, &lsbNonce);
//			LOG_DEBUG("Read small nonce counter = " << (int)lsbNonce << " . Need to reconstruct nonce counter...");
//			LOG_DEBUG("Session counter = " << session->receiveNonceCounter <<
//					" Session history size = " << sizeof(session->nonceCounterHistory)<<
//					" and compare value=" << ((1 + ((int)session->receiveNonceCounter & 0xFF) - (int)sizeof(session->nonceCounterHistory) * 8)));
			counter = ReconstructNonceCounter(*session, lsbNonce);
//			LOG_DEBUG("Reconstructed nonce counter=" << counter);
		} else
		{
			STREAM_READ_UINT32(&stream, &counter);
//			LOG_DEBUG("Read entire nonce counter=" << counter);
		}


		// reconstruct nonce counter end

		WHartPayload spdu(stream.nextByte, stream.remainingBytes);

		uint8_t dataTPDU[CommonData::MAX_PAYLOAD];
		WHartPayload tpdu(dataTPDU, sizeof(dataTPDU));

//		LOG_DEBUG("Creating PDU with size=" << stream.nextByte - npdu.data << " with stream next byte="
//				<< nlib::detail::BytesToString(stream.nextByte, stream.nextByte - npdu.data));
		CryptoStatus decryptStatus = EncryptDecrypt(*session, counter, WHartPayload(npdu.data, stream.nextByte - npdu.data), spdu,
				tpdu, src, dest, false);

		if (decryptStatus != whartcsSuccess)
		{
			LOG_WARN("transmitIndicate failed to decrypt Error=" << CryptoStatusToString(decryptStatus));
			return;
		}

		//reconstruct nonce counter
		if (sessionType == WHartSessionKey::joinKeyed) // join flow
		{
			LOG_DEBUG("Join flow detected.");
			session->receiveNonceCounter = counter;

		} else
		{
			int noncePosition = session->receiveNonceCounter - counter;
			if (noncePosition > (int) (NONCECOUNTERHISTORY_BIT_LENGTH))
			{
				//discard packet
				LOG_INFO("Received packet with nonce counter outside of the sliding window (nonce=" << counter
						<< ", windowStart=" << session->receiveNonceCounter << ", windowSize="
						<< (int) NONCECOUNTERHISTORY_BIT_LENGTH << "). Discarding packet...");
				return;
			}

			if (noncePosition < 0)
			{
//				LOG_DEBUG("New nonce received (old=" << session->receiveNonceCounter << ", new=" << counter << ". Sliding window to accommodate new nonce (with steps=" << -noncePosition << ")...");
				if (noncePosition <= -32)
				{
					session->nonceCounterHistory = 0;
				}
				else
				{
					session->nonceCounterHistory = session->nonceCounterHistory >> (-noncePosition);
				}

				noncePosition = 0;
				session->receiveNonceCounter = counter;
			}

			uint32_t mask = 0x01 << (NONCECOUNTERHISTORY_BIT_LENGTH - noncePosition - 1);
			if (session->nonceCounterHistory & mask)
			{
				// already received packet with this nonce, discard
				LOG_WARN("Already received packet with nonce=" << counter << " from src=" << src << ". Discarding...");
				return;
			}

			// mark nonce in history
			session->nonceCounterHistory = session->nonceCounterHistory | mask;
		}


		//forward to upper
		++nextRoutedHandler;
		WHartHandle handle = MAKE_WHARTHANDLE(1, nextRoutedHandler);

//		LOG_DEBUG("transmitIndicate forward to TL" << " handle=" << handle << ", src=" << src << ", priority="
//				<< priority << ", TPDU=" << tpdu);
		upper->TransmitIndicate(handle, src, priority, tpdu, (WHartSessionKey::SessionKeyCode) (securityControl & 0x0F));
	} else
	{
		//not for me
		//re forward to lower layer
		WHartDllAddress dllAddress(graphID, bool());
		WHartHandle routedHandle = NewRoutedHandle();

		if (commonData.IsGateway() && (dest == NetworkManager_Nickname() || dest == NetworkManager_UniqueID()))
		{
			dllAddress = WHartDllAddress(NetworkManager_Nickname());
		}
		else
		{


			// if TTL is 0xFF, do not decrement, just forward to the destination
			if (ttl != 0xFF)
			{
				//check TTL
				if (ttl == 0)
				{
					LOG_WARN("transmitIndicate drop npdu=" << npdu << " TTL underflow");
					return; //packet is dropped
				}

				//TODO [nicu.dascalu] - check ASN and generate timeout

				//packet is valid, so send back to dll

				//HACK [nicu.dascalu] - change in place the TTL
				(const_cast<uint8_t*> (npdu.data))[1]--;
				//in case there is a network header reconstruct
				ttl--;
			}

			//compute the next hop address
			bool foundDestinationAddress = false;

			if (controlByte & CONTROLTYPE_PROXYROUTE_MASK)
			{
				// final hop, send to unique id
				if (proxy == commonData.myNickname)
				{
					if (dest.type ==  WHartAddress::whartaNickname)
					{
						dllAddress = WHartDllAddress(dest.address.nickname);
					}
					else if (dest.type == WHartAddress::whartaUniqueID)
					{
						dllAddress = WHartDllAddress(dest.address.uniqueID);
					}
					else
					{
						LOG_WARN("Unexpected destination address of " << dest);
						return;
					}

					foundDestinationAddress = true;
//					LOG_DEBUG("Proxy routing. Final proxy=" << WHartAddress(proxy) << " which is me, so forwarding to " << dest);
				}
//				else
//					LOG_DEBUG("Proxy routing. Forwarding on graphID=" << graphID << " or source routing until proxy=" << WHartAddress(proxy).ToString() << "...");

			}

			if (!foundDestinationAddress && isSourceRoute)	//we have source routing, and we are not final proxy if proxyrouting
			{
				//find my address in source route position, send to next
				LOG_DEBUG("Attempting source routing...");
				dllAddress = GetDllDestinationAddress(tempRoute);
			}
			else	//graph routed
			{
				// address already after graph
			}
		}
		lower->TransmitRequest(routedHandle, dllAddress, priority, (WHartDllTimeout) 0, npdu);
	}
}

void WHartNetworkData::WriteService(C973_WriteService_Req* req, C973_WriteService_Resp* res)
{
	ServiceTable::iterator service = FindService(req->m_ucServiceID, req->m_unPeerNickname);
	if (service == serviceTable.end())
	{
		serviceTable.push_back(Service());
		service = serviceTable.end() - 1;
		service->serviceID = req->m_ucServiceID;
		service->peerNickname = req->m_unPeerNickname;
	}
	service->routeID = req->m_ucRouteID;
	service->serviceDomain = req->m_eApplicationDomain;
	service->serviceFlags = req->m_ucRequestFlags;
	service->servicePeriod.u32 = req->m_tPeriod.u32;

	LOG_INFO("Add serviceID=" << std::hex << (int)req->m_ucServiceID << ", per=" << std::dec << req->m_tPeriod.u32 << ", rId=" << std::hex << (int)req->m_ucRouteID);
}

void WHartNetworkData::DeleteService(C801_DeleteService_Req* req, C801_DeleteService_Resp* resp)
{
	ServiceTable::iterator service = FindService(req->m_ucServiceId, req->m_peerNickname);
	if (service != serviceTable.end())
	{
		LOG_INFO("Removed serviceId=" << std::hex << (int)req->m_ucServiceId << ", peer=" << std::hex << (int)req->m_peerNickname);
		serviceTable.erase(service);
	}
	else
	{
		LOG_INFO("Remove serviceId=" << std::hex << (int)req->m_ucServiceId << " not found.");
	}
}



void WHartNetworkData::WriteSession(const C963_WriteSession_Req* req, C963_WriteSession_Resp* res)
{
	SessionTable::iterator session = FindSession(WHartAddress(req->m_aPeerUniqueID),
			(WHartSessionKey::SessionKeyCode) req->m_eSessionType);
	if (session == sessionTable.end())
	{
		LOG_DEBUG("Adding new session type=" << (int)req->m_eSessionType << " and uniqueID=" << WHartAddress(req->m_aPeerUniqueID));
		sessionTable.push_back(Session());
		session = sessionTable.end() - 1;
		memcpy(session->peerUniqueID.bytes, req->m_aPeerUniqueID, 5);
	}
	else
	{
		LOG_DEBUG("Altering existing session type=" << (int)req->m_eSessionType <<" to have nick=" << WHartAddress(req->m_unPeerNickname));
	}
	session->nonceCounterHistory = 0;
	session->receiveNonceCounter = req->m_ulPeerNonceCounterValue;
	session->transmitNonceCounter = 0;
	session->peerNickName = req->m_unPeerNickname;
	session->sessionKey.keyCode = (WHartSessionKey::SessionKeyCode) req->m_eSessionType;
	COPY_FIXED_ARRAY(session->sessionKey.key, req->m_aKeyValue);
}

void WHartNetworkData::DeleteSession(C964_DeleteSession_Req* req, C964_DeleteSession_Resp* res)
{
	WHartAddress peer;
	peer.type = WHartAddress::whartaNickname;
	peer.address.nickname = req->m_unPeerNickname;
	SessionTable::iterator session = FindSession(peer, (WHartSessionKey::SessionKeyCode) req->m_eSessionType);
	if (session != sessionTable.end())
	{
    	LOG_DEBUG("Erasing session with dest=" << req->m_unPeerNickname<< " and type=" << (int)req->m_eSessionType);
		sessionTable.erase(session);
	}
}

void WHartNetworkData::DeleteSession(const WHartAddress& destination, WHartSessionKey::SessionKeyCode sessionType)
{
    SessionTable::iterator session = FindSession(destination, sessionType);
    if (session != sessionTable.end())
    {
    	LOG_DEBUG("Erasing session with dest=" << destination << " and type=" << (int)sessionType);
    	sessionTable.erase(session);
    }
}


void WHartNetworkData::SetSession(C963_WriteSession_Req* req)
{
    WHartAddress peer;
    peer.type = WHartAddress::whartaUniqueID;
    COPY_FIXED_ARRAY(peer.address.uniqueID.bytes, req->m_aPeerUniqueID);
    SessionTable::iterator session = FindSession(peer, (WHartSessionKey::SessionKeyCode) req->m_eSessionType);

    if (session == sessionTable.end())
    {
        C963_WriteSession_Resp * res = NULL;
        WriteSession(req, res);
        LOG_DEBUG("New session with " << peer.address.uniqueID);
    }
    else
    {
        if (0 != COMPARE_FIXED_ARRAY(session->sessionKey.key, req->m_aKeyValue))
        {
            LOG_DEBUG("Existing session with: " << peer.address.uniqueID  << " new key set");
            sessionTable.erase(session);
            C963_WriteSession_Resp * res = NULL;
            WriteSession(req, res);
        }
        else
        {
            LOG_DEBUG("Session exists with: " << peer.address.uniqueID);
        }
    }
}

void WHartNetworkData::WriteRoute(C974_WriteRoute_Req* req, C974_WriteRoute_Resp* res)
{
	RouteTable::iterator route = FindRoute(req->m_ucRouteID);
	if (route == routeTable.end())
	{
		routeTable.push_back(Route());
		route = routeTable.end() - 1;
		route->routeID = req->m_ucRouteID;
	}
	route->destinationType = Route::dtRegular;
	route->graphID = req->m_unGraphID;
	route->peerNickname = req->m_unPeerNickname;

	LOG_INFO("WriteRoute routeId=" << std::hex << (int)route->routeID << ", gId=" << route->graphID << ", dst=" << route->peerNickname);
}

void WHartNetworkData::DeleteRoute(C975_DeleteRoute_Req* req, C975_DeleteRoute_Resp* res)
{
	RouteTable::iterator route = FindRoute(req->m_ucRouteID);
	if (route != routeTable.end())
	{
		if (route->destinationType == Route::dtRegular)
		{
			routeTable.erase(route);
			LOG_DEBUG("DeleteRoute routeID=" << req->m_ucRouteID);
		}
		else
		{
			LOG_WARN("Deleting route with ID=" << (int)req->m_ucRouteID << " failed because it is not a normal route. Ignoring delete...");
		}
	}
}

int WHartNetworkData::ReadRouteList(C802_ReadRouteList_Req* req, C802_ReadRouteList_Resp* resp)
{
	memset(resp, 0, sizeof(C802_ReadRouteList_Resp));

	if ( req->m_ucRouteIndex >= routeTable.size() )
	{	return RCM_W08_SetToNearestPossibleValue;
	}

	RouteTable::iterator i = routeTable.begin() + req->m_ucRouteIndex;

	resp->m_ucRouteIndex = req->m_ucRouteIndex;
	resp->m_ucNoOfActiveRoutes = routeTable.size();

	for ( ; i != routeTable.end(); i++ )
	{	resp->m_aRoutes[resp->m_ucNoOfEntriesRead].routeId = i->routeID;
		resp->m_aRoutes[resp->m_ucNoOfEntriesRead].sourceRouteAttached =
			(i->destinationType == Route::dtSourceRoute) ? 1:0;
		resp->m_aRoutes[resp->m_ucNoOfEntriesRead].destinationNickname = i->peerNickname;
		resp->m_aRoutes[resp->m_ucNoOfEntriesRead++].graphId = i->graphID;

		//check if we have read all requested routes
		if (resp->m_ucNoOfEntriesRead == req->m_ucNoOfEntriesToRead)
		{	resp->m_ucNoOfRoutesRemaining = resp->m_ucNoOfActiveRoutes - (resp->m_ucRouteIndex + resp->m_ucNoOfEntriesRead);
			return RCS_N00_Success;
		}

		if (resp->m_ucNoOfEntriesRead == C802_MAX_ROUTES_LIST)
		{	break;
		}
	}

	resp->m_ucNoOfRoutesRemaining = resp->m_ucNoOfActiveRoutes - (resp->m_ucRouteIndex + resp->m_ucNoOfEntriesRead);

	return RCM_W08_SetToNearestPossibleValue;
}

void WHartNetworkData::WriteSourceRoute(C976_WriteSourceRoute_Req* req, C976_WriteSourceRoute_Resp* resp)
{
	RouteTable::iterator route = FindRoute(req->m_ucRouteID);
	if (route == routeTable.end())
	{
		routeTable.push_back(Route());
		route = routeTable.end() - 1;
	}
	route->routeID = req->m_ucRouteID;
	route->destinationType = Route::dtSourceRoute;
	route->sourceRoutePath.clear();	// clear route path, just in case the route already existed.
	for (int i = 0; i < req->m_ucHopsNo; ++i)
	{
		route->sourceRoutePath.push_back((WHartShortAddress)req->m_aNicknameHopEntries[i]);
	}
}

int WHartNetworkData::ReadSourceRoute(C803_ReadSourceRoute_Req* req, C803_ReadSourceRoute_Resp* resp)
{
	memset(resp, 0, sizeof(C803_ReadSourceRoute_Resp));

	RouteTable::iterator route = FindRoute(req->m_ucRouteId);
	if (route == routeTable.end())
	{	return RCM_E65_EntryNotFound;
	}

	resp->m_ucRouteId = req->m_ucRouteId;

	std::vector<WHartShortAddress>::iterator it = route->sourceRoutePath.begin();
	for (; it != route->sourceRoutePath.end() ; ++it)
	{	resp->m_aHopNicknames[resp->m_ucNoOfHops++] = *it;
	}

	return RCS_N00_Success;
}

void WHartNetworkData::DeleteSourceRoute(C977_DeleteSourceRoute_Req* req, C977_DeleteSourceRoute_Resp* resp)
{
	RouteTable::iterator route = FindRoute(req->m_ucRouteID);
	if (route != routeTable.end())
	{
		if (route->destinationType == Route::dtSourceRoute)
		{
			routeTable.erase(route);
			LOG_DEBUG("Deleted source route with ID=" << req->m_ucRouteID);
		}
		else
		{
			LOG_WARN("Deleting route with ID=" << (int) req->m_ucRouteID << " failed because it is not a source route. Ignoring delete...");
		}
	}
}

WHartNetworkData::ServiceTable::iterator WHartNetworkData::FindService(WHartServiceID serviceID,
		const WHartAddress& dest)
{
	for (ServiceTable::iterator it = serviceTable.begin(); it != serviceTable.end(); it++)
		if (it->serviceID == serviceID)
		{
			if ((dest == it->peerNickname) //
					|| (dest.type == WHartAddress::whartaProxy && dest.address.proxy.nickname == it->peerNickname) //
                    || (dest.type == WHartAddress::whartaProxyShort && dest.address.proxyShort.nickname == it->peerNickname))
			{
				return it;
			}
		}

	return serviceTable.end();
}

WHartNetworkData::RouteTable::iterator WHartNetworkData::FindRoute(uint8_t routeID)
{
	for (RouteTable::iterator it = routeTable.begin(); it != routeTable.end(); it++)
		if (it->routeID == routeID)
		{
				return it;
		}

	return routeTable.end();
}

WHartNetworkData::RouteTable::iterator WHartNetworkData::GetRoute(WHartServiceID serviceID, const WHartAddress& dest,
		WHartLocalStatus& error)
{
	error.status = WHartLocalStatus::whartlsSuccess;

	//if is NM pick first route from route table to NM
	if (dest == NetworkManager_Nickname() || dest == NetworkManager_UniqueID())
	{
		for (RouteTable::iterator it = routeTable.begin(); it != routeTable.end(); it++)
			if (it->destinationType == Route::dtRegular && it->peerNickname == NetworkManager_Nickname())
				return it;
	}

	ServiceTable::iterator service = FindService(serviceID, dest);
	if (service == serviceTable.end())
	{
		LOG_WARN("ServiceID=" << serviceID << " for dest=" << dest << " cannot be found!");
		error.status = (WHartLocalStatus::Status) (WHartLocalStatus::whartlsError_Start + 11);
		return routeTable.end();//invalid service
	}

	RouteTable::iterator route = FindRoute(service->routeID);
	if (route == routeTable.end())
	{
		LOG_ERROR("transmitRequest routeID=" << (int) service->routeID << " not found!");
		error.status = (WHartLocalStatus::Status) (WHartLocalStatus::whartlsError_Start + 12);
		return routeTable.end();//invalid route for service
	}

	return route;
}

WHartNetworkData::SessionTable::iterator WHartNetworkData::FindSession(const WHartAddress& peer,
		WHartSessionKey::SessionKeyCode type)
{
	for (SessionTable::iterator it = sessionTable.begin(); it != sessionTable.end(); it++)
	{
	    if (type == it->sessionKey.keyCode) {
	        if (peer.type == WHartAddress::whartaProxy) {
	            if (peer.address.proxy.uniqueID == it->peerUniqueID) {
	                return it;
	            }
            } else if (peer.type == WHartAddress::whartaProxyShort) {
                if (peer.address.proxyShort.destNickname == it->peerNickName) {
                    return it;
                }
            } else if ((WHartAddress(it->peerNickName) == peer) || (WHartAddress(it->peerUniqueID) == peer))  {
	            return it;
	        }
	    }
	}

	return sessionTable.end();
}

WHartHandle WHartNetworkData::NewRoutedHandle()
{
	return 0;
}

bool WHartNetworkData::IsRoutedHandle(WHartHandle handle)
{
	return false;
}

void WHartNetworkData::DeleteRoutedHandle(WHartHandle handle)
{
}
/**
 * See Network Manager specification: Section 9.1.3.3
 */
static
bool IsJoinRequestResponse(bool isJoinKey, const WHartAddress& src, const WHartAddress& dest)
{
	if (isJoinKey)
	{
		return (src == NetworkManager_Nickname() || src == NetworkManager_UniqueID()) ? true : false;
	} else
	{
		return false;
	}
}

LOG_DEF("h7.s.n.WHartNetworkData.Security");
static
CryptoStatus EncryptDecrypt(const Session& session, uint32_t nonceCounterReconstructed, const WHartPayload& header, const WHartPayload& input,
		WHartPayload& output, const WHartAddress& src, const WHartAddress& dest, bool encrypt)
{
	stack_crypto::Ccm ccm;
	ccm.SetAuthenFldSz(4); // MIC is on 4 bytes
	ccm.SetLenFldSz(2); // (16(totalAESBlockSize) - 13(nonce) -1(firstBlock))

	// set 'a'
	stack_crypto::Bytes a;
	{
		a.insert(a.end(), header.data, header.data + header.dataLen);
		a[1] = 0; //TTL to 0

		//reset counter
		if (session.sessionKey.keyCode == WHartSessionKey::sessionKeyed)
		{
			memset((uint8_t*) a.data() + (header.dataLen - 1), 0, 1);
		} else
		{
			memset((uint8_t*) a.data() + (header.dataLen - 4), 0, 4); //reset counter
		}

		//add MIC = 0
		a.push_back(0);
		a.push_back(0);
		a.push_back(0);
		a.push_back(0);
	}

	// set 'N'
	stack_crypto::Bytes nonce;
	{
		nonce.resize(13);

		BinaryStream s;
		STREAM_INIT(&s, nonce.data(), nonce.size());
		uint32_t nonceCounter = 0;
		bool isJoinKey = session.sessionKey.keyCode == WHartSessionKey::joinKeyed;
//		LOG_DEBUG("ReceiveNonceCounter is: " << session.receiveNonceCounter << " TransmitNonceCounter is:"
//				<< session.transmitNonceCounter);
		bool isJoinRequestResponse = IsJoinRequestResponse(isJoinKey, src, dest);
		WHartAddress nonceAddress;
		if (isJoinRequestResponse)
		{
//			LOG_DEBUG("Security: is join response, writing nonce=" << nonceCounterReconstructed);//session.receiveNonceCounter);
			//N[0] = 1
			STREAM_WRITE_UINT8(&s, 1);
			nonceCounter = nonceCounterReconstructed;//session.receiveNonceCounter;
			STREAM_WRITE_UINT32(&s, nonceCounter);
			nonceAddress = dest;
		} else // other message
		{
			//N[0] = 0
			STREAM_WRITE_UINT8(&s, 0);
			if (encrypt)
				nonceCounter = session.transmitNonceCounter;
			else
			{
				// use the nonce counter that was reconstructed
				nonceCounter = nonceCounterReconstructed;//session.receiveNonceCounter;
			}
			STREAM_WRITE_UINT32(&s, nonceCounter);
			nonceAddress = src;
		}

		if (nonceAddress.type == WHartAddress::whartaUniqueID)
		{
			STREAM_WRITE_UINT8(&s, 0x00);
			STREAM_WRITE_UINT8(&s, 0x1B);
			STREAM_WRITE_UINT8(&s, 0x1E);

			STREAM_WRITE_UINT8(&s, nonceAddress.address.uniqueID.bytes[0]);
			STREAM_WRITE_UINT8(&s, nonceAddress.address.uniqueID.bytes[1]);
			STREAM_WRITE_UINT8(&s, nonceAddress.address.uniqueID.bytes[2]);
			STREAM_WRITE_UINT8(&s, nonceAddress.address.uniqueID.bytes[3]);
			STREAM_WRITE_UINT8(&s, nonceAddress.address.uniqueID.bytes[4]);
		} else if (nonceAddress.type == WHartAddress::whartaProxy)
		{
			STREAM_WRITE_UINT8(&s, 0x00);
			STREAM_WRITE_UINT8(&s, 0x1B);
			STREAM_WRITE_UINT8(&s, 0x1E);

			STREAM_WRITE_UINT8(&s, nonceAddress.address.proxy.uniqueID.bytes[0]);
			STREAM_WRITE_UINT8(&s, nonceAddress.address.proxy.uniqueID.bytes[1]);
			STREAM_WRITE_UINT8(&s, nonceAddress.address.proxy.uniqueID.bytes[2]);
			STREAM_WRITE_UINT8(&s, nonceAddress.address.proxy.uniqueID.bytes[3]);
			STREAM_WRITE_UINT8(&s, nonceAddress.address.proxy.uniqueID.bytes[4]);
		}
		else
		{
			for (int i = 0; i < 6; i++)
				STREAM_WRITE_UINT8(&s, 0x00);
			STREAM_WRITE_UINT16(&s, nonceAddress.address.nickname);
		}
	} //end construct the nonce

	if (encrypt)
	{
		uint8_t encipheredPayload[CRYPT_BUFF_SIZE];//contains auth + encyphered + MIC

		uint16_t messageSize = ccm.AuthEncrypt(session.sessionKey.key, nonce.data(), a.data(), (uint16_t) a.size(),
				input.data, input.dataLen, encipheredPayload);

		//the output will contains MIC + encipheredPayload
		memcpy((uint8_t*) output.data, encipheredPayload + (messageSize - 4), 4);
		memcpy(((uint8_t*) output.data) + 4, encipheredPayload + a.size(), messageSize - (a.size() + 4));
		output.dataLen = messageSize - a.size();

//		LOG_DEBUG("AuthEncrypt:" << " nonce=" << nlib::detail::BytesToString(nonce.data(), (int) nonce.size()) << " a="
//				<< nlib::detail::BytesToString(a.data(), (int) a.size()) << " input=" << nlib::detail::BytesToString(
//				input.data, input.dataLen) << " ouput=" << nlib::detail::BytesToString(output.data, output.dataLen));
	} else
	{
		//put MIC at the end
		uint8_t encipheredPayload[CRYPT_BUFF_SIZE];
		memcpy(encipheredPayload, input.data + 4, input.dataLen - 4);
		memcpy(encipheredPayload + (input.dataLen - 4), input.data, 4); //MIC

		uint8_t clearText[CRYPT_BUFF_SIZE];
		uint16_t clearTextSize = ccm.CheckAuthDecrypt(session.sessionKey.key, nonce.data(), a.data(), (uint16_t) a.size(),
				encipheredPayload, input.dataLen, clearText);

//		LOG_DEBUG("AuthDecrypt:" << " nonce=" << nlib::detail::BytesToString(nonce.data(), nonce.size()) << " a="
//				<< nlib::detail::BytesToString(a.data(), a.size()) << " input=" << nlib::detail::BytesToString(
//				encipheredPayload, input.dataLen));
		//<< " ouput=" << nlib::detail::BytesToString(output.data, output.dataLen));

		if (clearTextSize == 0)
		{
			return whartcsFailed_Authentication;
		}
		memcpy((uint8_t*) output.data, clearText, clearTextSize);
		output.dataLen = clearTextSize;

	}

	return whartcsSuccess;
}

int WHartNetworkData::GetServicePeriod(WHartServiceID serviceID, const WHartAddress& dest)
{
	ServiceTable::iterator it = FindService(serviceID, dest);
	if (it == serviceTable.end())
	{
		return -1;
	}

	return it->servicePeriod.u32 / 32;
}


void WHartNetworkData::Reset()
{
	sessionTable.clear();
	routeTable.clear();
	serviceTable.clear();
	routedHandlersTable.clear();

	nextRoutedHandler = 1;
	PDUTimeout = 0;
	TTL = 0x20; // default TTL

}

}// namespace network
}// namespace stack
}// namespace hart7
