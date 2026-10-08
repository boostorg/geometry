// Boost.Geometry (aka GGL, Generic Geometry Library)

// Copyright (c) 2019 Tinko Bartels, Berlin, Germany.

// Contributed and/or modified by Tinko Bartels,
//   as part of Google Summer of Code 2019 program.

// Use, modification and distribution is subject to the Boost Software License,
// Version 1.0. (See accompanying file LICENSE_1_0.txt or copy at
// http://www.boost.org/LICENSE_1_0.txt)

#ifndef BOOST_GEOMETRY_EXTENSIONS_TRIANGULATION_STRATEGIES_DELAUNAY_TRIANGULATION_HPP
#define BOOST_GEOMETRY_EXTENSIONS_TRIANGULATION_STRATEGIES_DELAUNAY_TRIANGULATION_HPP

#include <boost/mpl/assert.hpp>

namespace boost { namespace geometry { namespace strategy
{

namespace delaunay_triangulation { namespace services
{

/*!
\brief Traits class binding a default delaunay triangulation construction strategy to a coordinate system
\ingroup util
\tparam Tag tag of geometry
\tparam CSTag tag of coordinate system
*/
template <typename Tag, typename CSTag, int Dimension>
struct default_strategy
{
    BOOST_MPL_ASSERT_MSG
        (
            false, NOT_IMPLEMENTED_FOR_THIS_TYPE
            , (types<Tag, CSTag>)
        );
};

}}}}} // namesspace boost::geometry::strategy::delaunay_triangulation::services

#endif // BOOST_GEOMETRY_EXTENSIONS_TRIANGULATION_STRATEGIES_DELAUNAY_TRIANGULATION_HPP
