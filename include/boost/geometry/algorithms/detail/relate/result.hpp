// Boost.Geometry (aka GGL, Generic Geometry Library)

// Copyright (c) 2007-2012 Barend Gehrels, Amsterdam, the Netherlands.

// This file was modified by Oracle on 2013.
// Modifications copyright (c) 2013, Oracle and/or its affiliates.

// Use, modification and distribution is subject to the Boost Software License,
// Version 1.0. (See accompanying file LICENSE_1_0.txt or copy at
// http://www.boost.org/LICENSE_1_0.txt)

#ifndef BOOST_GEOMETRY_ALGORITHMS_DETAIL_RELATE_RESULT_HPP
#define BOOST_GEOMETRY_ALGORITHMS_DETAIL_RELATE_RESULT_HPP

namespace boost { namespace geometry
{

#ifndef DOXYGEN_NO_DETAIL
namespace detail { namespace relate {

enum field { interior = 0, boundary = 1, exterior = 2 };

// With DE9IM only Dimension < 10 is supported
class result
{
public:
    result()
    {
        set('F');
    }

    result(const char * str)
    {
        ::memcpy(array, str, 9);
    }

    template <field F1, field F2>
    char get() const
    {
        return array[F1 * 3 + F2];
    }

    template <field F1, field F2>
    void set(char v)
    {
        array[F1 * 3 + F2] = v;
    }

    template <field F1, field F2>
    void update_dimension(char v)
    {
        char * c = array + F1 * 3 + F2;
        if ( v > *c || *c > '9')
            *c = v;
    }

    template <field F1, field F2>
    void update(char v)
    {
        char * c = array + F1 * 3 + F2;
        if ( *c < '0' || '9' < *c)
            *c = v;
    }

    void set(char v)
    {
        ::memset(array, v, 9);
    }

private:
    char array[9];
};

}} // namespace detail::relate
#endif // DOXYGEN_NO_DETAIL

}} // namespace boost::geometry

#endif // BOOST_GEOMETRY_ALGORITHMS_DETAIL_RELATE_RESULT_HPP
