// Boost.Geometry (aka GGL, Generic Geometry Library)

// Copyright (c) 2014, Oracle and/or its affiliates.

// Contributed and/or modified by Menelaos Karavelas, on behalf of Oracle

// Licensed under the Boost Software License version 1.0.
// http://www.boost.org/users/license.html

#ifndef BOOST_GEOMETRY_ALGORITHMS_DETAIL_CHECK_ITERATOR_RANGE_HPP
#define BOOST_GEOMETRY_ALGORITHMS_DETAIL_CHECK_ITERATOR_RANGE_HPP

#include <boost/core/ignore_unused.hpp>
#include <boost/geometry/strategies/intersection_result.hpp>

namespace boost { namespace geometry
{

#ifndef DOXYGEN_NO_DETAIL
namespace detail
{

// Check whether (each element of) an iterator range satisfies a given
// predicate.
// The predicate must be implemented as having a static apply unary
// method that returns a bool.
// By default an empty range is accepted
template <typename Predicate, bool AllowEmptyRange = true>
struct check_iterator_range
{
    template <typename InputIterator>
    static inline bool apply(InputIterator first, InputIterator beyond)
    {
        for (InputIterator it = first; it != beyond; ++it)
        {
            if (! Predicate::apply(*it))
            {
                return false;
            }
        }
        return AllowEmptyRange || first != beyond;
    }


    // version where we can pass a predicate object
    template <typename InputIterator>
    static inline bool apply(InputIterator first,
                             InputIterator beyond,
                             Predicate const& predicate)
    {
        // in case predicate's apply method is static, MSVC will
        // complain that predicate is not used
        boost::ignore_unused(predicate);

        for (InputIterator it = first; it != beyond; ++it)
        {
            if (! predicate.apply(*it))
            {
                return false;
            }
        }

        return AllowEmptyRange || first != beyond;
    }
};

// As check_iterator_range but the index of the element that
// satisfies the predicate is returned otherwise -1 is returned
template <typename Predicate>
struct check_iterator_range_with_index
{

    // version where we can pass a predicate object
    template <typename InputIterator>
    static inline int apply(InputIterator first,
                             InputIterator beyond,
                             Predicate const& predicate)
    {
        // in case predicate's apply method is static, MSVC will
        // complain that predicate is not used
        boost::ignore_unused(predicate);

        for (InputIterator it = first; it != beyond; ++it)
        {
            if (! predicate.apply(*it))
            {
                return it - first;
            }
        }

        return -1;
    }
};


// As check_iterator_range but the index of the element that
// satisfies the predicate is returned otherwise -1 is returned
template <typename Geometry, typename Construction>
struct check_iterator_range_with_info
{
    typedef typename point_type<Geometry>::type point_type;

    typedef geometry::segment_intersection_points<point_type>
    intersection_return_type;

    // version where we can pass a predicate object
    template <typename InputIterator>
    static inline intersection_return_type
    apply(InputIterator first,
          InputIterator beyond,
          Construction const& construction)
    {
        // in case predicate's apply method is static, MSVC will
        // complain that predicate is not used
        boost::ignore_unused(construction);

        for (InputIterator it = first; it != beyond; ++it)
        {
            auto res = construction.apply(*it);
            if (res.count != 0)
            {
                return res;
            }
        }

        return intersection_return_type();
    }
};

} // namespace detail
#endif // DOXYGEN_NO_DETAIL

}} // namespace boost::geometry

#endif // BOOST_GEOMETRY_ALGORITHMS_DETAIL_CHECK_ITERATOR_RANGE_HPP
