// Boost.Geometry
// Copyright (c) 2026 Tinko Bartels.
// Distributed under the Boost Software License, Version 1.0.

#ifndef BOOST_GEOMETRY_STRATEGIES_CARTESIAN_EXACT_SEGMENT_POSITION_HPP
#define BOOST_GEOMETRY_STRATEGIES_CARTESIAN_EXACT_SEGMENT_POSITION_HPP

#include <cstddef>
#include <boost/geometry/strategies/intersection_result.hpp>
#include <boost/geometry/strategies/cartesian/exact/predicate_dispatch.hpp>

namespace boost { namespace geometry { namespace strategies { namespace detail
{ namespace exact_cartesian
{

struct segment_position_tag {};

struct ratio_free_segment_order
{
    using no_fraction = void;
    template <typename A, typename B, typename P, typename Q>
    static int compare(A const& a, B const& b, P const& p, Q const& q)
    {
        return compare_along_segment<void, typename P::root_type>(a, b, p, q);
    }
    template <typename P, typename Q>
    static bool equal(P const& p, Q const& q) { return root_equal<typename P::root_type>(p, q); }
};

template <typename Coordinate>
struct segment_position
{
    template <typename A, typename B, typename P, typename Q>
    static int apply(A const& a, B const& b, P const& p, Q const& q)
    {
        return compare_along_segment<Coordinate>(a, b, p, q);
    }


    template <typename A, typename B, typename P>
    static segment_location classify(A const& a, B const& b, P const& p)
    {
        return classify_point_on_segment<Coordinate>(a, b, p);
    }
};

}}}}} // namespace boost::geometry::strategies::detail::exact_cartesian


#endif
