// Boost.Geometry
// Copyright (c) 2026 Tinko Bartels.
// Distributed under the Boost Software License, Version 1.0.

#ifndef BOOST_GEOMETRY_STRATEGIES_CARTESIAN_EXACT_BOX_RELATIONS_HPP
#define BOOST_GEOMETRY_STRATEGIES_CARTESIAN_EXACT_BOX_RELATIONS_HPP

#include <cstddef>
#include <type_traits>
#include <boost/geometry/core/tag.hpp>
#include <boost/geometry/core/cs.hpp>
#include <boost/geometry/core/tags.hpp>

namespace boost { namespace geometry { namespace strategies { namespace detail
{ namespace exact_cartesian
{

enum class box_relation { within, covered_by, disjoint, touches, overlaps, equals };

template <typename Access, box_relation Relation>
struct box_relation_predicate
{
    using cs_tag = cartesian_tag;
private:
    template <bool Strict, std::size_t D, typename Point, typename Box>
    static bool point_axis(Point const& point, Box const& box)
    {
        int const low = Access::template compare_point<D, 0>(point, box);
        int const high = Access::template compare_point<D, 1>(point, box);
        return Strict ? low > 0 && high < 0 : low >= 0 && high <= 0;
    }

    template <bool Strict, typename Point, typename Box>
    static bool point_inside(Point const& point, Box const& box)
    {
        return point_axis<Strict, 0>(point, box) && point_axis<Strict, 1>(point, box);
    }

    template <std::size_t D, typename A, typename B>
    static bool axis(A const& a, B const& b)
    {
        return Access::template compare_bounds<D, 0, 0>(a, b) >= 0
            && Access::template compare_bounds<D, 1, 1>(a, b) <= 0
            && (!Access::check_both_corners
                || (Access::template compare_bounds<D, 1, 0>(a, b) >= 0
                    && Access::template compare_bounds<D, 0, 1>(a, b) <= 0));
    }

    template <typename A, typename B>
    static bool covered(A const& a, B const& b) { return axis<0>(a, b) && axis<1>(a, b); }

    template <bool Closed, typename A, typename B>
    static bool separated(A const& a, B const& b)
    {
        constexpr int limit = Closed ? 0 : 1;
        return Access::template compare_bounds<0, 1, 0>(a, b) < limit
            || Access::template compare_bounds<0, 1, 0>(b, a) < limit
            || Access::template compare_bounds<1, 1, 0>(a, b) < limit
            || Access::template compare_bounds<1, 1, 0>(b, a) < limit;
    }

public:
    template <typename A, typename B>
    static bool apply(A const& a, B const& b)
    {
        if constexpr (Relation == box_relation::equals)
            return Access::template compare_bounds<0, 0, 0>(a, b) == 0
                && Access::template compare_bounds<1, 0, 0>(a, b) == 0
                && Access::template compare_bounds<0, 1, 1>(a, b) == 0
                && Access::template compare_bounds<1, 1, 1>(a, b) == 0;
        if (Access::empty(a) || Access::empty(b)) return Relation == box_relation::disjoint;
        if constexpr (std::is_same<geometry::tag_t<A>, point_tag>::value)
        {
            if constexpr (Relation == box_relation::within) return point_inside<true>(a, b);
            if constexpr (Relation == box_relation::covered_by) return point_inside<false>(a, b);
            if constexpr (Relation == box_relation::disjoint) return !point_inside<false>(a, b);
            if constexpr (Relation == box_relation::touches)
                return point_inside<false>(a, b) && !point_inside<true>(a, b);
            return false;
        }
        else
        {
            if constexpr (Relation == box_relation::covered_by) return covered(a, b);
            if constexpr (Relation == box_relation::within)
                return covered(a, b) && Access::template compare_bounds<0, 0, 1>(a, a) < 0
                    && Access::template compare_bounds<1, 0, 1>(a, a) < 0;
            if constexpr (Relation == box_relation::disjoint) return separated<true>(a, b);
            if constexpr (Relation == box_relation::touches)
                return !separated<true>(a, b) && separated<false>(a, b);
            if constexpr (Relation == box_relation::overlaps)
                return !covered(a, b) && !covered(b, a) && !separated<false>(a, b);
            return false;
        }
    }
};

}}}}} // namespace boost::geometry::strategies::detail::exact_cartesian

#endif
