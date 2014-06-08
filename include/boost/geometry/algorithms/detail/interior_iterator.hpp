// Boost.Geometry

// Copyright (c) 2014 Adam Wulkiewicz, Lodz, Poland.

// Use, modification and distribution is subject to the Boost Software License,
// Version 1.0. (See accompanying file LICENSE_1_0.txt or copy at
// http://www.boost.org/LICENSE_1_0.txt)

#ifndef BOOST_GEOMETRY_ALGORITHMS_DETAIL_INTERIOR_ITERATOR_HPP
#define BOOST_GEOMETRY_ALGORITHMS_DETAIL_INTERIOR_ITERATOR_HPP

#include <boost/mpl/if.hpp>
#include <boost/range/iterator.hpp>
#include <boost/range/value_type.hpp>
#include <boost/type_traits/is_const.hpp>

#include <boost/geometry/core/interior_type.hpp>

namespace boost { namespace geometry
{

#ifndef DOXYGEN_NO_DETAIL
namespace detail
{

/*!
\brief Structure defining the type of interior rings iterator
\note If the Geometry is const, const iterator is defined.
\tparam Geometry \tparam_geometry
 */
template <typename Geometry>
struct interior_iterator
{
    typedef typename boost::range_iterator
        <
            typename geometry::interior_type<Geometry>::type        
        >::type type;
};

template <typename BaseT, typename T>
struct const_conformant
{
    typedef typename boost::mpl::if_
        <
            boost::is_const<BaseT>,
            T const,
            T
        >::type type;
};

template <typename Geometry>
struct interior_ring_iterator
{
    typedef typename boost::range_iterator
        <
            typename const_conformant
                <
                    typename geometry::interior_type<Geometry>::type,
                    typename boost::range_value
                        <
                            typename geometry::interior_type<Geometry>::type
                        >::type
                >::type
        >::type type;
};

} // namespace detail
#endif // DOXYGEN_NO_DETAIL

}} // namespace boost::geometry

#endif // BOOST_GEOMETRY_ALGORITHMS_DETAIL_INTERIOR_ITERATOR_HPP
