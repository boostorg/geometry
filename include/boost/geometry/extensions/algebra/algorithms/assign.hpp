// Boost.Geometry (aka GGL, Generic Geometry Library)

// Copyright (c) 2007-2012 Barend Gehrels, Amsterdam, the Netherlands.
// Copyright (c) 2008-2012 Bruno Lalande, Paris, France.
// Copyright (c) 2009-2012 Mateusz Loskot, London, UK.
// Copyright (c) 2013 Adam Wulkiewicz, Lodz, Poland.

// Parts of Boost.Geometry are redesigned from Geodan's Geographic Library
// (geolib/GGL), copyright (c) 1995-2010 Geodan, Amsterdam, the Netherlands.

// Use, modification and distribution is subject to the Boost Software License,
// Version 1.0. (See accompanying file LICENSE_1_0.txt or copy at
// http://www.boost.org/LICENSE_1_0.txt)

#ifndef BOOST_GEOMETRY_EXTENSIONS_ALGEBRA_ALGORITHMS_ASSIGN_HPP
#define BOOST_GEOMETRY_EXTENSIONS_ALGEBRA_ALGORITHMS_ASSIGN_HPP

#include <boost/geometry/algorithms/assign.hpp>

#include <boost/geometry/extensions/algebra/core/tags.hpp>

#include <boost/geometry/extensions/algebra/algorithms/detail.hpp>

namespace boost { namespace geometry
{

#ifndef DOXYGEN_NO_DISPATCH
namespace dispatch
{

template <typename Vector>
struct assign_zero<vector_tag, Vector>
{
    static inline void apply(Vector & g)
    {
        detail::algebra::assign_value<
            Vector,
            typename coordinate_type<Vector>::type,
            0, dimension<Vector>::type::value
        >::apply(g, 0);
    }
};

template <typename R>
struct assign_zero<rotation_quaternion_tag, R>
{
    static inline void apply(R & g)
    {
        detail::algebra::assign_value<
            R, typename coordinate_type<R>::type,
            0, 4
        >::apply(g, 0);
    }
};

template <typename R>
struct assign_zero<rotation_matrix_tag, R>
{
    static inline void apply(R & g)
    {
        detail::algebra::indexed_assign_value<
            R, typename coordinate_type<R>::type,
            0, 0, dimension<R>::type::value, dimension<R>::type::value
        >::apply(g, 0);
    }
};

template <typename V>
struct assign<vector_tag, V, 2>
{
    typedef typename coordinate_type<V>::type coordinate_type;

    template <typename T>
    static inline void apply(V& v, T const& c1, T const& c2)
    {
        set<0>(v, boost::numeric_cast<coordinate_type>(c1));
        set<1>(v, boost::numeric_cast<coordinate_type>(c2));
    }
};

template <typename V>
struct assign<vector_tag, V, 3>
{
    typedef typename coordinate_type<V>::type coordinate_type;

    template <typename T>
    static inline void apply(V& v, T const& c1, T const& c2, T const& c3)
    {
        set<0>(v, boost::numeric_cast<coordinate_type>(c1));
        set<1>(v, boost::numeric_cast<coordinate_type>(c2));
        set<2>(v, boost::numeric_cast<coordinate_type>(c3));
    }
};


} // namespace dispatch
#endif // DOXYGEN_NO_DISPATCH


}} // namespace boost::geometry

#endif // BOOST_GEOMETRY_EXTENSIONS_ALGEBRA_ALGORITHMS_ASSIGN_HPP
