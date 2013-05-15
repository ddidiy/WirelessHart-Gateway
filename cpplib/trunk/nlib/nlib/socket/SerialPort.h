/*
 * SerialPort.h
 *
 *  Created on: Jan 13, 2009
 *      Author: nicu.dascalu
 */

#ifndef NLIB_SERIALPORT_H_
#define NLIB_SERIALPORT_H_

#include <nlib/log.h>

#include <boost/system/error_code.hpp> //for error codes
#include <boost/cstdint.hpp>
#include <boost/smart_ptr.hpp> //for scoped and shared ptr
#include <boost/noncopyable.hpp>

//#include <boost/function.hpp> //for callback
#include <loki/Function.h>

namespace nlib {
namespace socket {

class SerialPort
{
public:
};

} //namespace socket
} //namespace nlib


#endif /* NLIB_SERIALPORT_H_ */
