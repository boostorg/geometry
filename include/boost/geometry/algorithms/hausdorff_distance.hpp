// Boost.Geometry

// Copyright (c) 2018 Yaghyavardhan Singh Khangarot, Hyderabad, India.

// Contributed and/or modified by Adeel Ahmad, as part of Google Summer of Code 2018 program.

// Use, modification and distribution is subject to the Boost Software License,
// Version 1.0. (See accompanying file LICENSE_1_0.txt or copy at
// http://www.boost.org/LICENSE_1_0.txt)

#include <algorithm> 

#ifdef BOOST_GEOMETRY_DEBUG_FRECHET_DISTANCE
#include <iostream>
#endif

#include <iterator>
#include <utility>
#include <vector>
#include <limits>

#include <boost/geometry/geometry.hpp>
#include <boost/geometry/geometries/linestring.hpp>
#include <boost/geometry/geometries/point_xy.hpp>
#include <boost/geometry/geometries/polygon.hpp>
#include <boost/range.hpp>
#include <boost/mpl/assert.hpp>
#include <boost/geometry/core/tag.hpp>


namespace boost { namespace geometry
{

#ifndef DOXYGEN_NO_DETAIL
namespace detail { namespace hausdorff_distance
{

struct linestring_linestring
{
    template <typename Linestring1, typename Linestring2, typename Strategy>
    static inline typename distance_result
        <
            typename point_type<Linestring1>::type,
            typename point_type<Linestring2>::type,
            Strategy
        >::type apply(Linestring1 const& ls1, Linestring2 const& ls2, Strategy const& strategy)
    {
        
        typedef typename distance_result
            <
                typename point_type<Linestring1>::type,
                typename point_type<Linestring2>::type,
                Strategy
            >::type result_type;
        typedef typename boost::range_size<Linestring1>::type size_type;
        
        result_type dis_max=0,dis_min;
        boost::geometry::detail::throw_on_empty_input(ls1);
        boost::geometry::detail::throw_on_empty_input(ls2);
        size_type  a = boost::size(ls1);
        size_type  b = boost::size(ls2);

        //Computing the HausdorffDistance
        for(size_type i=0;i<a;i++)
        {
            bool is_dis_min_set = false;
                for(size_type j=0;j<b;j++)
                {
                    result_type dis_temp = geometry::distance(range::at(ls1, i), range::at(ls2, j), strategy);
                    if(dis_temp < dis_max)
                        break; //Early Break
                    if(!is_dis_min_set || dis_temp < dis_min)
                    {    
                        dis_min=dis_temp;
                        is_dis_min_set = true;
                    }
                }

            if (dis_min > dis_max && is_dis_min_set)
            {
              dis_max = dis_min;
            }
        }
        
        return dis_max;
    }
};

struct linestring_multi_linestring
{
    template <typename Linestring, typename Multi_linestring, typename Strategy>
    static inline typename distance_result
        <
            typename point_type<Linestring>::type,
            typename point_type<Multi_linestring>::type,
            Strategy
        >::type apply(Linestring const& ls, Multi_linestring const& mls, Strategy const& strategy)
    {
        
        typedef typename distance_result
            <
                typename point_type<Linestring>::type,
                typename point_type<Multi_linestring>::type,
                Strategy
            >::type result_type;
        typedef typename boost::range_size<Linestring>::type size_type1;
        typedef typename boost::range_size<Multi_linestring>::type size_type2;
        
        boost::geometry::detail::throw_on_empty_input(ls);
        boost::geometry::detail::throw_on_empty_input(mls);
        size_type1  a = boost::size(ls);
        size_type2  b = boost::size(mls);

        result_type haus_dis_max=0;
        //Computing the HausdorffDistance
        for(size_type1 i=0;i<a;i++)
        {
                result_type dis_max=0;
                for(size_type2 j=0;j<b;j++)
                {
                    result_type dis_min;
                    dis_max =linestring_linestring::apply(ls,range::at(mls,j),strategy);
                }

                if(dis_max > haus_dis_max)
                {
                    haus_dis_max=dis_max;
                }    
        }

        
        return haus_dis_max;
    }
};

}} // namespace detail::hausdorff_distance
#endif // DOXYGEN_NO_DETAIL

#ifndef DOXYGEN_NO_DISPATCH
namespace dispatch
{
template
<
    typename Geometry1,
    typename Geometry2,
    typename Tag1 = typename tag<Geometry1>::type,
    typename Tag2 = typename tag<Geometry2>::type
>
struct hausdorff_distance : not_implemented<Tag1, Tag2>
{};

// Specialization for linestrings 
template <typename Linestring1, typename Linestring2>
struct hausdorff_distance<Linestring1,Linestring2,linestring_tag,linestring_tag>
    : detail::hausdorff_distance::linestring_linestring
{};

// Specialization for linestring and multi_linestring 
template <typename linestring, typename multi_linestring>
struct hausdorff_distance<linestring,multi_linestring,linestring_tag,multi_linestring_tag>
    : detail::hausdorff_distance::linestring_multi_linestring
{};

} // namespace dispatch
#endif // DOXYGEN_NO_DISPATCH

// Algorithm overload using explicitly passed Pt-Pt distance strategy
template <typename Geometry1, typename Geometry2, typename Strategy>
inline typename distance_result
        <
            typename point_type<Geometry1>::type,
            typename point_type<Geometry2>::type,
            Strategy
        >::type
hausdorff_distance(Geometry1 const& g1, Geometry2 const& g2, Strategy const& strategy)
{
    return dispatch::hausdorff_distance<Geometry1, Geometry2>::apply(g1, g2, strategy);
}

// Algorithm overload using default Pt-Pt distance strategy
template <typename Geometry1, typename Geometry2>
inline typename distance_result
        <
            typename point_type<Geometry1>::type,
            typename point_type<Geometry2>::type
        >::type
hausdorff_distance(Geometry1 const& g1, Geometry2 const& g2)
{
    typedef typename strategy::distance::services::default_strategy
              <
                  point_tag, point_tag,
                  typename point_type<Geometry1>::type,
                  typename point_type<Geometry2>::type
              >::type strategy_type;
    
    return hausdorff_distance(g1, g2, strategy_type());
}

}} // namespace boost::geometry
