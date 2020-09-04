// Boost.Geometry

// Copyright (c) 2017 Adam Wulkiewicz, Lodz, Poland.

// This file was modified by Oracle on 2020.
// Modifications copyright (c) 2020 Oracle and/or its affiliates.
// Contributed and/or modified by Adam Wulkiewicz, on behalf of Oracle

// Use, modification and distribution is subject to the Boost Software License,
// Version 1.0. (See accompanying file LICENSE_1_0.txt or copy at
// http://www.boost.org/LICENSE_1_0.txt)

#ifndef BOOST_GEOMETRY_UTIL_SELECT_PACK_ELEMENT
#define BOOST_GEOMETRY_UTIL_SELECT_PACK_ELEMENT


#include <type_traits>


namespace boost { namespace geometry
{

namespace detail
{


template
<
    template <typename, typename> class LessPred,
    typename ...Ts
>
struct select_pack_element;

template
<
    template <typename, typename> class LessPred,
    typename T
>
struct select_pack_element<LessPred, T>
{
    typedef T type;
};

template
<
    template <typename, typename> class LessPred,
    typename T1, typename T2
>
struct select_pack_element<LessPred, T1, T2>
{
    typedef std::conditional_t<LessPred<T1, T2>::value, T1, T2> type;
};

template
<
    template <typename, typename> class LessPred,
    typename T1, typename T2, typename ...Ts
>
struct select_pack_element<LessPred, T1, T2, Ts...>
{
    typedef typename select_pack_element
        <
            LessPred,
            typename select_pack_element<LessPred, T1, T2>::type,
            typename select_pack_element<LessPred, Ts...>::type
        >::type type;
};


}


}} // namespace boost::geometry


#endif // BOOST_GEOMETRY_UTIL_SELECT_PACK_ELEMENT
