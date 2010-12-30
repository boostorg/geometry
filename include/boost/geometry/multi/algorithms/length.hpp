// Boost.Geometry (aka GGL, Generic Geometry Library)
//
// Copyright Barend Gehrels 2007-2009, Geodan, Amsterdam, the Netherlands.
// Use, modification and distribution is subject to the Boost Software License,
// Version 1.0. (See accompanying file LICENSE_1_0.txt or copy at
// http://www.boost.org/LICENSE_1_0.txt)

#ifndef BOOST_GEOMETRY_MULTI_ALGORITHMS_LENGTH_HPP
#define BOOST_GEOMETRY_MULTI_ALGORITHMS_LENGTH_HPP


#include <boost/range/metafunctions.hpp>

#include <boost/geometry/algorithms/length.hpp>
#include <boost/geometry/multi/core/tags.hpp>
#include <boost/geometry/multi/algorithms/detail/multi_sum.hpp>


namespace boost { namespace geometry
{

#ifndef DOXYGEN_NO_DISPATCH
namespace dispatch
{

template <typename MultiLinestring, typename Strategy>
struct length<multi_linestring_tag, MultiLinestring, Strategy>
    : detail::multi_sum
        <
            typename length_result<MultiLinestring>::type,
            MultiLinestring,
            Strategy,
            detail::length::range_length
                <
                    typename boost::range_value<MultiLinestring>::type,
                    Strategy,
					closed // no need to close it explicitly
                >
        >
{};


} // namespace dispatch
#endif


}} // namespace boost::geometry


#endif // BOOST_GEOMETRY_MULTI_ALGORITHMS_LENGTH_HPP
