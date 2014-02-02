// Boost.Geometry (aka GGL, Generic Geometry Library)

// Copyright (c) 2007-2012 Barend Gehrels, Amsterdam, the Netherlands.

// This file was modified by Oracle on 2013.
// Modifications copyright (c) 2013, Oracle and/or its affiliates.

// Use, modification and distribution is subject to the Boost Software License,
// Version 1.0. (See accompanying file LICENSE_1_0.txt or copy at
// http://www.boost.org/LICENSE_1_0.txt)

#ifndef BOOST_GEOMETRY_ALGORITHMS_DETAIL_RELATE_RESULT_HPP
#define BOOST_GEOMETRY_ALGORITHMS_DETAIL_RELATE_RESULT_HPP

namespace boost { namespace geometry {

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

    template <field F1, field F2, char V>
    void set()
    {
        array[F1 * 3 + F2] = V;
    }

    template <field F1, field F2, char D>
    void update()
    {
        BOOST_STATIC_ASSERT('0' <= D && D <= '9');
        char * c = array + F1 * 3 + F2;
        if ( D > *c || *c > '9')
            *c = D;
    }

    void set(char v)
    {
        ::memset(array, v, 9);
    }

    std::pair<const char*, const char*> get_code() const
    {
        return std::make_pair(array, array+9);
    }

    void transpose()
    {
        std::swap(array[1], array[3]);
        std::swap(array[2], array[6]);
        std::swap(array[5], array[7]);
    }

private:
    char array[9];
};

}} // namespace detail::relate
#endif // DOXYGEN_NO_DETAIL

}} // namespace boost::geometry

#endif // BOOST_GEOMETRY_ALGORITHMS_DETAIL_RELATE_RESULT_HPP
