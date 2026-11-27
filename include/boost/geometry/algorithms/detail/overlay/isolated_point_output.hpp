// Boost.Geometry (aka GGL, Generic Geometry Library)

// Copyright (c) 2007-2014 Barend Gehrels, Amsterdam, the Netherlands.
// Copyright (c) 2017 Adam Wulkiewicz, Lodz, Poland.

// This file was modified by Oracle on 2014-2024.
// Modifications copyright (c) 2014-2024 Oracle and/or its affiliates.
// Contributed and/or modified by Vissarion Fysikopoulos, on behalf of Oracle
// Contributed and/or modified by Menelaos Karavelas, on behalf of Oracle
// Contributed and/or modified by Adam Wulkiewicz, on behalf of Oracle

// Use, modification and distribution is subject to the Boost Software License,
// Version 1.0. (See accompanying file LICENSE_1_0.txt or copy at
// http://www.boost.org/LICENSE_1_0.txt)

#ifndef BOOST_GEOMETRY_ALGORITHMS_DETAIL_OVERLAY_ISOLATED_POINT_OUTPUT_HPP
#define BOOST_GEOMETRY_ALGORITHMS_DETAIL_OVERLAY_ISOLATED_POINT_OUTPUT_HPP

#include <set>
#include <utility>

#include <boost/geometry/algorithms/append.hpp>
#include <boost/geometry/algorithms/detail/overlay/output_conversion.hpp>
#include <boost/geometry/algorithms/detail/tupled_output.hpp>
#include <boost/geometry/core/point_type.hpp>
#include <boost/geometry/core/tag.hpp>
#include <boost/geometry/core/tags.hpp>
#include <boost/geometry/strategies/detail/output_point_key.hpp>
#include <boost/geometry/strategies/detail/point_conversion.hpp>

namespace boost { namespace geometry
{

#ifndef DOXYGEN_NO_DETAIL
namespace detail { namespace overlay { namespace following
{

template
<
    typename GeometryOut,
    typename Tag = geometry::tag_t<GeometryOut>
>
struct add_isolated_point
{};

template <typename LineStringOut>
struct add_isolated_point<LineStringOut, linestring_tag>
{
    template <typename Point, typename OutputIterator>
    static inline void apply(Point const& point, OutputIterator& out)
    {
        LineStringOut isolated_point_ls;
        geometry::append(isolated_point_ls, point);

#ifndef BOOST_GEOMETRY_ALLOW_ONE_POINT_LINESTRINGS
        geometry::append(isolated_point_ls, point);
#endif // BOOST_GEOMETRY_ALLOW_ONE_POINT_LINESTRINGS

        *out++ = isolated_point_ls;
    }
};

template <typename PointOut>
struct add_isolated_point<PointOut, point_tag>
{
    template <typename OutputIterator>
    static inline void apply(PointOut const& point, OutputIterator& out)
    {
        *out++ = point;
    }
};


// Remember only the points emitted by this operation, independently of the
// output iterator and any points already present in the caller's output.
template <typename GeometryOut, typename Strategy, bool Enabled>
struct isolated_point_output
{
    explicit isolated_point_output(Strategy const&) {}

    template <typename Point, typename OutputIterator>
    inline void apply(Point const&, OutputIterator&, Strategy const&)
    {}
};

template <typename GeometryOut, typename Strategy>
class isolated_point_output<GeometryOut, Strategy, true>
{
    using pointlike = geometry::detail::output_geometry_access
        <
            GeometryOut, point_tag, linestring_tag
        >;
    using point_type = point_type_t<typename pointlike::type>;
    using key = decltype(strategies::detail::get_output_point_key<point_type>
        (std::declval<Strategy const&>()));

public:
    explicit isolated_point_output(Strategy const& strategy)
        : m_points(strategies::detail::get_output_point_key<point_type>(strategy))
    {}

    template <typename Point, typename OutputIterator>
    inline void apply(Point const& point, OutputIterator& out, Strategy const& strategy)
    {
        point_type converted;
        strategies::detail::convert_output_point(strategy, point, converted);
        if (m_points.insert(m_points.key_comp().apply(converted)).second)
        {
            auto& destination = geometry::detail::overlay::output_iterator_base(out);
            add_isolated_point<typename pointlike::type>::apply(converted, pointlike::get(destination));
        }
    }

private:
    // Compare canonical coordinates exactly, keeping the set ordering strict.
    std::set<typename key::type, key> m_points;
};

}}} // namespace detail::overlay::following
#endif // DOXYGEN_NO_DETAIL

}} // namespace boost::geometry

#endif // BOOST_GEOMETRY_ALGORITHMS_DETAIL_OVERLAY_ISOLATED_POINT_OUTPUT_HPP
