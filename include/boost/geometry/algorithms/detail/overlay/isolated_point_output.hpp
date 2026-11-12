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
#include <type_traits>

#include <boost/geometry/algorithms/append.hpp>
#include <boost/geometry/algorithms/detail/convert_point_to_point.hpp>
#include <boost/geometry/algorithms/detail/tupled_output.hpp>
#include <boost/geometry/core/coordinate_promotion.hpp>
#include <boost/geometry/core/coordinate_type.hpp>
#include <boost/geometry/core/cs.hpp>
#include <boost/geometry/core/point_type.hpp>
#include <boost/geometry/core/tag.hpp>
#include <boost/geometry/core/tags.hpp>
#include <boost/geometry/geometries/helper_geometry.hpp>
#include <boost/geometry/strategies/compare.hpp>
#include <boost/geometry/strategies/normalize.hpp>

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
    template <typename Point, typename OutputIterator>
    static inline void apply(Point const& point, OutputIterator& out)
    {
        PointOut isolated_point;

        geometry::detail::conversion::convert_point_to_point(point, isolated_point);

        *out++ = isolated_point;
    }
};


// Remember only the points emitted by this operation, independently of the
// output iterator and any points already present in the caller's output.
template <typename GeometryOut, typename Strategy, bool Enabled>
struct isolated_point_output
{
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
    // Integral angular coordinates need floating-point spheroidal constants.
    // Use the widest standard floating type; its precision is platform-dependent.
    using key_type = std::conditional_t
        <
            std::is_base_of<spherical_tag, cs_tag_t<point_type>>::value
                && std::is_integral<coordinate_type_t<point_type>>::value,
            typename helper_geometry
                <
                    point_type,
                    typename promote_floating_point
                        <coordinate_type_t<point_type>, long double>::type
                >::type,
            point_type
        >;
    using normalize = std::conditional_t
        <
            std::is_base_of<spherical_tag, cs_tag_t<point_type>>::value,
            strategy::normalize::spherical_point,
            strategy::normalize::cartesian_point
        >;

    struct less
    {
        inline bool operator()(key_type const& left, key_type const& right) const
        {
            return strategy::compare::cartesian
                <
                    strategy::compare::less,
                    strategy::compare::equals_exact
                >::apply(left, right);
        }
    };

public:
    template <typename Point, typename OutputIterator>
    inline void apply(Point const& point, OutputIterator& out, Strategy const&)
    {
        point_type converted;
        geometry::detail::conversion::convert_point_to_point(point, converted);
        // Canonical keys make equivalent pole and antimeridian coordinates
        // identical without changing the representation emitted to the caller.
        key_type key;
        geometry::detail::conversion::convert_point_to_point(converted, key);
        normalize::apply(key, key);
        if (m_points.insert(key).second)
        {
            add_isolated_point<typename pointlike::type>::apply(converted, out);
        }
    }

private:
    // Compare canonical coordinates exactly, keeping the set ordering strict.
    std::set<key_type, less> m_points;
};

}}} // namespace detail::overlay::following
#endif // DOXYGEN_NO_DETAIL

}} // namespace boost::geometry

#endif // BOOST_GEOMETRY_ALGORITHMS_DETAIL_OVERLAY_ISOLATED_POINT_OUTPUT_HPP
