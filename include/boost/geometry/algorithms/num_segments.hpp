// Boost.Geometry (aka GGL, Generic Geometry Library)

// Copyright (c) 2014, Oracle and/or its affiliates.

// Contributed and/or modified by Menelaos Karavelas, on behalf of Oracle

// Licensed under the Boost Software License version 1.0.
// http://www.boost.org/users/license.html

#ifndef BOOST_GEOMETRY_ALGORITHMS_NUM_SEGMENTS_HPP
#define BOOST_GEOMETRY_ALGORITHMS_NUM_SEGMENTS_HPP

#include <cstddef>

#include <boost/range.hpp>

#include <boost/variant/static_visitor.hpp>
#include <boost/variant/apply_visitor.hpp>
#include <boost/variant/variant_fwd.hpp>

#include <boost/geometry/core/closure.hpp>
#include <boost/geometry/core/exterior_ring.hpp>
#include <boost/geometry/core/interior_rings.hpp>
#include <boost/geometry/core/interior_type.hpp>
#include <boost/geometry/core/tag.hpp>
#include <boost/geometry/core/tags.hpp>

#include <boost/geometry/util/range.hpp>

#include <boost/geometry/geometries/concepts/check.hpp>

#include <boost/geometry/algorithms/detail/interior_iterator.hpp>
#include <boost/geometry/algorithms/disjoint.hpp>
#include <boost/geometry/algorithms/not_implemented.hpp>
#include <boost/geometry/algorithms/num_points.hpp>


namespace boost { namespace geometry
{



#ifndef DOXYGEN_NO_DISPATCH
namespace dispatch
{

template
<
    typename Geometry,
    typename Tag = typename tag<Geometry>::type
>
struct num_segments
    : not_implemented<Tag>
{};

} // namespace dispatch
#endif // DOXYGEN_NO_DISPATCH



#ifndef DOXYGEN_NO_DETAIL
namespace detail { namespace num_segments
{


struct range_count
{
    template <typename Range>
    static inline std::size_t apply(Range const& range, bool add_for_open)
    {
        std::size_t n = boost::size(range);
        if ( n <= 1 )
        {
            return 0;
        }
        if (add_for_open
            && geometry::closure<Range>::value == open
            && geometry::disjoint(range::front(range), range::at(range, n - 1))
            )
        {
            return n;
        }
        return static_cast<std::size_t>(n - 1);
    }
};


struct polygon_count
    : private range_count
{
    template <typename Polygon>
    static inline std::size_t apply(Polygon const& poly, bool add_for_open)
    {
        std::size_t n = range_count::apply(exterior_ring(poly), add_for_open);

        typename interior_return_type<Polygon const>::type
            rings = interior_rings(poly);
        for (typename detail::interior_iterator<Polygon const>::type
                 it = boost::begin(rings); it != boost::end(rings); ++it)
        {
            n += range_count::apply(*it, add_for_open);
        }

        return n;
    }
};


struct multi_count
{
    template <typename MultiGeometry>
    static inline
    std::size_t apply(MultiGeometry const& geometry, bool add_for_open)
    {
        typedef typename boost::range_value<MultiGeometry>::type geometry_type;
        typedef typename boost::range_iterator
            <
                MultiGeometry const
            >::type iterator_type;

        std::size_t n = 0;
        for (iterator_type it = boost::begin(geometry);
            it != boost::end(geometry); ++it)
        {
            n += dispatch::num_segments
                <
                    geometry_type
                >::apply(*it, add_for_open);
        }
        return n;
    }
};


}} // namespace detail::num_segments
#endif // DOXYGEN_NO_DETAIL



#ifndef DOXYGEN_NO_DISPATCH
namespace dispatch
{


template <typename Geometry>
struct num_segments<Geometry, point_tag>
    : detail::num_points::other_count<0>
{};

template <typename Geometry>
struct num_segments<Geometry, box_tag>
    : detail::num_points::other_count<4>
{};

template <typename Geometry>
struct num_segments<Geometry, segment_tag>
    : detail::num_points::other_count<1>
{};

template <typename Geometry>
struct num_segments<Geometry, linestring_tag>
    : detail::num_segments::range_count
{};

template <typename Geometry>
struct num_segments<Geometry, ring_tag>
    : detail::num_segments::range_count
{};

template <typename Geometry>
struct num_segments<Geometry, polygon_tag>
    : detail::num_segments::polygon_count
{};

template <typename Geometry>
struct num_segments<Geometry, multi_point_tag>
    : detail::num_points::other_count<0>
{};

template <typename Geometry>
struct num_segments<Geometry, multi_linestring_tag>
    : detail::num_segments::multi_count
{};

template <typename Geometry>
struct num_segments<Geometry, multi_polygon_tag>
    : detail::num_segments::multi_count
{};


} // namespace dispatch
#endif // DOXYGEN_NO_DISPATCH



namespace resolve_variant
{


template <typename Geometry>
struct num_segments
    : dispatch::num_segments<Geometry>
{};


template <BOOST_VARIANT_ENUM_PARAMS(typename T)>
struct num_segments<boost::variant<BOOST_VARIANT_ENUM_PARAMS(T)> >
{
    struct visitor: boost::static_visitor<std::size_t>
    {
        bool m_add_for_open;

        visitor(bool add_for_open): m_add_for_open(add_for_open) {}

        template <typename Geometry>
        typename std::size_t operator()(Geometry const& geometry) const
        {
            return num_segments<Geometry>::apply(geometry, m_add_for_open);
        }
    };

    static inline std::size_t
    apply(boost::variant<BOOST_VARIANT_ENUM_PARAMS(T)> const& geometry,
          bool add_for_open)
    {
        return boost::apply_visitor(visitor(add_for_open), geometry);
    }
};


} // namespace resolve_variant



template <typename Geometry>
inline std::size_t num_segments(Geometry const& geometry,
                                bool add_for_open = false)
{
    concept::check<Geometry const>();

    return resolve_variant::num_segments
        <
            Geometry
        >::apply(geometry, add_for_open);
}



}} // namespace boost::geometry

#endif // BOOST_GEOMETRY_ALGORITHMS_NUM_SEGMENTS_HPP
