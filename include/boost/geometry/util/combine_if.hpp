// Boost.Geometry (aka GGL, Generic Geometry Library)

// Copyright (c) 2007-2014 Barend Gehrels, Amsterdam, the Netherlands.
// Copyright (c) 2008-2014 Bruno Lalande, Paris, France.
// Copyright (c) 2009-2014 Mateusz Loskot, London, UK.
// Copyright (c) 2013-2014 Adam Wulkiewicz, Lodz, Poland.
// Copyright (c) 2014 Samuel Debionne, Grenoble, France.

// Parts of Boost.Geometry are redesigned from Geodan's Geographic Library
// (geolib/GGL), copyright (c) 1995-2010 Geodan, Amsterdam, the Netherlands.

// Use, modification and distribution is subject to the Boost Software License,
// Version 1.0. (See accompanying file LICENSE_1_0.txt or copy at
// http://www.boost.org/LICENSE_1_0.txt)

#ifndef BOOST_GEOMETRY_UTIL_COMBINE_IF_HPP
#define BOOST_GEOMETRY_UTIL_COMBINE_IF_HPP

#include <boost/mpl/if.hpp>
#include <boost/type_traits.hpp>


namespace boost { namespace geometry
{

namespace util
{


/*!
    \brief Meta-function to generate all the combination of pairs of types
        from a given sequence Sequence except those that does not satisfy the
        predicate Pred
    \ingroup utility
*/
template <typename Sequence, typename Pred>
struct combine_if
{
    struct combine
    {
        template <typename Result, typename T>
        struct apply
        {
            typedef typename mpl::fold<Sequence, Result,
                mpl::if_<
                    typename mpl::apply<Pred, T, mpl::_2>::type,
                    mpl::insert<mpl::_1, mpl::pair<T, mpl::_2> >,
                    mpl::_1
                >
            >::type type;
        };
    };

    typedef typename mpl::fold<Sequence, mpl::set0<>, combine>::type type;
};


} // namespace util

}} // namespace boost::geometry

#endif // BOOST_GEOMETRY_UTIL_COMBINE_IF_HPP
