// Boost.Geometry
// Unit Test

// Copyright (c) 2024 Barend Gehrels, Amsterdam, the Netherlands.

// Use, modification and distribution is subject to the Boost Software License,
// Version 1.0. (See accompanying file LICENSE_1_0.txt or copy at
// http://www.boost.org/LICENSE_1_0.txt)

#include <boost/geometry.hpp>

#include <algorithms/overlay/overlay_cases.hpp>
#include <algorithms/overlay/multi_overlay_cases.hpp>

#include <boost/test/included/test_exec_monitor.hpp>

// Use another alias, on purpose it is not "bg::" here
namespace bgeo = boost::geometry;

// For convenience and to test current failures
#if defined(BOOST_GEOMETRY_TEST_FAILURES)
#define BG_IF_TEST_FAILURES true
#else
#define BG_IF_TEST_FAILURES false
#endif

struct ut_settings
{
    explicit ut_settings()
    {}

    inline ut_settings& ignore_reverse()
    {
        test_reverse = BG_IF_TEST_FAILURES;
        return *this;
    }

    inline ut_settings& ignore_diff()
    {
        test_difference = BG_IF_TEST_FAILURES;
        return *this;
    }

    inline ut_settings& ignore_validity()
    {
        test_validity = BG_IF_TEST_FAILURES;
        return *this;
    }

    inline ut_settings& ignore_validity_union()
    {
        test_validity_union = BG_IF_TEST_FAILURES;
        return *this;
    }

    inline ut_settings& ignore_validity_intersection()
    {
        test_validity_intersection = BG_IF_TEST_FAILURES;
        return *this;
    }

    inline ut_settings& ignore_validity_diff()
    {
        test_validity_diff1 = BG_IF_TEST_FAILURES;
        test_validity_diff2 = BG_IF_TEST_FAILURES;
        test_validity_diff_sym = BG_IF_TEST_FAILURES;
        return *this;
    }

    inline ut_settings& ignore_validity_diff1()
    {
        test_validity_diff1 = BG_IF_TEST_FAILURES;
        return *this;
    }

    inline ut_settings& ignore_validity_diff2()
    {
        test_validity_diff2 = BG_IF_TEST_FAILURES;
        return *this;
    }

    inline ut_settings& ignore_validity_diff_sym()
    {
        test_validity_diff_sym = BG_IF_TEST_FAILURES;
        return *this;
    }

    inline ut_settings& set_epsilon(long double epsilon)
    {
        epsilon = epsilon;
        return *this;
    }

    bool test_reverse{true};
    bool test_difference{true};

    bool test_validity{true};
    bool test_validity_union{true};
    bool test_validity_intersection{true};
    bool test_validity_diff1{true};
    bool test_validity_diff2{true};
    bool test_validity_diff_sym{true};

    long double epsilon{1.0e-6};
};
template <typename T, typename Multi, typename Geometry1, typename Geometry2>
void test_detail(std::string const& name, std::string const& wkt1, std::string const& wkt2,
    ut_settings const& settings)
{
    Geometry1 geometry1;
    Geometry2 geometry2;
    bgeo::read_wkt(wkt1, geometry1);
    bgeo::read_wkt(wkt2, geometry2);
    bgeo::correct(geometry1);
    bgeo::correct(geometry2);

    Multi result_intersection, result_union;
    Multi result_intersection_rev, result_union_rev;
    Multi diff1, diff2, sym_diff;

    bgeo::intersection(geometry1, geometry2, result_intersection);
    bgeo::intersection(geometry2, geometry1, result_intersection_rev);
    bgeo::union_(geometry1, geometry2, result_union);
    bgeo::union_(geometry2, geometry1, result_union_rev);
    bgeo::difference(geometry1, geometry2, diff1);
    bgeo::difference(geometry2, geometry1, diff2);
    bgeo::sym_difference(geometry1, geometry2, sym_diff);

    auto const area1 = bgeo::area(geometry1);
    auto const area2 = bgeo::area(geometry2);
    auto const area_union = bgeo::area(result_union);
    auto const area_intersection = bgeo::area(result_intersection);
    auto const area_diff1 = bgeo::area(diff1);
    auto const area_diff2 = bgeo::area(diff2);
    auto const area_sym_diff = bgeo::area(sym_diff);

    auto const balance = (area1 + area2) - area_union - area_intersection;
    auto const balance_d1 = (area_union - area2) - area_diff1;
    auto const balance_d2 = (area_union - area1) - area_diff2;
    auto const balance_sym = (area_diff1 + area_diff2) - area_sym_diff;

    auto const eps = settings.epsilon;

    BOOST_CHECK_MESSAGE(area_union + eps >= area1 && area_union + eps >= area2 ,
        "Case: " << name << " wrong union " << area_union << " vs " << area1 << " and " << area2);
    BOOST_CHECK_MESSAGE(area_intersection - eps <= area1  && area_intersection - eps <= area2,
        "Case: " << name << " wrong intersection " << area_intersection << " vs " << area1 << " and " << area2);

    BOOST_CHECK_MESSAGE(bgeo::math::abs(balance) < eps,
        "Case: " << name << " wrong union or intersection " << balance);
    if (settings.test_difference)
    {
        BOOST_CHECK_MESSAGE(bgeo::math::abs(balance_d1) < eps,
            "Case: " << name << " wrong difference (a-b) " << balance_d1);
        BOOST_CHECK_MESSAGE(bgeo::math::abs(balance_d2) < eps,
            "Case: " << name << " wrong difference (b-a) " << balance_d2);
        BOOST_CHECK_MESSAGE(bgeo::math::abs(balance_sym) < eps,
            "Case: " << name << " wrong symmetric difference " << balance_sym);
    }

    if (settings.test_reverse)
    {
        BOOST_CHECK_MESSAGE(bgeo::math::abs(area_union - bgeo::area(result_union_rev)) < eps,
            "Case: " << name << " wrong union reversed: " << area_union << " != " << bgeo::area(result_union_rev));
        BOOST_CHECK_MESSAGE(bgeo::math::abs(area_intersection - bgeo::area(result_intersection_rev)) < eps,
            "Case: " << name << " wrong intersection reversed: " << area_intersection << " != " << bgeo::area(result_intersection_rev));
    }

    if (settings.test_validity)
    {
        BOOST_CHECK_MESSAGE(bgeo::is_valid(geometry1),
            "Case: " << name << " geometry1 is not valid");
        BOOST_CHECK_MESSAGE(bgeo::is_valid(geometry2),
            "Case: " << name << " geometry2 is not valid");
        if (settings.test_validity_union)
        {
            BOOST_CHECK_MESSAGE(bgeo::is_valid(result_union),
                "Case: " << name << " union is not valid");
        }
        if (settings.test_validity_intersection)
        {
            BOOST_CHECK_MESSAGE(bgeo::is_valid(result_intersection),
                "Case: " << name << " intersection is not valid");
        }
        if (settings.test_validity_diff1)
        {
            BOOST_CHECK_MESSAGE(bgeo::is_valid(diff1),
                "Case: " << name << " difference (a-b) is not valid");
        }
        if (settings.test_validity_diff2)
        {
            BOOST_CHECK_MESSAGE(bgeo::is_valid(diff2),
                "Case: " << name << " difference (b-a) is not valid");
        }
        if (settings.test_validity_diff_sym)
        {
            BOOST_CHECK_MESSAGE(bgeo::is_valid(sym_diff),
                "Case: " << name << " symmetric difference is not valid");
        }
    }
}

template <typename T>
void test(std::string const& name, std::string const& wkt1, std::string const& wkt2,
    ut_settings const& settings)
{
	using point_t = bgeo::model::point<T, 2, bgeo::cs::cartesian>;
	using polygon_t = bgeo::model::polygon<point_t>;
	using mp_t = bgeo::model::multi_polygon<polygon_t>;

    constexpr auto multipolygon_tag = "MULTIPOLYGON";
    bool const is_polygon1 = ! boost::icontains(wkt1, multipolygon_tag);
    bool const is_polygon2 = ! boost::icontains(wkt2, multipolygon_tag);

    if (is_polygon1 && is_polygon2)
    {
        test_detail<T, mp_t, polygon_t, polygon_t>(name, wkt1, wkt2, settings);
    }
    else if (! is_polygon1 && ! is_polygon2)
    {
        test_detail<T, mp_t, mp_t, mp_t>(name, wkt1, wkt2, settings);
    }
    else if (is_polygon1)
    {
        test_detail<T, mp_t, polygon_t, mp_t>(name, wkt1, wkt2, settings);
    }
    else
    {
        test_detail<T, mp_t, mp_t, polygon_t>(name, wkt1, wkt2, settings);
    }
}

void test_all(std::string const& name, std::string const& wkt1, std::string const& wkt2,
    ut_settings const& settings = ut_settings())
{
    test<double>(name, wkt1, wkt2, settings);
}

#define TEST_CASE(caseid) (test_all(#caseid, caseid[0], caseid[1]))
#define TEST_CASE_WITH(caseid, index1, index2, settings) (test_all(#caseid "_" #index1 "_" #index2, caseid[index1], caseid[index2], settings))

int test_main(int, char* [])
{
    TEST_CASE_WITH(case_141_multi, 0, 1, ut_settings().ignore_reverse());
    TEST_CASE(case_142_multi);
    TEST_CASE(case_143_multi);
    TEST_CASE(case_144_multi);
    TEST_CASE(case_145_multi);
    TEST_CASE_WITH(case_146_multi, 0, 1, ut_settings().ignore_validity_intersection());
    TEST_CASE(case_147_multi);
    TEST_CASE(case_148_multi);
    TEST_CASE(case_149_multi);

    TEST_CASE_WITH(issue_1221, 0, 1, ut_settings().ignore_validity_diff());

    TEST_CASE(issue_1222);
    TEST_CASE_WITH(issue_1226, 0, 1, ut_settings().ignore_validity_diff());
    TEST_CASE(issue_1231);
    TEST_CASE_WITH(issue_1241, 0, 1, ut_settings().ignore_diff());
    TEST_CASE(issue_1244);
    TEST_CASE_WITH(issue_1288, 0, 1, ut_settings().ignore_validity_diff());
    TEST_CASE_WITH(issue_1288, 0, 2, ut_settings());
    TEST_CASE(issue_1293);
    TEST_CASE_WITH(issue_1295, 0, 1, ut_settings().ignore_validity_diff());
    TEST_CASE(issue_1299);

    TEST_CASE(issue_1326);
    TEST_CASE(issue_1342_a);
    TEST_CASE(issue_1342_b);

    TEST_CASE(issue_1345_a);
    TEST_CASE(issue_1345_b);
    TEST_CASE_WITH(issue_1345_a, 1, 0, ut_settings());
    TEST_CASE_WITH(issue_1345_b, 1, 0, ut_settings());

    TEST_CASE_WITH(issue_1349, 0, 1, ut_settings().ignore_diff());
    TEST_CASE(issue_1349_inverse);

#if defined(BOOST_GEOMETRY_TEST_FAILURES)
    // Fails in nearly all operations
    TEST_CASE(issue_1363);
#endif

    TEST_CASE(case_bitset_1);
    TEST_CASE(case_bitset_2);
#if defined(BOOST_GEOMETRY_TEST_FAILURES)
    // Needs a fix in assign_parents
    TEST_CASE(case_bitset_3);
#endif

    TEST_CASE(case_recursive_boxes_89);
    TEST_CASE(case_recursive_boxes_90);
#if defined(BOOST_GEOMETRY_TEST_FAILURES)
    TEST_CASE(case_recursive_boxes_91);
#endif
    TEST_CASE(case_recursive_boxes_92);
    TEST_CASE(case_recursive_boxes_93);
    TEST_CASE(case_recursive_boxes_94);
    TEST_CASE(case_recursive_boxes_95);
    TEST_CASE(case_recursive_boxes_96);
    TEST_CASE(case_recursive_boxes_97);
    TEST_CASE(case_recursive_boxes_98);
    TEST_CASE(case_recursive_boxes_99);
    TEST_CASE_WITH(case_recursive_boxes_99, 2, 3, ut_settings());

#if defined(BOOST_GEOMETRY_TEST_FAILURES)
    // Fails in union or intersection, and in difference. Also the union is invalid.
    TEST_CASE_WITH(issue_1354, 0, 1, ut_settings().ignore_validity_union().ignore_diff());
#endif

    return 0;
}
