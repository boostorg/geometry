#ifndef BOOST_GEOMETRY_ALGORITHMS_OGC_IS_SIMPLE_HPP
#define BOOST_GEOMETRY_ALGORITHMS_OGC_IS_SIMPLE_HPP

#include <boost/geometry/geometry.hpp>
#include <boost/geometry/policies/compare.hpp>
#include <boost/geometry/algorithms/unique.hpp>
#include <boost/geometry/algorithms/equals.hpp>
#include <boost/geometry/algorithms/intersects.hpp>
#include <boost/geometry/algorithms/disjoint.hpp>
#include <boost/geometry/algorithms/detail/point_is_spike_or_equal.hpp>
#include <boost/geometry/views/closeable_view.hpp>
#include <deque>
#include <algorithm>
#include <boost/geometry/algorithms/detail/overlay/self_turn_points.hpp>

namespace boost { namespace geometry { namespace ogc
{

#ifndef DOXYGEN_NO_DISPATCH
namespace dispatch
{

template <typename Geometry, typename Tag = typename tag<Geometry>::type>
struct is_simple
    : not_implemented<Geometry>
{};

} // namespace dispatch
#endif // DOXYGEN_NO_DISPATCH

#ifndef DOXYGEN_NO_DETAIL
namespace detail { namespace is_simple
{



template <typename Range, closure_selector Closure>
struct has_spikes;

template <typename Range>
struct has_spikes<Range, closed>
{
    static inline bool apply(Range const& range)
    {
        if ( boost::size(range) < 3 )
        {
            return false;
        }

        BOOST_AUTO_TPL(it1, boost::begin(range));
        BOOST_AUTO_TPL(it2, ++boost::begin(range));
        BOOST_AUTO_TPL(it3, ++(++boost::begin(range)));

        for (; it3 != boost::end(range); ++it1, ++it2, ++it3)
        {
            if ( geometry::detail::point_is_spike_or_equal(*it3, *it1, *it2) )
            {
                return true;
            }
        }

        return false;
    }
};


template <typename Range>
struct has_spikes<Range, open>
{
    static inline bool apply(Range const& range)
    {
        typedef typename closeable_view<Range, open>::type ClosedRange;

        ClosedRange crange(const_cast<Range&>(range));
        return has_spikes<ClosedRange, closed>::apply(crange);
    }

};


template
<
    typename Geometry,
    bool AllowEmpty = true,
    typename Tag = typename tag<Geometry>::type
>
struct is_below_minimal_size
{
    static inline bool apply(Geometry const&)
    {
        return false;
    }
};


template <typename Linestring>
struct is_below_minimal_size<Linestring, true, linestring_tag>
{
    static inline bool apply(Linestring const& linestring)
    {
        return boost::size(linestring) == 1;
    }
};





template <typename Range>
struct is_simple_range
{
    static inline bool apply(Range const& range)
    {
        if ( is_below_minimal_size<Range>::apply(range) )
        {
            return false;
        }

        Range other(range);
        geometry::unique(other);

        if ( boost::size(other) == 1 )
        {
            return true;
        }

        if ( has_spikes<Range, closure<Range>::value>::apply(other) )
        {
            return false;
        }

        return !geometry::intersects(other);
    }
};



template <typename Polygon>
struct is_simple_polygon
{
    static inline bool apply(Polygon const& polygon)
    {
        typedef typename ring_type<Polygon>::type Ring;

        BOOST_AUTO_TPL(it, boost::begin(geometry::interior_rings(polygon)));
        for (; it != boost::end(geometry::interior_rings(polygon)); ++it)
        {
            if ( !is_simple_range<Ring>::apply(*it) )
            {
                return false;
            }
        }

        return is_simple_range<Ring>::apply(geometry::exterior_ring(polygon));
    }
};


template <typename MultiPoint>
struct is_simple_multipoint
{
    static inline bool apply(MultiPoint const& multipoint)
    {
        if ( boost::size(multipoint) <= 1 )
        {
            return true;
        }

        MultiPoint mp(multipoint);

        typedef typename point_type<MultiPoint>::type Point;

        std::sort(boost::begin(mp), boost::end(mp), geometry::less<Point>());

        geometry::equal_to<Point> equal;

        BOOST_AUTO_TPL(it_next, ++boost::begin(mp));
        BOOST_AUTO_TPL(it, boost::begin(mp));
        for (; it_next != boost::end(mp); ++it, ++it_next)
        {
            if ( equal(*it, *it_next) )
            {
                return false;
            }
        }
        return true;
    }
};


template <typename MultiLinestring>
struct is_simple_multilinestring
{
    static inline bool apply(MultiLinestring const& multilinestring)
    {
        typedef typename boost::range_value<MultiLinestring>::type Linestring;
        typedef typename point_type<MultiLinestring>::type Point;

        BOOST_AUTO_TPL(it, boost::begin(multilinestring));
        for (; it != boost::end(multilinestring); ++it)
        {
            if ( !dispatch::is_simple<Linestring>::apply(*it) )
            {
                return false;
            }
        }


        typedef geometry::detail::overlay::turn_info
            <
                Point
            > turn_info;

        std::deque<turn_info> turns;

        typedef typename strategy_intersection
            <
                typename cs_tag<MultiLinestring>::type,
                MultiLinestring,
                MultiLinestring,
                Point
            >::segment_intersection_strategy_type segment_intersection_strategy_type;

        typedef geometry::detail::overlay::get_turn_info
            <
                geometry::detail::disjoint::assign_disjoint_policy
            > TurnPolicy;

        geometry::detail::self_get_turn_points::no_interrupt_policy
            interrupt_policy;

        geometry::detail::self_get_turn_points::get_turns
            <
                TurnPolicy
            >::apply(multilinestring,
                     geometry::detail::no_rescale_policy(),
                     turns,
                     interrupt_policy);

        std::cout << "turns:";
        for (typename std::deque<turn_info>::const_iterator tit = turns.begin();
             tit != turns.end(); ++tit)
        {
            std::cout << " " << geometry::dsv(tit->point);
        }
        std::cout << std::endl << std::endl;

        if ( turns.size() == 0 )
        {
            return true;
        }

        std::vector<Point> endpoints;
        for (it = boost::begin(multilinestring);
             it != boost::end(multilinestring); ++it)
        {
            BOOST_ASSERT ( boost::size(*it) != 1 );
            if ( boost::size(*it) != 0 )
            {
                endpoints.push_back( *boost::begin(*it) );
                endpoints.push_back( *(--boost::end(*it)) );
            }
        }


        std::sort(endpoints.begin(), endpoints.end(), geometry::less<Point>());

        std::cout << "endpoints: ";
        for (typename std::vector<Point>::iterator pit = endpoints.begin();
             pit != endpoints.end(); ++pit)
        {
            std::cout << " " << geometry::dsv(*pit);
        }
        std::cout << std::endl << std::endl;

        for (typename std::deque<turn_info>::const_iterator tit = turns.begin();
             tit != turns.end(); ++tit)
        {            
            Linestring const& ls1 =
                *(boost::begin(multilinestring)
                  + tit->operations[0].seg_id.multi_index);
            Linestring const& ls2 =
                *(boost::begin(multilinestring)
                  + tit->operations[0].other_id.multi_index);

            if ( ( !geometry::equals(*boost::begin(ls1), tit->point)
                   && !geometry::equals(*--boost::end(ls1), tit->point) )
                 ||
                 ( !geometry::equals(*boost::begin(ls2), tit->point)
                   && !geometry::equals(*--boost::end(ls2), tit->point) )
                 )
            {
                return false;
            }
        }
        return true;
    }

};


template <typename MultiGeometry, bool CheckIntersections>
struct is_simple_multigeometry
{
    static inline bool apply(MultiGeometry const& multigeometry)
    {
        typedef typename boost::range_value<MultiGeometry>::type Geometry;

        BOOST_AUTO_TPL(it, boost::begin(multigeometry));
        for (; it != boost::end(multigeometry); ++it)
        {
            if ( !dispatch::is_simple<Geometry>::apply(*it) )
            {
                return false;
            }
        }


        if ( CheckIntersections )
        {
            BOOST_AUTO_TPL(it1, boost::begin(multigeometry));
            it = it1;
            for (; it != boost::end(multigeometry); ++it)
            {
                for (; it1 != boost::end(multigeometry); ++it1)
                {
                    if ( geometry::intersects(*it, *it1) )
                    {
                        return false;
                    }
                }
            }
        }
        return true;
    }
};



}} // namespace detail::is_simple
#endif // DOXYGEN_NO_DETAIL




#ifndef DOXYGEN_NO_DISPATCH
namespace dispatch
{


// A point is always simple
template <typename Point>
struct is_simple<Point, point_tag>
{
    static inline bool apply(Point const&)
    {
        return true;
    }
};


// A segment is always simple.
// A segment is a curve.
// A curve is simple if it does not pass through the same point twice,
// with the possible exception of its two endpoints
//
// Reference: OGC 06-103r4 (§6.1.6.1)
template <typename Segment>
struct is_simple<Segment, segment_tag>
{
    static inline bool apply(Segment const&)
    {
        return true;
    }
};


// A box is always simple
// A box is a Polygon, and it satisfies the conditions for Polygon validity.
//
// Reference (for polygon validity): OGC 06-103r4 (§6.1.11.1)
template <typename Box>
struct is_simple<Box, box_tag>
{
    static inline bool apply(Box const&)
    {
        return true;
    }
};


// A linestring is a curve.
// A curve is simple if it does not pass through the same point twice,
// with the possible exception of its two endpoints
//
// Reference: OGC 06-103r4 (§6.1.6.1)
template <typename Linestring>
struct is_simple<Linestring, linestring_tag>
    : detail::is_simple::is_simple_range<Linestring>
{};


// A Ring is a Polygon.
// A Polygon is always a simple geometric object provided that it is valid.
//
// Reference (for polygon validity): OGC 06-103r4 (§6.1.11.1)
template <typename Ring>
struct is_simple<Ring, ring_tag>
    : detail::is_simple::is_simple_range<Ring>
{};


// A Polygon is always a simple geometric object provided that it is valid.
//
// Reference (for validity of Polygons): OGC 06-103r4 (§6.1.11.1)
template <typename Polygon>
struct is_simple<Polygon, polygon_tag>
    : detail::is_simple::is_simple_polygon<Polygon>
{};


// A MultiPoint is simple if no two Points in the MultiPoint are equal
// (have identical coordinate values in X and Y)
//
// Reference: OGC 06-103r4 (§6.1.5)
template <typename MultiPoint>
struct is_simple<MultiPoint, multi_point_tag>
    : detail::is_simple::is_simple_multipoint<MultiPoint>
{};


// A MultiLinestring is a MultiCurve
// A MultiCurve is simple if all of its elements are simple and the
// only intersections between any two elements occur at Points that
// are on the boundaries of both elements.
//
// Reference: OGC 06-103r4 (§6.1.8.1; Fig. 9)
template <typename MultiLinestring>
struct is_simple<MultiLinestring, multi_linestring_tag>
    : detail::is_simple::is_simple_multilinestring<MultiLinestring>
{};


// Not clear what the definition is.
// Right now we check that each element is simple (in fact valid), and
// that the MultiPolygon is also valid.
//
// Reference (for validity of MultiPolygons): OGC 06-103r4 (§6.1.14)
template <typename MultiPolygon>
struct is_simple<MultiPolygon, multi_polygon_tag>
    : detail::is_simple::is_simple_multigeometry<MultiPolygon, false>
{};


} // namespace dispatch
#endif // DOXYGEN_NO_DISPATCH



template <typename Geometry>
inline bool is_simple(Geometry const& g)
{
    return dispatch::is_simple<Geometry>::apply(g);
}



}}} // namespace boost::geometry::ogc



#endif // BOOST_GEOMETRY_MYSQL_ALGORITHMS_IS_SIMPLE_HPP
