// Boost.Geometry (aka GGL, Generic Geometry Library)

// Copyright (c) 2019-2019 Barend Gehrels, Amsterdam, the Netherlands.

// Use, modification and distribution is subject to the Boost Software License,
// Version 1.0. (See accompanying file LICENSE_1_0.txt or copy at
// http://www.boost.org/LICENSE_1_0.txt)

#ifndef BOOST_GEOMETRY_VIEWS_DETAIL_SEGMENT_AS_SUBRANGE_HPP
#define BOOST_GEOMETRY_VIEWS_DETAIL_SEGMENT_AS_SUBRANGE_HPP

#include <cstddef>
#include <type_traits>

#include <boost/geometry/algorithms/detail/assign_indexed_point.hpp>
#include <boost/geometry/core/point_type.hpp>
#include <boost/geometry/geometries/pointing_segment.hpp>
#include <boost/geometry/geometries/segment.hpp>

namespace boost { namespace geometry { namespace detail
{

namespace segment_as_subrange_detail
{

template <std::size_t Index, typename Point>
inline Point endpoint(model::segment<Point> const& segment, int)
{
    return Index == 0 ? segment.first : segment.second;
}

template <std::size_t Index, typename Point>
inline Point endpoint(model::referring_segment<Point> const& segment, int)
{
    return Index == 0 ? segment.first : segment.second;
}

template <std::size_t Index, typename Point,
          typename = typename std::remove_cv_t<Point>::turn_record_type>
inline std::remove_cv_t<Point> endpoint(model::pointing_segment<Point> const& segment, int)
{
    return Index == 0 ? *segment.first : *segment.second;
}

template <std::size_t Index, typename Segment>
inline geometry::point_type_t<Segment> endpoint(Segment const& segment, long)
{
    geometry::point_type_t<Segment> point;
    geometry::detail::assign_point_from_index<Index>(segment, point);
    return point;
}

} // namespace segment_as_subrange_detail

template <typename Segment>
struct segment_as_subrange
{
    segment_as_subrange(Segment const& s)
        : m_p1(segment_as_subrange_detail::endpoint<0>(s, 0))
        , m_p2(segment_as_subrange_detail::endpoint<1>(s, 0))
    {}

    using point_type = geometry::point_type_t<Segment>;

    point_type const& at(std::size_t index) const
    {
        return index == 0 ? m_p1 : m_p2;
    }

    static inline bool is_last_segment()
    {
        return true;
    }

    point_type m_p1, m_p2;

};

}}} // namespace boost::geometry::detail

#endif // BOOST_GEOMETRY_VIEWS_DETAIL_SEGMENT_AS_SUBRANGE_HPP
