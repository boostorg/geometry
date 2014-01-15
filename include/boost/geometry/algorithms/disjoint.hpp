// Boost.Geometry (aka GGL, Generic Geometry Library)

// Copyright (c) 2007-2012 Barend Gehrels, Amsterdam, the Netherlands.
// Copyright (c) 2008-2012 Bruno Lalande, Paris, France.
// Copyright (c) 2009-2012 Mateusz Loskot, London, UK.
// Copyright (c) 2013 Adam Wulkiewicz, Lodz, Poland.

// This file was modified by Oracle on 2013.
// Modifications copyright (c) 2013, Oracle and/or its affiliates.

// Parts of Boost.Geometry are redesigned from Geodan's Geographic Library
// (geolib/GGL), copyright (c) 1995-2010 Geodan, Amsterdam, the Netherlands.

// Use, modification and distribution is subject to the Boost Software License,
// Version 1.0. (See accompanying file LICENSE_1_0.txt or copy at
// http://www.boost.org/LICENSE_1_0.txt)

// Contributed/Modified by Adam Wulkiewicz, Poland, 2013, on behalf of Oracle.
// Contributed/Modified by Menelaos Karavelas, Greece, 2014, on behalf of Oracle.

#ifndef BOOST_GEOMETRY_ALGORITHMS_DISJOINT_HPP
#define BOOST_GEOMETRY_ALGORITHMS_DISJOINT_HPP

#include <cstddef>
#include <deque>

#include <boost/mpl/if.hpp>
#include <boost/range.hpp>
#include <boost/variant/static_visitor.hpp>
#include <boost/variant/apply_visitor.hpp>
#include <boost/variant/variant_fwd.hpp>

#include <boost/static_assert.hpp>

#include <boost/geometry/core/access.hpp>
#include <boost/geometry/core/coordinate_dimension.hpp>
#include <boost/geometry/core/reverse_dispatch.hpp>

#include <boost/geometry/algorithms/detail/disjoint.hpp>
#include <boost/geometry/algorithms/detail/overlay/get_turns.hpp>
#include <boost/geometry/algorithms/detail/point_on_border.hpp>
#include <boost/geometry/algorithms/point_on_surface.hpp>
#include <boost/geometry/algorithms/detail/for_each_range.hpp>
#include <boost/geometry/algorithms/detail/rescale.hpp>

#include <boost/geometry/geometries/concepts/check.hpp>

#include <boost/geometry/util/math.hpp>

#include <boost/geometry/algorithms/detail/overlay/do_reverse.hpp>
#include <boost/geometry/algorithms/detail/within/point_in_geometry.hpp>


namespace boost { namespace geometry
{


#ifndef DOXYGEN_NO_DETAIL
namespace detail { namespace disjoint
{

template<typename Geometry>
struct check_each_ring_for_within
{
    bool has_within;
    Geometry const& m_geometry;

    inline check_each_ring_for_within(Geometry const& g)
        : has_within(false)
        , m_geometry(g)
    {}

    template <typename Range>
    inline void apply(Range const& range)
    {
        if ( detail::within::within_point_geometry(geometry::return_point_on_surface(range), m_geometry) )
        {
            has_within = true;
        }
    }
};

template <typename FirstGeometry, typename SecondGeometry>
inline bool rings_containing(FirstGeometry const& geometry1,
                SecondGeometry const& geometry2)
{
    check_each_ring_for_within<FirstGeometry> checker(geometry1);
    geometry::detail::for_each_range(geometry2, checker);
    return checker.has_within;
}



struct assign_disjoint_policy
{
    // We want to include all points:
    static bool const include_no_turn = true;
    static bool const include_degenerate = true;
    static bool const include_opposite = true;

    // We don't assign extra info:
    template
    <
        typename Info,
        typename Point1,
        typename Point2,
        typename IntersectionInfo,
        typename DirInfo
    >
    static inline void apply(Info& , Point1 const& , Point2 const&,
                IntersectionInfo const&, DirInfo const&)
    {}
};


template <typename Geometry1, typename Geometry2>
struct disjoint_linear
{
    static inline bool apply(Geometry1 const& geometry1, Geometry2 const& geometry2)
    {
        typedef typename geometry::point_type<Geometry1>::type point_type;

        typedef overlay::turn_info<point_type> turn_info;
        std::deque<turn_info> turns;

        static const bool reverse1 = overlay::do_reverse<geometry::point_order<Geometry1>::value>::value; // should be false
        static const bool reverse2 = overlay::do_reverse<geometry::point_order<Geometry2>::value>::value; // should be false

        // Specify two policies:
        // 1) Stop at any intersection
        // 2) In assignment, include also degenerate points (which are normally skipped)
        disjoint_interrupt_policy policy;
        geometry::get_turns
            <
                reverse1, reverse2,
                assign_disjoint_policy
            >(geometry1, geometry2, detail::no_rescale_policy(), turns, policy);

        return !policy.has_intersections;
    }
};

template <typename Segment1, typename Segment2>
struct disjoint_segment
{
    static inline bool apply(Segment1 const& segment1, Segment2 const& segment2)
    {
        typedef typename point_type<Segment1>::type point_type;

        segment_intersection_points<point_type> is
            = strategy::intersection::relate_cartesian_segments
            <
                policies::relate::segments_intersection_points
                    <
                        Segment1,
                        Segment2,
                        segment_intersection_points<point_type>
                    >
            >::apply(segment1, segment2);

        return is.count == 0;
    }
};

template <typename Geometry1, typename Geometry2>
struct general_areal
{
    static inline bool apply(Geometry1 const& geometry1, Geometry2 const& geometry2)
    {
        if (! disjoint_linear<Geometry1, Geometry2>::apply(geometry1, geometry2))
        {
            return false;
        }

        // If there is no intersection of segments, they might located
        // inside each other

        // We check that using a point on the surface, and see if that is inside
        // the other geometry. And vice versa.

        typedef typename geometry::point_type<Geometry1>::type point_type1;
        typedef typename geometry::point_type<Geometry2>::type point_type2;

        if (rings_containing(geometry1, geometry2)
            || rings_containing(geometry2, geometry1))
        {
            return false;
        }

        return true;
    }
};

template <typename Segment, typename Box>
struct disjoint_segment_box
{
    static inline bool apply(Segment const& segment, Box const& box)
    {
        typedef typename point_type<Segment>::type point_type;
        point_type p0, p1;
        geometry::detail::assign_point_from_index<0>(segment, p0);
        geometry::detail::assign_point_from_index<1>(segment, p1);

        return ! detail::disjoint::segment_box_intersection<point_type, Box>::apply(p0, p1, box);
    }
};

template <typename Linestring, typename Box>
struct disjoint_linestring_box
{
    static inline bool apply(Linestring const& linestring, Box const& box)
    {
        typedef typename ::boost::range_value<Linestring>::type point_type;
        typedef typename ::boost::range_const_iterator<Linestring>::type const_iterator;
        typedef typename ::boost::range_size<Linestring>::type size_type;

        const size_type count = ::boost::size(linestring);

        if ( count == 0 )
            return false;
        else if ( count == 1 )
            return detail::disjoint::point_box<point_type, Box, 0, dimension<point_type>::value>
                   ::apply(*::boost::begin(linestring), box);
        else
        {
            const_iterator it0 = ::boost::begin(linestring);
            const_iterator it1 = ::boost::begin(linestring) + 1;
            const_iterator last = ::boost::end(linestring);

            for ( ; it1 != last ; ++it0, ++it1 )
            {
                if ( detail::disjoint::segment_box_intersection<point_type, Box>::apply(*it0, *it1, box) )
                    return false;
            }
            return true;
        }
    }
};

template<typename Point, typename Geometry>
struct disjoint_point_linear
{
    static inline
    bool apply(Point const& pt, Geometry const& g)
    {
        return !geometry::covered_by(pt, g);
    }
};

// computes disjointness of segment and linestring
template<typename Linestring, typename Segment>
struct disjoint_linestring_segment
{
    static inline
    bool apply(Linestring const& ls, Segment const& seg)
    {
        typedef typename boost::range_value<Linestring>::type LS_Point;
        typedef geometry::model::segment<LS_Point> LS_Segment;

        if ( boost::size(ls) == 1 )
        {
            LS_Segment ls_seg(*boost::begin(ls), *boost::begin(ls));
            return disjoint_segment<Segment, LS_Segment>::apply(seg, ls_seg);
        }

        BOOST_AUTO_TPL(it1, boost::begin(ls));
        BOOST_AUTO_TPL(it2, ++boost::begin(ls));
        for (; it2 != boost::end(ls); ++it1, ++it2)
        {
            LS_Segment ls_seg(*it1, *it2);
            if ( !disjoint_segment<Segment, LS_Segment>::apply(seg, ls_seg) )
            {
                return false;
            }
        }
        return true;
    }
};

template<typename Geometry1, typename Geometry2>
struct disjoint_linear_areal
{
    static inline
    bool apply(Geometry1 const& g1, Geometry2 const& g2)
    {
        // if there are intersections - return false
        if ( !disjoint_linear<Geometry1, Geometry2>::apply(g1, g2) )
            return false;

        typedef typename point_type<Geometry1>::type point1_type;
        point1_type p;
        geometry::point_on_border(p, g1);
        return !geometry::covered_by(p, g2);
    }
};

template<typename Segment, typename Ring>
struct disjoint_segment_ring
{
    static inline
    bool apply(Segment const& seg, Ring const& r)
    {
        if ( !disjoint_linestring_segment<Ring, Segment>::apply(r, seg) )
        {
            return false;
        }

        typename point_type<Segment>::type p;
        detail::assign_point_from_index<0>(seg, p);

        return !covered_by(p, r);
    }
};

template<typename Segment, typename Polygon>
struct disjoint_segment_polygon
{
    static inline
    bool apply(Segment const& seg, Polygon const& polygon)
    {
        typedef typename geometry::ring_type<Polygon>::type Ring;
        typedef typename geometry::interior_return_type
            <
                Polygon const
            >::type InteriorRings;

        if ( !disjoint_linestring_segment
                  <
                      Ring, Segment
             >::apply(geometry::exterior_ring(polygon), seg) )
        {
            return false;
        }

        InteriorRings const& irings = geometry::interior_rings(polygon);

        BOOST_AUTO_TPL(it, boost::begin(irings));
        for (; it != boost::end(irings); ++it)
        {
            if ( !disjoint_linestring_segment
                      <
                          Ring, Segment
                      >::apply(*it, seg) )
            {
                return false;
            }
        }

        typename point_type<Segment>::type p;
        detail::assign_point_from_index<0>(seg, p);

        return !covered_by(p, polygon);
    }
};

}} // namespace detail::disjoint
#endif // DOXYGEN_NO_DETAIL


#ifndef DOXYGEN_NO_DISPATCH
namespace dispatch
{


template
<
    typename Geometry1, typename Geometry2,
    std::size_t DimensionCount = dimension<Geometry1>::type::value,
    typename Tag1 = typename tag<Geometry1>::type,
    typename Tag2 = typename tag<Geometry2>::type,
    bool Reverse = reverse_dispatch<Geometry1, Geometry2>::type::value
>
struct disjoint
    : detail::disjoint::general_areal<Geometry1, Geometry2>
{};


// If reversal is needed, perform it
template
<
    typename Geometry1, typename Geometry2,
    std::size_t DimensionCount,
    typename Tag1, typename Tag2
>
struct disjoint<Geometry1, Geometry2, DimensionCount, Tag1, Tag2, true>
    : disjoint<Geometry2, Geometry1, DimensionCount, Tag2, Tag1, false>
{
    static inline bool apply(Geometry1 const& g1, Geometry2 const& g2)
    {
        return disjoint
            <
                Geometry2, Geometry1,
                DimensionCount,
                Tag2, Tag1
            >::apply(g2, g1);
    }
};


template <typename Point1, typename Point2, std::size_t DimensionCount, bool Reverse>
struct disjoint<Point1, Point2, DimensionCount, point_tag, point_tag, Reverse>
    : detail::disjoint::point_point<Point1, Point2, 0, DimensionCount>
{};


template <typename Box1, typename Box2, std::size_t DimensionCount, bool Reverse>
struct disjoint<Box1, Box2, DimensionCount, box_tag, box_tag, Reverse>
    : detail::disjoint::box_box<Box1, Box2, 0, DimensionCount>
{};


template <typename Point, typename Box, std::size_t DimensionCount, bool Reverse>
struct disjoint<Point, Box, DimensionCount, point_tag, box_tag, Reverse>
    : detail::disjoint::point_box<Point, Box, 0, DimensionCount>
{};

template <typename Point, typename Ring, std::size_t DimensionCount, bool Reverse>
struct disjoint<Point, Ring, DimensionCount, point_tag, ring_tag, Reverse>
    : detail::disjoint::reverse_covered_by<Point, Ring>
{};

template <typename Point, typename Polygon, std::size_t DimensionCount, bool Reverse>
struct disjoint<Point, Polygon, DimensionCount, point_tag, polygon_tag, Reverse>
    : detail::disjoint::reverse_covered_by<Point, Polygon>
{};

template <typename Linestring1, typename Linestring2, bool Reverse>
struct disjoint<Linestring1, Linestring2, 2, linestring_tag, linestring_tag, Reverse>
    : detail::disjoint::disjoint_linear<Linestring1, Linestring2>
{};

template <typename Segment1, typename Segment2, bool Reverse>
struct disjoint<Segment1, Segment2, 2, segment_tag, segment_tag, Reverse>
    : detail::disjoint::disjoint_segment<Segment1, Segment2>
{};

template <typename Segment, typename Box, std::size_t DimensionCount, bool Reverse>
struct disjoint<Segment, Box, DimensionCount, segment_tag, box_tag, Reverse>
    : detail::disjoint::disjoint_segment_box<Segment, Box>
{};

template <typename Linestring, typename Box, std::size_t DimensionCount, bool Reverse>
struct disjoint<Linestring, Box, DimensionCount, linestring_tag, box_tag, Reverse>
    : detail::disjoint::disjoint_linestring_box<Linestring, Box>
{};

//template <typename Linestring, typename Segment, bool Reverse>
//struct disjoint<Linestring, Segment, 2, linestring_tag, segment_tag, Reverse>
//    : detail::disjoint::disjoint_linear<Linestring, Segment>
//{};
template<typename Linestring, typename Segment, std::size_t DimensionCount, bool Reverse>
struct disjoint<Linestring, Segment, DimensionCount, linestring_tag, segment_tag, Reverse>
    : detail::disjoint::disjoint_linestring_segment<Linestring, Segment>
{};

template<typename Segment, typename Ring, std::size_t DimensionCount, bool Reverse>
struct disjoint<Segment, Ring, DimensionCount, segment_tag, ring_tag, Reverse>
    : detail::disjoint::disjoint_segment_ring<Segment, Ring>
{};

template<typename Polygon, typename Segment, std::size_t DimensionCount, bool Reverse>
struct disjoint<Polygon, Segment, DimensionCount, polygon_tag, segment_tag, Reverse>
{
    static inline
    bool apply(Polygon const& g1, Segment const& g2)
    {
        return detail::disjoint::disjoint_segment_polygon<Segment, Polygon>::apply(g2, g1);
    }
};

template<typename Linestring, typename Polygon, std::size_t DimensionCount, bool Reverse>
struct disjoint<Linestring, Polygon, DimensionCount, linestring_tag, polygon_tag, Reverse>
    : public detail::disjoint::disjoint_linear_areal<Linestring, Polygon>
{};

template<typename Linestring, typename Ring, std::size_t DimensionCount, bool Reverse>
struct disjoint<Linestring, Ring, DimensionCount, linestring_tag, ring_tag, Reverse>
    : public detail::disjoint::disjoint_linear_areal<Linestring, Ring>
{};

// move the following specializations to multi/algorithms/disjoint.hpp?

template<typename MultiLinestring, typename Polygon, std::size_t DimensionCount, bool Reverse>
struct disjoint<MultiLinestring, Polygon, DimensionCount, multi_linestring_tag, polygon_tag, Reverse>
    : public detail::disjoint::disjoint_linear_areal<MultiLinestring, Polygon>
{};

template<typename MultiLinestring, typename Ring, std::size_t DimensionCount, bool Reverse>
struct disjoint<MultiLinestring, Ring, DimensionCount, multi_linestring_tag, ring_tag, Reverse>
    : public detail::disjoint::disjoint_linear_areal<MultiLinestring, Ring>
{};

template<typename Linestring, typename MultiPolygon, std::size_t DimensionCount, bool Reverse>
struct disjoint<Linestring, MultiPolygon, DimensionCount, linestring_tag, multi_polygon_tag, Reverse>
    : public detail::disjoint::disjoint_linear_areal<Linestring, MultiPolygon>
{};

template<typename MultiLinestring, typename MultiPolygon, std::size_t DimensionCount, bool Reverse>
struct disjoint<MultiLinestring, MultiPolygon, DimensionCount, multi_linestring_tag, multi_polygon_tag, Reverse>
    : public detail::disjoint::disjoint_linear_areal<MultiLinestring, MultiPolygon>
{};

template<typename Point, typename Linestring, std::size_t DimensionCount, bool Reverse>
struct disjoint<Point, Linestring, DimensionCount, point_tag, linestring_tag, Reverse>
    : public detail::disjoint::disjoint_point_linear<Point, Linestring>
{};

template<typename Point, typename MultiLinestring, std::size_t DimensionCount, bool Reverse>
struct disjoint<Point, MultiLinestring, DimensionCount, point_tag, multi_linestring_tag, Reverse>
    : public detail::disjoint::disjoint_point_linear<Point, MultiLinestring>
{};

} // namespace dispatch
#endif // DOXYGEN_NO_DISPATCH


namespace resolve_variant {

template <typename Geometry1, typename Geometry2>
struct disjoint
{
    static inline bool apply(Geometry1 const& geometry1, Geometry2 const& geometry2)
    {
        concept::check_concepts_and_equal_dimensions
            <
                Geometry1 const,
                Geometry2 const
            >();

        return dispatch::disjoint<Geometry1, Geometry2>::apply(geometry1, geometry2);
    }
};

template <BOOST_VARIANT_ENUM_PARAMS(typename T), typename Geometry2>
struct disjoint<boost::variant<BOOST_VARIANT_ENUM_PARAMS(T)>, Geometry2>
{
    struct visitor: boost::static_visitor<bool>
    {
        Geometry2 const& m_geometry2;

        visitor(Geometry2 const& geometry2): m_geometry2(geometry2) {}

        template <typename Geometry1>
        bool operator()(Geometry1 const& geometry1) const
        {
            return disjoint<Geometry1, Geometry2>::apply(geometry1, m_geometry2);
        }
    };

    static inline bool
    apply(boost::variant<BOOST_VARIANT_ENUM_PARAMS(T)> const& geometry1,
          Geometry2 const& geometry2)
    {
        return boost::apply_visitor(visitor(geometry2), geometry1);
    }
};

template <typename Geometry1, BOOST_VARIANT_ENUM_PARAMS(typename T)>
struct disjoint<Geometry1, boost::variant<BOOST_VARIANT_ENUM_PARAMS(T)> >
{
    struct visitor: boost::static_visitor<bool>
    {
        Geometry1 const& m_geometry1;

        visitor(Geometry1 const& geometry1): m_geometry1(geometry1) {}

        template <typename Geometry2>
        bool operator()(Geometry2 const& geometry2) const
        {
            return disjoint<Geometry1, Geometry2>::apply(m_geometry1, geometry2);
        }
    };

    static inline bool
    apply(Geometry1 const& geometry1,
          boost::variant<BOOST_VARIANT_ENUM_PARAMS(T)> const& geometry2)
    {
        return boost::apply_visitor(visitor(geometry1), geometry2);
    }
};

template <
    BOOST_VARIANT_ENUM_PARAMS(typename T1),
    BOOST_VARIANT_ENUM_PARAMS(typename T2)
>
struct disjoint<
    boost::variant<BOOST_VARIANT_ENUM_PARAMS(T1)>,
    boost::variant<BOOST_VARIANT_ENUM_PARAMS(T2)>
>
{
    struct visitor: boost::static_visitor<bool>
    {
        template <typename Geometry1, typename Geometry2>
        bool operator()(Geometry1 const& geometry1,
                        Geometry2 const& geometry2) const
        {
            return disjoint<Geometry1, Geometry2>::apply(geometry1, geometry2);
        }
    };

    static inline bool
    apply(boost::variant<BOOST_VARIANT_ENUM_PARAMS(T1)> const& geometry1,
          boost::variant<BOOST_VARIANT_ENUM_PARAMS(T2)> const& geometry2)
    {
        return boost::apply_visitor(visitor(), geometry1, geometry2);
    }
};

} // namespace resolve_variant



/*!
\brief \brief_check2{are disjoint}
\ingroup disjoint
\tparam Geometry1 \tparam_geometry
\tparam Geometry2 \tparam_geometry
\param geometry1 \param_geometry
\param geometry2 \param_geometry
\return \return_check2{are disjoint}

\qbk{[include reference/algorithms/disjoint.qbk]}
*/
template <typename Geometry1, typename Geometry2>
inline bool disjoint(Geometry1 const& geometry1,
            Geometry2 const& geometry2)
{
    return resolve_variant::disjoint<Geometry1, Geometry2>::apply(geometry1, geometry2);
}


}} // namespace boost::geometry


#endif // BOOST_GEOMETRY_ALGORITHMS_DISJOINT_HPP
