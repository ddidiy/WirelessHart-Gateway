/*
 * test.h
 * Abstracts a test framework, currently based on Boost.Test.
 *
 *  Created on: Nov 21, 2008
 *      Author: nicu.dascalu
 */

#ifndef NLIB_TEST_H
#define NLIB_TEST_H

#include <boost/test/unit_test.hpp>
#include <nlib/detail/bytes_print.h>

#define BOOST_CHECK_EQUAL_BYTES(L, LS, R, RS)\
	BOOST_CHECK_EQUAL(nlib::detail::BytesToString(L, LS), nlib::detail::BytesToString(R, RS))

#define BOOST_REQUIRE_EQUAL_BYTES(L, LS, R, RS)\
	BOOST_REQUIRE_EQUAL(nlib::detail::BytesToString(L, LS), nlib::detail::BytesToString(R, RS))


#endif //NLIB_TEST_H
