// TODO: OLD!

// Boost.Geometry (aka GGL, Generic Geometry Library)
//
// Copyright Barend Gehrels, Geodan B.V. Amsterdam, the Netherlands.
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

#include <boost/geometry/multi/core/tags.hpp>
#include <boost/geometry/algorithms/for_each.hpp>
#include <boost/geometry/algorithms/area.hpp>
#include <boost/geometry/algorithms/centroid.hpp>
#include <boost/geometry/algorithms/correct.hpp>
#include <boost/geometry/algorithms/distance.hpp>
#include <boost/geometry/algorithms/envelope.hpp>
#include <boost/geometry/algorithms/for_each.hpp>
#include <boost/geometry/algorithms/intersection.hpp>
#include <boost/geometry/algorithms/length.hpp>
#include <boost/geometry/algorithms/simplify.hpp>
#include <boost/geometry/algorithms/within.hpp>
#include <boost/geometry/algorithms/overlaps.hpp>

#include <boost/geometry/geometries/cartesian2d.hpp>
#include <boost/geometry/geometries/geometries.hpp>
#include <boost/geometry/multi/geometries/multi_point.hpp>
#include <boost/geometry/multi/geometries/multi_linestring.hpp>
#include <boost/geometry/multi/geometries/multi_polygon.hpp>

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

    inline void operator()(boost::geometry::segment<P>& s)
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

    inline void operator()(boost::geometry::segment<P> const& s)
    {
        sum += s.first.x() - s.second.x();
    }
};

template <typename T, template<typename, typename> class V>
void check_linestring()
{
    typedef boost::geometry::point_xy<T> P;
    typedef boost::geometry::linestring<P, V, std::allocator> line_type;
    line_type line;
    line.push_back(P(0,0));
    line.push_back(P(1,1));

    typedef boost::geometry::multi_linestring<line_type, V, std::allocator> multi_line_type;
    multi_line_type multi;
    multi.push_back(line);

    double len = boost::geometry::length(line);
    len = boost::geometry::length(multi);
    double d = boost::geometry::distance(P(0,1), line);
    //d = boost::geometry::distance(P(0,1), multi); not defined yet!

    line_type simp;
    boost::geometry::simplify(line, simp, 3);
    multi_line_type simpm;
    boost::geometry::simplify(multi, simpm, 3);

    typedef boost::geometry::box<P> box_type;
    box_type box1;
    boost::geometry::envelope(line, box1);
    box_type box2;
    boost::geometry::envelope(multi, box2);

    // FIXME: Where is output stream op for line/multi --mloskot
    //std::stringstream out;
    // out << line << std::endl;
    //out << multi << std::endl;

    // For each, const
    const_functor<P> cf;
    std::for_each(line.begin(), line.end(), cf);

    const line_type& cl = line;
    const multi_line_type& cm = multi;

    boost::geometry::for_each_point(cl, cf);
    boost::geometry::for_each_point(cm, cf);
    boost::geometry::for_each_segment(cl, cf);
    boost::geometry::for_each_segment(cm, cf);

    // For each, modifying
    modifying_functor<P> mf;
    line_type& ml = line;
    multi_line_type& mm = multi;
    std::for_each(line.begin(), line.end(), mf);
    boost::geometry::for_each_point(ml, mf);
    boost::geometry::for_each_point(mm, mf);
    boost::geometry::for_each_segment(ml, mf);
    boost::geometry::for_each_segment(mm, mf);
}

template
<
    typename T,
    template<typename, typename> class VP,
    template<typename, typename> class VR
>
void check_polygon()
{
    typedef boost::geometry::point_xy<T> P;
    typedef boost::geometry::polygon<P, VP, VR, std::allocator, std::allocator> Y;
    Y poly;
    poly.outer().push_back(P(0,0));
    poly.outer().push_back(P(2,0));
    poly.outer().push_back(P(2,2));
    poly.outer().push_back(P(0,2));

    boost::geometry::correct(poly);

    // multi
    typedef boost::geometry::multi_polygon<Y, VP, std::allocator> MY;
    MY multi;
    multi.push_back(poly);

    double a = boost::geometry::area(poly);
    a = boost::geometry::area(multi);

    //double d = boost::geometry::distance(P(0,1), poly);

    Y simp;
    boost::geometry::simplify(poly, simp, 3);
    MY msimp;
    boost::geometry::simplify(multi, msimp, 3);

    typedef boost::geometry::box<P> box_type;
    box_type box1;
    boost::geometry::envelope(poly, box1);
    box_type box2;
    boost::geometry::envelope(multi, box2);

    P ctr;
    boost::geometry::centroid(poly, ctr);

    // within
    boost::geometry::point_2d circ_centre(10,10);
    boost::geometry::circle circ(circ_centre, 10);

    bool w = boost::geometry::within(P(1, 1), poly);
    w = boost::geometry::within(poly, circ);
    //w = boost::geometry::within(poly, b); tbd
    w = boost::geometry::within(P(1, 1), multi);
    w = boost::geometry::within(multi, circ);
    //w = boost::geometry::within(multi, b); tbd

    // For each, const
    const_functor<P> cf;
    std::for_each(poly.outer().begin(), poly.outer().end(), cf);

    const Y& cp = poly;
    const MY& cm = multi;

    boost::geometry::for_each_point(cp, cf);
    boost::geometry::for_each_point(cm, cf);
    boost::geometry::for_each_segment(cp, cf);
    boost::geometry::for_each_segment(cm, cf);

    // For each, modifying
    modifying_functor<P> mf;
    Y& mp = poly;
    MY& mm = multi;
    std::for_each(poly.outer().begin(), poly.outer().end(), mf);
    boost::geometry::for_each_point(mp, mf);
    boost::geometry::for_each_point(mm, mf);
    boost::geometry::for_each_segment(mp, mf);
    boost::geometry::for_each_segment(mm, mf);
}


int main()
{
    check_linestring<double, std::vector>();
    check_linestring<float, std::vector>();
    check_linestring<int, std::vector>();
    check_linestring<char, std::vector>();

    check_linestring<double, std::list>();
    check_linestring<double, std::deque>();

    check_polygon<double, std::vector, std::vector>();
    check_polygon<float, std::vector, std::vector>();
    check_polygon<int, std::vector, std::vector>();
    check_polygon<char, std::vector, std::vector>();

    check_polygon<double, std::list, std::vector>();
    check_polygon<double, std::deque, std::vector>();
    check_polygon<double, std::list, std::list>();
    check_polygon<double, std::deque, std::deque>();

    return 0;
}

