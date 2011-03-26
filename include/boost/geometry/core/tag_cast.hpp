// Boost.Geometry (aka GGL, Generic Geometry Library)

// Copyright (c) 2007-2011 Barend Gehrels, Amsterdam, the Netherlands.
// Copyright (c) 2008-2011 Bruno Lalande, Paris, France.
// Copyright (c) 2009-2011 Mateusz Loskot, London, UK.

// Parts of Boost.Geometry are redesigned from Geodan's Geographic Library
// (geolib/GGL), copyright (c) 1995-2010 Geodan, Amsterdam, the Netherlands.

// Use, modification and distribution is subject to the Boost Software License,
// Version 1.0. (See accompanying file LICENSE_1_0.txt or copy at
// http://www.boost.org/LICENSE_1_0.txt)

#ifndef BOOST_GEOMETRY_CORE_TAG_CAST_HPP
#define BOOST_GEOMETRY_CORE_TAG_CAST_HPP


#include <boost/mpl/if.hpp>
#include <boost/type_traits.hpp>

namespace boost { namespace geometry
{

/// Generic tag_cast utility
template
<
    typename Tag,
    typename BaseTag,
    typename BaseTag2 = void,
    typename BaseTag3 = void,
    typename BaseTag4 = void,
    typename BaseTag5 = void,
    typename BaseTag6 = void,
    typename BaseTag7 = void
>
struct tag_cast
{
    typedef typename boost::mpl::if_
        <
          typename boost::is_base_of<BaseTag, Tag>::type,
          BaseTag,
          // Try next one in line:
          typename tag_cast
            <
                Tag, BaseTag2, BaseTag3, BaseTag4,
                BaseTag5, BaseTag6, BaseTag7, void
            >::type
        >::type type;
};

// Specialization for last one
template <typename Tag>
struct tag_cast<Tag, void, void, void, void, void, void, void>
{
    // If not found, take specified tag, so do not cast
    typedef Tag type;
};


}} // namespace boost::geometry

#endif // BOOST_GEOMETRY_CORE_TAG_CAST_HPP
