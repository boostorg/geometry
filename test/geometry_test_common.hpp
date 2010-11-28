// Boost.Geometry (aka GGL, Generic Geometry Library) test file
//
// Copyright Barend Gehrels 2007-2009, Geodan, Amsterdam, the Netherlands
// Copyright Bruno Lalande 2008, 2009
// Use, modification and distribution is subject to the Boost Software License,
// Version 1.0. (See accompanying file LICENSE_1_0.txt or copy at
// http://www.boost.org/LICENSE_1_0.txt)


#ifndef GEOMETRY_TEST_GEOMETRY_TEST_COMMON_HPP
#define GEOMETRY_TEST_GEOMETRY_TEST_COMMON_HPP

#if defined(_MSC_VER)
// We deliberately mix float/double's  so turn off warnings
#pragma warning( disable : 4244 )
// For (new since Boost 1.40) warning in Boost.Test on putenv/posix
#pragma warning( disable : 4996 )

//#pragma warning( disable : 4305 )
#endif // defined(_MSC_VER)

#include <boost/foreach.hpp>


// Include some always-included-for-testing files
#if ! defined(BOOST_GEOMETRY_NO_BOOST_TEST)
#  include <boost/test/floating_point_comparison.hpp>
#  include <boost/test/included/test_exec_monitor.hpp>
//#  include <boost/test/included/prg_exec_monitor.hpp>
#  include <boost/test/impl/execution_monitor.ipp>
#endif


#if defined(HAVE_TTMATH)
#  include <boost/geometry/extensions/contrib/ttmath_stub.hpp>
#endif

#if defined(HAVE_CLN) || defined(HAVE_GMP)
#  include <boost/numeric_adaptor/numeric_adaptor.hpp>
#endif


#if defined(HAVE_GMP)
#  include <boost/numeric_adaptor/gmp_value_type.hpp>
#endif
#if defined(HAVE_CLN)
#  include <boost/numeric_adaptor/cln_value_type.hpp>
#endif


template <typename T>
struct string_from_type {};

template <> struct string_from_type<void>
{ static std::string name() { return "v"; }  };

template <> struct string_from_type<float>
{ static std::string name() { return "f"; }  };

template <> struct string_from_type<double>
{ static std::string name() { return "d"; }  };

template <> struct string_from_type<long double>
{ static std::string name() { return "e"; }  };

#if defined(HAVE_TTMATH)
    template <> struct string_from_type<ttmath_big>
    { static std::string name() { return "t"; }  };
#endif


#if defined(HAVE_GMP)
    template <> struct string_from_type<boost::numeric_adaptor::gmp_value_type>
    { static std::string name() { return "g"; }  };
#endif

#if defined(HAVE_CLN)
    template <> struct string_from_type<boost::numeric_adaptor::cln_value_type>
    { static std::string name() { return "c"; }  };
#endif


// For all tests:
// - do NOT use "using namespace boost::geometry" to make clear what is Boost.Geometry
// - use bg:: as short alias
#include <boost/geometry/core/tag.hpp>
namespace bg = boost::geometry;


#endif // GEOMETRY_TEST_GEOMETRY_TEST_COMMON_HPP
