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
namespace detail { namespace frechet_distance
{

template <typename size_type1 , typename size_type2,typename result_type>
class coup_mat
{
public:
    coup_mat(size_type1 w, size_type2 h)
        : m_data(w * h,-1), m_width(w), m_height(h)
    {}

    result_type & operator()(size_type1 i, size_type2 j)
    {
        BOOST_ASSERT(i < m_width && j < m_height);
        return m_data[j * m_width + i];
    }

private:
    std::vector<result_type> m_data;
    size_type1 m_width;
    size_type2 m_height;
};

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
        typedef typename boost::range_size<Linestring1>::type size_type1;
        typedef typename boost::range_size<Linestring2>::type size_type2;
        

        boost::geometry::detail::throw_on_empty_input(ls1);
        boost::geometry::detail::throw_on_empty_input(ls2);

        size_type1 const a = boost::size(ls1);
        size_type2 const b = boost::size(ls2);


        //Coupling Matrix CoupMat(a,b,-1);
        coup_mat<size_type1,size_type2,result_type>  coup_matrix(a,b);

        result_type const not_feasible = -100;
        //findin the Coupling Measure
        for(size_type1 i=0;i<a;i++)
        {
            for(size_type2 j=0;j<b;j++)
            {
                if(i==0 && j==0)
                    coup_matrix(i,j)= 
                    geometry::distance(range::at(ls1,i),range::at(ls2,j),strategy);
                else if(i==0 && j>0)
                    coup_matrix(i,j)=
                    (std::max)(coup_matrix(i,j-1),geometry::distance(range::at(ls1,i),range::at(ls2,j),strategy));
                else if(i>0 && j==0)
                    coup_matrix(i,j)=
                    (std::max)(coup_matrix(i-1,j),geometry::distance(range::at(ls1,i),range::at(ls2,j),strategy));
                else if(i>0 && j>0)
                    coup_matrix(i,j)=
                    (std::max)((std::min)((std::min)(coup_matrix(i,j-1),coup_matrix(i-1,j)),coup_matrix(i-1,j-1)),geometry::distance(range::at(ls1,i),range::at(ls2,j),strategy));
                else
                    coup_matrix(i,j)=not_feasible;
            }
        }

        #ifdef BOOST_GEOMETRY_DEBUG_FRECHET_DISTANCE
        //Print CoupLing Matrix
        for(size_type i = 0; i <a; i++)
        {
            for(size_type j = 0; j <b; j++)
            {   
                std::cout << coup_matrix(i,j) << " ";
            }
            std::cout << std::endl;
        }
        #endif
            
        return coup_matrix(a-1,b-1);
    }
};

}} // namespace detail::frechet_distance
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
struct frechet_distance : not_implemented<Tag1, Tag2>
{};
template <typename Linestring1, typename Linestring2>
struct frechet_distance<Linestring1,Linestring2,linestring_tag,linestring_tag>
    : detail::frechet_distance::linestring_linestring
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
frechet_distance(Geometry1 const& g1, Geometry2 const& g2, Strategy const& strategy)
{
    return dispatch::frechet_distance<Geometry1, Geometry2>::apply(g1, g2, strategy);
}

// Algorithm overload using default Pt-Pt distance strategy
template <typename Geometry1, typename Geometry2>
inline typename distance_result
        <
            typename point_type<Geometry1>::type,
            typename point_type<Geometry2>::type
        >::type
frechet_distance(Geometry1 const& g1, Geometry2 const& g2)
{
    typedef typename strategy::distance::services::default_strategy
              <
                  point_tag, point_tag,
                  typename point_type<Geometry1>::type,
                  typename point_type<Geometry2>::type
              >::type strategy_type;
    
    return frechet_distance(g1, g2, strategy_type());
}

}} // namespace boost::geometry
