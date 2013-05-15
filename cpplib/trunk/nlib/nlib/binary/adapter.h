#ifndef NLIB_BINARY_STREAM_H_
#define NLIB_BINARY_STREAM_H_

#include <nlib/exception.h>
#include <iterator>
#include <climits>

namespace nlib {
namespace binary {

class BinaryException: public Exception
{
public:
};

/**
 * this template class should not be instanciated
 */
template<typename Iter, typename IterTag>
class Range
{
private:
	typedef Iter Iterator;
	typedef IterTag IteratorTag;

private:
	Range();
	std::size_t Distance() const;
};

/**
 * Just interface for writting a value to the range using the Format.
 * @return the begin of output iter range
 * @throws BinaryException when we try to write out of the output range.
 */
template<typename OutIter, typename Format, typename ValueType>
OutIter BinaryWrite(const OutIter& outIt, Format* format, const ValueType& value);

/**
 * Just interface for reading a from the input range [begin, end) using the Foramt.
 * @throws BinaryException when try to read from out of the input range
 */
template<typename InIter, typename Format, typename ValueType>
InIter BinaryRead(const InIter& begin, const InIter& end, Format* format, ValueType& value);

/**
 * Adaptees/Converts the provided values throug the specified formater, and writes to the output iterator.
 */
template<typename OutIter, typename OutIterTag = typename OutIter::iterator_category>
class BinaryWriter
{
public:
	explicit BinaryWriter(const OutIter& begin) :
		outRange(begin)
	{
	}

	explicit BinaryWriter(const OutIter& begin, const OutIter& end) :
		outRange(begin, end)
	{
	}

	template<typename Format, typename ValueType>
	BinaryWriter& Write(const ValueType& value)
	{
		outRange = BinaryWrite(outRange, (Format*) 0, value);
		return *this;
	}

	template<typename Format, typename InIter>
	BinaryWriter& Write(const InIter& inFirst, const InIter& inLast)
	{
		for (InIter it = inFirst; it != inLast; it++)
			Write<Format> (*it);

		return *this;
	}

private:
	Range<OutIter, OutIterTag> outRange;
};

/**
 *
 */
template<typename InIter>
class BinaryReader
{
public:
	explicit BinaryReader(const InIter& begin, const InIter& end)
	: inRange(begin, end)
	{
	}

	explicit BinaryReader(const InIter& begin)
	: inRange(begin)
	{
	}

//	void Reset(const InIter& begin_, const InIter& end_)
//	{
//		inBegin = begin_;
//		inEnd = end_;
//	}

	template<typename Format, typename ValueType>
	BinaryReader& Read(ValueType& value)
	{
		inRange = BinaryRead(inRange, (Format*) 0, value);
		return *this;
	}

	template<typename Format, typename ValueType>
	ValueType Read()
	{
		ValueType value;
		Read<Format> (value);

		return value;
	}

	template<typename Format, typename OutIter>
	BinaryReader& Read(const OutIter& outFirst, const OutIter& outLast)
	{
		for (OutIter it = outFirst; it != outLast; it++)
		{
			*it = Read<Format, typename OutIter::value_type> ();
		}
		return *this;
	}

private:
	Range<InIter, typename InIter::iterator_category> inRange;

	//InIter inBegin, inEnd;
};


/**
 * Input iterator range specialization
 */
template<typename Iter>
class Range<Iter, std::input_iterator_tag>
{
public:
	typedef Iter Iterator;
	typedef std::input_iterator_tag IteratorTag;

public:
	Range(const Iter& begin_) :
		begin(begin_)
	{
	}

	std::size_t Distance() const
	{
		return ULONG_MAX;
	}

	Iter begin;
};

/**
 * Output iterator range specialization
 */
template<typename Iter>
class Range<Iter, std::output_iterator_tag>
{
public:
	typedef Iter Iterator;
	typedef std::output_iterator_tag IteratorTag;

public:
	Range(const Iter& begin_) :
		begin(begin_)
	{
	}

	std::size_t Distance() const
	{
		return ULONG_MAX;
	}

	Iter begin;
};

/**
 * Forward iterator range specialization
 */
template<typename Iter>
class Range<Iter, std::forward_iterator_tag>
{
public:
	typedef Iter Iterator;
	typedef std::forward_iterator_tag IteratorTag;

public:
	Range(const Iter& begin_, const Iter& end_) :
		begin(begin_), end(end_)
	{
	}

	std::size_t Distance() const
	{
		return std::distance(begin, end);
	}

	Iter begin, end;
};

/**
 * Forward iterator range specialization
 */
template<typename Iter>
class Range<Iter, std::random_access_iterator_tag>
{
public:
	typedef Iter Iterator;
	typedef std::random_access_iterator_tag IteratorTag;

public:
	Range(const Iter& begin_, const Iter& end_) :
		begin(begin_), end(end_)
	{
	}

	std::size_t Distance() const
	{
		return std::distance(begin, end);
	}

	Iter begin, end;
};

} //namespace binary
} //namespace nlib


#endif /*NLIB_BINARY_STREAM_H_*/
