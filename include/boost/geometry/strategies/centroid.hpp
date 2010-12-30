// Boost.Geometry (aka GGL, Generic Geometry Library)
//
// Copyright Barend Gehrels 2007-2009, Geodan, Amsterdam, the Netherlands.
// Copyright Bruno Lalande 2008, 2009
// Use, modification and distribution is subject to the Boost Software License,
// Version 1.0. (See accompanying file LICENSE_1_0.txt or copy at
// http://www.boost.org/LICENSE_1_0.txt)

#ifndef BOOST_GEOMETRY_STRATEGIES_CENTROID_HPP
#define BOOST_GEOMETRY_STRATEGIES_CENTROID_HPP


#include <cstddef>

#include <boost/mpl/assert.hpp>

#include <boost/geometry/core/tags.hpp>
#include <boost/geometry/strategies/tags.hpp>


namespace boost { namespace geometry
{


namespace strategy { namespace centroid
{

struct not_applicable_strategy
{
};


namespace services
{

/*!
    \brief Traits class binding a centroid calculation strategy to a coordinate system
    \ingroup centroid
    \tparam CsTag tag of coordinate system, for specialization
    \tparam GeometryTag tag of geometry, for specialization
    \tparam Dimension dimension of geometry, for specialization
    \tparam Point point-type
    \tparam Geometry
*/
template
<
    typename CsTag,
    typename GeometryTag,
    std::size_t Dimension,
    typename Point,
    typename Geometry
>
struct default_strategy
{
    BOOST_MPL_ASSERT_MSG
        (
            false, NOT_IMPLEMENTED_FOR_THIS_TYPES
            , (types<Point, Geometry>)
        );
};

// Register NA-strategy for all where not applicable
template <typename Point, typename Geometry, std::size_t Dimension>
struct default_strategy<cartesian_tag, point_tag, Dimension, Point, Geometry>
{
    typedef not_applicable_strategy type;
};

template <typename Point, typename Geometry, std::size_t Dimension>
struct default_strategy<cartesian_tag, box_tag, Dimension, Point, Geometry>
{
    typedef not_applicable_strategy type;
};

template <typename Point, typename Geometry, std::size_t Dimension>
struct default_strategy<cartesian_tag, linestring_tag, Dimension, Point, Geometry>
{
    typedef not_applicable_strategy type;
};


} // namespace services


}} // namespace strategy::centroid


}} // namespace boost::geometry

#endif // BOOST_GEOMETRY_STRATEGIES_CENTROID_HPP
