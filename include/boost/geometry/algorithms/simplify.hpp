// Boost.Geometry (aka GGL, Generic Geometry Library)

// Copyright (c) 2007-2015 Barend Gehrels, Amsterdam, the Netherlands.
// Copyright (c) 2008-2015 Bruno Lalande, Paris, France.
// Copyright (c) 2009-2015 Mateusz Loskot, London, UK.

// Parts of Boost.Geometry are redesigned from Geodan's Geographic Library
// (geolib/GGL), copyright (c) 1995-2010 Geodan, Amsterdam, the Netherlands.

// Use, modification and distribution is subject to the Boost Software License,
// Version 1.0. (See accompanying file LICENSE_1_0.txt or copy at
// http://www.boost.org/LICENSE_1_0.txt)

#ifndef BOOST_GEOMETRY_ALGORITHMS_SIMPLIFY_HPP
#define BOOST_GEOMETRY_ALGORITHMS_SIMPLIFY_HPP

#include <cstddef>

#include <boost/core/ignore_unused.hpp>
#include <boost/range.hpp>
#include <boost/range/adaptor/sliced.hpp>

#include <boost/variant/apply_visitor.hpp>
#include <boost/variant/static_visitor.hpp>
#include <boost/variant/variant_fwd.hpp>

#include <boost/geometry/core/cs.hpp>
#include <boost/geometry/core/closure.hpp>
#include <boost/geometry/core/exterior_ring.hpp>
#include <boost/geometry/core/interior_rings.hpp>
#include <boost/geometry/core/mutable_range.hpp>
#include <boost/geometry/core/tags.hpp>

#include <boost/geometry/geometries/concepts/check.hpp>
#include <boost/geometry/strategies/agnostic/simplify_douglas_peucker.hpp>
#include <boost/geometry/strategies/concepts/simplify_concept.hpp>
#include <boost/geometry/strategies/default_strategy.hpp>
#include <boost/geometry/strategies/distance.hpp>

#include <boost/geometry/algorithms/area.hpp>
#include <boost/geometry/algorithms/clear.hpp>
#include <boost/geometry/algorithms/convert.hpp>
#include <boost/geometry/algorithms/correct_closure.hpp>
#include <boost/geometry/algorithms/equals.hpp>
#include <boost/geometry/algorithms/not_implemented.hpp>
#include <boost/geometry/algorithms/is_empty.hpp>

#include <boost/geometry/algorithms/detail/distance/default_strategies.hpp>

namespace boost { namespace geometry
{

#ifndef DOXYGEN_NO_DETAIL
namespace detail { namespace simplify
{

struct simplify_range_insert
{
    template<typename Range, typename Strategy, typename OutputIterator, typename Distance>
    static inline void apply(Range const& range, OutputIterator out,
                             Distance const& max_distance, Strategy const& strategy)
    {
        boost::ignore_unused(strategy);

        if (boost::size(range) <= 2 || max_distance < 0)
        {
            std::copy(boost::begin(range), boost::end(range), out);
        }
        else
        {
            strategy.apply(range, out, max_distance);
        }
    }
};


struct simplify_copy
{
    template <typename RangeIn, typename RangeOut, typename Strategy, typename Distance>
    static inline void apply(RangeIn const& range, RangeOut& out,
                             Distance const& , Strategy const& )
    {
        std::copy
            (
                boost::begin(range), boost::end(range),
                    geometry::range::back_inserter(out)
            );
    }
};


template <std::size_t MinimumToUseStrategy>
struct simplify_range
{
    template <typename RangeIn, typename RangeOut, typename Strategy, typename Distance>
    static inline void apply(RangeIn const& range, RangeOut& out,
                    Distance const& max_distance, Strategy const& strategy)
    {
        // For a RING:
        // Note that, especially if max_distance is too large,
        // the output ring might be self intersecting while the input ring is
        // not, although chances are low in normal polygons

        if (boost::size(range) <= static_cast<int>(MinimumToUseStrategy) || max_distance < 0)
        {
            simplify_copy::apply(range, out, max_distance, strategy);
        }
        else
        {
            simplify_range_insert::apply
                (
                    range, geometry::range::back_inserter(out), max_distance, strategy
                );
        }

        // Verify the two remaining points are equal. If so, remove one of them.
        // This can cause the output being under the minimum size
        if (boost::size(out) == 2
            && geometry::equals(geometry::range::front(out),
                    geometry::range::back(out)))
        {
            traits::resize<RangeOut>::apply(out, 1);
        }
    }
};

struct simplify_closure_inspector
{
private:
    template
    <
        typename Range,
        typename Strategy,
        typename Distance
    >
    static inline std::size_t simplified_size(Range const& range,
            Distance const& distance, Strategy const& strategy)
    {
        Range simplified;
        simplify_range<0>::apply(range, simplified, distance, strategy);
        return boost::size(simplified);
    }

public:
    template
    <
        typename Ring,
        typename Strategy,
        typename Distance
    >
    static inline void get_start_end(std::size_t& start, std::size_t& end,
        Ring const& ring, Distance const& distance, Strategy const& strategy)
    {
        std::size_t const size = boost::size(ring);
        if (size < 5)
        {
            // Smallest useful closed polygon, where the closing point could
            // be simplified away, would have 4 points (to result into a
            // triangle)
            return;
        }

        // Assuming a CLOSED polygon:
        // Create 3-point range and verify if middle point would be
        // simplified away. If so, set start accordingly
        Ring closing;
        range::push_back(closing, range::at(ring, end - 1));
        range::push_back(closing, range::at(ring, 0));
        range::push_back(closing, range::at(ring, 1));

        std::size_t simp_size = simplified_size(closing, distance, strategy);
        while (simp_size == 2 && start < size - 2)
        {
            start++;

            // Try to add more points at start:
            range::push_back(closing, range::at(ring, start + 1));
            simp_size = simplified_size(closing, distance, strategy);
        }

        // Verify if more points from end can be added
        // Note that simplify works same in two directions.
        // Avoid simplifying the whole geometry away,
        // so avoid using the same point at end as at start,
        geometry::clear(closing);
        range::push_back(closing, range::at(ring, 0));
        range::push_back(closing, range::at(ring, end - 1));
        range::push_back(closing, range::at(ring, end - 2));

        simp_size = simplified_size(closing, distance, strategy);
        while (simp_size == 2 && end - 3 > start)
        {
            end--;

            // Try to add more points from end:
            range::push_back(closing, range::at(ring, end - 2));
            simp_size = simplified_size(closing, distance, strategy);
        }
    }
};


struct simplify_ring
{
    template <typename Area>
    static inline int area_sign(Area const& area)
    {
        return area > 0 ? 1 : area < 0 ? -1 : 0;
    }

    template <typename Ring, typename Strategy, typename Distance>
    static inline void apply(Ring const& ring, Ring& out,
                    Distance const& max_distance, Strategy const& strategy)
    {
        if (boost::empty(ring))
        {
            return;
        }

        static closure_selector const closure = geometry::closure<Ring>::value;

        std::size_t const size = boost::size(ring);
        std::size_t start = 0;
        std::size_t end = size - 1; // index of last point

        // Verify that what was positive, stays positive (or goes to 0)
        // and what was negative stays negative (or goes to 0)
        int const input_sign = area_sign(geometry::area(ring));

        if (closure == geometry::closed)
        {
            // Verify area around closing point, if that can be simplified,
            // start/end are modified and a corresponding slice will be used
            // for simplification

            // Take only a part of simplify distance, to avoid aggressive
            // behaviour at closing points (it is known as a "open problem")

            // TODO: for open polygons, implementation should be modified
            simplify_closure_inspector::get_start_end(start, end, ring,
                    max_distance / 2.0, strategy);
        }

        bool apply_unsliced = true;

        if (start > 0) // checking end is not necessary
        {
            using namespace boost::adaptors;

            apply_unsliced = false;
            simplify_range<0>::apply(ring | sliced(start, end),
                         out, max_distance, strategy);

            // Close it
            geometry::correct_closure(out);

            // Verify if result did not botch original behaviour
            int const sliced_sign = area_sign(geometry::area(out));
            if (input_sign != sliced_sign)
            {
                // Redo
                geometry::clear(out);
                apply_unsliced = true;
            }
        }

        if (apply_unsliced)
        {
            simplify_range<0>::apply(ring, out, max_distance, strategy);
        }

        int const output_sign = area_sign(geometry::area(out));
        if (output_sign != input_sign)
        {
            // Original is simplified away, or inversed by simplification
            geometry::clear(out);
        }
    }

};


struct simplify_polygon
{
private:

    template
    <
        typename IteratorIn,
        typename InteriorRingsOut,
        typename Distance,
        typename Strategy
    >
    static inline void iterate(IteratorIn begin, IteratorIn end,
                    InteriorRingsOut& interior_rings_out,
                    Distance const& max_distance, Strategy const& strategy)
    {
        typedef typename boost::range_value<InteriorRingsOut>::type single_type;
        for (IteratorIn it_in = begin; it_in != end; ++it_in)
        {
            single_type out;
            simplify_ring::apply(*it_in, out, max_distance, strategy);
            if (! geometry::is_empty(out))
            {
                traits::push_back<InteriorRingsOut>::apply(interior_rings_out, out);
            }
        }
    }

    template
    <
        typename InteriorRingsIn,
        typename InteriorRingsOut,
        typename Distance,
        typename Strategy
    >
    static inline void apply_interior_rings(
                    InteriorRingsIn const& interior_rings_in,
                    InteriorRingsOut& interior_rings_out,
                    Distance const& max_distance, Strategy const& strategy)
    {
        traits::clear<InteriorRingsOut>::apply(interior_rings_out);

        iterate(
            boost::begin(interior_rings_in), boost::end(interior_rings_in),
            interior_rings_out,
            max_distance, strategy);
    }

public:
    template <typename Polygon, typename Strategy, typename Distance>
    static inline void apply(Polygon const& poly_in, Polygon& poly_out,
                    Distance const& max_distance, Strategy const& strategy)
    {
        // Note that if there are inner rings, and distance is too large,
        // they might intersect with the outer ring in the output,
        // while it didn't in the input.
        simplify_ring::apply(exterior_ring(poly_in), exterior_ring(poly_out),
            max_distance, strategy);

        apply_interior_rings(interior_rings(poly_in),
            interior_rings(poly_out), max_distance, strategy);
    }
};


template<typename Policy>
struct simplify_multi
{
    template <typename MultiGeometry, typename Strategy, typename Distance>
    static inline void apply(MultiGeometry const& multi, MultiGeometry& out,
                    Distance const& max_distance, Strategy const& strategy)
    {
        traits::clear<MultiGeometry>::apply(out);

        typedef typename boost::range_value<MultiGeometry>::type single_type;

        for (typename boost::range_iterator<MultiGeometry const>::type
                it_in = boost::begin(multi);
             it_in != boost::end(multi);
             ++it_in)
        {
            single_type single_out;
            Policy::apply(*it_in, single_out, max_distance, strategy);
            if (! geometry::is_empty(single_out))
            {
                traits::push_back<MultiGeometry>::apply(out, single_out);
            }
        }
    }
};


}} // namespace detail::simplify
#endif // DOXYGEN_NO_DETAIL


#ifndef DOXYGEN_NO_DISPATCH
namespace dispatch
{

template
<
    typename Geometry,
    typename Tag = typename tag<Geometry>::type
>
struct simplify: not_implemented<Tag>
{};

template <typename Point>
struct simplify<Point, point_tag>
{
    template <typename Distance, typename Strategy>
    static inline void apply(Point const& point, Point& out,
                    Distance const& , Strategy const& )
    {
        geometry::convert(point, out);
    }
};

// Linestring, keep 2 points (unless those points are the same)
template <typename Linestring>
struct simplify<Linestring, linestring_tag>
    : detail::simplify::simplify_range<2>
{};

template <typename Ring>
struct simplify<Ring, ring_tag>
    : detail::simplify::simplify_ring
{};

template <typename Polygon>
struct simplify<Polygon, polygon_tag>
    : detail::simplify::simplify_polygon
{};


template
<
    typename Geometry,
    typename Tag = typename tag<Geometry>::type
>
struct simplify_insert: not_implemented<Tag>
{};


template <typename Linestring>
struct simplify_insert<Linestring, linestring_tag>
    : detail::simplify::simplify_range_insert
{};

template <typename Ring>
struct simplify_insert<Ring, ring_tag>
    : detail::simplify::simplify_range_insert
{};

template <typename MultiPoint>
struct simplify<MultiPoint, multi_point_tag>
    : detail::simplify::simplify_copy
{};


template <typename MultiLinestring>
struct simplify<MultiLinestring, multi_linestring_tag>
    : detail::simplify::simplify_multi<detail::simplify::simplify_range<2> >
{};


template <typename MultiPolygon>
struct simplify<MultiPolygon, multi_polygon_tag>
    : detail::simplify::simplify_multi<detail::simplify::simplify_polygon>
{};


} // namespace dispatch
#endif // DOXYGEN_NO_DISPATCH


namespace resolve_strategy
{

struct simplify
{
    template <typename Geometry, typename Distance, typename Strategy>
    static inline void apply(Geometry const& geometry,
                             Geometry& out,
                             Distance const& max_distance,
                             Strategy const& strategy)
    {
        dispatch::simplify<Geometry>::apply(geometry, out, max_distance, strategy);
    }

    template <typename Geometry, typename Distance>
    static inline void apply(Geometry const& geometry,
                             Geometry& out,
                             Distance const& max_distance,
                             default_strategy)
    {
        typedef typename point_type<Geometry>::type point_type;

        typedef typename strategy::distance::services::default_strategy
        <
            point_tag, segment_tag, point_type
        >::type ds_strategy_type;

        typedef strategy::simplify::douglas_peucker
        <
            point_type, ds_strategy_type
        > strategy_type;

        BOOST_CONCEPT_ASSERT(
            (concepts::SimplifyStrategy<strategy_type, point_type>)
        );

        apply(geometry, out, max_distance, strategy_type());
    }
};

struct simplify_insert
{
    template
    <
        typename Geometry,
        typename OutputIterator,
        typename Distance,
        typename Strategy
    >
    static inline void apply(Geometry const& geometry,
                             OutputIterator& out,
                             Distance const& max_distance,
                             Strategy const& strategy)
    {
        dispatch::simplify_insert<Geometry>::apply(geometry, out, max_distance, strategy);
    }

    template <typename Geometry, typename OutputIterator, typename Distance>
    static inline void apply(Geometry const& geometry,
                             OutputIterator& out,
                             Distance const& max_distance,
                             default_strategy)
    {
        typedef typename point_type<Geometry>::type point_type;

        typedef typename strategy::distance::services::default_strategy
        <
            point_tag, segment_tag, point_type
        >::type ds_strategy_type;

        typedef strategy::simplify::douglas_peucker
        <
            point_type, ds_strategy_type
        > strategy_type;

        BOOST_CONCEPT_ASSERT(
            (concepts::SimplifyStrategy<strategy_type, point_type>)
        );

        apply(geometry, out, max_distance, strategy_type());
    }
};

} // namespace resolve_strategy


namespace resolve_variant {

template <typename Geometry>
struct simplify
{
    template <typename Distance, typename Strategy>
    static inline void apply(Geometry const& geometry,
                             Geometry& out,
                             Distance const& max_distance,
                             Strategy const& strategy)
    {
        resolve_strategy::simplify::apply(geometry, out, max_distance, strategy);
    }
};

template <BOOST_VARIANT_ENUM_PARAMS(typename T)>
struct simplify<boost::variant<BOOST_VARIANT_ENUM_PARAMS(T)> >
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


/*!
\brief Simplify a geometry using a specified strategy
\ingroup simplify
\tparam Geometry \tparam_geometry
\tparam Distance A numerical distance measure
\tparam Strategy A type fulfilling a SimplifyStrategy concept
\param strategy A strategy to calculate simplification
\param geometry input geometry, to be simplified
\param out output geometry, simplified version of the input geometry
\param max_distance distance (in units of input coordinates) of a vertex
    to other segments to be removed
\param strategy simplify strategy to be used for simplification, might
    include point-distance strategy

\image html svg_simplify_country.png "The image below presents the simplified country"
\qbk{distinguish,with strategy}
*/
template<typename Geometry, typename Distance, typename Strategy>
inline void simplify(Geometry const& geometry, Geometry& out,
                     Distance const& max_distance, Strategy const& strategy)
{
    concepts::check<Geometry>();

    geometry::clear(out);

    resolve_variant::simplify<Geometry>::apply(geometry, out, max_distance, strategy);
}




/*!
\brief Simplify a geometry
\ingroup simplify
\tparam Geometry \tparam_geometry
\tparam Distance \tparam_numeric
\note This version of simplify simplifies a geometry using the default
    strategy (Douglas Peucker),
\param geometry input geometry, to be simplified
\param out output geometry, simplified version of the input geometry
\param max_distance distance (in units of input coordinates) of a vertex
    to other segments to be removed

\qbk{[include reference/algorithms/simplify.qbk]}
 */
template<typename Geometry, typename Distance>
inline void simplify(Geometry const& geometry, Geometry& out,
                     Distance const& max_distance)
{
    concepts::check<Geometry>();

    geometry::simplify(geometry, out, max_distance, default_strategy());
}


#ifndef DOXYGEN_NO_DETAIL
namespace detail { namespace simplify
{


/*!
\brief Simplify a geometry, using an output iterator
    and a specified strategy
\ingroup simplify
\tparam Geometry \tparam_geometry
\param geometry input geometry, to be simplified
\param out output iterator, outputs all simplified points
\param max_distance distance (in units of input coordinates) of a vertex
    to other segments to be removed
\param strategy simplify strategy to be used for simplification,
    might include point-distance strategy

\qbk{distinguish,with strategy}
\qbk{[include reference/algorithms/simplify.qbk]}
*/
template<typename Geometry, typename OutputIterator, typename Distance, typename Strategy>
inline void simplify_insert(Geometry const& geometry, OutputIterator out,
                            Distance const& max_distance, Strategy const& strategy)
{
    concepts::check<Geometry const>();

    resolve_strategy::simplify_insert::apply(geometry, out, max_distance, strategy);
}

/*!
\brief Simplify a geometry, using an output iterator
\ingroup simplify
\tparam Geometry \tparam_geometry
\param geometry input geometry, to be simplified
\param out output iterator, outputs all simplified points
\param max_distance distance (in units of input coordinates) of a vertex
    to other segments to be removed

\qbk{[include reference/algorithms/simplify_insert.qbk]}
 */
template<typename Geometry, typename OutputIterator, typename Distance>
inline void simplify_insert(Geometry const& geometry, OutputIterator out,
                            Distance const& max_distance)
{
    // Concept: output point type = point type of input geometry
    concepts::check<Geometry const>();
    concepts::check<typename point_type<Geometry>::type>();

    simplify_insert(geometry, out, max_distance, default_strategy());
}

}} // namespace detail::simplify
#endif // DOXYGEN_NO_DETAIL



}} // namespace boost::geometry

#endif // BOOST_GEOMETRY_ALGORITHMS_SIMPLIFY_HPP
