// TODO: OLD!

// Boost.Geometry (aka GGL, Generic Geometry Library)
//
// Copyright (c) 2007-2011 Barend Gehrels, Amsterdam, the Netherlands.
// Use, modification and distribution is subject to the Boost Software License,
// Version 1.0. (See accompanying file LICENSE_1_0.txt or copy at
// http://www.boost.org/LICENSE_1_0.txt)

#include <deque>
#include <iostream>
#include <limits>
#include <list>
#include <memory>
#include <sstream>
#include <string>
#include <vector>

#include <boost/geometry/geometry.hpp>
#include <boost/geometry/multi/multi.hpp>

#include <boost/geometry/geometries/geometries.hpp>

template<typename P>
struct modifying_functor
{
    double sum;
    modifying_functor() : sum(0)
    {}
    inline void operator()(P& p)
    {
        p.x(1);
    }

    inline void operator()(bg::segment<P>& s)
    {
        s.first.x(1);
    }
};

template<typename P>
struct const_functor
{
    double sum;
    const_functor() : sum(0)
    {}
    inline void operator()(P const& p)
    {
        sum += p.x();
    }

    inline void operator()(bg::segment<P> const& s)
    {
        sum += s.first.x() - s.second.x();
    }
};

template <typename T, template<typename, typename> class V>
void check_linestring()
{
    typedef bg::model::d2::point_xy<T> P;
    typedef bg::model::linestring<P, V, std::allocator> line_type;
    line_type line;
    line.push_back(P(0,0));
    line.push_back(P(1,1));

    typedef bg::multi_linestring<line_type, V, std::allocator> multi_line_type;
    multi_line_type multi;
    multi.push_back(line);

    double len = bg::length(line);
    len = bg::length(multi);
    double d = bg::distance(P(0,1), line);
    //d = bg::distance(P(0,1), multi); not defined yet!

    line_type simp;
    bg::simplify(line, simp, 3);
    multi_line_type simpm;
    bg::simplify(multi, simpm, 3);

    typedef bg::model::box<P> box_type;
    box_type box1;
    bg::envelope(line, box1);
    box_type box2;
    bg::envelope(multi, box2);

    // FIXME: Where is output stream op for line/multi --mloskot
    //std::stringstream out;
    // out << line << std::endl;
    //out << multi << std::endl;

    // For each, const
    /* TODO: Fix for_each/functor
    const_functor<P> cf;
    std::for_each(line.begin(), line.end(), cf);

    const line_type& cl = line;
    const multi_line_type& cm = multi;

    bg::for_each_point(cl, cf);
    bg::for_each_point(cm, cf);
    bg::for_each_segment(cl, cf);
    bg::for_each_segment(cm, cf);

    // For each, modifying
    modifying_functor<P> mf;
    line_type& ml = line;
    multi_line_type& mm = multi;
    std::for_each(line.begin(), line.end(), mf);
    bg::for_each_point(ml, mf);
    bg::for_each_point(mm, mf);
    bg::for_each_segment(ml, mf);
    bg::for_each_segment(mm, mf);
    */

}

template
<
    typename T,
    template<typename, typename> class VP,
    template<typename, typename> class VR
>
void check_polygon()
{
    typedef bg::model::d2::point_xy<T> P;
    typedef bg::model::polygon<P, VP, VR, true, std::allocator, std::allocator> Y;
    Y poly;
    poly.outer().push_back(P(0,0));
    poly.outer().push_back(P(2,0));
    poly.outer().push_back(P(2,2));
    poly.outer().push_back(P(0,2));

    bg::correct(poly);

    // multi
    typedef bg::multi_polygon<Y, VP, std::allocator> MY;
    MY multi;
    multi.push_back(poly);

    double a = bg::area(poly);
    a = bg::area(multi);

    //double d = bg::distance(P(0,1), poly);

    Y simp;
    bg::simplify(poly, simp, 3);
    MY msimp;
    bg::simplify(multi, msimp, 3);

    typedef bg::model::box<P> box_type;
    box_type box1;
    bg::envelope(poly, box1);
    box_type box2;
    bg::envelope(multi, box2);

    P ctr;
    bg::centroid(poly, ctr);

    // within
    bg::model::d2::point_xy<double> circ_centre(10,10);

    bool w = bg::within(P(1, 1), poly);
    //w = bg::within(poly, b); tbd
    w = bg::within(P(1, 1), multi);

    //bg::circle circ(circ_centre, 10);
    //w = bg::within(poly, circ);
    //w = bg::within(multi, circ);
    //w = bg::within(multi, b); tbd

    // For each, const
    /* TODO: Fix for_each/functor
    const_functor<P> cf;
    std::for_each(poly.outer().begin(), poly.outer().end(), cf);

    const Y& cp = poly;
    const MY& cm = multi;

    bg::for_each_point(cp, cf);
    bg::for_each_point(cm, cf);
    bg::for_each_segment(cp, cf);
    bg::for_each_segment(cm, cf);

    // For each, modifying
    modifying_functor<P> mf;
    Y& mp = poly;
    MY& mm = multi;
    std::for_each(poly.outer().begin(), poly.outer().end(), mf);
    bg::for_each_point(mp, mf);
    bg::for_each_point(mm, mf);
    bg::for_each_segment(mp, mf);
    bg::for_each_segment(mm, mf);
    */
}


int main()
{
    check_linestring<double, std::vector>();
    check_linestring<float, std::vector>();
    check_linestring<int, std::vector>();
    check_linestring<char, std::vector>();

    //check_linestring<double, std::list>();
    check_linestring<double, std::deque>();

    check_polygon<double, std::vector, std::vector>();
    check_polygon<float, std::vector, std::vector>();
    check_polygon<int, std::vector, std::vector>();
    check_polygon<char, std::vector, std::vector>();

    //check_polygon<double, std::list, std::vector>();
    check_polygon<double, std::deque, std::vector>();
    //check_polygon<double, std::list, std::list>();
    check_polygon<double, std::deque, std::deque>();

    return 0;
}

