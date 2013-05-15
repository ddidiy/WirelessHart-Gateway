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

#ifndef	GATEWAYPROCESSOR_H_
#define	GATEWAYPROCESSOR_H_

#include <WHartStack/WHartStack.h>
#include <WHartStack/CommInterfaceOverIP/UDPClientIP.h>
#include <WHartStack/util/WHartCmdWrapper.h>
#include <WHartHost/gateway/GatewayIO.h>



#include <iomanip>
#include <netinet/in.h>



#define WHART_DEV_ADDRESS_SIZE				(5*sizeof(uint8_t))
#define WHART_CMD_NO_SIZE					sizeof(uint16_t)
#define WHART_DATA_LENGTH_SIZE				sizeof(uint8_t)
#define WHART_RESPONSE_CODE_SIZE			sizeof(uint8_t)
#define WHART_DEVICE_STATUS_SIZE			sizeof(uint8_t)


//unserialize standard packet
//{
#define WHART_DEV_ADDRESS_OFFSET			0 
#define WHART_CMD_NO_OFFSET					(WHART_DEV_ADDRESS_OFFSET + WHART_DEV_ADDRESS_SIZE)
#define WHART_DATA_LENGTH_OFFSET			(WHART_CMD_NO_OFFSET + WHART_CMD_NO_SIZE)
#define WHART_RESPONSE_CODE_OFFSET			(WHART_DATA_LENGTH_OFFSET + WHART_DATA_LENGTH_SIZE)
#define WHART_DEVICE_STATUS_OFFSET			(WHART_RESPONSE_CODE_OFFSET + WHART_RESPONSE_CODE_SIZE)
#define WHART_RESP_DATA_BUFFER_OFFSET		(WHART_DEVICE_STATUS_OFFSET + WHART_DEVICE_STATUS_SIZE)
//}

#define META_WHART_DEV_EXT_TYEPE_SIZE		sizeof(uint16_t)

//unserialize meta response packet
//{
#define META_WHART_DEV_ADDRESS_OFFSET				0
#define META_WHART_CMD_NO_OFFSET					(META_WHART_DEV_ADDRESS_OFFSET + WHART_DEV_ADDRESS_SIZE)
#define META_WHART_DATA_LENGTH_OFFSET				(META_WHART_CMD_NO_OFFSET + WHART_CMD_NO_SIZE)
#define META_WHART_RESPONSE_CODE_OFFSET				(META_WHART_DATA_LENGTH_OFFSET + WHART_DATA_LENGTH_SIZE)
#define META_WHART_DEVICE_STATUS_OFFSET				(META_WHART_RESPONSE_CODE_OFFSET + WHART_RESPONSE_CODE_SIZE)
#define META_INNER_WHART_DEV_EXT_TYPE_OFFSET		(META_WHART_DEVICE_STATUS_OFFSET + WHART_DEVICE_STATUS_SIZE)
#define META_INNER_WHART_DEV_ADDRESS_OFFSET			(META_INNER_WHART_DEV_EXT_TYPE_OFFSET + META_WHART_DEV_EXT_TYEPE_SIZE)
#define META_INNER_WHART_CMD_NO_OFFSET				(META_INNER_WHART_DEV_ADDRESS_OFFSET + WHART_DEV_ADDRESS_SIZE)
#define META_INNER_WHART_DATA_LENGTH_OFFSET			(META_INNER_WHART_CMD_NO_OFFSET + WHART_CMD_NO_SIZE)
#define META_INNER_WHART_RESPONSE_CODE_OFFSET		(META_INNER_WHART_DATA_LENGTH_OFFSET + WHART_DATA_LENGTH_SIZE)
#define META_INNER_WHART_DEVICE_STATUS_OFFSET		(META_INNER_WHART_RESPONSE_CODE_OFFSET + WHART_RESPONSE_CODE_SIZE)
#define META_INNER_WHART_RESP_DATA_BUFFER_OFFSET	(META_INNER_WHART_DEVICE_STATUS_OFFSET + WHART_DEVICE_STATUS_SIZE)
//}


//our definition
#define NotificationMaskCodesMask_ResponseReceived	0x4000



namespace hart7 {
namespace gateway {



// 0  - ok
// <0 - not ok
int SerializeWHCommand(const WHartUniqueID &whAddr, const stack::WHartCommand &whCmd, 
							stack::transport::HARTDirectPDU_Request &req);

/*
 * Visitor Pattern
 */
class MessageChecker:public stack::transport::IMessageIPVisitor
{
private:
	bool m_isChangeNotification;
	unsigned short *m_pNotificationMask;
	hostapp::AppNoBurstRspNotification::CMD_Type *m_cmdType;
	WHartUniqueID *m_pAddr;
	bool* m_pIsMetaPacket;

public:
	bool IsChangeNotificationMsg(const stack::transport::MessageIP& packet, WHartUniqueID& addr/*[out]*/, 
			unsigned short &notificationMask/*[out]*/, 
			hostapp::AppNoBurstRspNotification::CMD_Type &cmdType/*[out]*/, bool &isMetaPacket/*out*/);

private:
	virtual void Visit(stack::transport::HARTDirectPDU_Notify& notif);
};


/*
 * Visitor Pattern
 */
class MsgUnserializer: public stack::transport::IMessageIPVisitor
{
private:
	stack::CHartCmdWrapper	*m_pParser;
	stack::WHartCommand		*m_pWHCmd;
	int						m_error;

public:
	/*
	 * 0  success
	 * -1 failed
	 */
	int Unserialize(const stack::transport::MessageIP& packet, stack::CHartCmdWrapper &parser, 
						stack::WHartCommand &whCmd /*[in/out]*/);
	
private:
	void UnserializeHARTDirectPDUMessage(stack::transport::MessageIP& msg, bool p_bIsNotify = false);

	virtual void Visit(stack::transport::HARTDirectPDU_Response& res);
	virtual void Visit(stack::transport::HARTDirectPDU_Notify& notif);
};


/*
 * Visitor Pattern
 */
class MsgSerializer: public stack::transport::IMessageIPVisitor
{
private:
	stack::CHartCmdWrapper	*m_pParser;
	stack::WHartCommand		m_whCmd;
	WHartUniqueID			m_addr;
	int						m_error;


public:
	/*
	 * 0  success
	 * -1 failed
	 */
	int Serialize(const WHartUniqueID &addr, const stack::WHartCommand &whCmd, stack::CHartCmdWrapper &parser, 
						stack::transport::MessageIP& packet/*[in/out]*/);

private:
	virtual void Visit(stack::transport::HARTDirectPDU_Request& req);
};




}
}

#endif	
