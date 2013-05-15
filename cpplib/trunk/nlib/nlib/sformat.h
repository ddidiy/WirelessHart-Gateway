#ifndef NLIB_SFORMAT_H_
#define NLIB_SFORMAT_H_



#ifdef ENABLE_NLIB_SFORMAT
#  include <boost/format.hpp>
#  define NLIB_SFORMAT(F, P) boost::str(boost::format(F) % P)
#else
#  define NLIB_SFORMAT(F, P) F
#endif


#endif /*NLIB_SFORMAT_H_*/
