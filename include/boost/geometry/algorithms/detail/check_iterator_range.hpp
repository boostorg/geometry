// Boost.Geometry (aka GGL, Generic Geometry Library)

// Copyright (c) 2014, Oracle and/or its affiliates.

// Contributed and/or modified by Menelaos Karavelas, on behalf of Oracle

// Licensed under the Boost Software License version 1.0.
// http://www.boost.org/users/license.html

#ifndef BOOST_GEOMETRY_ALGORITHMS_DETAIL_CHECK_ITERATOR_RANGE_HPP
#define BOOST_GEOMETRY_ALGORITHMS_DETAIL_CHECK_ITERATOR_RANGE_HPP


namespace boost { namespace geometry
{

#ifndef DOXYGEN_NO_DETAIL
namespace detail
{

// Check whether (each element of) an iterator range satisfies a given
// predicate.
// The predicate must be implemented as having a static apply unary
// method that returns a bool.
template <typename Predicate, bool AllowEmptyMultiRange = false>
struct check_iterator_range
{
    template <typename InputIterator>
    static inline bool apply(InputIterator first, InputIterator beyond)
    {
        for (InputIterator it = first; it != beyond; ++it)
        {
            if ( !Predicate::apply(*it) )
            {
                return false;
            }
        }
        return AllowEmptyMultiRange || first != beyond;
    };
};

} // namespace detail
#endif // DOXYGEN_NO_DETAIL

}} // namespace boost::geometry

#endif // BOOST_GEOMETRY_ALGORITHMS_DETAIL_CHECK_ITERATOR_RANGE_HPP
