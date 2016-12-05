// Boost.Geometry
// Unit Test

// Copyright (c) 2016, Oracle and/or its affiliates.
// Contributed and/or modified by Adam Wulkiewicz, on behalf of Oracle

// Use, modification and distribution is subject to the Boost Software License,
// Version 1.0. (See accompanying file LICENSE_1_0.txt or copy at
// http://www.boost.org/LICENSE_1_0.txt)

#ifndef BOOST_GEOMETRY_TEST_STRATEGIES_SEGMENT_INTERSECTION_GEO_HPP
#define BOOST_GEOMETRY_TEST_STRATEGIES_SEGMENT_INTERSECTION_GEO_HPP


#include "segment_intersection_sph.hpp"


#include <boost/geometry/formulas/andoyer_inverse.hpp>
#include <boost/geometry/formulas/thomas_inverse.hpp>
#include <boost/geometry/formulas/vincenty_inverse.hpp>

#include <boost/geometry/strategies/geographic/intersection.hpp>
#include <boost/geometry/strategies/geographic/geodesic_intersection.hpp>


template <typename S, typename P>
void test_default_strategy(std::string const& s1_wkt, std::string const& s2_wkt,
                           char m, std::size_t expected_count,
                           std::string const& ip0_wkt = "", std::string const& ip1_wkt = "")
{
    typedef typename bg::coordinate_type<P>::type coord_t;
    typedef bg::policies::relate::segments_tupled
                <
                    bg::policies::relate::segments_intersection_points
                        <
                            bg::segment_intersection_points<P, bg::segment_ratio<coord_t> >
                        >,
                    bg::policies::relate::segments_direction
                > policy_t;

    typename bg::strategy::intersection::services::default_strategy
        <
            bg::geographic_tag,
            policy_t,
            void
        >::type strategy;

    test_strategy<S, S, P>(s1_wkt, s2_wkt, strategy, m, expected_count, ip0_wkt, ip1_wkt);
}

template <typename S, typename P>
void test_great_elliptic(std::string const& s1_wkt, std::string const& s2_wkt,
                         char m, std::size_t expected_count,
                         std::string const& ip0_wkt = "", std::string const& ip1_wkt = "")
{
    typedef typename bg::coordinate_type<P>::type coord_t;
    typedef bg::policies::relate::segments_tupled
                <
                    bg::policies::relate::segments_intersection_points
                        <
                            bg::segment_intersection_points<P, bg::segment_ratio<coord_t> >
                        >,
                    bg::policies::relate::segments_direction
                > policy_t;

    bg::strategy::intersection::relate_great_elliptic_segments
        <
            policy_t
        > strategy;

    test_strategy<S, S, P>(s1_wkt, s2_wkt, strategy, m, expected_count, ip0_wkt, ip1_wkt);
}
/*
template <typename S, typename P>
void test_experimental_elliptic(std::string const& s1_wkt, std::string const& s2_wkt,
                                char m, std::size_t expected_count,
                                std::string const& ip0_wkt = "", std::string const& ip1_wkt = "")
{
    typedef typename bg::coordinate_type<P>::type coord_t;
    typedef bg::policies::relate::segments_tupled
                <
                    bg::policies::relate::segments_intersection_points
                        <
                            bg::segment_intersection_points<P, bg::segment_ratio<coord_t> >
                        >,
                    bg::policies::relate::segments_direction
                > policy_t;

    bg::strategy::intersection::relate_experimental_elliptic_segments
        <
            policy_t
        > strategy;

    test_strategy<S, S, P>(s1_wkt, s2_wkt, strategy, m, expected_count, ip0_wkt, ip1_wkt);
}
*/
template <typename S, typename P>
void test_geodesic_vincenty(std::string const& s1_wkt, std::string const& s2_wkt,
                            char m, std::size_t expected_count,
                            std::string const& ip0_wkt = "", std::string const& ip1_wkt = "")
{
    typedef typename bg::coordinate_type<P>::type coord_t;
    typedef bg::policies::relate::segments_tupled
                <
                    bg::policies::relate::segments_intersection_points
                        <
                            bg::segment_intersection_points<P, bg::segment_ratio<coord_t> >
                        >,
                    bg::policies::relate::segments_direction
                > policy_t;

    bg::strategy::intersection::relate_geodesic_segments
        <
            policy_t,
            bg::srs::spheroid<double>,
            bg::formula::vincenty_inverse,
            4
        > strategy;

    test_strategy<S, S, P>(s1_wkt, s2_wkt, strategy, m, expected_count, ip0_wkt, ip1_wkt);
}

template <typename S, typename P>
void test_geodesic_thomas(std::string const& s1_wkt, std::string const& s2_wkt,
                          char m, std::size_t expected_count,
                          std::string const& ip0_wkt = "", std::string const& ip1_wkt = "")
{
    typedef typename bg::coordinate_type<P>::type coord_t;
    typedef bg::policies::relate::segments_tupled
                <
                    bg::policies::relate::segments_intersection_points
                        <
                            bg::segment_intersection_points<P, bg::segment_ratio<coord_t> >
                        >,
                    bg::policies::relate::segments_direction
                > policy_t;

    bg::strategy::intersection::relate_geodesic_segments
        <
            policy_t,
            bg::srs::spheroid<double>,
            bg::formula::thomas_inverse,
            2
        > strategy;

    test_strategy<S, S, P>(s1_wkt, s2_wkt, strategy, m, expected_count, ip0_wkt, ip1_wkt);
}

template <typename S, typename P>
void test_geodesic_andoyer(std::string const& s1_wkt, std::string const& s2_wkt,
                           char m, std::size_t expected_count,
                           std::string const& ip0_wkt = "", std::string const& ip1_wkt = "")
{
    typedef typename bg::coordinate_type<P>::type coord_t;
    typedef bg::policies::relate::segments_tupled
                <
                    bg::policies::relate::segments_intersection_points
                        <
                            bg::segment_intersection_points<P, bg::segment_ratio<coord_t> >
                        >,
                    bg::policies::relate::segments_direction
                > policy_t;

    bg::strategy::intersection::relate_geodesic_segments
        <
            policy_t,
            bg::srs::spheroid<double>,
            bg::formula::andoyer_inverse,
            1
        > strategy;

    test_strategy<S, S, P>(s1_wkt, s2_wkt, strategy, m, expected_count, ip0_wkt, ip1_wkt);
}


struct strategy_base
{
    strategy_base(char m_, std::size_t expected_count_, std::string const& wkt1_ = "", std::string const& wkt2_ = "")
        : m(m_), expected_count(expected_count_), wkt1(wkt1_), wkt2(wkt2_)
    {}
    char m;
    std::size_t expected_count;
    std::string wkt1, wkt2;
};
struct strategy_default : strategy_base
{
    strategy_default(char m, std::size_t expected_count, std::string const& wkt1 = "", std::string const& wkt2 = "")
        : strategy_base(m, expected_count, wkt1, wkt2)
    {}
};
struct geodesic_vincenty : strategy_base
{
    geodesic_vincenty(char m, std::size_t expected_count, std::string const& wkt1 = "", std::string const& wkt2 = "")
        : strategy_base(m, expected_count, wkt1, wkt2)
    {}
};
struct geodesic_thomas : strategy_base
{
    geodesic_thomas(char m, std::size_t expected_count, std::string const& wkt1 = "", std::string const& wkt2 = "")
        : strategy_base(m, expected_count, wkt1, wkt2)
    {}
};
struct geodesic_andoyer : strategy_base
{
    geodesic_andoyer(char m, std::size_t expected_count, std::string const& wkt1 = "", std::string const& wkt2 = "")
        : strategy_base(m, expected_count, wkt1, wkt2)
    {}
};
struct great_elliptic : strategy_base
{
    great_elliptic(char m, std::size_t expected_count, std::string const& wkt1 = "", std::string const& wkt2 = "")
        : strategy_base(m, expected_count, wkt1, wkt2)
    {}
};


template <typename S, typename P>
void test_strategy(std::string const& s1_wkt, std::string const& s2_wkt,
                   strategy_default const& s)
{
    test_default_strategy<S, P>(s1_wkt, s2_wkt, s.m, s.expected_count, s.wkt1, s.wkt2);
}

template <typename S, typename P>
void test_strategy(std::string const& s1_wkt, std::string const& s2_wkt,
                   great_elliptic const& s)
{
    test_great_elliptic<S, P>(s1_wkt, s2_wkt, s.m, s.expected_count, s.wkt1, s.wkt2);
}

template <typename S, typename P>
void test_strategy(std::string const& s1_wkt, std::string const& s2_wkt,
                   geodesic_vincenty const& s)
{
    test_geodesic_vincenty<S, P>(s1_wkt, s2_wkt, s.m, s.expected_count, s.wkt1, s.wkt2);
}

template <typename S, typename P>
void test_strategy(std::string const& s1_wkt, std::string const& s2_wkt,
                   geodesic_thomas const& s)
{
    test_geodesic_thomas<S, P>(s1_wkt, s2_wkt, s.m, s.expected_count, s.wkt1, s.wkt2);
}

template <typename S, typename P>
void test_strategy(std::string const& s1_wkt, std::string const& s2_wkt,
                   geodesic_andoyer const& s)
{
    test_geodesic_andoyer<S, P>(s1_wkt, s2_wkt, s.m, s.expected_count, s.wkt1, s.wkt2);
}


template <typename S, typename P, typename SR1>
void test_strategies(std::string const& s1_wkt, std::string const& s2_wkt,
                     SR1 const& sr1)
{
    test_strategy<S, P>(s1_wkt, s2_wkt, sr1);
}
template <typename S, typename P, typename SR1, typename SR2>
void test_strategies(std::string const& s1_wkt, std::string const& s2_wkt,
                     SR1 const& sr1, SR2 const& sr2)
{
    test_strategy<S, P>(s1_wkt, s2_wkt, sr1);
    test_strategy<S, P>(s1_wkt, s2_wkt, sr2);
}
template <typename S, typename P, typename SR1, typename SR2, typename SR3>
void test_strategies(std::string const& s1_wkt, std::string const& s2_wkt,
                     SR1 const& sr1, SR2 const& sr2, SR3 const& sr3)
{
    test_strategy<S, P>(s1_wkt, s2_wkt, sr1);
    test_strategy<S, P>(s1_wkt, s2_wkt, sr2);
    test_strategy<S, P>(s1_wkt, s2_wkt, sr3);
}
template <typename S, typename P, typename SR1, typename SR2, typename SR3, typename SR4>
void test_strategies(std::string const& s1_wkt, std::string const& s2_wkt,
                     SR1 const& sr1, SR2 const& sr2, SR3 const& sr3, SR4 const& sr4)
{
    test_strategy<S, P>(s1_wkt, s2_wkt, sr1);
    test_strategy<S, P>(s1_wkt, s2_wkt, sr2);
    test_strategy<S, P>(s1_wkt, s2_wkt, sr3);
    test_strategy<S, P>(s1_wkt, s2_wkt, sr4);
}


template <typename S, typename P>
void test_all_strategies(std::string const& s1_wkt, std::string const& s2_wkt,
                         char m, std::size_t expected_count,
                         std::string const& ip0_wkt = "", std::string const& ip1_wkt = "")
{
    test_default_strategy<S, P>(s1_wkt, s2_wkt, m, expected_count, ip0_wkt, ip1_wkt);
    test_great_elliptic<S, P>(s1_wkt, s2_wkt, m, expected_count, ip0_wkt, ip1_wkt);
    //test_experimental_elliptic<S, P>(s1_wkt, s2_wkt, m, expected_count, ip0_wkt, ip1_wkt);
    test_geodesic_vincenty<S, P>(s1_wkt, s2_wkt, m, expected_count, ip0_wkt, ip1_wkt);
    test_geodesic_thomas<S, P>(s1_wkt, s2_wkt, m, expected_count, ip0_wkt, ip1_wkt);
    test_geodesic_andoyer<S, P>(s1_wkt, s2_wkt, m, expected_count, ip0_wkt, ip1_wkt);
}

#endif // BOOST_GEOMETRY_TEST_STRATEGIES_SEGMENT_INTERSECTION_GEO_HPP
