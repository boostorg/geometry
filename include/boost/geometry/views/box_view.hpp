// Boost.Geometry (aka GGL, Generic Geometry Library)

// Copyright (c) 2007-2011 Barend Gehrels, Amsterdam, the Netherlands.
// Copyright (c) 2008-2011 Bruno Lalande, Paris, France.
// Copyright (c) 2009-2011 Mateusz Loskot, London, UK.

// Parts of Boost.Geometry are redesigned from Geodan's Geographic Library
// (geolib/GGL), copyright (c) 1995-2010 Geodan, Amsterdam, the Netherlands.

// Use, modification and distribution is subject to the Boost Software License,
// Version 1.0. (See accompanying file LICENSE_1_0.txt or copy at
// http://www.boost.org/LICENSE_1_0.txt)

#ifndef BOOST_GEOMETRY_VIEWS_BOX_VIEW_HPP
#define BOOST_GEOMETRY_VIEWS_BOX_VIEW_HPP


#include <boost/range.hpp>

#include <boost/geometry/core/point_type.hpp>
#include <boost/geometry/views/detail/points_view.hpp>
#include <boost/geometry/algorithms/assign.hpp>


namespace boost { namespace geometry
{


/*!
\brief Range, walking over the four points of a box
\tparam Box box type
\ingroup views
*/
template <typename Box, bool Clockwise = true>
struct box_view 
    : public detail::points_view
        <
            typename geometry::point_type<Box>::type, 
            5
        >
{
    typedef typename geometry::point_type<Box>::type point_type;
    
    explicit box_view(Box const& box)
        : detail::points_view<point_type, 5>(copy_policy(box))
    {}
    
private :    
    
    class copy_policy
    {
    public :
        inline copy_policy(Box const& box)
            : m_box(box)
        {}
        
        inline void apply(point_type* points) const
        {
            detail::assign_box_corners_oriented<!Clockwise>(m_box, points);
            points[4] = points[0];
        }
    private :
        Box m_box;
    };

};


#ifndef DOXYGEN_NO_TRAITS_SPECIALIZATIONS

// All views on boxes are handled as rings
namespace traits
{
    template<typename Box, bool Clockwise>
    struct tag<box_view<Box, Clockwise> >
    {
        typedef ring_tag type;
    };
}

#endif // DOXYGEN_NO_TRAITS_SPECIALIZATIONS


}} // namespace boost::geometry


#endif // BOOST_GEOMETRY_VIEWS_BOX_VIEW_HPP
