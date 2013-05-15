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

#ifndef BINARIZATION_H_
#define BINARIZATION_H_

#include "WirelessStackTypes.h"
#include <string.h>

typedef enum
{
	//operation succeeds
	BinaryStream_Success = 0,
	//last operation failed, to few bytes written or read from stream
	BinaryStream_Err_TooFewBytes = 1
} BinaryStreamErrors;

/**
 * Abstracts a stream of bytes. Provides the reading/writing primitive types (int8,16,24,32,strings,bytes, etc.)
 *  in little-endian or big-endian bytes order.
 *  To enable little-endian add a #define IS_MACHINE_BIG_ENDIAN
 *
 *  The stream hold a single pointer for writing or reading(Normaly will be used only for reading or only for writing);
 *  If you want to have diffrent pointer fro read & read declar 2 stream over the same buffer.
 */
typedef struct
{
	/** holds a pointer of the next written/read byte from the stream */
	uint8_t* nextByte;
	/** holds the available bytes to read from the stream*/
	uint16_t remainingBytes;

#if defined(_DEBUG)
	//TODO [nicu.dascalu] - add another info for debug
	BinaryStreamErrors error; //holds the latest error operation
#endif
} BinaryStream;

/**
 * Internaly used. for converting int 2 bytes.
 */
typedef union
{
	uint8_t u8;
	uint16_t u16;
	uint32_t u32;
	float fl;
	uint8_t bytes[4];
} Binarization_Type2Bytes;

#ifndef internal_nocall_
#		define internal_nocall_ (void)0
#endif

/**
 * Used just for debugging purpose, for assembly generated code.
 */
//#ifdef _DEBUG
//#	define STREAM_PRINT_FUNC_NAME(name) {char macro_##name; macro_##name;}
//#else
#	define STREAM_PRINT_FUNC_NAME(name)
//#endif

/**
 * Initialize the stream. This method should be called before using the stream (is like a c++ ctor).
 * Alos can be called more than once (second time will reset the stream)
 */
#define STREAM_INIT(stream, buffer, bufferLength)\
	{\
		STREAM_PRINT_FUNC_NAME(STREAM_INIT) \
		(stream)->nextByte = (uint8_t*)(buffer);\
		(stream)->remainingBytes = (bufferLength);\
		STREAM_RESET_ERROR(stream);\
	}

/**
 * Stream error code access macros.
 */
#ifdef _DEBUG
#	define STREAM_RESET_ERROR(stream)	(stream)->error = BinaryStream_Success

#	define STREAM_SET_ERROR_ON_COND(stream, err, cond)\
		if (cond) (stream)->error = (err)

#	define STREAM_GET_ERROR(stream) (stream)->error

#else
#	define STREAM_RESET_ERROR(stream) internal_nocall_
#	define STREAM_SET_ERROR_ON_COND(stream, err, cond) internal_nocall_
# define STREAM_GET_ERROR(stream) BinaryStream_Success
#endif

/**
 * Navigate forward into stream.
 */
#define STREAM_SKIP(stream, count)\
	{\
		STREAM_PRINT_FUNC_NAME(STREAM_SKIP) \
		STREAM_SET_ERROR_ON_COND(stream, BinaryStream_Err_TooFewBytes, count > (stream)->remainingBytes);\
		(stream)->nextByte += count;\
		(stream)->remainingBytes -= count;\
	}

/*
 #define STREAM_CHECK_AVAILABLE(stream, availableBytes)\
	if ((stream)->remainingBytes < availableBytes)\
		return RCS_E05_TooFewDataBytesReceived; //HARCODED [nicu.dascalu]
 */

/**
 * Writes bytes into stream. The stream memory should not overlap with provided buffer.
 * @stream - pointer to writable stream
 * @bytes - address of starting bytes buffer
 * @length - number of bytes from buffer that should be written
 */
#define STREAM_WRITE_BYTES(stream, bytes, length)\
	{\
		STREAM_PRINT_FUNC_NAME(STREAM_WRITE_BYTES) \
		memcpy((stream)->nextByte, (void*)(bytes), (length));\
		STREAM_SKIP(stream, length);\
	}
/**
 * Read bytes from stream.. The stream memory should not overlap with provided buffer.
 * @stream - pointer to readable stream
 * @bytes - address of starting bytes buffer
 * @length - number of bytes from buffer that should be written
 */
#define STREAM_READ_BYTES(stream, bytes, length)\
	{\
		STREAM_PRINT_FUNC_NAME(STREAM_READ_BYTES) \
		memcpy((void*)(bytes), (stream)->nextByte, (length));\
		STREAM_SKIP(stream, length);\
	}
/**
 * Write an effective uint8 into stream.
 */
#define STREAM_WRITE_UINT8(stream, value)\
	{\
		STREAM_PRINT_FUNC_NAME(STREAM_WRITE_UINT8) \
		*(stream)->nextByte++ = value;\
		(stream)->remainingBytes--;\
	}

#define STREAM_PEEK_UINT8(stream, steps, value)\
	{\
		STREAM_PRINT_FUNC_NAME(STREAM_PEEK_UINT8) \
		*(value) = *((stream)->nextByte + steps);\
	}
/**
 * Read an uint8 from the stream.
 */
#define STREAM_READ_UINT8(stream, value)\
	{\
		STREAM_PRINT_FUNC_NAME(STREAM_READ_UINT8) \
		*(value) = *(stream)->nextByte++;\
		(stream)->remainingBytes--;\
	}

#if defined(IS_MACHINE_BIG_ENDIAN) && defined(IS_MACHINE_LITTLE_ENDIAN)
#	error "Only one byte order(IS_MACHINE_BIG_ENDIAN or IS_MACHINE_LITTLE_ENDIAN) should be defined!"

#elif !defined(IS_MACHINE_BIG_ENDIAN) && !defined(IS_MACHINE_LITTLE_ENDIAN)
#	error "No byte order(IS_MACHINE_LITTLE_ENDIAN or IS_MACHINE_BIG_ENDIAN) specified!"

#elif defined(IS_MACHINE_BIG_ENDIAN) //or network order
//#	warning "Binarization will use IS_MACHINE_BIG_ENDIAN"

/**
 * Writes an effective uint16 value.
 */
#	define STREAM_WRITE_UINT16(stream, value)\
	{\
		STREAM_PRINT_FUNC_NAME(STREAM_WRITE_UINT16_bigendian) \
		uint16_t v = (uint16_t)(value); /*write efective values as uint16*/ \
		memcpy((stream)->nextByte, &v, 2);\
		STREAM_SKIP(stream, 2);\
	}
/**
 * Reads effective uint16 value.
 */
#	define STREAM_READ_UINT16(stream, value)\
	{\
		STREAM_PRINT_FUNC_NAME(STREAM_READ_UINT16_bigendian) \
		uint16_t v; /*just protect written memory*/\
		memcpy(&v, (stream)->nextByte, 2);\
		STREAM_SKIP(stream, 2);\
		*(value) = v;\
	}

#	define STREAM_WRITE_UINT24(stream, value)\
	{\
		STREAM_PRINT_FUNC_NAME(STREAM_WRITE_UINT24_bigendian) \
		uint32_t v = ((uint32_t)(value)) << 8; /*write efective values as uint24*/ \
		memcpy((stream)->nextByte, &v, 3);\
		STREAM_SKIP(stream, 3);\
	}
//the most significant byte will be always 0 (or ignored)
#	define STREAM_READ_UINT24(stream, value)\
	{\
		STREAM_PRINT_FUNC_NAME(STREAM_READ_UINT24_bigendian) \
		uint32_t v;/*just protect written memory*/ \
		memcpy(&v, (stream)->nextByte, 3);\
		STREAM_SKIP(stream, 3);\
		*(value) = ((uint32_t)v) >> 8;\
	}

#	define STREAM_WRITE_UINT32(stream, value)\
	{\
		STREAM_PRINT_FUNC_NAME(STREAM_WRITE_UINT32_bigendian) \
		uint32_t v = (uint32_t)(value);/*write efective values as uint32*/\
		memcpy((stream)->nextByte, &v, 4);\
		STREAM_SKIP(stream, 4);\
	}

#define STREAM_READ_UINT32(stream, value)\
	{\
		STREAM_PRINT_FUNC_NAME(STREAM_READ_UINT32_bigendian) \
		uint32_t v;/*just protect written memory*/\
		memcpy(&v, (stream)->nextByte, 4);\
		STREAM_SKIP(stream, 4);\
		*(value) = v;\
	}

#elif defined(IS_MACHINE_LITTLE_ENDIAN)
//LITTLE_ENDIAN or intel order
//#	warning "Binarization will use IS_MACHINE_LITTLE_ENDIAN"


#	define STREAM_WRITE_UINT16(stream, value)\
	{\
		STREAM_PRINT_FUNC_NAME(STREAM_WRITE_UINT16_littleendian) \
		Binarization_Type2Bytes i2b;\
		i2b.u16 = value;\
		*(stream)->nextByte = i2b.bytes[1];\
		*((stream)->nextByte + 1) = i2b.bytes[0];\
		STREAM_SKIP(stream, 2);\
	}

#	define STREAM_READ_UINT16(stream, value)\
	{\
		STREAM_PRINT_FUNC_NAME(STREAM_READ_UINT16_littleendian) \
		Binarization_Type2Bytes i2b;\
		i2b.bytes[1] = *(stream)->nextByte;\
		i2b.bytes[0] = *((stream)->nextByte + 1);\
		STREAM_SKIP(stream, 2);\
		*(value) = i2b.u16;\
	}

#	define STREAM_WRITE_UINT24(stream, value)\
	{\
		STREAM_PRINT_FUNC_NAME(STREAM_WRITE_UINT24_littleendian) \
		Binarization_Type2Bytes i2b;\
		i2b.u32 = value;\
		*(stream)->nextByte = i2b.bytes[2];\
		*((stream)->nextByte + 1) = i2b.bytes[1];\
		*((stream)->nextByte + 2) = i2b.bytes[0];\
		STREAM_SKIP(stream, 3);\
	}

#	define STREAM_READ_UINT24(stream, value)\
	{\
		STREAM_PRINT_FUNC_NAME(STREAM_READ_UINT24_littleendian) \
		Binarization_Type2Bytes i2b;\
		i2b.bytes[3] = 0;\
		i2b.bytes[2] = *(stream)->nextByte;\
		i2b.bytes[1] = *((stream)->nextByte + 1);\
		i2b.bytes[0] = *((stream)->nextByte + 2);\
		STREAM_SKIP(stream, 3);\
		*(value) = i2b.u32;\
	}

#	define STREAM_WRITE_UINT32(stream, value)\
	{\
		STREAM_PRINT_FUNC_NAME(STREAM_WRITE_UINT32_littleendian) \
		Binarization_Type2Bytes i2b;\
		i2b.u32 = value;\
		*(stream)->nextByte = i2b.bytes[3];\
		*((stream)->nextByte + 1) = i2b.bytes[2];\
		*((stream)->nextByte + 2) = i2b.bytes[1];\
		*((stream)->nextByte + 3) = i2b.bytes[0];\
		STREAM_SKIP(stream, 4);\
	}

#define STREAM_READ_UINT32(stream, value)\
	{\
		STREAM_PRINT_FUNC_NAME(STREAM_READ_UINT32_littleendian) \
		Binarization_Type2Bytes i2b;\
		i2b.bytes[3] = *(stream)->nextByte;\
		i2b.bytes[2] = *((stream)->nextByte + 1);\
		i2b.bytes[1] = *((stream)->nextByte + 2);\
		i2b.bytes[0] = *((stream)->nextByte + 3);\
		STREAM_SKIP(stream, 4);\
		*(value) = i2b.u32;\
	}

#endif

#define STREAM_WRITE_INT8(stream, value) STREAM_WRITE_UINT8(stream, (uint8_t)value)
#define STREAM_READ_INT8(stream, value)\
	{\
		uint8_t v; /*just protect written value memory*/ \
		STREAM_READ_UINT8(stream, &v);\
	 *(value) = (int8_t)v;\
	}

#define STREAM_WRITE_INT16(stream, value) STREAM_WRITE_UINT16(stream, (uint16_t)value)
#define STREAM_READ_INT16(stream, value)\
	{\
		uint16_t v; /*just protect written value memory*/ \
		STREAM_READ_UINT16(stream, &v);\
	 *(value) = (int16_t)v;\
	}

#define STREAM_WRITE_INT24(stream, value) STREAM_WRITE_UINT24(stream, (uint32_t)value)
#define STREAM_READ_INT24(stream, value)\
	{\
		uint32_t v; /*just protect written value memory*/ \
		STREAM_READ_UINT24(stream, &v);\
	 *(value) = (int32_t)v;\
	}

#define STREAM_WRITE_INT32(stream, value) STREAM_WRITE_UINT32(stream, (uint32_t)value)
#define STREAM_READ_INT32(stream, value)\
	{\
		uint32_t v; /*just protect written memory*/ \
		STREAM_READ_UINT32(stream, &v);\
	 *(value) = (int32_t)v;\
	}

#define STREAM_WRITE_TIME(stream, value)\
	{\
		STREAM_PRINT_FUNC_NAME(STREAM_WRITE_TIME) \
		STREAM_WRITE_UINT32(stream, value.u32);\
	}

#define STREAM_READ_TIME(stream, value)\
	{\
		STREAM_PRINT_FUNC_NAME(STREAM_READ_TIME) \
		STREAM_READ_UINT32(stream, &(value)->u32);\
	}

#define STREAM_WRITE_DATE(stream, value) \
	{\
		STREAM_PRINT_FUNC_NAME(STREAM_WRITE_DATE) \
		STREAM_WRITE_UINT8(stream, value.day);\
		STREAM_WRITE_UINT8(stream, value.month);\
		STREAM_WRITE_UINT8(stream, value.year);\
	}

#define STREAM_READ_DATE(stream, value) \
	{\
		STREAM_PRINT_FUNC_NAME(STREAM_READ_DATE) \
		STREAM_READ_UINT8(stream, &(value)->day);\
		STREAM_READ_UINT8(stream, &(value)->month);\
		STREAM_READ_UINT8(stream, &(value)->year);\
	}

/*
 * A string using the 8-bit ISO Latin-1 character set. Latin-1 strings are padded out
 *	with zeroes (0x00).
 */
#define STREAM_WRITE_LATIN(stream, str, length) STREAM_WRITE_BYTES(stream, str, length)
#define STREAM_READ_LATIN(stream, str, length) STREAM_READ_BYTES(stream, str, length)

/*
 * A string consisting of 6-bit alpha-numeric characters that are a subset of the
 *	ASCII character set. This allows four characters to be packed into three bytes.
 *	Packed ASCII strings are padded out with space (0x20) characters.
 * The memory shouldn't be overlapped.
 * length must be a constant (such as sizeof()); it represents the no of unpacked chars
 *      and must be multiple of 4, while the consumed stream bytes will be a multiple of 3.
 */
#define STREAM_READ_PACKED(stream, bytes, length)\
	{\
		STREAM_PRINT_FUNC_NAME(STREAM_READ_PACKED) \
		STREAM_SET_ERROR_ON_COND(stream, BinaryStream_Err_TooFewBytes, (length%4)!=0);\
/* doinel.alban		uint8_t str[len];*/\
		uint8_t str[length];\
		memcpy(str, (stream)->nextByte, length);\
		uint8_t pack;\
		uint8_t i=0, j=0;\
		for (i=0,j=0;j+4<=length; i=i+3, j=j+4) {\
			pack = str[i]>>2;\
			if(pack != 0x20)\
				pack |= 0x40;\
			bytes[j] = pack;\
			pack = (((str[i]<<4) & 0x3F) | (str[i+1]>>4));\
			if(pack != 0x20)\
				pack |= 0x40;\
			bytes[j+1] = pack;\
			pack = (((uint8_t)(str[i+1]<<2)) & 0x3F) | ((uint8_t)(str[i+2]>>6));\
			if(pack != 0x20)\
				pack |= 0x40;\
			bytes[j+2] = pack;\
			pack = ((uint8_t)(str[i+2]& 0x3F));\
			if(pack != 0x20)\
				pack |= 0x40;\
			bytes[j+3] = pack;\
		}\
		bytes[length-1]=0x00;\
		STREAM_SKIP(stream, (uint8_t)(length * 6 /8));\
	}

#define STREAM_WRITE_PACKED(stream, bytes, length)\
	{\
		STREAM_PRINT_FUNC_NAME(STREAM_WRITE_PACKED) \
		uint8_t ch[3]={'?','?','?'};\
		STREAM_SET_ERROR_ON_COND(stream,BinaryStream_Err_TooFewBytes, (length%4)!=0);\
		uint8_t i=0, j=0;\
		for (i=0;i<length;i++) {\
			if(bytes[i]==0x00){\
				break;\
			}\
		}\
		for (j=i;j<length;j++)\
			bytes[j]=0x20;\
		for (i=0;i<length;i=i+4) {\
			ch[0] = (uint8_t)((bytes[i]&0x3F)<<2) | ((bytes[i+1]&0x3F)>>4);\
			ch[1] = (uint8_t)((bytes[i+1]&0x3F)<<4) | ((bytes[i+2]&0x3F)>>2);\
			ch[2] = (uint8_t)((bytes[i+2]&0x3F)<<6) | (bytes[i+3]&0x3F);\
			memcpy((stream)->nextByte, ch, 3);\
                        STREAM_SKIP(stream, 3);\
		}\
		bytes[length-1]=0x00;\
	}
//FLOAT binarization
#include <float.h>

#if defined(IS_MACHINE_BIG_ENDIAN)

#	define STREAM_WRITE_FLOAT(stream, value)\
	{\
		STREAM_PRINT_FUNC_NAME(STREAM_WRITE_UINT32_bigendian)\
		float v = (float)(value);/*just protect written memory*/\
		memcpy((stream)->nextByte, &v, 4);\
		STREAM_SKIP(stream, 4);\
	}

#	define STREAM_READ_FLOAT(stream, value)\
	{\
		STREAM_PRINT_FUNC_NAME(STREAM_READ_UINT32_bigendian)\
		float v;/*just protect written memory*/\
		memcpy(&v, (stream)->nextByte, 4);\
		STREAM_SKIP(stream, 4);\
		*(value) = v;\
	}

#elif defined(IS_MACHINE_LITTLE_ENDIAN)

#	define STREAM_WRITE_FLOAT(stream, value)\
	{\
		STREAM_PRINT_FUNC_NAME(STREAM_WRITE_FLOAT_littleendian) \
		Binarization_Type2Bytes i2b;\
		i2b.fl = (value);\
		*(stream)->nextByte = i2b.bytes[3];\
		*((stream)->nextByte + 1) = i2b.bytes[2];\
		*((stream)->nextByte + 2) = i2b.bytes[1];\
		*((stream)->nextByte + 3) = i2b.bytes[0];\
		STREAM_SKIP(stream, 4);\
	}

#define STREAM_READ_FLOAT(stream, value)\
	{\
		STREAM_PRINT_FUNC_NAME(STREAM_READ_FLOAT_littleendian) \
		Binarization_Type2Bytes i2b;\
		i2b.bytes[3] = *(stream)->nextByte;\
		i2b.bytes[2] = *((stream)->nextByte + 1);\
		i2b.bytes[1] = *((stream)->nextByte + 2);\
		i2b.bytes[0] = *((stream)->nextByte + 3);\
		STREAM_SKIP(stream, 4);\
		*(value) = i2b.fl;\
	}

#endif

#endif /*BINARIZATION_H_*/
