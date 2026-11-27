// Boost.Geometry
// Copyright (c) 2019 Tinko Bartels, Berlin, Germany.
// Copyright (c) 2021 Oracle and/or its affiliates.
// Copyright (c) 2023 Adam Wulkiewicz, Lodz, Poland.
// Copyright (c) 2026 Tinko Bartels.
// Distributed under the Boost Software License, Version 1.0.

#ifndef BOOST_GEOMETRY_UTIL_DETAIL_EXPANSION_HPP
#define BOOST_GEOMETRY_UTIL_DETAIL_EXPANSION_HPP

#include <algorithm>
#include <array>
#include <cmath>
#include <cstddef>
#include <numeric>

#include <boost/geometry/util/error_free_transforms.hpp>
#include <boost/geometry/util/detail/precise_fp.hpp>

BOOST_GEOMETRY_DETAIL_PRECISE_FP_BEGIN

namespace boost { namespace geometry { namespace detail { namespace precise_math
{

// These operations assume the range and rounding contract of the error-free transforms.
// see theorem 12, figure 7, page 11 - 12,
// this is the 2 by 2 case for the corresponding diff-method
// note that this method takes input in descending order of magnitude and
// returns components in ascending order of magnitude
template
<
    typename RealNumber
>
inline std::array<RealNumber, 4> two_two_expansion_diff(
        std::array<RealNumber, 2> const a,
        std::array<RealNumber, 2> const b)
{
    std::array<RealNumber, 4> h;
    std::array<RealNumber, 2> Qh = two_diff(a[1], b[1]);
    h[0] = Qh[1];
    Qh = two_sum( a[0], Qh[0] );
    RealNumber _j = Qh[0];
    Qh = two_diff(Qh[1], b[0]);
    h[1] = Qh[1];
    Qh = two_sum( _j, Qh[0] );
    h[2] = Qh[1];
    h[3] = Qh[0];
    return h;
}

// see theorem 13, figure 8. This implementation uses zero elimination as
// suggested on page 17, second to last paragraph. Returns the number of
// non-zero components in the result and writes the result to h.
// the merger into a single sequence g is done implicitly
template
<
    typename RealNumber,
    std::size_t InSize1,
    std::size_t InSize2,
    std::size_t OutSize
>
inline int fast_expansion_sum_zeroelim(
        std::array<RealNumber, InSize1> const& e,
        std::array<RealNumber, InSize2> const& f,
        std::array<RealNumber, OutSize> & h,
        int m = InSize1,
        int n = InSize2)
{
    static_assert(OutSize > 0 && OutSize >= InSize1 + InSize2,
                  "Expansion sum needs room for both operands and a zero result");
    if (m == 0 || n == 0)
    {
        RealNumber const* const input = m == 0 ? f.data() : e.data();
        int const size = m == 0 ? n : m;
        int used = 0;
        for (int i = 0; i < size; ++i)
        {
            if (input[i] != 0) h[used++] = input[i];
        }
        if (used == 0) h[used++] = RealNumber(0);
        return used;
    }

    std::array<RealNumber, 2> Qh;
    int i_e = 0;
    int i_f = 0;
    int i_h = 0;
    if (std::abs(f[0]) > std::abs(e[0]))
    {
        Qh[0] = e[i_e++];
    }
    else
    {
        Qh[0] = f[i_f++];
    }
    i_h = 0;
    if ((i_e < m) && (i_f < n))
    {
        if (std::abs(f[i_f]) > std::abs(e[i_e]))
        {
            Qh = fast_two_sum(e[i_e++], Qh[0]);
        }
        else
        {
            Qh = fast_two_sum(f[i_f++], Qh[0]);
        }
        if (Qh[1] != 0.0)
        {
            h[i_h++] = Qh[1];
        }
        while ((i_e < m) && (i_f < n))
        {
            if (std::abs(f[i_f]) > std::abs(e[i_e]))
            {
                Qh = two_sum(Qh[0], e[i_e++]);
            }
            else
            {
                Qh = two_sum(Qh[0], f[i_f++]);
            }
            if (Qh[1] != 0.0)
            {
                h[i_h++] = Qh[1];
            }
        }
    }
    while (i_e < m)
    {
        Qh = two_sum(Qh[0], e[i_e++]);
        if (Qh[1] != 0.0)
        {
            h[i_h++] = Qh[1];
        }
    }
    while (i_f < n)
    {
        Qh = two_sum(Qh[0], f[i_f++]);
        if (Qh[1] != 0.0)
        {
            h[i_h++] = Qh[1];
        }
    }
    if ((Qh[0] != 0.0) || (i_h == 0))
    {
        h[i_h++] = Qh[0];
    }
    return i_h;
}

// see theorem 19, figure 13, page 20 - 21. This implementation uses zero
// elimination as suggested on page 17, second to last paragraph. Returns the
// number of non-zero components in the result and writes the result to h.
template
<
    typename RealNumber,
    std::size_t InSize
>
inline int scale_expansion_zeroelim(
        std::array<RealNumber, InSize> const& e,
        RealNumber const b,
        std::array<RealNumber, 2 * InSize> & h,
        int e_non_zeros = InSize)
{
    std::array<RealNumber, 2> Qh = two_product(e[0], b);
    int i_h = 0;
    if (Qh[1] != 0)
    {
        h[i_h++] = Qh[1];
    }
    for (int i_e = 1; i_e < e_non_zeros; i_e++)
    {
        std::array<RealNumber, 2> Tt = two_product(e[i_e], b);
        Qh = two_sum(Qh[0], Tt[1]);
        if (Qh[1] != 0)
        {
            h[i_h++] = Qh[1];
        }
        Qh = fast_two_sum(Tt[0], Qh[0]);
        if (Qh[1] != 0)
        {
            h[i_h++] = Qh[1];
        }
    }
    if ((Qh[0] != 0.0) || (i_h == 0))
    {
        h[i_h++] = Qh[0];
    }
    return i_h;
}

// A fixed-capacity expansion for callers that retain results. Capacity failure
// invalidates the result instead of truncating its exact value.
template <std::size_t Capacity>
struct expansion
{
    std::array<double, Capacity> components;
    std::size_t size = 0;
    bool valid = true;

    // Unused components have no value; every read is bounded by size.
    expansion() noexcept {}
    expansion(expansion const& other) noexcept : size(other.size), valid(other.valid)
    {
        std::copy_n(other.components.begin(), size, components.begin());
    }
    expansion& operator=(expansion const& other) noexcept
    {
        if (this != &other)
        {
            std::copy_n(other.components.begin(), other.size, components.begin());
            size = other.size;
            valid = other.valid;
        }
        return *this;
    }
    expansion(expansion&& other) noexcept : expansion(static_cast<expansion const&>(other)) {}
    expansion& operator=(expansion&& other) noexcept
    { return *this = static_cast<expansion const&>(other); }

    // The active input components are nonoverlapping and ordered by magnitude.
    template <std::size_t N>
    bool add(std::array<double, N> const& terms, int count = N)
    {
        if (!valid) return false;
        std::array<double, Capacity + N> sum;
        int const used = fast_expansion_sum_zeroelim(components, terms, sum, int(size), count);
        if (std::size_t(used) > Capacity) return valid = false;
        size = used == 1 && sum[0] == 0 ? 0 : std::size_t(used);
        std::copy_n(sum.begin(), size, components.begin());
        return true;
    }

    bool add(double value)
    {
        return add(std::array<double, 1>{{value}});
    }

    bool product(double a, double b, bool subtract = false)
    {
        auto const [head, tail] = two_product(a, b);
        return add(std::array<double, 2>{{subtract ? -tail : tail, subtract ? -head : head}});
    }

    int sign() const
    {
        return size == 0 ? 0 : components[size - 1] > 0 ? 1 : -1;
    }

    double estimate() const
    {
        return std::accumulate(components.begin(), components.begin() + size, 0.0);
    }
};

template <std::size_t Out, std::size_t A, std::size_t B>
inline bool add_product(expansion<Out>& result, expansion<A> const& a,
                        expansion<B> const& b, bool subtract = false)
{
    if constexpr (A < B)
    {
        return add_product(result, b, a, subtract);
    }
    else
    {
        if (!a.valid || !b.valid) return result.valid = false;
        if (a.size == 0 || b.size == 0) return result.valid;
        std::array<double, 2 * A> scaled;
        for (std::size_t i = 0; i < b.size; ++i)
        {
            int const count = scale_expansion_zeroelim(
                a.components, subtract ? -b.components[i] : b.components[i], scaled, int(a.size));
            if (!result.add(scaled, count)) return false;
        }
        return result.valid;
    }
}

inline expansion<2> expand_difference(double a, double b)
{
    auto const [head, tail] = two_diff(a, b);
    expansion<2> result;
    result.add(std::array<double, 2>{{tail, head}});
    return result;
}

}}}} // namespace boost::geometry::detail::precise_math

BOOST_GEOMETRY_DETAIL_PRECISE_FP_END

#endif // BOOST_GEOMETRY_UTIL_DETAIL_EXPANSION_HPP
