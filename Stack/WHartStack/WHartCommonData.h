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
 * CommonData.h
 *
 *  Created on: Dec 11, 2008
 *      Author: Andy
 */

#ifndef WHART_COMMONDATA_H_
#define WHART_COMMONDATA_H_

#include <WHartStack/WHartStack.h>

namespace hart7 {
namespace stack {

class CommonData
{
public:
	const static uint16_t MAX_PAYLOAD = 1024;

	WHartUniqueID myUniqueID;
	WHartShortAddress myNickname;

	//HACK:[andrei.petrut] - save this so that we can send it in the next packet, until a real ASN is kept
	uint32_t lastReadASN;

	bool isJoined;

	CommonData()
	{
		myNickname = No_Nickname();
		isJoined = false;
		lastReadASN = 0;
	}

	WHartTime40 CurrentTime40() const
	{
		WHartTime40 t;
		t.hi = 0;
		t.u32 = 0;

		t.u32 = lastReadASN;//HACK [andrei.petrut] - until a real time exists
		return t;
	}

	bool IsGateway()
	{
		return myUniqueID == Gateway_UniqueID() || myNickname == Gateway_Nickname();
	}

	bool IsNetworkManager()
	{
		return myUniqueID == NetworkManager_UniqueID() || myNickname == NetworkManager_Nickname();
	}

};

}
}

#endif /* WHART_COMMONDATA_H_ */
