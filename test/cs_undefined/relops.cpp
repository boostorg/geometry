// Boost.Geometry

// Copyright (c) 2019, Oracle and/or its affiliates.

// Contributed and/or modified by Adam Wulkiewicz, on behalf of Oracle

// Licensed under the Boost Software License version 1.0.
// http://www.boost.org/users/license.html

#include "common.hpp"

#include <boost/geometry/algorithms/covered_by.hpp>
#include <boost/geometry/algorithms/crosses.hpp>
#include <boost/geometry/algorithms/disjoint.hpp>
#include <boost/geometry/algorithms/equals.hpp>
#include <boost/geometry/algorithms/intersects.hpp>
#include <boost/geometry/algorithms/overlaps.hpp>
#include <boost/geometry/algorithms/relate.hpp>
#include <boost/geometry/algorithms/relation.hpp>
#include <boost/geometry/algorithms/touches.hpp>
#include <boost/geometry/algorithms/within.hpp>

template
<
    typename G1,
    typename G2,
    std::size_t Dim1 = bg::topological_dimension<G1>::value,
    std::size_t Dim2 = bg::topological_dimension<G2>::value
>
struct call_equals
{
    template <typename S>
    static void apply(G1 const& g1, G2 const& g2, S const& s) {}
};

template <typename G1, typename G2, std::size_t Dim>
struct call_equals<G1, G2, Dim, Dim>
{
    template <typename S>
    static void apply(G1 const& g1, G2 const& g2, S const& s)
    {
        bg::equals(g1, g2, s);
    }
};

template
<
    typename G1,
    typename G2,
    std::size_t Dim1 = bg::topological_dimension<G1>::value,
    std::size_t Dim2 = bg::topological_dimension<G2>::value
>
struct call_overlaps
{
    template <typename S>
    static void apply(G1 const& g1, G2 const& g2, S const& s) {}
};

template <typename G1, typename G2, std::size_t Dim>
struct call_overlaps<G1, G2, Dim, Dim>
{
    template <typename S>
    static void apply(G1 const& g1, G2 const& g2, S const& s)
    {
        bg::overlaps(g1, g2, s);
    }
};

template
<
    typename G1,
    typename G2,
    std::size_t Dim1 = bg::topological_dimension<G1>::value,
    std::size_t Dim2 = bg::topological_dimension<G2>::value
>
struct call_touches
{
    template <typename S>
    static void apply(G1 const& g1, G2 const& g2, S const& s)
    {
        bg::touches(g1, g2, s);
    }
};

template <typename G1, typename G2>
struct call_touches<G1, G2, 0, 0>
{
    template <typename S>
    static void apply(G1 const& g1, G2 const& g2, S const& s) {}
};

template
<
    typename G1,
    typename G2,
    std::size_t Dim1 = bg::topological_dimension<G1>::value,
    std::size_t Dim2 = bg::topological_dimension<G2>::value
>
struct call_crosses
{
    template <typename S>
    static void apply(G1 const& g1, G2 const& g2, S const& s)
    {
        bg::crosses(g1, g2, s);
    }
};

template <typename G1, typename G2>
struct call_crosses<G1, G2, 0, 0>
{
    template <typename S>
    static void apply(G1 const& g1, G2 const& g2, S const& s) {}
};

template <typename G1, typename G2>
struct call_crosses<G1, G2, 2, 2>
{
    template <typename S>
    static void apply(G1 const& g1, G2 const& g2, S const& s) {}
};

template <typename G1, typename G2, typename S>
inline void rel(G1 const& g1, G2 const& g2, S const& s)
{
    bg::relation(g1, g2, s);
    bg::relate(g1, g2, bg::de9im::mask("*********"), s);
    bg::covered_by(g1, g2, s);
    call_crosses<G1, G2>::apply(g1, g2, s);
    bg::disjoint(g1, g2, s);
    call_equals<G1, G2>::apply(g1, g2, s);
    bg::intersects(g1, g2, s);    
    call_overlaps<G1, G2>::apply(g1, g2, s);
    call_touches<G1, G2>::apply(g1, g2, s);
    bg::within(g1, g2, s);
}

template <typename G1, typename G2>
inline void rel_pp(G1 const& g1, G2 const& g2)
{
    ::rel(g1, g2, bg::strategy::within::cartesian_point_point());
    ::rel(g1, g2, bg::strategy::within::spherical_point_point());
}

template <typename G1, typename G2>
inline void rel_ps(G1 const& g1, G2 const& g2)
{
    typedef typename bg::point_type<G1>::type point;
    ::rel(g1, g2, bg::strategy::within::cartesian_winding<point>());
    ::rel(g1, g2, bg::strategy::within::spherical_winding<point>());
    ::rel(g1, g2, bg::strategy::within::geographic_winding<point>());
}

template <typename G1, typename G2>
inline void rel_ss(G1 const& g1, G2 const& g2)
{
    ::rel(g1, g2, bg::strategy::intersection::cartesian_segments<>());
    ::rel(g1, g2, bg::strategy::intersection::spherical_segments<>());
    ::rel(g1, g2, bg::strategy::intersection::geographic_segments<>());
}

int test_main(int, char*[])
{
    geom g;

    bg::disjoint(g.pt, g.b, bg::strategy::covered_by::cartesian_point_box());
    bg::disjoint(g.pt, g.b, bg::strategy::covered_by::spherical_point_box());
    bg::disjoint(g.b, g.b, bg::strategy::disjoint::cartesian_box_box());
    bg::disjoint(g.b, g.b, bg::strategy::disjoint::spherical_box_box());
    bg::within(g.pt, g.b, bg::strategy::within::cartesian_point_box());
    bg::within(g.pt, g.b, bg::strategy::within::spherical_point_box());
    bg::within(g.b, g.b, bg::strategy::within::cartesian_box_box());
    bg::within(g.b, g.b, bg::strategy::within::spherical_box_box());
    bg::covered_by(g.pt, g.b, bg::strategy::covered_by::cartesian_point_box());
    bg::covered_by(g.pt, g.b, bg::strategy::covered_by::spherical_point_box());
    bg::covered_by(g.b, g.b, bg::strategy::covered_by::cartesian_box_box());
    bg::covered_by(g.b, g.b, bg::strategy::covered_by::spherical_box_box());

    // P/P
    ::rel_pp(g.pt, g.pt);
    ::rel_pp(g.pt, g.mpt);
    ::rel_pp(g.mpt, g.mpt);
    // P/L
    //::rel_ps(g.pt, g.s); // relate not implemented
    ::rel_ps(g.pt, g.ls);
    ::rel_ps(g.pt, g.mls);
    //::rel_ps(g.mpt, g.s); // relate not implemented
    ::rel_ps(g.mpt, g.ls);
    ::rel_ps(g.mpt, g.mls);
    // P/A
    ::rel_ps(g.pt, g.r);
    ::rel_ps(g.mpt, g.r);
    ::rel_ps(g.pt, g.po);
    ::rel_ps(g.mpt, g.po);
    ::rel_ps(g.pt, g.mpo);
    ::rel_ps(g.mpt, g.mpo);
    // L/L
    //::rel_ss(g.s, g.s); // relate not implemented
    //::rel_ss(g.s, g.ls); // relate not implemented
    //::rel_ss(g.s, g.mls); // relate not implemented
    //::rel_ss(g.ls, g.s); // relate not implemented
    ::rel_ss(g.ls, g.ls);
    ::rel_ss(g.ls, g.mls);
    //::rel_ss(g.mls, g.s); // relate not implemented
    ::rel_ss(g.mls, g.ls);
    ::rel_ss(g.mls, g.mls);
    // L/A
    //::rel_ss(g.s, g.r); // relate not implemented
    ::rel_ss(g.ls, g.r);
    ::rel_ss(g.mls, g.r);
    //::rel_ss(g.s, g.po); // relate not implemented
    ::rel_ss(g.ls, g.po);
    ::rel_ss(g.mls, g.po);
    //::rel_ss(g.s, g.mpo); // relate not implemented
    ::rel_ss(g.ls, g.mpo);
    ::rel_ss(g.mls, g.mpo);
    // A/A
    ::rel_ss(g.r, g.r);
    ::rel_ss(g.po, g.r);
    ::rel_ss(g.mpo, g.r);
    ::rel_ss(g.r, g.po);
    ::rel_ss(g.po, g.po);
    ::rel_ss(g.mpo, g.po);
    ::rel_ss(g.r, g.mpo);
    ::rel_ss(g.po, g.mpo);
    ::rel_ss(g.mpo, g.mpo);

    return 0;
}
