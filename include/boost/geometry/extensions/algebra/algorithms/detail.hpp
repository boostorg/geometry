// Boost.Geometry (aka GGL, Generic Geometry Library)

// Copyright (c) 2013 Adam Wulkiewicz, Lodz, Poland.

// Use, modification and distribution is subject to the Boost Software License,
// Version 1.0. (See accompanying file LICENSE_1_0.txt or copy at
// http://www.boost.org/LICENSE_1_0.txt)

#ifndef BOOST_GEOMETRY_EXTENSIONS_ALGEBRA_ALGORITHMS_DETAIL_HPP
#define BOOST_GEOMETRY_EXTENSIONS_ALGEBRA_ALGORITHMS_DETAIL_HPP

namespace boost { namespace geometry
{

namespace detail { namespace algebra {

// Cross 3D of 3 components of Vectors/Quaternion starting from IS1 and IS2 and placing the result starting from D
template <std::size_t IS1, std::size_t IS2, std::size_t ID, typename S1, typename S2, typename D>
inline void cross(S1 const& s1, S2 const& s2, D & d)
{
    set<ID+0>(d, get<IS1+1>(s1)*get<IS2+2>(s2) - get<IS1+2>(s1)*get<IS2+1>(s2));
    set<ID+1>(d, get<IS1+2>(s1)*get<IS2+0>(s2) - get<IS1+0>(s1)*get<IS2+2>(s2));
    set<ID+2>(d, get<IS1+0>(s1)*get<IS2+1>(s2) - get<IS1+1>(s1)*get<IS2+0>(s2));
}

// Dot of N components of Vectors/Quaternion starting from IS1 and IS2

template <typename S1, typename S2, std::size_t IS1, std::size_t IS2, std::size_t N>
struct dot_impl
{
    BOOST_STATIC_ASSERT(0 < N);

    typedef typename geometry::select_most_precise<
        typename traits::coordinate_type<S1>::type,
        typename traits::coordinate_type<S2>::type
    >::type coordinate_type;

    static inline coordinate_type apply(S1 const& s1, S2 const& s2)
    {
        return get<IS1>(s1)*get<IS2>(s2) + dot_impl<S1, S2, IS1+1, IS2+1, N-1>::apply(s1, s2);
    }
};

template <typename S1, typename S2, std::size_t IS1, std::size_t IS2>
struct dot_impl<S1, S2, IS1, IS2, 1>
{
    typedef typename geometry::select_most_precise<
        typename traits::coordinate_type<S1>::type,
        typename traits::coordinate_type<S2>::type
    >::type coordinate_type;

    static inline coordinate_type apply(S1 const& s1, S2 const& s2)
    {
        return get<IS1>(s1)*get<IS2>(s2);
    }
};

template <std::size_t IS1, std::size_t IS2, std::size_t N, typename S1, typename S2>
inline static
typename geometry::select_most_precise<
    typename traits::coordinate_type<S1>::type,
    typename traits::coordinate_type<S2>::type
>::type
dot(S1 const& s1, S2 const& s2)
{
    return dot_impl<S1, S2, IS1, IS2, N>::apply(s1, s2);
}

// Multiplication of N components starting from Ith by Value v

template <typename S, typename T, std::size_t IS, std::size_t I, std::size_t N>
struct mul_impl
{
    BOOST_STATIC_ASSERT(0 < N);

    static inline void apply(S & s, T const& v)
    {
        set<IS>(s, get<IS>(s) * v);
        mul_impl<S, T, IS+1, I+1, N>::apply(s, v);
    }
};

template <typename S, typename T, std::size_t IS, std::size_t N>
struct mul_impl<S, T, IS, N, N>
{
    static inline void apply(S &, T const&) {}
};

template <std::size_t IS, std::size_t N, typename S, typename T>
inline static void mul(S & s, T const& v)
{
    return mul_impl<S, T, IS, 0, N>::apply(s, v);
}

// Normalization of N components starting from Ith

template <std::size_t I, std::size_t N, typename S>
inline static void normalize(S & s)
{
    typedef typename traits::coordinate_type<S>::type T;

    T lsqr = dot<I, I, N>(s, s);
    if ( std::numeric_limits<T>::epsilon() < lsqr )
        mul<I, N>(s, 1.0f / ::sqrt(lsqr));    
}

// Square matrix * Vector of the same dimension 

template <typename M, typename V, typename VD, std::size_t I, std::size_t N>
struct matrix_mul_row_impl
{
    BOOST_STATIC_ASSERT(0 < N);

    static const std::size_t dimension = traits::dimension<M>::value;

    static inline
    typename traits::coordinate_type<VD>::type
    apply(M const& m, V const& v)
    {
        return matrix_mul_row_impl<M, V, VD, I, N-1>::apply(m, v) + get<I, N-1>(m) * get<N-1>(v);
    }
};

template <typename M, typename V, typename VD, std::size_t I>
struct matrix_mul_row_impl<M, V, VD, I, 1>
{
    static const std::size_t dimension = traits::dimension<M>::value;

    static inline
    typename traits::coordinate_type<VD>::type
    apply(M const& m, V const& v)
    {
        return get<I, 0>(m) * get<0>(v);
    }
};

template <typename M, typename V, typename VD, std::size_t I, std::size_t N>
struct matrix_mul_impl
{
    static inline void apply(M const& m, V const& v, VD & vd)
    {
        set<I>(vd, matrix_mul_row_impl<M, V, VD, I, N>::apply(m, v));
        matrix_mul_impl<M, V, VD, I+1, N>::apply(m, v, vd);
    }
};

template <typename M, typename V, typename VD, std::size_t N>
struct matrix_mul_impl<M, V, VD, N, N>
{
    static inline void apply(M const&, V const&, VD &) {}
};

template <typename M, typename V, typename VD>
inline static void matrix_rotate(M const& m, V const& v, VD & vd)
{
    static const std::size_t dimension = traits::dimension<M>::value;

    matrix_mul_impl<M, V, VD, 0, dimension>::apply(m, v, vd);
}

template <typename V, typename Q>
inline static void quaternion_rotate(V & v, Q const& q)
{
    // TODO - choose more precise type?

    typedef typename select_most_precise<
        typename traits::coordinate_type<V>::type,
        typename traits::coordinate_type<Q>::type
    >::type T;

    // Hamilton product T=Q*V
    T a = /*get<0>(r) * 0 */- get<1>(r) * get<0>(v) - get<2>(r) * get<1>(v) - get<3>(r) * get<2>(v);
    T b = get<0>(r) * get<0>(v)/* + get<1>(r) * 0*/ + get<2>(r) * get<2>(v) - get<3>(r) * get<1>(v);
    T c = get<0>(r) * get<1>(v) - get<1>(r) * get<2>(v)/* + get<2>(r) * 0*/ + get<3>(r) * get<0>(v);
    T d = get<0>(r) * get<2>(v) + get<1>(r) * get<1>(v) - get<2>(r) * get<0>(v)/* + get<3>(r) * 0*/;
    // Hamilton product V=T*inv(Q)
    set<0>(v, - a * get<1>(r) + b * get<0>(r) - c * get<3>(r) + d * get<2>(r));
    set<1>(v, - a * get<2>(r) + b * get<3>(r) + c * get<0>(r) - d * get<1>(r));
    set<2>(v, - a * get<3>(r) - b * get<2>(r) + c * get<1>(r) + d * get<0>(r));
}

}} // namespace detail::algebra

}} // namespace boost::geometry

#endif // BOOST_GEOMETRY_EXTENSIONS_ALGEBRA_ALGORITHMS_DETAIL_HPP
