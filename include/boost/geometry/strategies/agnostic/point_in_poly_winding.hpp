// Boost.Geometry (aka GGL, Generic Geometry Library)
//
// Copyright Barend Gehrels 2007-2009, Geodan, Amsterdam, the Netherlands.
// Copyright Bruno Lalande 2008, 2009
// Use, modification and distribution is subject to the Boost Software License,
// Version 1.0. (See accompanying file LICENSE_1_0.txt or copy at
// http://www.boost.org/LICENSE_1_0.txt)

#ifndef BOOST_GEOMETRY_STRATEGY_AGNOSTIC_POINT_IN_POLY_WINDING_HPP
#define BOOST_GEOMETRY_STRATEGY_AGNOSTIC_POINT_IN_POLY_WINDING_HPP



#include <boost/geometry/util/select_calculation_type.hpp>

#include <boost/geometry/strategies/point_in_poly.hpp>



namespace boost { namespace geometry
{

namespace strategy { namespace within {

/*!
    \brief Within detection using winding rule
    \tparam Point point type of point to examine
    \tparam PointOfSegment point type of segments, defaults to Point
    \author Barend Gehrels
    \note The implementation is inspired by terralib http://www.terralib.org (LGPL)
    \note but totally revised afterwards, especially for cases on segments
    \note Only dependant on "side", -> agnostic, suitable for spherical/latlong
 */
template
<
    typename Point,
    typename PointOfSegment = Point,
    typename CalculationType = void
>
class winding
{
    typedef typename select_calculation_type
        <
            Point,
            PointOfSegment,
            CalculationType
        >::type calculation_type;


    typedef typename strategy_side
        <
            typename cs_tag<Point>::type
        >::type strategy_side_type;


    /*! subclass to keep state */
    class counter
    {
        int count;
        bool touches;

        inline bool within_no_touch() const
        {
            return ! touches && count != 0;
        }
    public :
        friend class winding;

        inline counter()
            : count(0)
            , touches(false)
        {}

    };


    template <size_t D>
    static inline int check_touch(Point const& point,
                PointOfSegment const& seg1, PointOfSegment const& seg2,
                counter& state)
    {
        calculation_type const p = get<D>(point);
        calculation_type const s1 = get<D>(seg1);
        calculation_type const s2 = get<D>(seg2);
        if ((s1 <= p && s2 >= p) || (s2 <= p && s1 >= p))
        {
            state.touches = true;
        }
        return 0;
    }


    template <size_t D>
    static inline int check_segment(Point const& point,
                PointOfSegment const& seg1, PointOfSegment const& seg2,
                counter& state)
    {
        calculation_type const p = get<D>(point);
        calculation_type const s1 = get<D>(seg1);
        calculation_type const s2 = get<D>(seg2);

        // Check if one of segment endpoints is at same level of point
        bool eq1 = math::equals(s1, p);
        bool eq2 = math::equals(s2, p);

        if (eq1 && eq2)
        {
            // Both equal p -> segment is horizontal (or vertical for D=0)
            // The only thing which has to be done is check if point is ON segment
            return check_touch<1 - D>(point, seg1, seg2,state);
        }

        return
              eq1 ? (s2 > p ?  1 : -1)  // Point on level s1, UP/DOWN depending on s2
            : eq2 ? (s1 > p ? -1 :  1)  // idem
            : s1 < p && s2 > p ?  2     // Point between s1 -> s2 --> UP
            : s2 < p && s1 > p ? -2     // Point between s2 -> s1 --> DOWN
            : 0;
    }




public :

    // Typedefs and static methods to fulfill the concept
    typedef Point point_type;
    typedef PointOfSegment segment_point_type;
    typedef counter state_type;

    static inline bool apply(Point const& point,
                PointOfSegment const& s1, PointOfSegment const& s2,
                counter& state)
    {
        int count = check_segment<1>(point, s1, s2, state);
        if (count != 0)
        {
            int side = strategy_side_type::apply(s1, s2, point);
            if (side == 0)
            {
                // Point is lying on segment
                state.touches = true;
                state.count = 0;
                return false;
            }

            // Side is NEG for right, POS for left.
            // The count is -2 for down, 2 for up (or -1/1)
            // Side positive thus means UP and LEFTSIDE or DOWN and RIGHTSIDE
            // See accompagnying figure (TODO)
            if (side * count > 0)
            {
                state.count += count;
            }

        }
        return ! state.touches;
    }

    static inline bool result(counter const& state)
    {
        return state.within_no_touch();
    }
};

}} // namespace strategy::within


#ifndef DOXYGEN_NO_STRATEGY_SPECIALIZATIONS

template <typename Point, typename PointOfSegment>
struct strategy_within<cartesian_tag, cartesian_tag, Point, PointOfSegment>
{
    typedef strategy::within::winding<Point, PointOfSegment> type;
};

template <typename Point, typename PointOfSegment>
struct strategy_within<geographic_tag, geographic_tag, Point, PointOfSegment>
{
    typedef strategy::within::winding<Point, PointOfSegment> type;
};

#endif


}} // namespace boost::geometry


#endif // BOOST_GEOMETRY_STRATEGY_AGNOSTIC_POINT_IN_POLY_WINDING_HPP
