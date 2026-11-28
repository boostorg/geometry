// Boost.Geometry
// Distributed under the Boost Software License, Version 1.0.

#ifndef BOOST_GEOMETRY_UTIL_DETAIL_PRECISE_FP_HPP
#define BOOST_GEOMETRY_UTIL_DETAIL_PRECISE_FP_HPP

#include <boost/config.hpp>

// Scope expression transformations, not the runtime FP environment. This
// cannot restore all the assumptions removed by full fast-math.
#if defined(BOOST_CLANG) && __clang_major__ >= 14
// Clang's explicit -ffp-contract=fast overrides the contraction pragma.
# define BOOST_GEOMETRY_DETAIL_PRECISE_FP_BEGIN \
    _Pragma("float_control(precise, on, push)") \
    _Pragma("clang fp contract(off)")
# define BOOST_GEOMETRY_DETAIL_PRECISE_FP_END _Pragma("float_control(pop)")
#elif defined(BOOST_MSVC) && BOOST_MSVC >= 1930
# define BOOST_GEOMETRY_DETAIL_PRECISE_FP_BEGIN \
    __pragma(float_control(precise, on, push)) \
    __pragma(fp_contract(off))
# define BOOST_GEOMETRY_DETAIL_PRECISE_FP_END __pragma(float_control(pop))
#elif defined(BOOST_GCC)
# define BOOST_GEOMETRY_DETAIL_PRECISE_FP_BEGIN \
    _Pragma("GCC push_options") \
    _Pragma("GCC optimize (\"no-fast-math\", \"fp-contract=off\")")
# define BOOST_GEOMETRY_DETAIL_PRECISE_FP_END _Pragma("GCC pop_options")
#endif

#if defined(BOOST_GEOMETRY_DETAIL_PRECISE_FP_BEGIN)
# define BOOST_GEOMETRY_DETAIL_HAS_PRECISE_FP_PRAGMAS
#else
# define BOOST_GEOMETRY_DETAIL_PRECISE_FP_BEGIN
# define BOOST_GEOMETRY_DETAIL_PRECISE_FP_END
#endif

#endif
