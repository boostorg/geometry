// Boost.Geometry (aka GGL, Generic Geometry Library)

// Copyright (c) 2015, Oracle and/or its affiliates.

// Contributed and/or modified by Adam Wulkiewicz, on behalf of Oracle

// Use, modification and distribution is subject to the Boost Software License,
// Version 1.0. (See accompanying file LICENSE_1_0.txt or copy at
// http://www.boost.org/LICENSE_1_0.txt)

#ifndef BOOST_GEOMETRY_ALGORITHMS_IS_NOT_IMPLEMENTED_HPP
#define BOOST_GEOMETRY_ALGORITHMS_IS_NOT_IMPLEMENTED_HPP


#include <boost/mpl/begin.hpp>
#include <boost/mpl/bool.hpp>
#include <boost/mpl/deref.hpp>
#include <boost/mpl/end.hpp>
#include <boost/mpl/next.hpp>
#include <boost/variant/variant_fwd.hpp>

#include <boost/geometry/algorithms/not_implemented.hpp>


namespace boost { namespace geometry {

namespace detail { namespace is_not_implemented {

template
<
    typename Seq, 
    typename MetaPred, 
    typename PrevState,
    typename First = typename boost::mpl::begin<Seq>::type,
    typename Last = typename boost::mpl::end<Seq>::type
>
struct for_each
{
    typedef typename for_each
        <
            Seq,
            MetaPred,
            typename MetaPred::template apply
                <
                    typename boost::mpl::deref<First>::type,
                    PrevState
                >,
            typename boost::mpl::next<First>::type,
            Last
        >::type type;
};

template
<
    typename Seq, 
    typename MetaPred,
    typename PrevState,
    typename Last
>
struct for_each<Seq, MetaPred, PrevState, Last, Last>
{
    typedef PrevState type;
};

template <typename MetaPolicy>
struct reverse
{
    template <typename Geometry1, typename Geometry2>
    struct apply
        : MetaPolicy::template apply<Geometry2, Geometry1>
    {};
};

template <typename Geometry, typename MetaPolicy>
struct metapred
{
    template <typename SeqEl, typename PrevState>
    struct apply
        : boost::mpl::bool_
            <
                PrevState::value &&
                MetaPolicy::template apply<Geometry, SeqEl>::value
            >
    {};
};

template <typename Seq2, typename MetaPolicy>
struct metapred_2
{
    template <typename Seq1El, typename PrevState>
    struct apply
        : boost::mpl::bool_
            <
                PrevState::value &&
                for_each
                    <
                        Seq2,
                        metapred<Seq1El, MetaPolicy>,
                        boost::mpl::bool_<true>
                    >::type::value
            >
    {};
};

}} // namespace detail::is_not_implemented


/*!
 A utility for checking if an algorithm is not implemented for some Geometries.
 For variants it checks all combinations of Geometries and sets value to false
 only if all combinations are not implemented.
 */
template <typename Geometry1, typename Geometry2, typename MetaPolicy>
struct is_not_implemented
    : MetaPolicy::template apply<Geometry1, Geometry2>
{};

template
<
    typename Geometry1,
    BOOST_VARIANT_ENUM_PARAMS(typename T),
    typename MetaPolicy
>
struct is_not_implemented
    <
        Geometry1,
        boost::variant<BOOST_VARIANT_ENUM_PARAMS(T)>,
        MetaPolicy
    >
    : detail::is_not_implemented::for_each
        <
            typename boost::variant<BOOST_VARIANT_ENUM_PARAMS(T)>::types,
            detail::is_not_implemented::metapred<Geometry1, MetaPolicy>,
            boost::mpl::bool_<true>
        >::type
{};

template
<
    BOOST_VARIANT_ENUM_PARAMS(typename T),
    typename Geometry2,
    typename MetaPolicy
>
struct is_not_implemented
    <
        boost::variant<BOOST_VARIANT_ENUM_PARAMS(T)>,
        Geometry2,
        MetaPolicy
    >
    : detail::is_not_implemented::for_each
        <
            typename boost::variant<BOOST_VARIANT_ENUM_PARAMS(T)>::types,
            detail::is_not_implemented::metapred
                <
                    Geometry2,
                    detail::is_not_implemented::reverse<MetaPolicy>
                >,
            boost::mpl::bool_<true>
        >::type
{};

template
<
    BOOST_VARIANT_ENUM_PARAMS(typename T1),
    BOOST_VARIANT_ENUM_PARAMS(typename T2),
    typename MetaPolicy
>
struct is_not_implemented
    <
        boost::variant<BOOST_VARIANT_ENUM_PARAMS(T1)>,
        boost::variant<BOOST_VARIANT_ENUM_PARAMS(T2)>,
        MetaPolicy
    >
    : detail::is_not_implemented::for_each
        <
            typename boost::variant<BOOST_VARIANT_ENUM_PARAMS(T1)>::types,
            detail::is_not_implemented::metapred_2
                <
                    typename boost::variant<BOOST_VARIANT_ENUM_PARAMS(T2)>::types,
                    MetaPolicy
                >,
            boost::mpl::bool_<true>
        >::type
{
};

}} // namespace boost::geometry


#endif // BOOST_GEOMETRY_ALGORITHMS_IS_NOT_IMPLEMENTED_HPP
