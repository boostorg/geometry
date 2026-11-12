// Boost.Geometry
// Unit Test

// Copyright (c) 2026 Tinko Sebastian Bartels.

// Use, modification and distribution is subject to the Boost Software License,
// Version 1.0. (See accompanying file LICENSE_1_0.txt or copy at
// http://www.boost.org/LICENSE_1_0.txt)

#include <algorithm>
#include <cmath>
#include <string>
#include <utility>

#include <boost/geometry.hpp>
#include <boost/test/included/test_exec_monitor.hpp>

namespace bg = boost::geometry;

struct difference_case
{
    char const* name;
    char const* a;
    char const* b;
    double area_ab;
    double area_ba;
    std::size_t count_ab;
    std::size_t count_ba;
};

// Each case has one triangle and two point-touching triangles. Roundoff makes
// the whole-segment i/u classification disagree with the clustered counts.
// The missing components still left valid output, so validity alone is not enough.
static difference_case const cases[] =
{
    {"gridtri_71_min",
        R"wkt(MULTIPOLYGON(((1.4686431351901716 2.8005512567097899,0.7753420716962518 3.5211993229378207,2.189291201418202 3.4938523202037097,1.4686431351901716 2.8005512567097899))))wkt",
        R"wkt(MULTIPOLYGON(((1.1083191020761562 2.4539007249628302,1.8289671683041866 3.14720178845675,2.5222682317981064 2.4265537222287197,1.1083191020761562 2.4539007249628302)),((1.8289671683041866 3.14720178845675,2.5496152345322174 3.8405028519506694,3.2429162980261372 3.1198547857226391,1.8289671683041866 3.14720178845675))))wkt",
        .5, 1, 1, 2},
    {"gridtri_172_min",
        R"wkt(MULTIPOLYGON(((0.7071067811865479 3.535533905932738,4.440892098500626e-16 4.242640687119286,1.414213562373095 4.242640687119285,0.7071067811865479 3.535533905932738))))wkt",
        R"wkt(MULTIPOLYGON(((2.220446049250313e-16 3.535533905932738,0.7071067811865477 4.242640687119285,1.414213562373095 3.535533905932737,2.220446049250313e-16 3.535533905932738)),((0.7071067811865477 4.242640687119285,1.414213562373096 4.949747468305833,2.121320343559643 4.242640687119286,0.7071067811865477 4.242640687119285))))wkt",
        .25, .75, 2, 2},
    {"gridtri_293_min",
        R"wkt(MULTIPOLYGON(((2.94654135716604 -1.14799565787946,3.9453033301038 -1.1977402195267,2.89679679551881 -2.14675763081722,2.94654135716604 -1.14799565787946))))wkt",
        R"wkt(MULTIPOLYGON(((4.4695565973963 -0.723231513881436,4.41981203574906 -1.7219934868192,3.4210500628113 -1.67224892517196,4.4695565973963 -0.723231513881436)),((3.4210500628113 -1.67224892517196,3.37130550116407 -2.67101089810972,2.37254352822631 -2.62126633646248,3.4210500628113 -1.67224892517196))))wkt",
        .5, 1, 1, 2},
    // A colocated turn and its original input vertex differ by a few ulps.
    // Emitting both after restoring the continuation used to cross an edge.
    {"clustered_endpoint_detour",
        R"wkt(MULTIPOLYGON(((3.29034946283945 0.442220641121,4.0 1.0,4.628648324219 -0.0148965841393,3.29034946283945 0.442220641121))))wkt",
        R"wkt(MULTIPOLYGON(((2.62120003215 0.670779253751,3.9594988935293 0.21366202849086,3.0 -0.0,2.62120003215 0.670779253751)),((3.9594988935293 0.21366202849086,4.0 1.0,5.0 1.0,3.9594988935293 0.21366202849086))))wkt",
        .39964656162770207, .61965462536202409, 2, 2},
    {"clustered_endpoint_detour_scaled",
        R"wkt(MULTIPOLYGON(((-2709.6505371605485379 -3557.7793588790113972,-1811.9424938405381909 -3117.1885408192642899,-1371.3516757807908562 -4014.8965841392755465,-2709.6505371605485379 -3557.7793588790113972))))wkt",
        R"wkt(MULTIPOLYGON(((-3378.799967850428402 -3329.2207462488804595,-2040.5011064706695834 -3786.3379715091427897,-2938.2091497906812947 -4226.9287895688903518,-3378.799967850428402 -3329.2207462488804595)),((-2040.5011064706695834 -3786.3379715091427897,-2481.0919245304166907 -2888.6299281891319879,-1142.7930631506587815 -3345.7471534493961371,-2040.5011064706695834 -3786.3379715091427897))))wkt",
        250000, 750000, 2, 2},
};

template <typename Ring>
void rotate_start(Ring& ring, int offset)
{
    auto const end = ring.end() - (bg::closure<Ring>::value == bg::closed ? 1 : 0);
    std::rotate(ring.begin(), ring.begin() + offset, end);
    if (bg::closure<Ring>::value == bg::closed)
    {
        ring.back() = ring.front();
    }
}

template <typename Ring>
void subdivide(Ring& ring)
{
    auto const size = ring.size() - (bg::closure<Ring>::value == bg::closed ? 1 : 0);
    Ring result;
    for (std::size_t index = 0; index < size; ++index)
    {
        auto const& from = ring[index];
        auto const& to = ring[(index + 1) % size];
        for (int quarter = 0; quarter < 4; ++quarter)
        {
            result.emplace_back(bg::get<0>(from) + quarter / 4.0 * (bg::get<0>(to) - bg::get<0>(from)),
                bg::get<1>(from) + quarter / 4.0 * (bg::get<1>(to) - bg::get<1>(from)));
        }
    }
    if (bg::closure<Ring>::value == bg::closed)
    {
        result.push_back(result.front());
    }
    ring = std::move(result);
}

template <typename MultiPolygon>
void check_difference(MultiPolygon const& a, MultiPolygon const& b,
    double area, std::size_t count, std::string const& name)
{
    MultiPolygon result;
    bg::difference(a, b, result);
    BOOST_CHECK_MESSAGE(std::abs(bg::area(result) - area) < 1.0e-12 * (std::max)(1.0, std::abs(area)),
        name << ": area " << bg::area(result));
    BOOST_CHECK_MESSAGE(result.size() == count,
        name << ": component count " << result.size());
    BOOST_CHECK_MESSAGE(bg::is_valid(result), name << ": invalid result");

    // Independently check support with point-in-polygon predicates. The sample
    // points avoid the nearly coincident boundaries and cover every component.
    bg::model::box<typename bg::point_type<MultiPolygon>::type> box;
    bg::envelope(a, box);
    bg::expand(box, bg::return_envelope<decltype(box)>(b));
    bool support_ok = true;
    for (int x = 0; x < 13; ++x)
    {
        for (int y = 0; y < 13; ++y)
        {
            double const minx = bg::get<bg::min_corner, 0>(box);
            double const miny = bg::get<bg::min_corner, 1>(box);
            typename bg::point_type<MultiPolygon>::type point(
                minx + (x + .37) / 13 * (bg::get<bg::max_corner, 0>(box) - minx),
                miny + (y + .61) / 13 * (bg::get<bg::max_corner, 1>(box) - miny));
            support_ok = support_ok
                && bg::within(point, result) == (bg::within(point, a) && !bg::within(point, b));
        }
    }
    BOOST_CHECK_MESSAGE(support_ok, name << ": wrong support");
}

template <bool Clockwise, bool Closed>
void test_all()
{
    using point = bg::model::d2::point_xy<double>;
    using polygon = bg::model::polygon<point, Clockwise, Closed>;
    using multi_polygon = bg::model::multi_polygon<polygon>;

    // A severely displaced intersection must not introduce a self-crossing
    // component. Check this contract without enshrining its preexisting support
    // loss as a correct expected area or component count.
    multi_polygon displaced_a, displaced_b;
    bg::read_wkt(R"wkt(MULTIPOLYGON(((-6241079443.2641353607 -4733828107.3444166183,-6240297789.6712398529 -4733204394.5413455963,-6240921502.4743108749 -4732422740.9484510422,-6241079443.2641353607 -4733828107.3444166183))))wkt", displaced_a);
    bg::read_wkt(R"wkt(MULTIPOLYGON(((-6240376760.066151619 -4733907077.7393283844,-6240218819.2763280869 -4732501711.3433628082,-6241000472.869222641 -4733125424.1464338303,-6240376760.066151619 -4733907077.7393283844)),((-6241624185.672293663 -4732343770.5535383224,-6241000472.869222641 -4733125424.1464338303,-6240842532.0793991089 -4731720057.7504682541,-6241624185.672293663 -4732343770.5535383224))))wkt", displaced_b);
    bg::correct(displaced_a);
    bg::correct(displaced_b);
    BOOST_CHECK(bg::is_valid(displaced_a));
    BOOST_CHECK(bg::is_valid(displaced_b));
    for (bool reverse : {false, true})
    {
        auto const& left = reverse ? displaced_b : displaced_a;
        auto const& right = reverse ? displaced_a : displaced_b;
        multi_polygon result;
        bg::difference(left, right, result);
        // The counterclockwise b-a direction already has a separate validity
        // defect. The introduced crossing was a-b in all four models.
        if (!reverse || Clockwise)
        {
            BOOST_CHECK_MESSAGE(bg::is_valid(result), "displaced target: invalid result");
        }
        BOOST_CHECK(bg::area(result) >= 0);
        BOOST_CHECK(bg::area(result) <= bg::area(left) * (1 + 1.0e-12));
    }

    for (auto const& test : cases)
    {
        multi_polygon original_a, original_b;
        bg::read_wkt(test.a, original_a);
        bg::read_wkt(test.b, original_b);
        bg::correct(original_a);
        bg::correct(original_b);
        BOOST_CHECK(bg::is_valid(original_a));
        BOOST_CHECK(bg::is_valid(original_b));

        // Independently move all three ring starts, and permute the two members.
        // Clockwise/counterclockwise and open/closed models reverse or shift the
        // segment identifiers without changing the expected set difference.
        for (int start_a = 0; start_a < 3; ++start_a)
        for (int start_b0 = 0; start_b0 < 3; ++start_b0)
        for (int start_b1 = 0; start_b1 < 3; ++start_b1)
        for (bool reverse_members : {false, true})
        {
            auto a = original_a;
            auto b = original_b;
            rotate_start(bg::exterior_ring(a.front()), start_a);
            rotate_start(bg::exterior_ring(b.front()), start_b0);
            rotate_start(bg::exterior_ring(b.back()), start_b1);
            if (reverse_members)
            {
                std::reverse(b.begin(), b.end());
            }
            std::string const name = std::string(test.name) + "_"
                + std::to_string(Clockwise) + std::to_string(Closed) + "_"
                + std::to_string(start_a) + std::to_string(start_b0)
                + std::to_string(start_b1) + std::to_string(reverse_members);
            check_difference(a, b, test.area_ab, test.count_ab, name + " a-b");
            check_difference(b, a, test.area_ba, test.count_ba, name + " b-a");

            if (std::string(test.name) == "gridtri_71_min")
            {
                multi_polygon result;
                bg::difference(b, a, result);
                point const witness(2.55, 3.37);
                BOOST_CHECK(bg::within(witness, b));
                BOOST_CHECK(!bg::within(witness, a));
                BOOST_CHECK_MESSAGE(bg::within(witness, result), name << ": lost witness");
            }
            if (std::string(test.name) == "clustered_endpoint_detour"
                || std::string(test.name) == "clustered_endpoint_detour_scaled")
            {
                multi_polygon result;
                bg::difference(a, b, result);
                point const witness = std::string(test.name) == "clustered_endpoint_detour"
                    ? point(3.7, .55) : point(-2400, -3500);
                BOOST_CHECK(bg::within(witness, a));
                BOOST_CHECK(!bg::within(witness, b));
                BOOST_CHECK_MESSAGE(bg::within(witness, result), name << ": lost witness");
            }
        }

        if (std::string(test.name) == "gridtri_71_min")
        {
            // The continuation can cross intermediate collinear vertices on
            // either input. Do not require a single segment per travel path.
            for (int subdivisions = 1; subdivisions <= 3; ++subdivisions)
            {
                auto a = original_a;
                auto b = original_b;
                if ((subdivisions & 1) != 0)
                {
                    subdivide(bg::exterior_ring(a.front()));
                }
                if ((subdivisions & 2) != 0)
                {
                    for (auto& polygon : b)
                    {
                        subdivide(bg::exterior_ring(polygon));
                    }
                }
                BOOST_CHECK(bg::is_valid(a));
                BOOST_CHECK(bg::is_valid(b));
                check_difference(a, b, test.area_ab, test.count_ab, "subdivided a-b");
                check_difference(b, a, test.area_ba, test.count_ba, "subdivided b-a");
            }
        }
    }
}

int test_main(int, char* [])
{
    test_all<true, true>();
    test_all<false, true>();
    test_all<true, false>();
    test_all<false, false>();
    return 0;
}
