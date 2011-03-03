// Boost.Geometry (aka GGL, Generic Geometry Library)
//
// Copyright Barend Gehrels 2011, Geodan, Amsterdam, the Netherlands.
// Use, modification and distribution is subject to the Boost Software License,
// Version 1.0. (See accompanying file LICENSE_1_0.txt or copy at
// http://www.boost.org/LICENSE_1_0.txt)

#ifndef BOOST_GEOMETRY_ALGORITHMS_DETAIL_OVERLAY_RING_PROPERTIES_HPP
#define BOOST_GEOMETRY_ALGORITHMS_DETAIL_OVERLAY_RING_PROPERTIES_HPP


#include <boost/geometry/algorithms/area.hpp>
#include <boost/geometry/algorithms/within.hpp>
#include <boost/geometry/algorithms/detail/point_on_border.hpp>


namespace boost { namespace geometry
{


#ifndef DOXYGEN_NO_DETAIL
namespace detail { namespace overlay
{

template <typename Point>
struct ring_properties
{
    typedef Point point_type;
    typedef typename area_result<Point>::type area_type;

    // Filled by "select_rings"
    Point point;
    area_type area;

    int within_code;
    bool reversed;
    bool discarded;

    // Filled/used by "assign_rings"
    ring_identifier parent;
    std::vector<ring_identifier> children;

    inline ring_properties()
        : area(area_type())
        , within_code(-1)
        , reversed(false)
        , discarded(false)
    {}

    template <typename RingOrBox>
    inline ring_properties(RingOrBox const& ring_or_box)
        : within_code(-1)
        , reversed(false)
        , discarded(false)
    {
        this->area = geometry::area(ring_or_box);
        geometry::point_on_border(this->point, ring_or_box, true);
    }

    template <typename RingOrBox, typename Geometry>
    inline ring_properties(RingOrBox const& ring_or_box, Geometry const& geometry)
        : reversed(false)
        , discarded(false)
    {
        this->area = geometry::area(ring_or_box);
        geometry::point_on_border(this->point, ring_or_box, true);
        this->within_code = geometry::within(this->point, geometry) ? 1 : -1;
    }

    area_type get_area() const
    {
        return reversed ? -area : area;
    }
};

}} // namespace detail::overlay
#endif // DOXYGEN_NO_DETAIL


}} // namespace boost::geometry


#endif // BOOST_GEOMETRY_ALGORITHMS_DETAIL_OVERLAY_RING_PROPERTIES_HPP
