// Boost.Geometry
// Copyright (c) 2026 Tinko Bartels.
// Distributed under the Boost Software License, Version 1.0.

#ifndef BOOST_GEOMETRY_STRATEGIES_CARTESIAN_EXACT_POINT_PRECISION_CACHE_HPP
#define BOOST_GEOMETRY_STRATEGIES_CARTESIAN_EXACT_POINT_PRECISION_CACHE_HPP

#include <climits>
#include <cstddef>
#include <limits>
#include <stdexcept>
#include <type_traits>
#include <utility>
#include <boost/assert.hpp>
#include <boost/geometry/strategies/cartesian/exact/arithmetic.hpp>
#include <boost/geometry/strategies/cartesian/exact/operation_context.hpp>
#include <boost/geometry/strategies/cartesian/exact/fixed_rounding.hpp>

namespace boost { namespace geometry { namespace strategies { namespace detail
{ namespace exact_cartesian
{

template <typename C>
using auxiliary_point_type = typename operation_detail::storage_for<C>::auxiliary_type;

template <typename C>
using precision_homogeneous_point = auxiliary_point_type<C>;
namespace point_precision_detail
{

template <typename C, std::size_t N, std::size_t D>
inline precision_homogeneous_point<C> promote(fixed_homogeneous<N, D> const& value)
{
    precision_homogeneous_point<C> output;
    if (!try_fixed_narrow(value, output))
        throw std::overflow_error("Exact auxiliary point exceeds the supported native expression capacity");
    return output;
}

template <typename C>
class point_handle
{
public:
    point_handle() = default;

    template <std::size_t N, std::size_t D>
    static point_handle create(fixed_homogeneous<N, D> const& value)
    {
        auto& cache = operation_detail::require<C>().storage();
        auto point = promote<C>(value);
        cache.auxiliary.push_back(std::move(point));
        point_handle output;
        output.m_index = cache.auxiliary.size() - 1;
        return output;
    }

    explicit operator bool() const { return m_index != (std::numeric_limits<std::size_t>::max)(); }
    auxiliary_point_type<C> get() const
    {
        return entry();
    }
    template <std::size_t N, std::size_t D>
    bool copy_compact(fixed_homogeneous<N, D>& output) const
    {
        return try_fixed_narrow(entry(), output);
    }
private:
    auxiliary_point_type<C> const& entry() const
    {
        auto const& cache = operation_detail::require<C>().storage();
        BOOST_ASSERT(m_index < cache.auxiliary.size());
        return cache.auxiliary[m_index];
    }

    std::size_t m_index = (std::numeric_limits<std::size_t>::max)();
};


} // namespace point_precision_detail
}}}}} // namespace boost::geometry::strategies::detail::exact_cartesian

#endif
