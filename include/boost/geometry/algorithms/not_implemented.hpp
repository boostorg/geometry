// Boost.Geometry (aka GGL, Generic Geometry Library)

// Copyright (c) 2008-2011 Bruno Lalande, Paris, France.
// Copyright (c) 2007-2011 Barend Gehrels, Amsterdam, the Netherlands.
// Copyright (c) 2009-2011 Mateusz Loskot, London, UK.

// Parts of Boost.Geometry are redesigned from Geodan's Geographic Library
// (geolib/GGL), copyright (c) 1995-2010 Geodan, Amsterdam, the Netherlands.

// Use, modification and distribution is subject to the Boost Software License,
// Version 1.0. (See accompanying file LICENSE_1_0.txt or copy at
// http://www.boost.org/LICENSE_1_0.txt)


#ifndef BOOST_GEOMETRY_ALGORITHMS_NOT_IMPLEMENTED_HPP
#define BOOST_GEOMETRY_ALGORITHMS_NOT_IMPLEMENTED_HPP


#include <boost/mpl/assert.hpp>
#include <boost/mpl/fold.hpp>
#include <boost/mpl/vector.hpp>
#include <boost/mpl/remove.hpp>
#include <boost/mpl/apply_wrap.hpp>


namespace boost { namespace geometry
{


// Class containing the elements of the actual user-facing error message

template <class>
struct FOR_GEOMETRY_TYPE
{
    template <class>
    struct AND_GEOMETRY_TYPE
    {
        template <size_t>
        struct IN_DIMENSION
        {};
    };

    template <size_t>
    struct IN_DIMENSION
    {};
};


// Unary metafunction class templates which return their corresponding error
// message element nested in the previous one.

template <class G>
struct for_geometry
{
    template <class>
    struct apply
    {
        typedef FOR_GEOMETRY_TYPE<G> type;
    };
};

template <class G>
struct and_geometry
{
    template <class Prev>
    struct apply
    {
        typedef typename Prev::template AND_GEOMETRY_TYPE<G> type;
    };
};

template <size_t D>
struct in_dimension
{
    template <class Prev>
    struct apply
    {
        typedef typename Prev::template IN_DIMENSION<D> type;
    };
};


#ifndef BOOST_GEOMETRY_IMPLEMENTATION_STATUS_BUILD
# define BOOST_GEOMETRY_IMPLEMENTATION_STATUS_BUILD false
#endif


struct not_implemented_base {};


template <
    typename Word1 = void,
    typename Word2 = void,
    typename Word3 = void
>
struct not_implemented: not_implemented_base
{
    BOOST_MPL_ASSERT_MSG
        (
            BOOST_GEOMETRY_IMPLEMENTATION_STATUS_BUILD,
            THIS_OPERATION_IS_NOT_OR_NOT_YET_IMPLEMENTED,
            (
                typename boost::mpl::fold<
                    typename boost::mpl::remove<
                        boost::mpl::vector<Word1, Word2, Word3>,
                        void
                    >::type,
                    void,
                    boost::mpl::apply_wrap1<boost::mpl::_2, boost::mpl::_1>
                >::type
            )
        );
};


}} // namespace boost::geometry


#endif // BOOST_GEOMETRY_ALGORITHMS_NOT_IMPLEMENTED_HPP
