// Boost.Geometry (aka GGL, Generic Geometry Library)

// Copyright (c) 2007-2015 Barend Gehrels, Amsterdam, the Netherlands.
// Copyright (c) 2008-2015 Bruno Lalande, Paris, France.
// Copyright (c) 2009-2015 Mateusz Loskot, London, UK.

// This file was modified by Oracle on 2014-2021.
// Modifications copyright (c) 2014-2021 Oracle and/or its affiliates.

// Contributed and/or modified by Vissarion Fysikopoulos, on behalf of Oracle
// Contributed and/or modified by Menelaos Karavelas, on behalf of Oracle
// Contributed and/or modified by Adam Wulkiewicz, on behalf of Oracle

// Parts of Boost.Geometry are redesigned from Geodan's Geographic Library
// (geolib/GGL), copyright (c) 1995-2010 Geodan, Amsterdam, the Netherlands.

// Use, modification and distribution is subject to the Boost Software License,
// Version 1.0. (See accompanying file LICENSE_1_0.txt or copy at
// http://www.boost.org/LICENSE_1_0.txt)

#ifndef BOOST_GEOMETRY_ALGORITHMS_CONVEX_HULL_INTERFACE_HPP
#define BOOST_GEOMETRY_ALGORITHMS_CONVEX_HULL_INTERFACE_HPP

#include <array>

#include <boost/geometry/algorithms/detail/assign_box_corners.hpp>
#include <boost/geometry/algorithms/detail/convex_hull/graham_andrew.hpp>
#include <boost/geometry/algorithms/detail/equals/point_point.hpp>
#include <boost/geometry/algorithms/detail/for_each_range.hpp>
#include <boost/geometry/algorithms/detail/select_geometry_type.hpp>
#include <boost/geometry/algorithms/detail/visit.hpp>
#include <boost/geometry/algorithms/is_empty.hpp>

#include <boost/geometry/core/closure.hpp>
#include <boost/geometry/core/cs.hpp>
#include <boost/geometry/core/exterior_ring.hpp>
#include <boost/geometry/core/geometry_types.hpp>
#include <boost/geometry/core/point_order.hpp>
#include <boost/geometry/core/ring_type.hpp>
#include <boost/geometry/core/tag.hpp>
#include <boost/geometry/core/tags.hpp>
#include <boost/geometry/core/visit.hpp>

#include <boost/geometry/geometries/adapted/boost_variant.hpp> // For backward compatibility
#include <boost/geometry/geometries/concepts/check.hpp>
#include <boost/geometry/geometries/ring.hpp>

#include <boost/geometry/strategies/convex_hull/cartesian.hpp>
#include <boost/geometry/strategies/convex_hull/geographic.hpp>
#include <boost/geometry/strategies/convex_hull/spherical.hpp>
#include <boost/geometry/strategies/default_strategy.hpp>

#include <boost/geometry/util/condition.hpp>
#include <boost/geometry/util/range.hpp>
#include <boost/geometry/util/sequence.hpp>
#include <boost/geometry/util/type_traits.hpp>


namespace boost { namespace geometry
{

// TODO: This file is named interface.hpp but the code below is not the interface.
//       It's the implementation of the algorithm.

#ifndef DOXYGEN_NO_DETAIL
namespace detail { namespace convex_hull
{

// Abstraction representing ranges/rings of a geometry
template <typename Geometry>
struct geometry_ranges
{
    geometry_ranges(Geometry const& geometry)
        : m_geometry(geometry)
    {}

    template <typename UnaryFunction>
    inline void for_each_range(UnaryFunction fun) const
    {
        geometry::detail::for_each_range(m_geometry, fun);
    }

    Geometry const& m_geometry;
};

// Abstraction representing ranges/rings of subgeometries of geometry collection
// with boxes converted to rings
template <typename Geometry, typename BoxRings>
struct geometry_collection_ranges
{
    geometry_collection_ranges(Geometry const& geometry, BoxRings const& box_rings)
        : m_geometry(geometry)
        , m_box_rings(box_rings)
    {}

    template <typename UnaryFunction>
    inline void for_each_range(UnaryFunction fun) const
    {
        detail::visit_breadth_first([&](auto const& g)
        {
            call_for_non_boxes(g, fun);
            return true;
        }, m_geometry);

        for (auto const& r : m_box_rings)
        {
            geometry::detail::for_each_range(r, fun);
        }
    }

private:
    template <typename G, typename F, std::enable_if_t<! util::is_box<G>::value, int> = 0>
    static inline void call_for_non_boxes(G const& g, F & f)
    {
        geometry::detail::for_each_range(g, f);
    }
    template <typename G, typename F, std::enable_if_t<util::is_box<G>::value, int> = 0>
    static inline void call_for_non_boxes(G const&, F &)
    {}

    Geometry const& m_geometry;
    BoxRings const& m_box_rings;
};


// TODO: Or just implement point_type<> for GeometryCollection
//   and enforce the same point_type used in the whole sequence in check().
template <typename Geometry, typename Tag = typename tag<Geometry>::type>
struct default_strategy
{
    using type = typename strategies::convex_hull::services::default_strategy
        <
            Geometry
        >::type;
};

template <typename Geometry>
struct default_strategy<Geometry, geometry_collection_tag>
    : default_strategy<typename detail::first_geometry_type<Geometry>::type>
{};


// Wrapper for output geometry. It contains reference to OutputGeometry and may contain
// a temporary geometry which is moved to the output at the end of the algorithm.
template <typename OutputGeometry, typename Tag = typename tag<OutputGeometry>::type>
struct output_geometry
{
    BOOST_GEOMETRY_STATIC_ASSERT_FALSE("This OutputGeometry is not supported.", OutputGeometry, Tag);
};

// For backward compatibility
template <typename OutputGeometry>
struct output_geometry<OutputGeometry, linestring_tag>
{
    output_geometry(OutputGeometry & out) : m_out(out) {}
    OutputGeometry & range() { return m_out; }
    template <typename Strategy>
    void move_to_out(Strategy const& ) {}
private:
    OutputGeometry & m_out;
};

template <typename OutputGeometry>
struct output_geometry<OutputGeometry, ring_tag>
{
    output_geometry(OutputGeometry & out) : m_out(out) {}
    OutputGeometry & range() { return m_out; }
    template <typename Strategy>
    void move_to_out(Strategy const& ) {}
private:
    OutputGeometry & m_out;
};

template <typename OutputGeometry>
struct output_geometry<OutputGeometry, polygon_tag>
{
    output_geometry(OutputGeometry & out) : m_out(out) {}
    decltype(auto) range() { return exterior_ring(m_out); }
    template <typename Strategy>
    void move_to_out(Strategy const& ) {}
private:
    OutputGeometry & m_out;
};

template <typename OutputGeometry>
struct output_geometry<OutputGeometry, multi_polygon_tag>
{
    output_geometry(OutputGeometry & out) : m_out(out) {}
    decltype(auto) range() { return exterior_ring(m_polygon); }
    template <typename Strategy>
    void move_to_out(Strategy const& )
    {
        if (! boost::empty(exterior_ring(m_polygon)))
        {
            range::push_back(m_out, std::move(m_polygon));
        }
    }
private:
    OutputGeometry & m_out;
    typename boost::range_value<OutputGeometry>::type m_polygon;
};

template <typename G1, typename G2>
struct output_polygonal_less
{
    template <typename G>
    using priority = std::integral_constant
        <
            int,
            (util::is_ring<G>::value ? 0 :
             util::is_polygon<G>::value ? 1 :
             util::is_multi_polygon<G>::value ? 2 : 3)
        >;

    static const bool value = priority<G1>::value < priority<G2>::value;
};

template <typename G1, typename G2>
struct output_linear_less
{
    template <typename G>
    using priority = std::integral_constant
        <
            int,
            (util::is_segment<G>::value ? 0 :
             util::is_linestring<G>::value ? 1 :
             util::is_multi_linestring<G>::value ? 2 : 3)
        >;

    static const bool value = priority<G1>::value < priority<G2>::value;
};

template <typename G1, typename G2>
struct output_pointlike_less
{
    template <typename G>
    using priority = std::integral_constant
        <
            int,
            (util::is_point<G>::value ? 0 :
             util::is_multi_point<G>::value ? 1 : 2)
        >;

    static const bool value = priority<G1>::value < priority<G2>::value;
};

template <typename OutputGeometry>
struct output_geometry<OutputGeometry, geometry_collection_tag>
{
    using polygonal_t = typename util::sequence_min_element
        <
            typename traits::geometry_types<OutputGeometry>::type,
            output_polygonal_less
        >::type;
    using linear_t = typename util::sequence_min_element
        <
            typename traits::geometry_types<OutputGeometry>::type,
            output_linear_less
        >::type;
    using pointlike_t = typename util::sequence_min_element
        <
            typename traits::geometry_types<OutputGeometry>::type,
            output_pointlike_less
        >::type;

    // select_element may define different kind of geometry than the one that is desired
    BOOST_GEOMETRY_STATIC_ASSERT(util::is_polygonal<polygonal_t>::value,
        "It must be possible to store polygonal geometry in GeometryCollection.", polygonal_t);
    BOOST_GEOMETRY_STATIC_ASSERT(util::is_linear<linear_t>::value,
        "It must be possible to store linear geometry in GeometryCollection.", linear_t);
    BOOST_GEOMETRY_STATIC_ASSERT(util::is_pointlike<pointlike_t>::value,
        "It must be possible to store pointlike geometry in GeometryCollection.", pointlike_t);

    output_geometry(OutputGeometry & out)
        : m_out(out), m_wrapper(m_polygonal)
    {}

    decltype(auto) range() { return m_wrapper.range(); }
    
    template <typename Strategy>
    void move_to_out(Strategy const& strategy)
    {
        auto&& out_range = m_wrapper.range();
        if (! boost::empty(out_range))
        {
            auto size = boost::size(out_range);
            if (size > minimum_ring_size<polygonal_t>::value)
            {
                m_wrapper.move_to_out(strategy);
                range::emplace_back(m_out, std::move(m_polygonal));
            }
            else // size == 3 || size == 4
            {
                if (detail::equals::equals_point_point(range::front(out_range), range::at(out_range, 1), strategy))
                {
                    pointlike_t pointlike;
                    move_to_pointlike(out_range, pointlike);
                    range::emplace_back(m_out, std::move(pointlike));
                }
                else if (detail::equals::equals_point_point(range::front(out_range), range::at(out_range, 2), strategy))
                {
                    linear_t linear;
                    move_to_linear(out_range, linear);
                    range::emplace_back(m_out, std::move(linear));
                }
                else
                {
                    m_wrapper.move_to_out(strategy);
                    range::emplace_back(m_out, std::move(m_polygonal));
                }
            }
        }
    }

private:
    template <typename Range, typename Linear, util::enable_if_segment_t<Linear, int> = 0>
    static void move_to_linear(Range & out_range, Linear & seg)
    {
        detail::assign_point_to_index<0>(range::front(out_range), seg);
        detail::assign_point_to_index<1>(range::at(out_range, 1), seg);
    }
    template <typename Range, typename Linear, util::enable_if_linestring_t<Linear, int> = 0>
    static void move_to_linear(Range & out_range, Linear & ls)
    {
        std::move(boost::begin(out_range), boost::begin(out_range) + 2, range::back_inserter(ls));
    }
    template <typename Range, typename Linear, util::enable_if_multi_linestring_t<Linear, int> = 0>
    static void move_to_linear(Range & out_range, Linear & mls)
    {
        typename boost::range_value<Linear>::type ls;
        std::move(boost::begin(out_range), boost::begin(out_range) + 2, range::back_inserter(ls));
        range::push_back(mls, std::move(ls));
    }

    template <typename Range, typename PointLike, util::enable_if_point_t<PointLike, int> = 0>
    static void move_to_pointlike(Range & out_range, PointLike & pt)
    {
        pt = range::front(out_range);
    }
    template <typename Range, typename PointLike, util::enable_if_multi_point_t<PointLike, int> = 0>
    static void move_to_pointlike(Range & out_range, PointLike & mpt)
    {
        range::push_back(mpt, std::move(range::front(out_range)));
    }

    OutputGeometry & m_out;
    polygonal_t m_polygonal;
    output_geometry<polygonal_t> m_wrapper;
};


}} // namespace detail::convex_hull
#endif // DOXYGEN_NO_DETAIL


#ifndef DOXYGEN_NO_DISPATCH
namespace dispatch
{


template
<
    typename Geometry,
    typename Tag = typename tag<Geometry>::type
>
struct convex_hull
{
    template <typename OutputGeometry, typename Strategy>
    static inline void apply(Geometry const& geometry,
                             OutputGeometry& out,
                             Strategy const& strategy)
    {
        detail::convex_hull::geometry_ranges<Geometry> ranges(geometry);

        detail::convex_hull::output_geometry<OutputGeometry> out_wrapper(out);
        // NOTE: A variable is created here because this can be a proxy range
        //       and back_insert_iterator<> can store a pointer to it.
        auto&& out_range = out_wrapper.range();

        detail::convex_hull::graham_andrew
            <
                typename point_type<Geometry>::type
            >::apply(ranges, out_range, strategy);

        out_wrapper.move_to_out(strategy);
    }
};


// A hull for boxes is trivial. Any strategy is (currently) skipped.
// TODO: This is not correct in spherical and geographic CS.
template <typename Box>
struct convex_hull<Box, box_tag>
{
    template <typename OutputGeometry, typename Strategy>
    static inline void apply(Box const& box,
                             OutputGeometry& out,
                             Strategy const& strategy)
    {
        detail::convex_hull::output_geometry<OutputGeometry> out_wrapper(out);
        // NOTE: A variable is created here because this can be a proxy range
        //       and back_insert_iterator<> can store a pointer to it.
        auto&& out_range = out_wrapper.range();

        using out_range_t = std::remove_reference_t<decltype(out_range)>;
        static bool const Close
            = geometry::closure<out_range_t>::value == closed;
        static bool const Reverse
            = geometry::point_order<out_range_t>::value == counterclockwise;

        std::array<typename point_type<out_range_t>::type, 4> arr;
        // TODO: This assigns only 2d cooridnates!
        //       And it is also used in box_view<>!
        geometry::detail::assign_box_corners_oriented<Reverse>(box, arr);

        std::move(arr.begin(), arr.end(), range::back_inserter(out_range));
        if (BOOST_GEOMETRY_CONDITION(Close))
        {
            range::push_back(out_range, range::front(out_range));
        }

        out_wrapper.move_to_out(strategy);
    }
};


template <typename GeometryCollection>
struct convex_hull<GeometryCollection, geometry_collection_tag>
{
    template <typename OutputGeometry, typename Strategy>
    static inline void apply(GeometryCollection const& geometry,
                             OutputGeometry& out,
                             Strategy const& strategy)
    {
        // Assuming that single point_type is used by the GeometryCollection
        using subgeometry_type = typename detail::first_geometry_type<GeometryCollection>::type;
        using point_type = typename geometry::point_type<subgeometry_type>::type;
        using ring_type = model::ring<point_type, true, false>;

        // Calculate box rings once
        std::vector<ring_type> box_rings;
        detail::visit_breadth_first([&](auto const& g)
        {
            add_ring_for_box(box_rings, g, strategy);
            return true;
        }, geometry);

        detail::convex_hull::geometry_collection_ranges
            <
                GeometryCollection, std::vector<ring_type>
            > ranges(geometry, box_rings);

        detail::convex_hull::output_geometry<OutputGeometry> out_wrapper(out);
        // NOTE: A variable is created here because this can be a proxy range
        //       and back_insert_iterator<> can store a pointer to it.
        auto&& out_range = out_wrapper.range();

        detail::convex_hull::graham_andrew
            <
                point_type
            >::apply(ranges, out_range, strategy);

        out_wrapper.move_to_out(strategy);
    }

private:
    template
    <
        typename Ring, typename SubGeometry, typename Strategy,
        std::enable_if_t<util::is_box<SubGeometry>::value, int> = 0
    >
    static inline void add_ring_for_box(std::vector<Ring> & rings, SubGeometry const& box,
                                        Strategy const& strategy)
    {
        Ring ring;
        convex_hull<SubGeometry>::apply(box, ring, strategy);
        rings.push_back(std::move(ring));
    }
    template
    <
        typename Ring, typename SubGeometry, typename Strategy,
        std::enable_if_t<! util::is_box<SubGeometry>::value, int> = 0
    >
    static inline void add_ring_for_box(std::vector<Ring> & , SubGeometry const& ,
                                        Strategy const& )
    {}
};


} // namespace dispatch
#endif // DOXYGEN_NO_DISPATCH


namespace resolve_strategy {

template <typename Strategies>
struct convex_hull
{
    template <typename Geometry, typename OutputGeometry>
    static inline void apply(Geometry const& geometry,
                             OutputGeometry& out,
                             Strategies const& strategies)
    {
        dispatch::convex_hull<Geometry>::apply(geometry, out, strategies);
    }
};

template <>
struct convex_hull<default_strategy>
{
    template <typename Geometry, typename OutputGeometry>
    static inline void apply(Geometry const& geometry,
                             OutputGeometry& out,
                             default_strategy const&)
    {
        using strategy_type = typename detail::convex_hull::default_strategy
            <
                Geometry
            >::type;

        dispatch::convex_hull<Geometry>::apply(geometry, out, strategy_type());
    }
};


} // namespace resolve_strategy


namespace resolve_dynamic {

template <typename Geometry, typename Tag = typename tag<Geometry>::type>
struct convex_hull
{
    template <typename OutputGeometry, typename Strategy>
    static inline void apply(Geometry const& geometry,
                             OutputGeometry& out,
                             Strategy const& strategy)
    {
        concepts::check_concepts_and_equal_dimensions<
            const Geometry,
            OutputGeometry
        >();

        resolve_strategy::convex_hull<Strategy>::apply(geometry, out, strategy);
    }
};

template <typename Geometry>
struct convex_hull<Geometry, dynamic_geometry_tag>
{
    template <typename OutputGeometry, typename Strategy>
    static inline void apply(Geometry const& geometry,
                             OutputGeometry& out,
                             Strategy const& strategy)
    {
        traits::visit<Geometry>::apply([&](auto const& g)
        {
            convex_hull<util::remove_cref_t<decltype(g)>>::apply(g, out, strategy);
        }, geometry);
    }
};


} // namespace resolve_dynamic


/*!
\brief \brief_calc{convex hull} \brief_strategy
\ingroup convex_hull
\details \details_calc{convex_hull,convex hull} \brief_strategy.
\tparam Geometry the input geometry type
\tparam OutputGeometry the output geometry type
\tparam Strategy the strategy type
\param geometry \param_geometry,  input geometry
\param out \param_geometry \param_set{convex hull}
\param strategy \param_strategy{area}

\qbk{distinguish,with strategy}

\qbk{[include reference/algorithms/convex_hull.qbk]}
 */
template<typename Geometry, typename OutputGeometry, typename Strategy>
inline void convex_hull(Geometry const& geometry, OutputGeometry& out, Strategy const& strategy)
{
    if (geometry::is_empty(geometry))
    {
        // Leave output empty
        return;
    }

    resolve_dynamic::convex_hull<Geometry>::apply(geometry, out, strategy);
}


/*!
\brief \brief_calc{convex hull}
\ingroup convex_hull
\details \details_calc{convex_hull,convex hull}.
\tparam Geometry the input geometry type
\tparam OutputGeometry the output geometry type
\param geometry \param_geometry,  input geometry
\param hull \param_geometry \param_set{convex hull}

\qbk{[include reference/algorithms/convex_hull.qbk]}
 */
template<typename Geometry, typename OutputGeometry>
inline void convex_hull(Geometry const& geometry, OutputGeometry& hull)
{
    geometry::convex_hull(geometry, hull, default_strategy());
}


}} // namespace boost::geometry


#endif // BOOST_GEOMETRY_ALGORITHMS_CONVEX_HULL_INTERFACE_HPP
