#include <string>
#include <deque>
#include <iterator> //for back_insert_iterator
#include <iostream> // for cout

#include <nlib/binary/stdint.h>

using nlib::binary::BigEndian;



inline
void test_BinaryWriterOnOutputIterator()
{
	typedef std::basic_string<boost::uint8_t> ContainerWriter;
	typedef nlib::binary::BinaryWriter<std::back_insert_iterator<ContainerWriter> > BinaryWriter;

	ContainerWriter w;
	BinaryWriter bw(std::back_inserter(w));

	bw.Write<BigEndian>((boost::uint16_t)0x2);
	bw.Write<BigEndian>((boost::uint8_t)0x80);
	bw.Write<BigEndian>((boost::uint32_t)0x01020304);

	std::cout << std::endl << "test_BinaryWriterOnOutputIterator:";
	for (int i = 0; i < (int)w.size(); i++)
	{
		std::cout << (int)w[i] << " ";
	}
}

inline
void test_BinaryWriterOnForwardIterator()
{
	typedef std::basic_string<boost::uint8_t> ContainerWriter;

	typedef nlib::binary::BinaryWriter<ContainerWriter::iterator> BinaryWriter;

	ContainerWriter w;
	w.resize(2 + 1 + 4);
	BinaryWriter bw(w.begin(), w.end());

	bw.Write<BigEndian>((boost::uint16_t)0x2);
	bw.Write<BigEndian>((boost::uint8_t)0x80);
	bw.Write<BigEndian>((boost::uint32_t)0x01020304);

	std::cout << std::endl << "test_BinaryWriterOnForwardIterator:";
	for (int i = 0; i < (int)w.size(); i++)
	{
		std::cout << (int)w[i] << " ";
	}
}

inline
void test_BinaryReaderOnForwardIterator()
{
	typedef std::deque<boost::uint8_t> ContainerReader;
	typedef nlib::binary::BinaryReader<ContainerReader::iterator> BinaryReader;

	ContainerReader r;
	boost::uint8_t arr[] = { 0, 0, 0x80, 0x1, 0x2, 0x3, 0x4 };
	r.insert(r.end(), arr, arr + sizeof(arr));
	BinaryReader br(r.begin(), r.end());

	boost::uint16_t one = br.Read<BigEndian, boost::uint16_t>();
	boost::uint8_t two = br.Read<BigEndian, boost::uint8_t>();
	boost::uint32_t three = br.Read<BigEndian, boost::uint32_t>();

	std::cout << std::endl << "test_BinaryReaderOnForwardIterator:" << std::hex
		<< " one=" << (int)one
		<< " two=" << (int)two
		<< " three=" << (int)three;
}

inline
void test_binary_adapter()
{
	test_BinaryWriterOnOutputIterator();
	test_BinaryWriterOnForwardIterator();
	test_BinaryReaderOnForwardIterator();
}

