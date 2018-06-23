// Boost.Geometry

// Copyright (c) 2018 Yaghyavardhan Singh Khangarot, Hyderabad, India.

// Contributed and/or modified by Yaghyavardhan Singh Khangarot, as part of Google Summer of Code 2018 program.

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

struct range_range
{
    template <typename Range1, typename Range2, typename Strategy>
    static inline typename distance_result
        <
            typename point_type<Range1>::type,
            typename point_type<Range2>::type,
            Strategy
        >::type apply(Range1 const& r1, Range2 const& r2, Strategy const& strategy)
    {
        
        typedef typename distance_result
            <
                typename point_type<Range1>::type,
                typename point_type<Range2>::type,
                Strategy
            >::type result_type;
        typedef typename boost::range_size<Range1>::type size_type1;
        typedef typename boost::range_size<Range2>:: type size_type2;
        
        result_type dis_max=0,dis_min;
        boost::geometry::detail::throw_on_empty_input(r1);
        boost::geometry::detail::throw_on_empty_input(r2);
        size_type1  a = boost::size(r1);
        size_type2  b = boost::size(r2);

        //Computing the HausdorffDistance
        for(size_type1 i=0;i<a;i++)
        {
            bool is_dis_min_set = false;
                for(size_type2 j=0;j<b;j++)
                {
                    result_type dis_temp = geometry::distance(range::at(r1, i), range::at(r2, j), strategy);
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

struct range_multi_range
{
    template <typename Range, typename Multi_range, typename Strategy>
    static inline typename distance_result
        <
            typename point_type<Range>::type,
            typename point_type<Multi_range>::type,
            Strategy
        >::type apply(Range const& rng, Multi_range const& mrng, Strategy const& strategy)
    {
        
        typedef typename distance_result
            <
                typename point_type<Range>::type,
                typename point_type<Multi_range>::type,
                Strategy
            >::type result_type;
        typedef typename boost::range_size<Range>::type size_type1;
        typedef typename boost::range_size<Multi_range>::type size_type2;
        
        boost::geometry::detail::throw_on_empty_input(rng);
        boost::geometry::detail::throw_on_empty_input(mrng);
        size_type1  a = boost::size(rng);
        size_type2  b = boost::size(mrng);

        result_type haus_dis=0;
        //Computing the HausdorffDistance
        for(size_type1 i=0;i<a;i++)
        {
                result_type dis_max=0;
                for(size_type2 j=0;j<b;j++)
                {
                    result_type dis_min;
                    dis_max =range_range::apply(rng,range::at(mrng,j),strategy);
                }

                if(dis_max > haus_dis)
                {
                    haus_dis=dis_max;
                }    
        }
        return haus_dis;
    }
};


struct multi_range_multi_range
{
    template <typename Multi_Range1, typename Multi_range2, typename Strategy>
    static inline typename distance_result
        <
            typename point_type<Multi_Range1>::type,
            typename point_type<Multi_range2>::type,
            Strategy
        >::type apply(Multi_Range1 const& mrng1, Multi_range2 const& mrng2, Strategy const& strategy)
    {
        
        typedef typename distance_result
            <
                typename point_type<Multi_Range1>::type,
                typename point_type<Multi_range2>::type,
                Strategy
            >::type result_type;
        typedef typename boost::range_size<Multi_Range1>::type size_type1;
        typedef typename boost::range_size<Multi_range2>::type size_type2;
        
        boost::geometry::detail::throw_on_empty_input(mrng1);
        boost::geometry::detail::throw_on_empty_input(mrng2);
        size_type1  a = boost::size(mrng1);
        size_type2  b = boost::size(mrng2);

        result_type haus_dis=0;
        //Computing the HausdorffDistance
        for(size_type1 i=0;i<a;i++)
        {
            result_type dis_max=0;
            dis_max =range_multi_range::apply(range::at(mrng1,i),mrng2,strategy);

            if(dis_max > haus_dis)
            {
                haus_dis=dis_max;
            }    
        }
        return haus_dis;
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
    : detail::hausdorff_distance::range_range
{};

// Specialization for linestring and multi_linestring 
template <typename linestring, typename multi_linestring>
struct hausdorff_distance<linestring,multi_linestring,linestring_tag,multi_linestring_tag>
    : detail::hausdorff_distance::range_multi_range
{};

// Specialization for multi_linestring and multi_linestring 
template <typename multi_linestring1, typename multi_linestring2>
struct hausdorff_distance<multi_linestring1,multi_linestring2,multi_linestring_tag,multi_linestring_tag>
    : detail::hausdorff_distance::multi_range_multi_range
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
