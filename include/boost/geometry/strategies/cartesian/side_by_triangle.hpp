// Boost.Geometry (aka GGL, Generic Geometry Library)
//
// Copyright Barend Gehrels 2007-2009, Geodan, Amsterdam, the Netherlands.
// Copyright Bruno Lalande 2008, 2009
// Use, modification and distribution is subject to the Boost Software License,
// Version 1.0. (See accompanying file LICENSE_1_0.txt or copy at
// http://www.boost.org/LICENSE_1_0.txt)

#ifndef BOOST_GEOMETRY_STRATEGIES_CARTESIAN_SIDE_BY_TRIANGLE_HPP
#define BOOST_GEOMETRY_STRATEGIES_CARTESIAN_SIDE_BY_TRIANGLE_HPP

#include <boost/mpl/if.hpp>
#include <boost/type_traits.hpp>

#include <boost/geometry/core/access.hpp>

#include <boost/geometry/util/select_coordinate_type.hpp>



namespace boost { namespace geometry
{

namespace strategy { namespace side
{

// Check at which side of a segment a point lies:
// left of segment (> 0), right of segment (< 0), on segment (0)
// In fact this is twice the area of a triangle
template <typename CalculationType>
struct side_by_triangle
{

    // Template member function, because it is not always trivial
    // or convenient to explicitly mention the typenames in the
    // strategy-struct itself.

    // Types can be all three different. Therefore it is
    // not implemented (anymore) as "segment"

    template <typename P1, typename P2, typename P>
    static inline int apply(P1 const& p1, P2 const& p2, P const& p)
    {
        typedef typename boost::mpl::if_c
            <
                boost::is_void<CalculationType>::type::value,
                typename select_most_precise
                    <
                        typename select_most_precise
                            <
                                typename coordinate_type<P1>::type,
                                typename coordinate_type<P2>::type
                            >::type,
                        typename coordinate_type<P>::type
                    >::type,
                CalculationType
            >::type coordinate_type;

//std::cout << "side: " << typeid(coordinate_type).name() << std::endl;
        coordinate_type const x = get<0>(p);
        coordinate_type const y = get<1>(p);

        coordinate_type const sx1 = get<0>(p1);
        coordinate_type const sy1 = get<1>(p1);
        coordinate_type const sx2 = get<0>(p2);
        coordinate_type const sy2 = get<1>(p2);

        // Promote float->double, small int->int
        typedef typename select_most_precise
            <
                coordinate_type,
                double
            >::type promoted_type;

        promoted_type const dx = sx2 - sx1;
        promoted_type const dy = sy2 - sy1;
        promoted_type const dpx = x - sx1;
        promoted_type const dpy = y - sy1;

        promoted_type const s = dx * dpy - dy * dpx;

        promoted_type zero = promoted_type();
        return math::equals(s, zero) ? 0 : s > zero ? 1 : -1;
        //return s > 0 ? 1 : s < 0 ? -1 : 0;
    }
};

}} // namespace strategy::side


#ifndef DOXYGEN_NO_STRATEGY_SPECIALIZATIONS
template <typename CalculationType>
struct strategy_side<cartesian_tag, CalculationType>
{
    typedef strategy::side::side_by_triangle<CalculationType> type;
};
#endif

}} // namespace boost::geometry


#endif // BOOST_GEOMETRY_STRATEGIES_CARTESIAN_SIDE_BY_TRIANGLE_HPP
