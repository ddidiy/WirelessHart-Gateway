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
 * WHartTypes.h
 *
 *  Created on: Nov 26, 2008
 *      Author: nicu.dascalu
 */

#ifndef WHART_TYPES_H_
#define WHART_TYPES_H_


#ifdef HAVE_STDINT
#	include <stdint.h>
#else
typedef unsigned char uint8_t;
typedef char int8_t;

typedef unsigned short uint16_t;
typedef short int16_t;

typedef unsigned int uint32_t;
typedef int int32_t;
#endif

typedef uint8_t bool_t;
static const bool_t BOOL_TRUE = 1;
static const bool_t BOOL_FALSE = 0;

#ifdef WIRELESS_HART_DEVICE
typedef uint8_t WHartCommandSize;
#else
typedef uint16_t WHartCommandSize;
#endif

typedef struct
{
	uint8_t bytes[5]; // is unsigned 40
} WHartUniqueID;




typedef uint16_t WHartShortAddress;//or nickname

/**
 *The Date consists of three 8-bit binary unsigned integers representing, respectively,
 *the day, month, and year minus 1900. Date is transmitted day first followed by the month and year bytes.
 */
typedef struct
{
	uint8_t day;
	uint8_t month;
	uint8_t year;
} WHartDate;

/**
 *The Time consists of a unsigned 32-bit binary integer,
 * with the least significant bit representing 1/32 of a millisecond (i.e., 0.03125 milliseconds).
 */
typedef union
{
	uint32_t u32;
	struct
	{
		uint32_t scalemiliseconds :5; //represents 1/32 of a millisecond
		uint32_t miliseconds :27;
	} tm;
} WHartTime;



struct DeviceIndicatedStatus
{
	uint8_t deviceStatus;
	uint8_t deviceExtendedStatus;

};

/**
 *
 */
typedef struct
{
	uint8_t hi;
	uint32_t u32;
} WHartTime40; // or ASN

#define RESET_WHART7_TIME40(value)\
{\
	(value)->hi = 0;\
	(value)->u32 = 0;\
}

#define RESET_WHART7_TIME40_RAW(var_time)\
{\
	memset(&var_time,0,5);\
}

typedef uint8_t _device_address_t[5];

//far back compatibility
typedef WHartCommandSize _command_size_t;
typedef WHartDate _date_t;
typedef WHartTime _time_t;
typedef uint8_t _time40_t[5];

#define MAX_VALUE(x, y) ((x > y) ? (x) : (y))
#define MIN_VALUE(x, y) ((x < y) ? (x) : (y))

#define COPY_FIXED_ARRAY(dest, src) memcpy(dest, src, MIN_VALUE(sizeof(src), sizeof(dest))) //need #include <stdio.h>
#define COMPARE_FIXED_ARRAY(left, right) memcmp(left, right, MIN_VALUE(sizeof(left), sizeof(right))) //need #include <stdio.h>
//extension for c++ only
#ifdef __cplusplus

#include <string>


namespace hart7 {
namespace stack {

WHartUniqueID MakeUniqueID(uint16_t deviceCode, uint32_t deviceID);
bool operator==(const WHartUniqueID& left, const WHartUniqueID& right);


bool operator==(const _device_address_t& left, const WHartUniqueID& right);
bool operator==(const WHartUniqueID& left, const _device_address_t& right);
bool operator<(const WHartUniqueID& left, const WHartUniqueID& right);


void ResetWHartTime40(WHartTime40& value);

std::string ToStringHexa(uint8_t value);
std::string ToStringHexa(uint16_t value);
std::string ToStringHexa(uint32_t value);

std::string ToString(WHartTime* value);
std::string ToString(WHartTime40* value);

class CompareUniqueID
{
public:
	bool operator()(const WHartUniqueID& p_oFirst, const WHartUniqueID& p_oSecond);

};

} // namespace stack
} // namespace hart7

std::ostream& operator<< (std::ostream& out, const WHartUniqueID& p_rUniqueId );

std::ostream& operator<<(std::ostream& out, const DeviceIndicatedStatus& status);

#endif //__cplusplus
#endif /* WHART_TYPES_H_ */
