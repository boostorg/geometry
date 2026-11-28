// Boost.Geometry (aka GGL, Generic Geometry Library)

// Copyright (c) 2007-2015 Barend Gehrels, Amsterdam, the Netherlands.
// Copyright (c) 2008-2015 Bruno Lalande, Paris, France.
// Copyright (c) 2009-2015 Mateusz Loskot, London, UK.

// This file was modified by Oracle on 2014-2022.
// Modifications copyright (c) 2014-2022 Oracle and/or its affiliates.

// Contributed and/or modified by Adam Wulkiewicz, on behalf of Oracle

// Parts of Boost.Geometry are redesigned from Geodan's Geographic Library
// (geolib/GGL), copyright (c) 1995-2010 Geodan, Amsterdam, the Netherlands.

// Use, modification and distribution is subject to the Boost Software License,
// Version 1.0. (See accompanying file LICENSE_1_0.txt or copy at
// http://www.boost.org/LICENSE_1_0.txt)

#ifndef BOOST_GEOMETRY_ALGORITHMS_DETAIL_OVERLAPS_IMPLEMENTATION_HPP
#define BOOST_GEOMETRY_ALGORITHMS_DETAIL_OVERLAPS_IMPLEMENTATION_HPP


#include <cstddef>

#include <boost/geometry/core/access.hpp>

#include <boost/geometry/algorithms/detail/gc_topological_dimension.hpp>
#include <boost/geometry/algorithms/detail/overlaps/interface.hpp>
#include <boost/geometry/algorithms/detail/relate/implementation.hpp>
#include <boost/geometry/algorithms/detail/relate/implementation_gc.hpp>
#include <boost/geometry/algorithms/not_implemented.hpp>

#include <boost/geometry/geometries/concepts/check.hpp>

#include <boost/geometry/strategies/detail/box_topology.hpp>
#include <boost/geometry/strategies/relate/cartesian.hpp>
#include <boost/geometry/strategies/relate/geographic.hpp>
#include <boost/geometry/strategies/relate/spherical.hpp>

#include <boost/geometry/views/detail/geometry_collection_view.hpp>


namespace boost { namespace geometry
{

#ifndef DOXYGEN_NO_DETAIL
namespace detail { namespace overlaps
{

template
<
    std::size_t Dimension,
    std::size_t DimensionCount
>
struct box_box_loop
    : strategies::detail::box_topology_detail::overlaps_loop<Dimension, DimensionCount>
{};

struct box_box
{
    template <typename Box1, typename Box2, typename Strategy>
    static inline bool apply(Box1 const& b1, Box2 const& b2, Strategy const& strategy)
    {
        return strategies::detail::get_box_topology(strategy).overlaps().apply(b1, b2);
    }
};

}} // namespace detail::overlaps
#endif // DOXYGEN_NO_DETAIL


#ifndef DOXYGEN_NO_DISPATCH
namespace dispatch
{

template <typename Box1, typename Box2>
struct overlaps<Box1, Box2, box_tag, box_tag>
    : detail::overlaps::box_box
{};


template <typename Geometry1, typename Geometry2>
struct overlaps<Geometry1, Geometry2, geometry_collection_tag, geometry_collection_tag>
{
    template <typename Strategy>
    static inline bool apply(Geometry1 const& geometry1, Geometry2 const& geometry2,
                             Strategy const& strategy)
    {
        int dimension1 = detail::gc_topological_dimension(geometry1);
        int dimension2 = detail::gc_topological_dimension(geometry2);

        if (dimension1 >= 0 && dimension2 >= 0)
        {
            if (dimension1 == 1 && dimension2 == 1)
            {
                return detail::relate::relate_impl
                    <
                        detail::de9im::static_mask_overlaps_d1_1_d2_1_type,
                        Geometry1,
                        Geometry2
                    >::apply(geometry1, geometry2, strategy);
            }
            else if (dimension1 == dimension2)
            {
                return detail::relate::relate_impl
                    <
                        detail::de9im::static_mask_overlaps_d1_eq_d2_type,
                        Geometry1,
                        Geometry2
                    >::apply(geometry1, geometry2, strategy);
            }
        }

        return false;
    }
};

template <typename Geometry1, typename Geometry2, typename Tag1>
struct overlaps<Geometry1, Geometry2, Tag1, geometry_collection_tag>
{
    template <typename Strategy>
    static inline bool apply(Geometry1 const& geometry1, Geometry2 const& geometry2,
                             Strategy const& strategy)
    {
        using gc1_view_t = detail::geometry_collection_view<Geometry1>;
        return overlaps
            <
                gc1_view_t, Geometry2
            >::apply(gc1_view_t(geometry1), geometry2, strategy);
    }
};

template <typename Geometry1, typename Geometry2, typename Tag2>
struct overlaps<Geometry1, Geometry2, geometry_collection_tag, Tag2>
{
    template <typename Strategy>
    static inline bool apply(Geometry1 const& geometry1, Geometry2 const& geometry2,
                             Strategy const& strategy)
    {
        using gc2_view_t = detail::geometry_collection_view<Geometry2>;
        return overlaps
            <
                Geometry1, gc2_view_t
            >::apply(geometry1, gc2_view_t(geometry2), strategy);
    }
};


} // namespace dispatch
#endif // DOXYGEN_NO_DISPATCH


}} // namespace boost::geometry

#endif // BOOST_GEOMETRY_ALGORITHMS_DETAIL_OVERLAPS_IMPLEMENTATION_HPP
