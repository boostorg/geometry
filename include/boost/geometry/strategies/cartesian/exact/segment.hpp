// Boost.Geometry
// Copyright (c) 2026 Tinko Bartels.
// Licensed under the Boost Software License, Version 1.0.
// http://www.boost.org/LICENSE_1_0.txt

#ifndef BOOST_GEOMETRY_STRATEGIES_CARTESIAN_EXACT_SEGMENT_HPP
#define BOOST_GEOMETRY_STRATEGIES_CARTESIAN_EXACT_SEGMENT_HPP

#include <array>
#include <boost/geometry/strategies/cartesian/exact/filtered_segment.hpp>
#include <boost/geometry/strategies/cartesian/exact/native_predicates.hpp>
#include <boost/geometry/strategies/side_info.hpp>

namespace boost { namespace geometry { namespace strategies { namespace detail
{ namespace exact_cartesian
{

template <typename Coordinate>
struct segment_intersection
{
    using cs_tag = cartesian_tag;
    using point_type = exact_point<Coordinate>;

    struct construction_info
    {
        int endpoint = -1;

        template <typename Segment1, typename Segment2>
        void calculate(point_type& point, Segment1 const& a, Segment2 const& b) const
        {
            switch (endpoint)
            {
            case 0: point = make_point<Coordinate>(a.first); return;
            case 1: point = make_point<Coordinate>(a.second); return;
            case 2: point = make_point<Coordinate>(b.first); return;
            case 3: point = make_point<Coordinate>(b.second); return;
            default:
                point = construct_intersection(make_point<Coordinate>(a.first),
                    make_point<Coordinate>(a.second), make_point<Coordinate>(b.first),
                    make_point<Coordinate>(b.second));
            }
        }

        template <typename Point, typename Segment1, typename Segment2>
        void calculate(Point& point, Segment1 const& a, Segment2 const& b) const
        {
            point_type result;
            calculate(result, a, b);
            assign_output(result, point);
        }
    };

    template <typename Range1, typename Range2, typename Policy>
    static typename Policy::return_type apply(Range1 const& p, Range2 const& q, Policy const& policy)
    {
        using selected = filtered_segment_detail::endpoint_policy<Policy, Coordinate>;
        int const side_a0 = static_orientation<Coordinate>(q.at(0), q.at(1), p.at(0));
        int const side_a1 = static_orientation<Coordinate>(q.at(0), q.at(1), p.at(1));
        if (side_a0 * side_a1 > 0) return selected::disjoint();
        int const side_b0 = static_orientation<Coordinate>(p.at(0), p.at(1), q.at(0));
        int const side_b1 = static_orientation<Coordinate>(p.at(0), p.at(1), q.at(1));
        if (side_b0 * side_b1 > 0) return selected::disjoint();

        if (side_a0 == 0 && side_a1 == 0 && side_b0 == 0 && side_b1 == 0)
            return filtered_collinear_intersection<Coordinate>::apply(p, q, policy);

        auto const& a0 = p.at(0);
        auto const& a1 = p.at(1);
        auto const& b0 = q.at(0);
        auto const& b1 = q.at(1);

        int const endpoint = side_a0 == 0 ? 0 : side_a1 == 0 ? 1
                           : side_b0 == 0 ? 2 : side_b1 == 0 ? 3 : -1;
        construction_info const info{endpoint};
        side_info sides;
        sides.set<0>(side_a0, side_a1);
        sides.set<1>(side_b0, side_b1);
        if (side_a0 == 0 && (static_equal_points<Coordinate>(a0, b0) || static_equal_points<Coordinate>(a0, b1)))
            return selected::segments_share_common_point(sides, info, a0);
        if (side_a1 == 0 && (static_equal_points<Coordinate>(a1, b0) || static_equal_points<Coordinate>(a1, b1)))
            return selected::segments_share_common_point(sides, info, a1);
        using first_point = std::remove_cv_t<std::remove_reference_t<decltype(a0)>>;
        using second_point = std::remove_cv_t<std::remove_reference_t<decltype(b0)>>;
        model::referring_segment<first_point const> const first(a0, a1);
        model::referring_segment<second_point const> const second(b0, b1);
        return selected::segments_crosses(sides, info, first, second);
    }
};

}}}}} // namespace boost::geometry::strategies::detail::exact_cartesian
#endif
