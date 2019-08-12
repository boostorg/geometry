// Boost.Geometry (aka GGL, Generic Geometry Library)

// Copyright (c) 2019 Tinko Bartels, Berlin, Germany.

// Contributed and/or modified by Tinko Bartels,
//   as part of Google Summer of Code 2019 program.

// Use, modification and distribution is subject to the Boost Software License,
// Version 1.0. (See accompanying file LICENSE_1_0.txt or copy at
// http://www.boost.org/LICENSE_1_0.txt)

#ifndef BOOST_GEOMETRY_EXTENSIONS_TRIANGULATION_STRATEGIES_CARTESIAN_DETAIL_ACCELERATED_SHULL_HPP
#define BOOST_GEOMETRY_EXTENSIONS_TRIANGULATION_STRATEGIES_CARTESIAN_DETAIL_ACCELERATED_SHULL_HPP

#include <vector>
#include <utility>
#include <tuple>
#include <iterator>
#include <algorithm>

#include <boost/geometry/algorithms/make.hpp>
#include <boost/geometry/algorithms/comparable_distance.hpp>
#include <boost/geometry/extensions/triangulation/geometries/triangulation.hpp>
#include <boost/geometry/extensions/triangulation/strategies/cartesian/in_circle_robust.hpp>
#include <boost/geometry/strategies/cartesian/side_by_triangle.hpp>

namespace boost { namespace geometry { namespace strategy
{

#ifndef DOXYGEN_NO_DETAIL

namespace delaunay_triangulation { namespace detail
{

template<typename Point, typename CalculationType, typename SideStrategy>
inline CalculationType comparable_circumcircle_diameter(
    const Point& p1, const Point& p2, const Point& p3)
{
    typedef typename coordinate_type<Point>::type coordinate_type;
    CalculationType comp_area = SideStrategy::template
        side_value<coordinate_type, CalculationType>(p1, p2, p3);
    return comparable_distance(p1, p2)
         * comparable_distance(p1, p3)
         * comparable_distance(p2, p3)
         / (comp_area * comp_area);
}

template
<
    typename PointOut,
    bool ClockWise,
    typename PointIn
>
inline PointOut circumcircle_center(const PointIn& p1,
                                    const PointIn& p2,
                                    const PointIn& p3)
{
    typedef typename coordinate_type<PointIn>::type coordinate_type;
    coordinate_type ax = get<0>( p1 );
    coordinate_type ay = get<1>( p1 );
    coordinate_type bx = get<0>( !ClockWise ? p2 : p3 );
    coordinate_type by = get<1>( !ClockWise ? p2 : p3 );
    coordinate_type cx = get<0>( !ClockWise ? p3 : p2 );
    coordinate_type cy = get<1>( !ClockWise ? p3 : p2 );
    coordinate_type d =
        2 * ( ax * (by - cy) + bx * (cy - ay) + cx * (ay - by));
    coordinate_type x = ( (ax * ax + ay * ay) * (by - cy)
                        + (bx * bx + by * by) * (cy - ay)
                        + (cx * cx + cy * cy) * (ay - by)) / d;
    coordinate_type y = ( (ax * ax + ay * ay) * (cx - bx )
                        + (bx * bx + by * by) * (ax - cx)
                        + (cx * cx + cy * cy) * (bx - ax)) / d;
    return boost::geometry::make<PointOut>(x, y);
}

/*
 * The following algorithm is based on s-hull by David Sinclair. More details can be found at
 * http://s-hull.org. A more detailed description can be found in the paper: 
 * http://www.s-hull.org/paper/s_hull.pdf. The segments that correspond to the steps in the paper
 * are marked by comments. An attempt has been made to speed up the algorithm by using a binary
 * search for the first and last visible edge in step 7. This reduces the number of calls to the
 * orientation test (SideStrategy).
 */
struct accelerated_shull
{
    template
    <
        typename PointContainer,
        typename Triangulation,
        typename SideStrategy,
        typename InCircleStrategy,
        typename CalculationType = double
    >
    static inline void apply(PointContainer const & in,
                             Triangulation& out,
                             bool legalize)
    {
        typedef typename PointContainer::value_type point_type;
        typedef typename default_distance_result<point_type, point_type>::type
            distance_type;
        const bool is_cw =
            boost::geometry::point_order<typename Triangulation::face_type>::value
                == clockwise;
        typedef CalculationType ct;
        std::vector<std::tuple<point_type, ct, ct>> points;
        points.reserve(in.size());
        //Step 1 & 2 & 3
        point_type zero;
        set<0>(zero, 0);
        set<1>(zero, 0);
        std::transform(std::begin(in), std::end(in), std::back_inserter(points),
            [&points, &zero](point_type const& p)
	    {
                return std::tuple<point_type, ct, ct>(p,
                boost::geometry::comparable_distance(p, zero),
                0);
            });
        std::sort(std::begin(points),
                  std::end(points),
                  [](std::tuple<point_type, ct, ct> const& p0,
                     std::tuple<point_type, ct, ct> const& p1)
                    { return std::get<1>(p0) < std::get<1>(p1); });
        for(std::tuple<point_type, ct, ct>& p : points)
        {
            std::get<1>(p) = boost::geometry::comparable_distance(std::get<0>(p),
                                                                  std::get<0>(points[ 0 ]));
        }
        std::sort(std::begin(points) + 1,
                  std::end(points),
                  [](std::tuple<point_type, ct, ct> const& p0,
                     std::tuple<point_type, ct, ct> const& p1)
                    { return std::get<1>(p0) < std::get<1>(p1); });
        //Step 4
        {
            distance_type min_circumdiameter =
                std::numeric_limits<distance_type>::max();
            std::size_t min_index = 2;
            for( std::size_t i = 2 ;
                std::get<1>(points[i]) < min_circumdiameter && i < points.size() ;
                ++i )
            {
                distance_type diam =
                    comparable_circumcircle_diameter<point_type, ct, SideStrategy>(
                        std::get<0>(points[ 0 ]),
                        std::get<0>(points[ 1 ]),
                        std::get<0>(points[ i ]));
                if(diam < min_circumdiameter)
		{
                    min_index = i;
                    min_circumdiameter = diam;
                }
            }
            std::swap(points[ 2 ], points[ min_index ]);
        }
        //Step 5
        if( (    !is_cw
            && SideStrategy::apply(std::get<0>(points[ 0 ]),
                                   std::get<0>(points[ 1 ]),
                                   std::get<0>(points[ 2 ])) < 0 )
            || ( is_cw
               && SideStrategy::apply(std::get<0>(points[ 0 ]),
                                      std::get<0>(points[ 1 ]),
                                      std::get<0>(points[ 2 ])) > 0 ))
        {
            std::swap(points[ 1 ], points[ 2 ]);
        }
        const double PI = 3.14159265358979323846;
        //Step 6

        typedef typename Triangulation::vertex_iterator vertex_iterator;
        typedef typename Triangulation::face_iterator face_iterator;
        vertex_iterator const p1 = out.add_vertex(std::get<0>(points[ 0 ]));
        vertex_iterator const p2 = out.add_vertex(std::get<0>(points[ 1 ]));
        vertex_iterator const p3 = out.add_vertex(std::get<0>(points[ 2 ]));
        face_iterator const seed_face = out.add_isolated_face(p1, p2, p3);
        point_type cen;
        set<0>(cen,
               ( get<0>(std::get<0>(points[ 0 ]))
               + get<0>(std::get<0>(points[ 1 ]))
               + get<0>(std::get<0>(points[ 2 ])) ) / 3);
        set<1>(cen,
               ( get<1>(std::get<0>(points[ 0 ]))
               + get<1>(std::get<0>(points[ 1 ]))
               + get<1>(std::get<0>(points[ 2 ])) ) / 3);

        point_type C = circumcircle_center<point_type, is_cw>(
            std::get<0>(points[ 0 ]),
            std::get<0>(points[ 1 ]),
            std::get<0>(points[ 2 ]));
        {
            std::for_each(std::begin(points), std::end(points),
                [&C, &cen, &PI](std::tuple<point_type, ct, ct>& p){
                    std::get<1>(p) = comparable_distance(std::get<0>(p), C);
                    std::get<2>(p) =
                        ! is_cw ?
                          std::atan2(
                                get<1>(std::get<0>(p)) - get<1>(cen),
                                get<0>(std::get<0>(p)) - get<0>(cen)) + PI
                          : 2 * PI - (std::atan2(
                                get<1>(std::get<0>(p)) - get<1>(cen),
                                get<0>(std::get<0>(p)) - get<0>(cen)) + PI);
                });
            std::sort(std::begin(points) + 3,
                      std::end(points),
                      [](std::tuple<point_type, ct, ct> const& p0,
                         std::tuple<point_type, ct, ct> const& p1)
                      { return std::get<1>(p0) < std::get<1>(p1); });
        }
        //Step 7 & 8
        typedef typename Triangulation::halfedge_index halfedge_index;
        typedef typename Triangulation::point_type out_point_type;
        typedef typename Triangulation::segment_type segment_type;
        {
            halfedge_index e1 = out.face_edge(seed_face);
            halfedge_index e2 = out.next(e1);
            halfedge_index e3 = out.next(e2);
            typedef std::pair<typename Triangulation::halfedge_index, ct>
                convex_hull_edge;
            typedef std::vector<convex_hull_edge> convex_hull_container;
            typedef typename convex_hull_container::iterator convex_hull_iterator;
            convex_hull_container convex_hull {
                std::make_pair(e1, ct(std::get<2>(points[1]))),
                std::make_pair(e2, ct(std::get<2>(points[2]))),
                std::make_pair(e3, ct(std::get<2>(points[0]))) };
            for (std::size_t i = 3; i < points.size(); ++i)
            {
                vertex_iterator new_vertex =
                    out.add_vertex(std::get<0>(points[ i ]));
                out_point_type const& p = out.vertex(new_vertex);
                auto is_visible = [&out, &p](convex_hull_edge const& bep)
                    {
                        const halfedge_index& be = std::get<0>(bep);
                        const segment_type s = out.face_segment(be);
                        const out_point_type p1 = s.first;
                        const out_point_type p2 = s.second;
                        auto det = SideStrategy::apply(p1,p2,p);
                        bool result = (!is_cw && det < 0) || (is_cw && det > 0);
                        return result;
                    };
                const ct& ref_angle = std::get<1>(*convex_hull.begin());
                ct angle = std::get<2>(points[ i ]) - ref_angle;
                if(angle < 0) angle += 2 * PI;
                ct opposite = angle - PI;
                if(opposite < 0) opposite += 2 * PI;

                auto pred = [&ref_angle, &PI]
                    (convex_hull_edge const& be, ct const& a)
                    {
                        ct ba = std::get<1>(be) - ref_angle;
                        if(ba < 0) ba += 2*PI;
                        return ba < a;
                    };
                convex_hull_iterator vis_edge = std::lower_bound(
                    convex_hull.begin(), convex_hull.end(),
                    angle, pred);
                if( vis_edge == convex_hull.begin() )
                    vis_edge = convex_hull.end() - 1;
                else --vis_edge;
                convex_hull_iterator invis_edge = std::lower_bound(
                    convex_hull.begin(), convex_hull.end(),
                    opposite, pred);
                if( invis_edge == convex_hull.begin() )
                    invis_edge = convex_hull.end() - 1;
                else --invis_edge;
                if(!is_visible(*vis_edge))
		{
                    invis_edge = vis_edge;
                    for(auto it = convex_hull.begin();
                        it != convex_hull.end();
                        ++it)
		    {
                        if(is_visible(*it))
			{
                            vis_edge = it;
                            break;
                        }
                    }
                }
                auto vis_small = [&is_visible]
                    (convex_hull_edge const& be, int const&)
                    {
                        if(is_visible(be)) return true;
                        else return false;
                    };
                auto vis_large = [&is_visible]
                    (convex_hull_edge const& be, int const&)
                    {
                        if(is_visible(be)) return false;
                        else return true;
                    };
                convex_hull_iterator first_visible, last_visible,
                    fv2 = convex_hull.end();
                if(vis_edge < invis_edge)
		{
                    if(is_visible(*convex_hull.begin()))
		    {
                        first_visible = convex_hull.begin();
                        last_visible = std::lower_bound(vis_edge, invis_edge,
                            0, vis_small);
                        fv2 = std::lower_bound(invis_edge, convex_hull.end(),
                            0, vis_large);
                    }
		    else
		    {
                        first_visible = std::lower_bound(convex_hull.begin(),
                            vis_edge, 0, vis_large);
                        last_visible = std::lower_bound(vis_edge,
                            convex_hull.end(), 0, vis_small);
                    }
                }
		else
		{
                    if(is_visible(*convex_hull.begin()))
		    {
                        first_visible = convex_hull.begin();
                        last_visible = std::lower_bound(
                            convex_hull.begin()+1, invis_edge,
                            0, vis_small);
                        fv2 = std::lower_bound(invis_edge, vis_edge,
                            0, vis_large);
                    }
		    else
		    {
                        first_visible = std::lower_bound(invis_edge, vis_edge,
                            0, vis_large);
                        last_visible = std::lower_bound(vis_edge,
                            convex_hull.end(), 0, vis_small);
                    }
                }
                ct first_visible_angle = std::get<1>(*first_visible);
                const bool begin_visible = first_visible == std::begin(convex_hull);
                face_iterator prev, fnf2;
                bool looped = false;
                ct fv2_angle;
                if(begin_visible && is_visible(convex_hull.back()))
                {
                    looped = true;
                    fv2_angle = std::get<1>(*fv2);
                    fnf2 = out.add_face_on_boundary(std::get<0>(*fv2), new_vertex);
                    prev = fnf2;
                    for(convex_hull_iterator it = fv2 + 1 ;
                        it != std::end(convex_hull) ;
                        ++it)
                    {
                        face_iterator next =
                            out.add_face_on_boundary(std::get<0>(*it), new_vertex);
                        out.connect(out.next(out.face_edge(next)),
                                    out.prev(out.face_edge(prev)));
                        prev = next;
                    }
                    convex_hull.erase(fv2, std::end(convex_hull));
                }
                const face_iterator fnf =
                    out.add_face_on_boundary(std::get<0>(*first_visible),
                                             new_vertex);
                if(looped)
		{
                    out.connect(out.next(out.face_edge(fnf)),
                                out.prev(out.face_edge(prev)));
                }
                prev = fnf;
                for (convex_hull_iterator it = first_visible + 1;
                     it != last_visible;
                     ++it)
                {
                    face_iterator next =
                        out.add_face_on_boundary(std::get<0>(*it), new_vertex);
                    out.connect(out.next(out.face_edge(next)),
                                out.prev(out.face_edge(prev)));
                    prev = next;
                }
                convex_hull_iterator ip =
                    convex_hull.erase(first_visible, last_visible);
                ip = convex_hull.insert(ip,
                    std::make_pair(out.prev(out.face_edge(prev)),
                                   ct(std::get<2>(points[i]))));
                if(looped)
		{
                    convex_hull.insert(ip,
                        std::make_pair(out.next(out.face_edge(fnf2)),
                                       ct(fv2_angle)));
		}
                else
		{
                    convex_hull.insert(ip,
                        std::make_pair(out.next(out.face_edge(fnf)),
                                       ct(first_visible_angle)));
		}
            }
        }
        //Step 9
        if(legalize)
        {
            typedef typename Triangulation::face_vertex_index face_vertex_index;
            std::vector<halfedge_index> L;
            for(face_iterator i = out.faces_begin(); i != out.faces_end(); ++i)
            {
                for(face_vertex_index j = 0; j<2 ; ++j)
                    if(   out.neighbour(i, j)!=Triangulation::invalid()
                          && &(*i) > &(*out.neighbour(i, j)))
		    {
                        L.push_back(halfedge_index(i,j));
                    }
            }
            auto edge_legal = [&out](halfedge_index const& e)
                {
                    out_point_type const& p1 = out.face_vertex(e.m_f, 0);
                    out_point_type const& p2 = out.face_vertex(e.m_f, 1);
                    out_point_type const& p3 = out.face_vertex(e.m_f, 2);
                    out_point_type const& p = out.face_vertex(
                            out.neighbour(e.m_f, e.m_v),
                            out.opposite(e.m_f, e.m_v)
                        );
                    return !is_cw ?
                             InCircleStrategy::apply(p1, p2, p3, p) <= 0
                          || InCircleStrategy::apply(p1, p2, p, p3) >  0
                        :    InCircleStrategy::apply(p1, p3, p2, p) <= 0
                          || InCircleStrategy::apply(p1, p3, p, p2) >  0;
                };
            while( !L.empty() )
            {
                halfedge_index e = L.back();
                L.pop_back();
                face_iterator const f1 = e.m_f;
                face_vertex_index const v1 = e.m_v;
                face_iterator const f2 = out.neighbour(f1, v1);
                if(f2 == face_iterator()) continue;
                face_vertex_index const v2 = out.opposite(f1, v1);
                if( !edge_legal(e) )
		{
                    L.emplace_back(f1, v1);
                    L.emplace_back(f2, v2);
                    L.emplace_back(f1, v1 == 0 ? 2 : v1 - 1);
                    L.emplace_back(f2, v2 == 0 ? 2 : v2 - 1);
                    out.flip(e);
                }
            }
        }
    }
};

}} // namespace delaunay_triangulation::detail

#endif // DOXYGEN_NO_DETAIL

}}} // namespace boost::geometry::strategy

#endif // BOOST_GEOMETRY_EXTENSIONS_TRIANGULATION_STRATEGIES_CARTESIAN_DETAIL_ACCELERATED_SHULL_HPP
