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


namespace boost { namespace geometry
{


#ifndef BOOST_GEOMETRY_IMPLEMENTATION_STATUS_BUILD
# define BOOST_GEOMETRY_IMPLEMENTATION_STATUS_BUILD false
#endif


template <typename Geometry1, typename Geometry2>
struct not_implemented
{
    BOOST_MPL_ASSERT_MSG
        (
            BOOST_GEOMETRY_IMPLEMENTATION_STATUS_BUILD,
            NOT_OR_NOT_YET_IMPLEMENTED_FOR_THIS_GEOMETRY_TYPE
            , (types<Geometry1, Geometry2>)
        );
};


}} // namespace boost::geometry


#endif // BOOST_GEOMETRY_ALGORITHMS_NOT_IMPLEMENTED_HPP
