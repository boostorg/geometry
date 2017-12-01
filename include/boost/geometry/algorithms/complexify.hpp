// Boost.Geometry

// Copyright (c) 2017, Oracle and/or its affiliates.

// Contributed and/or modified by Adam Wulkiewicz, on behalf of Oracle

// Licensed under the Boost Software License version 1.0.
// http://www.boost.org/users/license.html

#ifndef BOOST_GEOMETRY_ALGORITHMS_COMPLEXIFY_HPP
#define BOOST_GEOMETRY_ALGORITHMS_COMPLEXIFY_HPP


#include <boost/geometry/algorithms/clear.hpp>
#include <boost/geometry/algorithms/detail/convert_point_to_point.hpp>
#include <boost/geometry/algorithms/not_implemented.hpp>
#include <boost/geometry/core/closure.hpp>
#include <boost/geometry/core/point_type.hpp>
#include <boost/geometry/core/tag.hpp>
#include <boost/geometry/core/tags.hpp>
#include <boost/geometry/strategies/default_strategy.hpp>
#include <boost/geometry/util/range.hpp>

#include <boost/range/size.hpp>
#include <boost/range/value_type.hpp>


namespace boost { namespace geometry
{


#ifndef DOXYGEN_NO_DETAIL
namespace detail { namespace complexify
{

template <typename Range, typename Point>
inline void convert_and_push_back(Range & range, Point const& p)
{
    typename boost::range_value<Range>::type p2;
    geometry::detail::conversion::convert_point_to_point(p, p2);
    range::push_back(range, p2);
}

template <bool AppendLastPoint = true>
struct complexify_range
{
    template <typename Geometry, typename GeometryOut, typename T, typename Strategy>
    static inline void apply(Geometry const& rng, GeometryOut & rng_out,
                             T const& len, Strategy const& strategy)
    {
        typedef typename boost::range_value<Geometry>::type point_t;

        std::size_t count = boost::size(rng);

        if (count == 0)
            return;
        
        for (std::size_t i = 1 ; i < count ; ++i)
        {
            point_t const& p0 = range::at(rng, i - 1);
            point_t const& p1 = range::at(rng, i);

            convert_and_push_back(rng_out, p0);

            strategy.apply(p0, p1, rng_out, len);
        }

        if (BOOST_GEOMETRY_CONDITION(AppendLastPoint))
        {
            convert_and_push_back(rng_out, range::back(rng));
        }
    }
};

template <bool IsClosed1, bool IsClosed2> // false, X
struct complexify_ring
{
    template <typename Geometry, typename GeometryOut, typename T, typename Strategy>
    static inline void apply(Geometry const& ring, GeometryOut & ring_out,
                             T const& len, Strategy const& strategy)
    {
        geometry::detail::complexify::complexify_range<true>
            ::apply(ring, ring_out, len, strategy);

        if (boost::size(ring) <= 1)
            return;

        typedef typename point_type<Geometry>::type point_t;
        point_t const& p0 = range::back(ring);
        point_t const& p1 = range::front(ring);

        strategy.apply(p0, p1, ring_out, len);

        if (BOOST_GEOMETRY_CONDITION(IsClosed2))
        {
            convert_and_push_back(ring_out, p1);
        }
    }
};

template <>
struct complexify_ring<true, true>
    : complexify_range<true>
{};

template <>
struct complexify_ring<true, false>
    : complexify_range<false>
{};


}} // namespace detail::complexify
#endif // DOXYGEN_NO_DETAIL


#ifndef DOXYGEN_NO_DISPATCH
namespace dispatch
{


template
<
    typename Geometry,
    typename GeometryOut,
    typename Tag1 = typename tag<Geometry>::type,
    typename Tag2 = typename tag<GeometryOut>::type
>
struct complexify
    : not_implemented<Tag1, Tag2>
{};

template <typename Geometry, typename GeometryOut>
struct complexify<Geometry, GeometryOut, linestring_tag, linestring_tag>
    : geometry::detail::complexify::complexify_range<>
{};

template <typename Geometry, typename GeometryOut>
struct complexify<Geometry, GeometryOut, multi_linestring_tag, multi_linestring_tag>
{
    template <typename Geometry, typename GeometryOut, typename T, typename Strategy>
    static void apply(Geometry const& mls, GeometryOut & mls_out,
                      T const& len, Strategy const& strategy)
    {
        std::size_t count = boost::size(mls);
        range::resize(mls_out, count);

        for (std::size_t i = 0 ; i < count ; ++i)
        {
            geometry::detail::complexify::complexify_range<>
                ::apply(range::at(mls, i), range::at(mls_out, i),
                        len, strategy);
        }
    }
};

template <typename Geometry, typename GeometryOut>
struct complexify<Geometry, GeometryOut, ring_tag, ring_tag>
    : geometry::detail::complexify::complexify_ring
        <
            geometry::closure<Geometry>::value != geometry::open,
            geometry::closure<GeometryOut>::value != geometry::open
        >
{};

template <typename Geometry, typename GeometryOut>
struct complexify<Geometry, GeometryOut, polygon_tag, polygon_tag>
{
    template <typename Geometry, typename GeometryOut, typename T, typename Strategy>
    static void apply(Geometry const& poly, GeometryOut & poly_out,
                      T const& len, Strategy const& strategy)
    {
        apply_ring(exterior_ring(poly), exterior_ring(poly_out),
                   length_threshold, sph);

        std::size_t count = boost::size(interior_rings(poly));
        range::resize(interior_rings(poly_out), count);

        for (std::size_t i = 0 ; i < count ; ++i)
        {
            apply_ring(range::at(interior_rings(poly), i),
                       range::at(interior_rings(poly_out), i),
                       length_threshold, sph);
        }
    }

    template <typename Geometry, typename GeometryOut, typename T, typename Strategy>
    static void apply_ring(Geometry const& ring, GeometryOut & ring_out,
                           T const& len, Strategy const& strategy)
    {
        complexify<Geometry, GeometryOut, ring_tag, ring_tag>
            ::apply(ring, ring_out, len, strategy);
    }
};

template <typename Geometry, typename GeometryOut>
struct complexify<Geometry, GeometryOut, multi_polygon_tag, multi_polygon_tag>
{
    template <typename Geometry, typename GeometryOut, typename T, typename Strategy>
    static void apply(Geometry const& mpoly, GeometryOut & mpoly_out,
                      T const& len, Strategy const& strategy)
    {
        std::size_t count = boost::size(mpoly);
        range::resize(mpoly_out, mpoly);

        for (std::size_t i = 0 ; i < count ; ++i)
        {
            apply_poly(range::at(mpoly, i),
                       range::at(mpoly_out, i),
                       length_threshold, sph);
        }
    }

    template <typename Geometry, typename GeometryOut, typename T, typename Strategy>
    static void apply_poly(Geometry const& poly, GeometryOut & poly_out,
                           T const& len, Strategy const& strategy)
    {
        complexify<Geometry, GeometryOut, polygon_tag, polygon_tag>::
            apply(poly, poly_out, length_threshold, strategy);
    }
};


} // namespace dispatch
#endif // DOXYGEN_NO_DISPATCH


namespace resolve_strategy
{

struct complexify
{
    template <typename Geometry, typename Distance, typename Strategy>
    static inline void apply(Geometry const& geometry,
                             Geometry& out,
                             Distance const& max_distance,
                             Strategy const& strategy)
    {
        dispatch::complexify<Geometry, Geometry>
            ::apply(geometry, out, max_distance, strategy);
    }

    template <typename Geometry, typename Distance>
    static inline void apply(Geometry const& geometry,
                             Geometry& out,
                             Distance const& max_distance,
                             default_strategy)
    {
        typedef strategy::complexify::services::default_strategy
            <
                typename cs_tag<Geometry>::type
            > strategy_type;
        
        /*BOOST_CONCEPT_ASSERT(
            (concepts::ComplexifyStrategy<strategy_type>)
        );*/

        apply(geometry, out, max_distance, strategy_type());
    }
};

} // namespace resolve_strategy


namespace resolve_variant {

template <typename Geometry>
struct complexify
{
    template <typename Distance, typename Strategy>
    static inline void apply(Geometry const& geometry,
                             Geometry& out,
                             Distance const& max_distance,
                             Strategy const& strategy)
    {
        resolve_strategy::complexify::apply(geometry, out, max_distance, strategy);
    }
};

template <BOOST_VARIANT_ENUM_PARAMS(typename T)>
struct complexify<boost::variant<BOOST_VARIANT_ENUM_PARAMS(T)> >
{
    template <typename Distance, typename Strategy>
    struct visitor: boost::static_visitor<void>
    {
        Distance const& m_max_distance;
        Strategy const& m_strategy;

        visitor(Distance const& max_distance, Strategy const& strategy)
            : m_max_distance(max_distance)
            , m_strategy(strategy)
        {}

        template <typename Geometry>
        void operator()(Geometry const& geometry, Geometry& out) const
        {
            simplify<Geometry>::apply(geometry, out, m_max_distance, m_strategy);
        }
    };

    template <typename Distance, typename Strategy>
    static inline void
    apply(boost::variant<BOOST_VARIANT_ENUM_PARAMS(T)> const& geometry,
          boost::variant<BOOST_VARIANT_ENUM_PARAMS(T)>& out,
          Distance const& max_distance,
          Strategy const& strategy)
    {
        boost::apply_visitor(
            visitor<Distance, Strategy>(max_distance, strategy),
            geometry,
            out
        );
    }
};

} // namespace resolve_variant


template <typename Geometry, typename Distance, typename Strategy>
inline void complexify(Geometry const& geometry,
                       Geometry& out,
                       Distance const& max_distance,
                       Strategy const& strategy)
{
    concepts::check<Geometry>();

    geometry::clear(out);

    resolve_variant::complexify
        <
            Geometry
        >::apply(geometry, out, max_distance, strategy);
}

template <typename Geometry, typename Distance, typename Strategy>
inline void complexify(Geometry const& geometry,
                       Geometry& out,
                       Distance const& max_distance)
{
    complexify(geometry, out, max_distance, strategy_type());
}

}} // namespace boost::geometry

#endif // BOOST_GEOMETRY_ALGORITHMS_COMPLEXIFY_HPP
