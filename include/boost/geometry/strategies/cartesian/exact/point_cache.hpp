// Boost.Geometry
// Copyright (c) 2026 Tinko Bartels.
// Distributed under the Boost Software License, Version 1.0.

#ifndef BOOST_GEOMETRY_STRATEGIES_CARTESIAN_EXACT_POINT_CACHE_HPP
#define BOOST_GEOMETRY_STRATEGIES_CARTESIAN_EXACT_POINT_CACHE_HPP

#include <array>
#include <cstddef>
#include <cstdint>
#include <limits>
#include <utility>
#include <vector>
#include <boost/assert.hpp>
#include <boost/geometry/strategies/cartesian/exact/intersection_expansion.hpp>
#include <boost/geometry/strategies/cartesian/exact/fixed_arithmetic.hpp>
#include <boost/geometry/strategies/cartesian/exact/fixed_rounding.hpp>
#include <boost/geometry/strategies/cartesian/exact/operation_context.hpp>

#include <boost/geometry/util/detail/precise_fp.hpp>

BOOST_GEOMETRY_DETAIL_PRECISE_FP_BEGIN

namespace boost { namespace geometry { namespace strategies { namespace detail
{ namespace exact_cartesian
{

namespace point_cache_detail
{
class native_cache_handle
{
public:
    native_cache_handle() = default;
    // Copies share the hint within their enclosing operation.

private:
    template <typename> friend class native_cache;
    std::size_t m_index = (std::numeric_limits<std::size_t>::max)();
};

template <typename Coordinate>
class native_cache
{
    using cache_type = operation_detail::storage_for<Coordinate>;
    using expanded_coordinate = typename cache_type::expanded_coordinate;
    using entry = typename cache_type::native_entry;

public:
    using value_type = typename native_fixed_traits<Coordinate>::intersection_type;

    static bool has_exact(native_cache_handle const& handle)
    {
        if (handle.m_index == (std::numeric_limits<std::size_t>::max)()) { return false; }
        auto const* owner = operation_detail::current<Coordinate>();
        if (!owner) { return false; }
        auto const& entries = owner->storage().native;
        BOOST_ASSERT(handle.m_index < entries.size());
        return entries[handle.m_index].exact_ready;
    }

    static bool cached_approximation(native_cache_handle const& handle, std::size_t dimension,
                                     scalar_approximation& output)
    {
        if (handle.m_index == (std::numeric_limits<std::size_t>::max)() || dimension > 1) { return false; }
        auto* const owner = operation_detail::current<Coordinate>();
        if (!owner) { return false; }
        auto const& cache = owner->storage();
        BOOST_ASSERT(handle.m_index < cache.native.size());
        auto const& expanded = cache.native[handle.m_index].expanded[dimension];
        if (!expanded.bounded) { return false; }
        output = expanded.approximation;
        return true;
    }

    static value_type get(native_cache_handle& handle, bool intersection,
                          std::array<Coordinate, 8> const& coordinates)
    {
        if (!intersection || !operation_detail::active<Coordinate>()) { return evaluate(intersection, coordinates); }
        return with_entry(handle, [&](entry& item)
        {
            if (!item.exact_ready)
            {
                item.value = evaluate(true, coordinates);
                item.exact_ready = true;
                cache_fixed_approximations(item);
            }
            return item.value;
        });
    }

    template <std::size_t N, std::size_t D>
    static bool copy_compact(native_cache_handle& handle, bool intersection,
                             std::array<Coordinate, 8> const& coordinates, fixed_homogeneous<N, D>& output)
    {
        if (!intersection)
        {
            return try_fixed_input_homogeneous(coordinates[0], coordinates[1], output);
        }
        if (!operation_detail::active<Coordinate>()) { return try_fixed_narrow(evaluate(true, coordinates), output); }
        return with_entry(handle, [&](entry& item)
        {
            if (!item.exact_ready)
            {
                item.value = evaluate(true, coordinates);
                item.exact_ready = true;
                cache_fixed_approximations(item);
            }
            return try_fixed_narrow(item.value, output);
        });
    }

    static bool refine(native_cache_handle& handle, std::array<double, 8> const& coordinates,
                       std::size_t dimension, scalar_approximation& output)
    {
        if (dimension > 1
            || !construction_detail::supported<construction_detail::quotient_band>(coordinates)) { return false; }
        return with_entry(handle, [&](entry& item)
        {
            if (item.expanded[dimension].bounded)
            {
                output = item.expanded[dimension].approximation;
                return true;
            }
            auto& expanded = expand(item, coordinates, dimension);
            if (!expanded.valid) { return false; }
            if (!expanded.bounded)
            {
                double rounded;
                if (!try_round_quotient(expanded.numerator, expanded.denominator, rounded)) { return false; }
                double const width = rounded == 0 ? 0 : arithmetic_detail::max_adjacent_spacing(rounded);
                expanded.approximation = {rounded, width};
                expanded.bounded = true;
            }
            output = expanded.approximation;
            return true;
        });
    }

    static bool compare(native_cache_handle& handle, std::array<double, 8> const& coordinates,
                        std::size_t dimension, double target, int& result)
    {
        constexpr int band = construction_detail::boundary_band;
        if (dimension > 1 || !construction_detail::supported<band>(coordinates)
            || !exponent_range::contains<band>(target)) { return false; }
        return with_entry(handle, [&](entry& item)
        {
            auto const& expanded = expand(item, coordinates, dimension);
            return expanded.valid && construction_detail::compare_boundary
                (expanded.numerator, expanded.denominator, target, target, result);
        });
    }

    static bool compare(native_cache_handle& first, std::array<double, 8> const& a,
                        native_cache_handle& second, std::array<double, 8> const& b,
                        std::size_t dimension, int& result)
    {
        constexpr int band = construction_detail::pair_comparison_band;
        if (dimension > 1 || !construction_detail::supported<band>(a)
            || !construction_detail::supported<band>(b)) { return false; }
        // Copies keep both operands valid if obtaining the second grows the
        // vector. No cache reference survives the individual lookup.
        auto const x = with_entry(first, [&](entry& item) { return expand(item, a, dimension); });
        auto const y = with_entry(second, [&](entry& item) { return expand(item, b, dimension); });
        if (!x.valid || !y.valid) { return false; }
        geometry::detail::precise_math::expansion<256> difference;
        if (!geometry::detail::precise_math::add_product(difference, x.numerator, y.denominator)
            || !geometry::detail::precise_math::add_product(difference, y.numerator, x.denominator, true)) { return false; }
        result = difference.sign() * x.denominator.sign() * y.denominator.sign();
        return true;
    }

private:
    static void cache_fixed_approximations(entry& item)
    {
        for (std::size_t dimension = 0; dimension != 2; ++dimension)
        {
            auto& expanded = item.expanded[dimension];
            if (expanded.bounded) { continue; }
            double rounded;
            if (!try_fixed_round(item.value, dimension, rounded) || !std::isfinite(rounded)) { continue; }
            bool const zero = (dimension == 0 ? item.value.X : item.value.Y) == 0;
            double const width = zero ? 0 : arithmetic_detail::max_adjacent_spacing(rounded);
            if (!std::isfinite(width)) { continue; }
            expanded.approximation = {rounded, width};
            expanded.bounded = true;
        }
    }

    static expanded_coordinate& expand(entry& item, std::array<double, 8> const& coordinates,
                                       std::size_t dimension)
    {
        auto& result = item.expanded[dimension];
        if (!result.attempted)
        {
            result.valid = construction_detail::homogeneous_coordinate
                (coordinates, dimension, result.numerator, result.denominator)
                && result.denominator.sign() != 0;
            result.attempted = true;
        }
        return result;
    }

    template <typename Function>
    static auto with_entry(native_cache_handle& handle, Function function)
        -> decltype(function(std::declval<entry&>()))
    {
        if (!operation_detail::active<Coordinate>()) { entry temporary; return function(temporary); }
        auto& cache = storage();
        if (handle.m_index == (std::numeric_limits<std::size_t>::max)())
        {
            cache.native.emplace_back();
            handle.m_index = cache.native.size() - 1;
        }
        BOOST_ASSERT(handle.m_index < cache.native.size());
        return function(cache.native[handle.m_index]);
    }

    static value_type evaluate(bool intersection, std::array<Coordinate, 8> const& coordinates)
    {
        value_type result;
        if (intersection) { result = direct_native_homogeneous(coordinates); }
        else
        {
            auto const input = fixed_input_homogeneous(coordinates[0], coordinates[1]);
            result.X = input.X;
            result.Y = input.Y;
            result.W = input.W;
        }
        return result;
    }

    static cache_type& storage()
    {
        return operation_detail::require<Coordinate>().storage();
    }
};
} // namespace point_cache_detail

}}}}} // namespace boost::geometry::strategies::detail::exact_cartesian

BOOST_GEOMETRY_DETAIL_PRECISE_FP_END

#endif
