#ifndef NLIB_BINARY_STDINT_H
#define NLIB_BINARY_STDINT_H

#include "adapter.h"

#include <boost/cstdint.hpp> //used for inttypes

namespace nlib {
namespace binary {

/**
 * Its just a tag class.
 */
struct BigEndian
{
};

/**
 * Its just a tag class.
 */
struct LitleEndian
{
};

template<typename Iter, typename IterTag>
inline
Range<Iter, IterTag> BinaryWrite(const Range<Iter, IterTag>& range, BigEndian* f, const boost::uint8_t& value)
{
	if (range.Distance() < 1)
		THROW_EXCEPTION0(BinaryException);

	Range<Iter, IterTag> r = range;
	*r.begin++ = value;

	return r;
}

template<typename Iter, typename IterTag>
inline
Range<Iter, IterTag>  BinaryWrite(const Range<Iter, IterTag>& range, BigEndian* f, const boost::uint16_t& value)
{
	if (range.Distance() < 2)
		THROW_EXCEPTION0(BinaryException);

	Range<Iter, IterTag> r = BinaryWrite(range, f, boost::uint8_t((value >> 8) & 0xff));
	return BinaryWrite(r, f, boost::uint8_t(value & 0xff));
}

template<typename Iter, typename IterTag>
inline
Range<Iter, IterTag> BinaryWrite(const Range<Iter, IterTag>& range, BigEndian* f, const boost::uint32_t& value)
{
	if (range.Distance() < 4)
		THROW_EXCEPTION0(BinaryException);

	Range<Iter, IterTag> r = BinaryWrite(range, f, boost::uint8_t((value >> 24) & 0xff));
	r = BinaryWrite(r, f, boost::uint8_t((value >> 16) & 0xff));
	r = BinaryWrite(r, f, boost::uint8_t((value >> 8) & 0xff));
	return BinaryWrite(r, f, boost::uint8_t(value & 0xff));
}


template<typename InIter>
inline
InIter BinaryRead(const InIter& inBegin, const InIter& inLast, BigEndian* f, boost::uint8_t& value)
{
	if (inBegin == inLast)
		THROW_EXCEPTION0(BinaryException);

	InIter it = inBegin;
	value = *it++;
	return it;
}

template<typename InIter>
inline
InIter BinaryRead(const InIter& inBegin, const InIter& inEnd, BigEndian* f, boost::uint16_t& value)
{
	boost::uint8_t b0;
	InIter it = BinaryRead(inBegin, inEnd, f, b0);

	boost::uint8_t b1;
	it = BinaryRead(it, inEnd, f, b1);

	value = (b0 << 8) | b1;
	return it;
}

template<typename InIter>
inline
InIter BinaryRead(const InIter& inBegin, const InIter& inEnd, BigEndian* f, boost::uint32_t& value)
{
	boost::uint8_t b0;
	InIter it = BinaryRead(inBegin, inEnd, f, b0);

	boost::uint8_t b1;
	it = BinaryRead(it, inEnd, f, b1);

	boost::uint8_t b2;
	it = BinaryRead(it, inEnd, f, b2);

	boost::uint8_t b3;
	it = BinaryRead(it, inEnd, f, b3);

	value = (b0 << 24) | (b1 << 16) | (b2 << 8) | b3;
	return it;
}

template<typename InIter, typename InIterTag>
inline
Range<InIter, InIterTag> BinaryRead(const Range<InIter, InIterTag>& inRange, BigEndian* f, boost::uint8_t& value)
{
	if (inRange.Distance() <= 0)
		THROW_EXCEPTION0(BinaryException);

	Range<InIter, InIterTag> range = inRange;
	value = *range.begin++;
	return range;
}
template<typename InIter, typename InIterTag>
inline
Range<InIter, InIterTag> BinaryRead(const Range<InIter, InIterTag> inRange, BigEndian* f, boost::uint16_t& value)
{
	boost::uint8_t b0;
	Range<InIter, InIterTag> it = BinaryRead(inRange, f, b0);

	boost::uint8_t b1;
	it = BinaryRead(it, f, b1);

	value = (b0 << 8) | b1;
	return it;
}

template<typename InIter, typename InIterTag>
inline
Range<InIter, InIterTag> BinaryRead(const Range<InIter, InIterTag> inRange, BigEndian* f, boost::uint32_t& value)
{
	boost::uint8_t b0;
	Range<InIter, InIterTag> it = BinaryRead(inRange, f, b0);

	boost::uint8_t b1;
	it = BinaryRead(it, f, b1);

	boost::uint8_t b2;
	it = BinaryRead(it, f, b2);

	boost::uint8_t b3;
	it = BinaryRead(it, f, b3);

	value = (b0 << 24) | (b1 << 16) | (b2 << 8) | b3;
	return it;
}

} //namespace binary
} //namespace nlib


#endif /*NLIB_BINARY_STDINT_H*/
