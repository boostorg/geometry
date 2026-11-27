// Boost.Geometry
// Copyright (c) 2026 Tinko Bartels.
// Distributed under the Boost Software License, Version 1.0.

#ifndef BOOST_GEOMETRY_STRATEGIES_CARTESIAN_EXACT_CONSTRUCTION_ROUNDING_HPP
#define BOOST_GEOMETRY_STRATEGIES_CARTESIAN_EXACT_CONSTRUCTION_ROUNDING_HPP

#include <algorithm>
#include <array>
#include <cmath>
#include <cstddef>
#include <cstdint>
#include <limits>
#include <stdexcept>
#include <type_traits>

#include <boost/config.hpp>
#include <boost/geometry/strategies/cartesian/exact/forward_error.hpp>
#include <boost/geometry/strategies/cartesian/exact/output_policy.hpp>
#include <boost/geometry/strategies/cartesian/exact/point_cache.hpp>
#include <boost/geometry/util/numeric_cast.hpp>
#include <boost/geometry/util/error_free_transforms.hpp>

#include <boost/geometry/util/detail/precise_fp.hpp>

BOOST_GEOMETRY_DETAIL_PRECISE_FP_BEGIN

namespace boost { namespace geometry { namespace strategies { namespace detail
{ namespace exact_cartesian { namespace construction_rounding
{

// IEEE binary64, nearest/ties-even, no implicit contraction or fast-math.
// FP overflow/underflow in first-stage arithmetic is outside the contract.
// Producers are {ax,ay,bx,by,cx,cy,dx,dy}, supplied anew by the owner on every
// arithmetic call. A state never retains coordinates, pointers or callbacks.
// Copies and their plain cache indices belong to the enclosing operation;
// they must not outlive its point_cache_scope or be reused with other producers.
namespace detail
{
namespace fe = forward_error;

template <typename C>
inline bool represented(std::array<C, 8> const& input, std::array<double, 8>& output)
{
    for (std::size_t i = 0; i < input.size(); ++i)
    {
        double error;
        if (!fe::try_native_coordinate(input[i], output[i], error) || error != 0) return false;
    }
    return true;
}

struct storage
{
    std::array<double, 2> approximate{};
    std::array<double, 2> error{};
    point_cache_detail::native_cache_handle precision;
};
} // namespace detail

template <typename C, typename Policy = relate::approximate_output>
class point_state
{
    static_assert(fixed_native_supported<C>::value, "Native construction coordinates required");
    static_assert(std::numeric_limits<double>::is_iec559
        && std::numeric_limits<double>::radix == 2 && std::numeric_limits<double>::digits == 53,
        "Construction rounding requires IEEE binary64");
    using cache = point_cache_detail::native_cache<C>;

public:
    using coordinate_type = C;
    using policy_type = Policy;
    using producers_type = std::array<C, 8>;
    using storage_type = detail::storage;

    double approximate(std::size_t dimension) const
    { return m_state.approximate.at(dimension); }

    // Search boxes require an enclosure even when no per-axis error is stored.
    scalar_approximation approximation(std::size_t dimension) const
    {
        return {m_state.approximate.at(dimension), m_state.error[dimension]};

    }

    point_cache_detail::native_cache_handle& cache_handle() const
    { return m_state.precision; }

    void init(producers_type const& producers)
    {
        m_state = storage_type{};
        forward_error::point_approximation first;
        bool const ready = forward_error::try_intersection(producers, first);
        for (std::size_t d = 0; d < 2; ++d)
        {
            if (ready)
            {
                m_state.approximate[d] = first.value[d];
                if (first.value[d] == 0 && first.error[d] == 0) m_state.approximate[d] = 0;
                m_state.error[d] = first.error[d];
            }
            else
            {
                refine_value(producers, d);
            }
        }
    }

    // Cache refinement returns nearest binary64. No cache reference survives
    // another indexed lookup.
    // Without an enclosing scope, native_cache uses stack temporaries instead.
    scalar_approximation refine(producers_type const& producers, std::size_t dimension) const
    {
        refine_value(producers, dimension);
        return approximation(dimension);
    }

private:
    void refine_value(producers_type const& producers, std::size_t dimension) const
    {
        check_dimension(dimension);
        std::array<double, 8> represented;
        scalar_approximation refined;
        if (cache::cached_approximation(m_state.precision, dimension, refined)
            || (detail::represented(producers, represented)
                && cache::refine(m_state.precision, represented, dimension, refined)))
        {
            update(dimension, refined.value, refined.error == 0);
        }
        else
        {
            auto const exact = cache::get(m_state.precision, true, producers);
            double rounded;
            if (!try_fixed_round(exact, dimension, rounded) || !std::isfinite(rounded))
                throw std::domain_error("Intersection has no finite Cartesian approximation");
            update(dimension, rounded, (dimension == 0 ? exact.X : exact.Y) == 0);
        }
    }

private:
    static void check_dimension(std::size_t dimension)
    {
        if (dimension > 1) throw std::out_of_range("Exact Cartesian coordinate dimension");
    }

    void update(std::size_t dimension, double nearest, bool zero) const
    {
        m_state.approximate[dimension] = nearest;
        m_state.error[dimension] = zero ? 0 : 0.5 * arithmetic_detail::max_adjacent_spacing(nearest);
    }

    mutable storage_type m_state;
};

}}}}}} // namespace boost::geometry::strategies::detail::exact_cartesian::construction_rounding

BOOST_GEOMETRY_DETAIL_PRECISE_FP_END

#endif
