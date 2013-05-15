/*
 * bytes_print.cpp
 *
 *  Created on: Sep 2, 2010
 *      Author: andy
 */

#include <sstream>
#include <iomanip>

#include "bytes_print.h"

namespace nlib {
namespace detail {

BytesToString::BytesToString(const boost::uint8_t* data_, int size_) : data(data_), size(size_)
{

}

std::string BytesToString::toString()
{
    std::ostringstream output;
    output << *this;

    return output.str();
}

std::ostream& operator<< (std::ostream& output, const BytesToString& bytes)
{
    output << std::hex << std::uppercase;
    output << '<';
    for (int i = 0; i < bytes.size; i++)
    {
        if (bytes.data[i] < 0x10)
            output << '0';
        output << (int) bytes.data[i] << ' ';
    }

    output << '>';
    return output;
}

}
}
