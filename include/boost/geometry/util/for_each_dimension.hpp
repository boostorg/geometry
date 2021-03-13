// Boost.Geometry

// Copyright (c) 2021, Oracle and/or its affiliates.

// Contributed and/or modified by Adam Wulkiewicz, on behalf of Oracle

// Licensed under the Boost Software License version 1.0.
// http://www.boost.org/users/license.html

#ifndef BOOST_GEOMETRY_UTIL_FOR_EACH_DIMENSION_HPP
#define BOOST_GEOMETRY_UTIL_FOR_EACH_DIMENSION_HPP


#include <boost/config.hpp>

#include <boost/geometry/core/coordinate_dimension.hpp>
#include <boost/geometry/util/type_traits_std.hpp>


namespace boost { namespace geometry
{

#ifndef DOXYGEN_NO_DETAIL
namespace detail
{

// NOTE: Out of curiosity I tested the compilation times of various implementations.
//   Compilation times were tested for point and dimension 1000 with g++-6.3.0 and msvc-14.1.
//   Each coordinate of a point was set to the dimension value.
// In general we deal with small dimensions so the simplest O(N) should be the fastest
//   one anyway.
// It seems that simple O(N) version is faster to compile than O(logN) version based on
//   intervals even for a big value such as 1000. Which is wierd and may indicate that I
//   missed something.
//   The simple version generates deeper recursion but this is not a problem since in general we
//   deal with small number of dimensions.
// A third version I tested using C++17 fold expression was faster in some cases.
//   It depended on optimization level.
// This utility was also slightly faster than for_each_coordinate so I assume the compilation
//   time is at least similar which makes it a viable replacement.

// O(N) version

template <std::size_t I, std::size_t N>
struct for_each_index_impl
{
    template <typename UnaryPredicate>
    constexpr static inline void apply(UnaryPredicate& predicate)
    {
        predicate(util::index_constant<I>());
        for_each_index_impl<I + 1, N>::apply(predicate);
    }
};

template <std::size_t N>
struct for_each_index_impl<N, N>
{
    template <typename UnaryPredicate>
    constexpr static inline void apply(UnaryPredicate& )
    {}
};

template <>
struct for_each_index_impl<0, 1>
{
    template <typename UnaryPredicate>
    constexpr static inline void apply(UnaryPredicate& predicate)
    {
        predicate(util::index_constant<0>());
    }
};

template <>
struct for_each_index_impl<0, 2>
{
    template <typename UnaryPredicate>
    constexpr static inline void apply(UnaryPredicate& predicate)
    {
        predicate(util::index_constant<0>());
        predicate(util::index_constant<1>());
    }
};

template <>
struct for_each_index_impl<0, 3>
{
    template <typename UnaryPredicate>
    constexpr static inline void apply(UnaryPredicate& predicate)
    {
        predicate(util::index_constant<0>());
        predicate(util::index_constant<1>());
        predicate(util::index_constant<2>());
    }
};

// for_each_index_impl<0, N>
// for_each_index_impl<0, 0>

// O(logN) version 1

template <std::size_t F, std::size_t S, std::size_t L>
struct for_each_index_impl1
{
    static const std::size_t M = F + (L - F) / 2;

    template <typename UnaryPredicate>
    constexpr static inline void apply(UnaryPredicate& predicate)
    {
        for_each_index_impl1<F, F+1, M>::apply(predicate);
        for_each_index_impl1<M, M+1, L>::apply(predicate);
    }
};

template <std::size_t F, std::size_t S>
struct for_each_index_impl1<F, S, S>
{
    template <typename UnaryPredicate>
    constexpr static inline void apply(UnaryPredicate& predicate)
    {
        predicate(util::index_constant<F>());
    }
};

template <std::size_t F, std::size_t S>
struct for_each_index_impl1<F, S, F>
{
    template <typename UnaryPredicate>
    constexpr static inline void apply(UnaryPredicate& )
    {}
};

// O(logN) version 2

template <std::size_t N>
struct for_each_index_impl2
{
    static const std::size_t N1 = N / 2;
    static const std::size_t N2 = N - N1;

    template <std::size_t Offset, typename UnaryPredicate>
    constexpr static inline void apply(UnaryPredicate& predicate)
    {
        for_each_index_impl2<N1>::template apply<Offset>(predicate);
        for_each_index_impl2<N2>::template apply<Offset + N1>(predicate);
    }
};

template <>
struct for_each_index_impl2<3>
{
    template <std::size_t Offset, typename UnaryPredicate>
    constexpr static inline void apply(UnaryPredicate& predicate)
    {
        predicate(util::index_constant<Offset>());
        predicate(util::index_constant<Offset + 1>());
        predicate(util::index_constant<Offset + 2>());
    }
};

template <>
struct for_each_index_impl2<2>
{
    template <std::size_t Offset, typename UnaryPredicate>
    constexpr static inline void apply(UnaryPredicate& predicate)
    {
        predicate(util::index_constant<Offset>());
        predicate(util::index_constant<Offset + 1>());
    }
};

template <>
struct for_each_index_impl2<1>
{
    template <std::size_t Offset, typename UnaryPredicate>
    constexpr static inline void apply(UnaryPredicate& predicate)
    {
        predicate(util::index_constant<Offset>());
    }
};

template <>
struct for_each_index_impl2<0>
{
    template <std::size_t Offset, typename UnaryPredicate>
    constexpr static inline void apply(UnaryPredicate& )
    {}
};

// C++17 fold expression

#ifndef BOOST_NO_CXX17_FOLD_EXPRESSIONS

template <typename UnaryPredicate, std::size_t ...Is>
constexpr inline UnaryPredicate for_each_dimension_impl(UnaryPredicate& predicate,
                                                        std::index_sequence<Is...>)
{
    (predicate(util::index_constant<Is>()), ...);
    return predicate;
}

#endif

// Interface

template <std::size_t N, typename UnaryPredicate>
constexpr inline UnaryPredicate for_each_index(UnaryPredicate predicate)
{
    for_each_index_impl
        <
            0, N
        >::apply(predicate);
    return predicate;
}

template <typename Geometry, typename UnaryPredicate>
constexpr inline UnaryPredicate for_each_dimension(UnaryPredicate predicate)
{
//#ifndef BOOST_NO_CXX17_FOLD_EXPRESSIONS
//    for_each_dimension_impl(predicate,
//        std::make_index_sequence<geometry::dimension<Geometry>::value>());
//#else
    for_each_index_impl2
        <
            geometry::dimension<Geometry>::value
        >::template apply<0>(predicate);
//#endif
    return predicate;
}

// ----------------------------------------------------------------------------

// O(logN) version 2

template <std::size_t N>
struct all_indexes_of_impl2
{
    static const std::size_t N1 = N / 2;
    static const std::size_t N2 = N - N1;

    template <std::size_t Offset, typename UnaryPredicate>
    constexpr static inline bool apply(UnaryPredicate& predicate)
    {
        return all_indexes_of_impl2<N1>::template apply<Offset>(predicate)
            && all_indexes_of_impl2<N2>::template apply<Offset + N1>(predicate);
    }
};

template <>
struct all_indexes_of_impl2<3>
{
    template <std::size_t Offset, typename UnaryPredicate>
    constexpr static inline bool apply(UnaryPredicate& predicate)
    {
        return predicate(util::index_constant<Offset>())
            && predicate(util::index_constant<Offset + 1>())
            && predicate(util::index_constant<Offset + 2>());
    }
};

template <>
struct all_indexes_of_impl2<2>
{
    template <std::size_t Offset, typename UnaryPredicate>
    constexpr static inline bool apply(UnaryPredicate& predicate)
    {
        return predicate(util::index_constant<Offset>())
            && predicate(util::index_constant<Offset + 1>());
    }
};

template <>
struct all_indexes_of_impl2<1>
{
    template <std::size_t Offset, typename UnaryPredicate>
    constexpr static inline bool apply(UnaryPredicate& predicate)
    {
        return predicate(util::index_constant<Offset>());
    }
};

template <>
struct all_indexes_of_impl2<0>
{
    template <std::size_t Offset, typename UnaryPredicate>
    constexpr static inline bool apply(UnaryPredicate& )
    {
        return true;
    }
};

// Interface

template <std::size_t N, typename UnaryPredicate>
constexpr inline bool all_indexes_of(UnaryPredicate predicate)
{
    return all_indexes_of_impl2<N>::template apply<0>(predicate);
}

template <typename Geometry, typename UnaryPredicate>
constexpr inline bool all_dimensions_of(UnaryPredicate predicate)
{
    return all_indexes_of_impl2
        <
            geometry::dimension<Geometry>::value
        >::template apply<0>(predicate);
}

// ----------------------------------------------------------------------------

// O(logN) version 2

template <std::size_t N>
struct any_index_of_impl2
{
    static const std::size_t N1 = N / 2;
    static const std::size_t N2 = N - N1;

    template <std::size_t Offset, typename UnaryPredicate>
    constexpr static inline bool apply(UnaryPredicate& predicate)
    {
        return any_index_of_impl2<N1>::template apply<Offset>(predicate)
            || any_index_of_impl2<N2>::template apply<Offset + N1>(predicate);
    }
};

template <>
struct any_index_of_impl2<3>
{
    template <std::size_t Offset, typename UnaryPredicate>
    constexpr static inline bool apply(UnaryPredicate& predicate)
    {
        return predicate(util::index_constant<Offset>())
            || predicate(util::index_constant<Offset + 1>())
            || predicate(util::index_constant<Offset + 2>());
    }
};

template <>
struct any_index_of_impl2<2>
{
    template <std::size_t Offset, typename UnaryPredicate>
    constexpr static inline bool apply(UnaryPredicate& predicate)
    {
        return predicate(util::index_constant<Offset>())
            || predicate(util::index_constant<Offset + 1>());
    }
};

template <>
struct any_index_of_impl2<1>
{
    template <std::size_t Offset, typename UnaryPredicate>
    constexpr static inline bool apply(UnaryPredicate& predicate)
    {
        return predicate(util::index_constant<Offset>());
    }
};

template <>
struct any_index_of_impl2<0>
{
    template <std::size_t Offset, typename UnaryPredicate>
    constexpr static inline bool apply(UnaryPredicate& )
    {
        return false;
    }
};

// Interface

template <std::size_t N, typename UnaryPredicate>
constexpr inline bool any_index_of(UnaryPredicate predicate)
{
    return any_index_of_impl2<N>::template apply<0>(predicate);
}

template <typename Geometry, typename UnaryPredicate>
constexpr inline bool any_dimension_of(UnaryPredicate predicate)
{
    return any_index_of_impl2
        <
            geometry::dimension<Geometry>::value
        >::template apply<0>(predicate);
}

template <std::size_t N, typename UnaryPredicate>
constexpr inline bool none_index_of(UnaryPredicate predicate)
{
    return ! any_index_of_impl2<N>::template apply<0>(predicate);
}

template <typename Geometry, typename UnaryPredicate>
constexpr inline bool none_dimension_of(UnaryPredicate predicate)
{
    return ! any_index_of_impl2
        <
            geometry::dimension<Geometry>::value
        >::template apply<0>(predicate);
}

// ----------------------------------------------------------------------------


} // namespace detail
#endif // DOXYGEN_NO_DETAIL

}} // namespace boost::geometry

#endif // BOOST_GEOMETRY_UTIL_FOR_EACH_DIMENSION_HPP
