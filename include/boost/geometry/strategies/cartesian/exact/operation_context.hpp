// Boost.Geometry
// Copyright (c) 2026 Tinko Bartels.
// Distributed under the Boost Software License, Version 1.0.

#ifndef BOOST_GEOMETRY_STRATEGIES_CARTESIAN_EXACT_OPERATION_CONTEXT_HPP
#define BOOST_GEOMETRY_STRATEGIES_CARTESIAN_EXACT_OPERATION_CONTEXT_HPP

#include <array>
#include <cstddef>
#include <new>
#include <stdexcept>
#include <type_traits>
#include <vector>
#include <boost/geometry/strategies/cartesian/exact/arithmetic.hpp>
#include <boost/geometry/strategies/cartesian/exact/fixed_arithmetic.hpp>
#include <boost/geometry/util/detail/expansion.hpp>

namespace boost { namespace geometry { namespace strategies { namespace detail
{ namespace exact_cartesian
{

namespace operation_detail
{
template <std::size_t N, std::size_t D>
struct point_storage
{
    using native_type = fixed_homogeneous<N, D>;
    // A midpoint between two geometry vertices needs N+D+1 numerator
    // bits (sum of cross-products) and 2D+1 denominator bits (2*W1*W2).
    using auxiliary_type = fixed_homogeneous<N + D + 1, 2 * D + 1>;

    struct expanded_coordinate
    {
        geometry::detail::precise_math::expansion<32> numerator;
        geometry::detail::precise_math::expansion<16> denominator;
        bool attempted = false;
        bool valid = false;
        bool bounded = false;
        scalar_approximation approximation;
    };
    struct native_entry
    {
        // Vector emplacement must not zero the inactive expansion storage.
        native_entry() {}
        native_type value;
        bool exact_ready = false;
        std::array<expanded_coordinate, 2> expanded;
    };
    std::vector<native_entry> native;
    std::vector<auxiliary_type> auxiliary;
};

template <typename C>
using storage_for = point_storage<native_fixed_traits<C>::intersection_numerator_bits,
                                  native_fixed_traits<C>::intersection_denominator_bits>;

template <typename C>
class context
{
public:
    context() = default;
    context(context const&) = delete;
    context& operator=(context const&) = delete;

    storage_for<C>& storage() { return m_storage; }
    storage_for<C> const& storage() const { return m_storage; }

private:
    storage_for<C> m_storage;
};

template <typename C>
class context_storage
{
public:
    context_storage() = default;
    ~context_storage() { if (value) value->~context<C>(); }
    context_storage(context_storage const&) = delete;
    context_storage& operator=(context_storage const&) = delete;
    context<C>& get()
    {
        if (!value) value = new (&m_storage) context<C>();
        return *value;
    }
    context<C>* value = nullptr;
private:
    using storage_type = typename std::aligned_storage<sizeof(context<C>), alignof(context<C>)>::type;
    storage_type m_storage;
};

template <typename C>
inline context_storage<C>*& active()
{
    static thread_local context_storage<C>* value = nullptr;
    return value;
}

template <typename C>
inline context<C>* current()
{
    auto* const owner = active<C>();
    return owner ? owner->value : nullptr;
}

template <typename C>
inline context<C>& require()
{
    auto* const owner = active<C>();
    if (!owner)
        throw std::logic_error("Auxiliary exact point requires its enclosing operation context");
    return owner->get();
}
} // namespace operation_detail

// The context owns all numerical entries. Nested algorithm calls reuse it;
// there is no process-global numerical store or reference-counted point value.
template <typename C = double>
class point_cache_scope
{
public:
    point_cache_scope() : m_previous(operation_detail::active<C>())
    {
        if (!m_previous)
        {
            operation_detail::active<C>() = &m_owned;
        }
    }
    ~point_cache_scope()
    {
        operation_detail::active<C>() = m_previous;
    }
    point_cache_scope(point_cache_scope const&) = delete;
    point_cache_scope& operator=(point_cache_scope const&) = delete;
private:
    operation_detail::context_storage<C> m_owned;
    operation_detail::context_storage<C>* m_previous;
};

}}}}} // namespace boost::geometry::strategies::detail::exact_cartesian

#endif
