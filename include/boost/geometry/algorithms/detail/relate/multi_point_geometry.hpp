// Boost.Geometry

// Copyright (c) 2017 Oracle and/or its affiliates.

// Contributed and/or modified by Adam Wulkiewicz, on behalf of Oracle

// Use, modification and distribution is subject to the Boost Software License,
// Version 1.0. (See accompanying file LICENSE_1_0.txt or copy at
// http://www.boost.org/LICENSE_1_0.txt)

#ifndef BOOST_GEOMETRY_ALGORITHMS_DETAIL_RELATE_MULTI_POINT_GEOMETRY_HPP
#define BOOST_GEOMETRY_ALGORITHMS_DETAIL_RELATE_MULTI_POINT_GEOMETRY_HPP


#include <boost/range.hpp>

#include <boost/geometry/algorithms/detail/disjoint/point_box.hpp>
#include <boost/geometry/algorithms/detail/expand_by_epsilon.hpp>
#include <boost/geometry/algorithms/detail/relate/result.hpp>
#include <boost/geometry/algorithms/detail/relate/topology_check.hpp>
#include <boost/geometry/algorithms/detail/within/point_in_geometry.hpp>
#include <boost/geometry/algorithms/envelope.hpp>

#include <boost/geometry/core/is_areal.hpp>
#include <boost/geometry/core/point_type.hpp>

#include <boost/geometry/geometries/box.hpp>


namespace boost { namespace geometry
{

#ifndef DOXYGEN_NO_DETAIL
namespace detail { namespace relate {


/*
template <bool IsLinear>
struct multi_point_linear_exterior_boundary_check
{
    template <typename MultiPoint>
    struct exterior_boundary_visitor
    {
        typedef typename boost::range_value<MultiPoint>::type point_type;
        std::vector<point_type> points;
        bool found_exterior_boundary;

        exterior_boundary_visitor(MultiPoint const& multi_point)
            : points(boost::begin(multi_point), boost::end(multi_point))
            , found_exterior_boundary(false)
        {
            std::sort(points.begin(), points.end(), geometry::less<point_type>());
        }

        template <typename BoundaryPoint>
        bool apply(BoundaryPoint const& boundary_point)
        {
            if (! std::binary_search(points.begin(), points.end(), boundary_point, geometry::less<point_type>()))
            {
                found_exterior_boundary = true;
                return false;
            }
            return true;
        }
    };

    template <typename MultiPoint, typename TopologyCheck>
    static bool apply(MultiPoint const& multi_point, TopologyCheck const& topology_check)
    {
        exterior_boundary_visitor<MultiPoint> visitor(multi_point);
        topology_check.for_each_boundary_point(visitor);
        return visitor.found_exterior_boundary;
    }
};

template <>
struct multi_point_linear_exterior_boundary_check<false>
{
    template <typename MultiPoint, typename TopologyCheck>
    static bool apply(MultiPoint const& , TopologyCheck const& )
    {
        return false;
    }
};
*/

// SingleGeometry - Linear or Areal
template <typename MultiPoint, typename SingleGeometry, bool Transpose = false>
class multi_point_single_geometry
{
public:

    static const bool interruption_enabled = true;

    template <typename Result, typename Strategy>
    static inline void apply(MultiPoint const& multi_point,
                             SingleGeometry const& single_geometry,
                             Result & result,
                             Strategy const& strategy)
    {
        typedef typename point_type<MultiPoint>::type point1_type;
        typedef typename point_type<SingleGeometry>::type point2_type;
        typedef model::box<point2_type> box2_type;
        
        box2_type box2;
        geometry::envelope(single_geometry, box2, strategy.get_envelope_strategy());
        geometry::detail::expand_by_epsilon(box2);

        relate::set<exterior, exterior, result_dimension<MultiPoint>::value, Transpose>(result);

        typedef typename boost::range_const_iterator<MultiPoint>::type iterator;
        for ( iterator it = boost::begin(multi_point) ; it != boost::end(multi_point) ; ++it )
        {
            // The default strategy is enough for Point/Box
            if (detail::disjoint::disjoint_point_box(*it, box2))
            {
                relate::set<interior, exterior, '0', Transpose>(result);
            }
            else
            {
                int in_val = detail::within::point_in_geometry(*it, single_geometry, strategy);

                if ( in_val > 0 ) // within
                {
                    relate::set<interior, interior, '0', Transpose>(result);
                }
                else if ( in_val == 0 )
                {
                    relate::set<interior, boundary, '0', Transpose>(result);
                }
                else // in_val < 0 - not within
                {
                    relate::set<interior, exterior, '0', Transpose>(result);
                }
            }

            if ( BOOST_GEOMETRY_CONDITION(result.interrupt) )
            {
                return;
            }

            if (! (relate::may_update<interior, interior, '0', Transpose>(result)
                || relate::may_update<interior, boundary, '0', Transpose>(result)
                || relate::may_update<interior, exterior, '0', Transpose>(result) ) )
            {
                break;
            }
        }

        typedef detail::relate::topology_check<SingleGeometry> tc_t;
        if ( relate::may_update<exterior, interior, tc_t::interior, Transpose>(result)
          || relate::may_update<exterior, boundary, tc_t::boundary, Transpose>(result) )
        {
            // TODO: For Linear geometry check all boundary points and degenerated linestrings?
            tc_t tc(single_geometry);

            // TODO: this is not true if a linestring is degenerated to a point
            // then the interior has topological dimension = 0, not 1
            if ( tc.has_interior )
                relate::set<exterior, interior, tc_t::interior, Transpose>(result);
            if ( tc.has_boundary )
                relate::set<exterior, boundary, tc_t::boundary, Transpose>(result);
        }
    }

};


// transposed result of point_geometry
template <typename Geometry, typename MultiPoint>
struct single_geometry_multi_point
{
    static const bool interruption_enabled = true;

    template <typename Result, typename Strategy>
    static inline void apply(Geometry const& geometry, MultiPoint const& multi_point,
                             Result & result, Strategy const& strategy)
    {
        multi_point_single_geometry<MultiPoint, Geometry, true>::apply(multi_point, geometry, result, strategy);
    }
};


}} // namespace detail::relate
#endif // DOXYGEN_NO_DETAIL

}} // namespace boost::geometry

#endif // BOOST_GEOMETRY_ALGORITHMS_DETAIL_RELATE_MULTI_POINT_GEOMETRY_HPP
