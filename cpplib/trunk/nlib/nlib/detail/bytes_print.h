/*
 * bytes_print.h
 *
 *  Created on: Nov 21, 2008
 *      Author: nicu.dascalu
 */

#ifndef BYTES_PRINT_H_
#define BYTES_PRINT_H_

#include <sstream>
#include <iomanip>
#include <boost/cstdint.hpp>

namespace nlib {
namespace detail {

struct BytesToString
{
    public:
    BytesToString(const boost::uint8_t* data_, int size_);
    std::string toString();

    public:
        const boost::uint8_t* data;
        int size;


    friend std::ostream& operator<< (std::ostream& output, const BytesToString& bytes);
};

std::ostream& operator<< (std::ostream& output, const BytesToString& bytes);

} //namesapce detail
} //namespace nlib


#endif /* BYTES_PRINT_H_ */
