// Boost.Geometry
// Copyright (c) 2026 Tinko Bartels.
// Distributed under the Boost Software License, Version 1.0.

#ifndef BOOST_GEOMETRY_STRATEGIES_CARTESIAN_EXACT_AREAL_EQUALITY_HPP
#define BOOST_GEOMETRY_STRATEGIES_CARTESIAN_EXACT_AREAL_EQUALITY_HPP

#include <algorithm>
#include <cmath>
#include <cstddef>
#include <limits>
#include <type_traits>
#include <boost/range/iterator_range_core.hpp>
#include <boost/geometry/algorithms/detail/for_each_range.hpp>
#include <boost/geometry/core/access.hpp>
#include <boost/geometry/core/interior_rings.hpp>
#include <boost/geometry/core/point_order.hpp>
#include <boost/geometry/strategies/cartesian/exact/native_traits.hpp>

#include <boost/geometry/util/detail/precise_fp.hpp>

BOOST_GEOMETRY_DETAIL_PRECISE_FP_BEGIN

namespace boost { namespace geometry { namespace strategies { namespace detail { namespace exact_cartesian
{

template <typename T>
struct areal_equality
{
private:
    template <typename A, typename B, typename C>
    static constexpr bool lossless()
    {
        return lossless_coordinate_conversion<coordinate_type_t<A>, C>::value
            && lossless_coordinate_conversion<coordinate_type_t<B>, C>::value;
    }

    struct area_head
    {
        double sum = 0, magnitude = 0;
        std::size_t terms = 0;

        template <typename Ring>
        void add_ring(Ring const& ring)
        {
            auto it = boost::begin(ring);
            auto const end = boost::end(ring);
            if (it == end) return;
            double const ox = static_cast<double>(get<0>(*it)), oy = static_cast<double>(get<1>(*it));
            if (++it == end) return;
            double x = static_cast<double>(get<0>(*it)) - ox;
            double y = static_cast<double>(get<1>(*it)) - oy;
            for (++it; it != end; ++it)
            {
                double const next_x = static_cast<double>(get<0>(*it)) - ox;
                double const next_y = static_cast<double>(get<1>(*it)) - oy;
                double const l = x * next_y, r = y * next_x;
                sum += l - r;
                magnitude += std::abs(l) + std::abs(r);
                ++terms;
                x = next_x;
                y = next_y;
            }
        }

        template <typename G>
        void add(G const& geometry)
        {
            if constexpr (std::is_same<tag_t<G>, ring_tag>::value)
                add_ring(geometry);
            else if constexpr (std::is_same<tag_t<G>, polygon_tag>::value)
            {
                add_ring(exterior_ring(geometry));
                for (auto const& inner : boost::make_iterator_range(interior_rings(geometry)))
                    add_ring(inner);
            }
            else
                for (auto const& polygon : boost::make_iterator_range(geometry))
                    add(polygon);
        }
    };

public:
    template <typename A, typename B>
    static bool bounds_match(A const& a, B const& b)
    {
        if constexpr (!lossless<A, B, T>())
            return true;
        else
        {
            T min_x = (std::numeric_limits<T>::max)(), min_y = min_x;
            T max_x = (std::numeric_limits<T>::lowest)(), max_y = max_x;
            geometry::detail::all_ranges_of(a, [&](auto const& range)
            {
                for (auto const& p : boost::make_iterator_range(range))
                {
                    T const x = static_cast<T>(get<0>(p)), y = static_cast<T>(get<1>(p));
                    min_x = (std::min)(min_x, x); max_x = (std::max)(max_x, x);
                    min_y = (std::min)(min_y, y); max_y = (std::max)(max_y, y);
                }
                return true;
            });
            unsigned reached = 0;
            bool const covered = geometry::detail::all_ranges_of(b, [&](auto const& range)
            {
                for (auto const& p : boost::make_iterator_range(range))
                {
                    T const x = static_cast<T>(get<0>(p)), y = static_cast<T>(get<1>(p));
                    if (x < min_x || x > max_x || y < min_y || y > max_y) return false;
                    reached |= unsigned(x == min_x) | (unsigned(x == max_x) << 1)
                             | (unsigned(y == min_y) << 2) | (unsigned(y == max_y) << 3);
                }
                return true;
            });
            return covered && reached == 15;
        }
    }

    template <typename A, typename B>
    static bool area_can_match(A const& a, B const& b)
    {
        if constexpr (!lossless<A, B, double>())
            return true;
        else
        {
            area_head sa, sb;
            sa.add(a); sb.add(b);
            std::size_t const n = sa.terms + sb.terms;
            if (n > (1u << 22)) return true;
            double const difference = geometry::point_order<A>::value == geometry::point_order<B>::value
                ? sa.sum - sb.sum : sa.sum + sb.sum;
            double const magnitude = sa.magnitude + sb.magnitude;
            // Paths from the inputs to the difference and to its magnitude
            // have at most n+5 rounded operations. With (n+8)*epsilon < 2^-29,
            // 4*(n+8)*epsilon bounds their combined relative error, including
            // rounding the bound itself. The additive guard covers subnormal
            // rounding in the products and sums, without a per-term test.
            double const count = static_cast<double>(n + 8);
            double const error = (4 * std::numeric_limits<double>::epsilon() * count) * magnitude
                               + (8 * (std::numeric_limits<double>::min)()) * count;
            return !(std::isfinite(difference) && std::abs(difference) > error);
        }
    }
};

}}}}}

BOOST_GEOMETRY_DETAIL_PRECISE_FP_END

#endif
