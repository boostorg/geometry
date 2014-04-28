// Boost.Geometry (aka GGL, Generic Geometry Library)

// Copyright (c) 2013 Barend Gehrels, Amsterdam, the Netherlands.
// Copyright (c) 2013 Bruno Lalande, Paris, France.
// Copyright (c) 2013 Mateusz Loskot, London, UK.
// Copyright (c) 2013 Adam Wulkiewicz, Lodz, Poland.

// Use, modification and distribution is subject to the Boost Software License,
// Version 1.0. (See accompanying file LICENSE_1_0.txt or copy at
// http://www.boost.org/LICENSE_1_0.txt)

#ifndef BOOST_GEOMETRY_ALGORITHMS_DETAIL_ZOOM_TO_ROBUST_HPP
#define BOOST_GEOMETRY_ALGORITHMS_DETAIL_ZOOM_TO_ROBUST_HPP


#include <cstddef>

#include <boost/type_traits.hpp>

#include <boost/geometry/algorithms/envelope.hpp>
#include <boost/geometry/algorithms/expand.hpp>
#include <boost/geometry/algorithms/detail/recalculate.hpp>
#include <boost/geometry/algorithms/detail/get_max_size.hpp>

#include <boost/geometry/geometries/box.hpp>
#include <boost/geometry/geometries/point.hpp>

#include <boost/geometry/policies/robustness/rescale_policy.hpp>

namespace boost { namespace geometry
{


#ifndef DOXYGEN_NO_DISPATCH
namespace dispatch
{



template <typename IsFloatingPoint>
struct zoom_to_robust
{
    template
    <
        typename Geometry1, typename Geometry2, typename Geometry3,
        typename Geometry4, typename Geometry5, typename Geometry6,
        typename GeometryOut
    >
    static inline void apply(Geometry1 const& g1, Geometry2 const& g2, Geometry3 const& g3,
          Geometry4 const& g4, Geometry5 const& g5, Geometry6 const& g6,
          GeometryOut& og1, GeometryOut& og2, GeometryOut& og3,
          GeometryOut& og4, GeometryOut& og5, GeometryOut& og6)
    {
        // By default, just convert these geometries (until now: points or maybe segments)
        geometry::convert(g1, og1);
        geometry::convert(g2, og2);
        geometry::convert(g3, og3);
        geometry::convert(g4, og4);
        geometry::convert(g5, og5);
        geometry::convert(g6, og6);
    }
};

template <>
struct zoom_to_robust<boost::true_type>
{
    template
    <
        typename Geometry1, typename Geometry2, typename Geometry3,
        typename Geometry4, typename Geometry5, typename Geometry6,
        typename GeometryOut
    >
    static inline void apply(Geometry1 const& g1, Geometry2 const& g2, Geometry3 const& g3,
          Geometry4 const& g4, Geometry5 const& g5, Geometry6 const& g6,
          GeometryOut& og1, GeometryOut& og2, GeometryOut& og3,
          GeometryOut& og4, GeometryOut& og5, GeometryOut& og6)
    {
        typedef typename point_type<Geometry1>::type point1_type;

        // Get the envelop of inputs
        model::box<point1_type> env;
        geometry::assign_inverse(env);
        geometry::expand(env, g1);
        geometry::expand(env, g2);
        geometry::expand(env, g3);
        geometry::expand(env, g4);
        geometry::expand(env, g5);
        geometry::expand(env, g6);

        // Scale this to integer-range
        typename geometry::coordinate_type<point1_type>::type diff = detail::get_max_size(env);
        double range = 1000000000.0; // Define a large range to get precise integer coordinates
        double factor = double(boost::long_long_type(range / double(diff)));

        // Assign input/output minimal points
        point1_type min_point1;
        detail::assign_point_from_index<0>(env, min_point1);

        typedef typename point_type<GeometryOut>::type point2_type;
        point2_type min_point2;
        assign_values(min_point2, boost::long_long_type(-range/2.0), boost::long_long_type(-range/2.0));

        detail::robust_policy<point1_type, point2_type, double> strategy(min_point1, min_point2, factor);

        geometry::recalculate(og1, g1, strategy);
        geometry::recalculate(og2, g2, strategy);
        geometry::recalculate(og3, g3, strategy);
        geometry::recalculate(og4, g4, strategy);
        geometry::recalculate(og5, g5, strategy);
        geometry::recalculate(og6, g6, strategy);
    }
};


} // namespace dispatch
#endif // DOXYGEN_NO_DISPATCH

#ifndef DOXYGEN_NO_DETAIL
namespace detail
{


template <typename Geometry1, typename Geometry2>
inline void zoom_to_robust(Geometry1 const& g1a, Geometry1 const& g1b, Geometry2& g2a, Geometry2& g2b)
{
    typedef typename point_type<Geometry1>::type point1_type;
    typedef typename point_type<Geometry2>::type point2_type;

    point1_type min_point1;
    point2_type min_point2;

    // Get the envelop of inputs
    model::box<point1_type> env;
    geometry::envelope(g1a, env);
    geometry::expand(env, g1b);

    // Scale this to integer-range
    typename coordinate_type<point1_type>::type diff = detail::get_max_size(env);
    double range = 1000000000.0; // Define a large range to get precise integer coordinates
    double factor = range / diff;

    // Assign input/output minimal points
    detail::assign_point_from_index<0>(env, min_point1);
    assign_values(min_point2, boost::long_long_type(-range/2.0), boost::long_long_type(-range/2.0));

    detail::robust_policy<point1_type, point2_type, double> strategy(min_point1, min_point2, factor);
    geometry::recalculate(g2a, g1a, strategy);
    geometry::recalculate(g2b, g1b, strategy);
}

template
<
    typename Geometry1, typename Geometry2, typename Geometry3,
    typename Geometry4, typename Geometry5, typename Geometry6,
    typename GeometryOut
>
void zoom_to_robust(Geometry1 const& g1, Geometry2 const& g2, Geometry3 const& g3,
          Geometry4 const& g4, Geometry5 const& g5, Geometry6 const& g6,
          GeometryOut& og1, GeometryOut& og2, GeometryOut& og3,
          GeometryOut& og4, GeometryOut& og5, GeometryOut& og6)
{
    // Make FP robust (so dispatch to true), so float and double
    // Other types as int, boost::rational, or ttmath are considered to be already robust
    dispatch::zoom_to_robust
        <
            typename boost::is_floating_point
                <
                    typename geometry::coordinate_type<Geometry1>::type
                >::type
        >::apply(g1, g2, g3, g4, g5, g6, og1, og2, og3, og4, og5, og6);
}

} // namespace detail
#endif // DOXYGEN_NO_DETAIL


}} // namespace boost::geometry


#endif // BOOST_GEOMETRY_ALGORITHMS_DETAIL_ZOOM_TO_ROBUST_HPP
