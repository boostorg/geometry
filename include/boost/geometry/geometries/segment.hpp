// Boost.Geometry (aka GGL, Generic Geometry Library)
//
// Copyright Barend Gehrels 2007-2009, Geodan, Amsterdam, the Netherlands.
// Copyright Bruno Lalande 2008, 2009
// Use, modification and distribution is subject to the Boost Software License,
// Version 1.0. (See accompanying file LICENSE_1_0.txt or copy at
// http://www.boost.org/LICENSE_1_0.txt)

#ifndef BOOST_GEOMETRY_GEOMETRIES_SEGMENT_HPP
#define BOOST_GEOMETRY_GEOMETRIES_SEGMENT_HPP

#include <cstddef>

#include <boost/concept/assert.hpp>
#include <boost/mpl/if.hpp>
#include <boost/type_traits/is_const.hpp>

#include <boost/geometry/geometries/concepts/point_concept.hpp>

namespace boost { namespace geometry
{

/*!
\brief Class segment: small class containing two (templatized) point references
\ingroup geometries
\details From Wikipedia: In geometry, a line segment is a part of a line that is bounded
 by two distinct end points, and contains every point on the line between its end points.
\note The structure is like std::pair, and can often be used interchangeable.
Difference is that it refers to points, does not have points.
\note Like std::pair, points are public available.
\note type is const or non const, so geometry::segment<P> or geometry::segment<const P>
\note We cannot derive from std::pair<P&, P&> because of
reference assignments.
\tparam ConstOrNonConstPoint point type of the segment, maybe a point or a const point
*/
template<typename ConstOrNonConstPoint>
class segment
{
    BOOST_CONCEPT_ASSERT( (
        typename boost::mpl::if_
            <
                boost::is_const<ConstOrNonConstPoint>,
                concept::Point<ConstOrNonConstPoint>,
                concept::ConstPoint<ConstOrNonConstPoint>
            >
    ) );

    typedef ConstOrNonConstPoint point_type;

public:

    point_type& first;
    point_type& second;

    inline segment(point_type& p1, point_type& p2)
        : first(p1)
        , second(p2)
    {}
};

// Traits specializations for segment above
#ifndef DOXYGEN_NO_TRAITS_SPECIALIZATIONS
namespace traits
{

template <typename ConstOrNonConstPoint>
struct tag<segment<ConstOrNonConstPoint> >
{
    typedef segment_tag type;
};

template <typename ConstOrNonConstPoint>
struct point_type<segment<ConstOrNonConstPoint> >
{
    typedef ConstOrNonConstPoint type;
};

template <typename ConstOrNonConstPoint, std::size_t Dimension>
struct indexed_access<segment<ConstOrNonConstPoint>, 0, Dimension>
{
    typedef segment<ConstOrNonConstPoint> segment_type;
    typedef typename geometry::coordinate_type<segment_type>::type coordinate_type;

    static inline coordinate_type get(segment_type const& s)
    {
        return geometry::get<Dimension>(s.first);
    }

    static inline void set(segment_type& s, coordinate_type const& value)
    {
        geometry::set<Dimension>(s.first, value);
    }
};


template <typename ConstOrNonConstPoint, std::size_t Dimension>
struct indexed_access<segment<ConstOrNonConstPoint>, 1, Dimension>
{
    typedef segment<ConstOrNonConstPoint> segment_type;
    typedef typename geometry::coordinate_type<segment_type>::type coordinate_type;

    static inline coordinate_type get(segment_type const& s)
    {
        return geometry::get<Dimension>(s.second);
    }

    static inline void set(segment_type& s, coordinate_type const& value)
    {
        geometry::set<Dimension>(s.second, value);
    }
};

} // namespace traits
#endif // DOXYGEN_NO_TRAITS_SPECIALIZATIONS

}} // namespace boost::geometry

#endif // BOOST_GEOMETRY_GEOMETRIES_SEGMENT_HPP
