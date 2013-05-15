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
 * C2009_NM_WriteUniqueIDNickname.h
 *
 *  Created on: Sep 1, 2009
 *      Author: andrei.petrut
 */

#ifndef C2009_NM_WRITEUNIQUEIDNICKNAME_H_
#define C2009_NM_WRITEUNIQUEIDNICKNAME_H_

enum
{
	CMDID_C2009_NM_WriteUniqueIdNickname = 2009
};

/**
 * Custom command sent to the GW stack to allow pairing of EUI64 with assigned Nicknames.
 */
struct C2009_NM_WriteUniqueIdNickname_Req
{
	WHartUniqueID uniqueId;
	WHartShortAddress nickname;

	bool isTR;
};

#endif /* C2009_NM_WRITEUNIQUEIDNICKNAME_H_ */
