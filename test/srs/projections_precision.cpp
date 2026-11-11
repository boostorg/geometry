// Boost.Geometry
// Unit Test

// Copyright (c) 2026, Space Ocean Corp.

// Use, modification and distribution is subject to the Boost Software License,
// Version 1.0. (See accompanying file LICENSE_1_0.txt or copy at
// http://www.boost.org/LICENSE_1_0.txt)

// Precision of spherical projections close to the poles, the equator and the projection
// centres. Close to the poles the longitude is ill-conditioned, and a tolerance in degrees
// cannot tell the error of a projection from the rounding of its input, so the inverse is
// checked as a great-circle distance in metres. Expected values are from a 50-digit
// computation on the same input. All cases use a sphere of radius 6371000 m.


#include <geometry_test_common.hpp>

#include <iostream>
#include <string>

#include <boost/geometry/srs/projection.hpp>


namespace
{

typedef bg::model::point<double, 2, bg::cs::geographic<bg::degree> > ll_t;
typedef bg::model::point<double, 2, bg::cs::cartesian> xy_t;

// fwd_xy: compare x and y in metres; fwd_eqc (ob_tran with o_proj=eqc): compare x / R and
// y / R as longitude and latitude on the rotated sphere
enum fwd_check { fwd_xy, fwd_eqc };

struct precision_case
{
    char const* def;
    double lon, lat;
    double x, y;
    fwd_check fwd;
    bool inv;
    // for the inverse: at least 1e-6 m, more where moving the input by 1 ulp moves the exact
    // inverse by about as much (close to the poles of the pseudocylindrical projections)
    double inv_tolerance;
};

precision_case const precision_cases[] =
{
    // tmerc sphere: close to the equator acos() and asin() lost half the digits
    { "+proj=tmerc +R=6371000 +k=0.999", 2, 1e-7, 222212.59457055129, 0.011115144217327468, fwd_xy, true, 1.0e-06 },
    // stere sphere, polar: latitude from asin() lost precision near the pole
    { "+proj=stere +lat_0=90 +R=6371000 +k_0=0.994", 30, 89.9999999, 0.0055263875261430016, -0.0095719839775945559, fwd_xy, true, 1.0e-06 },
    // ortho sphere, oblique: latitude from asin() lost precision near the pole
    { "+proj=ortho +lat_0=45 +R=6371000", 10, 89.9999999, 0.0019308795231307598, 4504977.2951962777, fwd_xy, true, 1.0e-06 },
    // aeqd sphere, oblique: latitude from asin() lost precision near the pole
    { "+proj=aeqd +lat_0=-30 +R=6371000", -120, -89.9999999, -0.011644304801302344, -6671695.6042332702, fwd_xy, true, 1.0e-06 },
    // aeqd sphere, oblique: the forward lost half the digits near the centre (acos)
    { "+proj=aeqd +lat_0=45 +R=6371000", 2e-06, 45.000001, 0.15725337058322724, 0.11119492830453609, fwd_xy, true, 1.0e-06 },
    // laea sphere, oblique: latitude from asin() lost precision near the pole
    { "+proj=laea +lat_0=45 +R=6371000", 60, 89.9999999, 0.01042317987605592, 4876152.2900594383, fwd_xy, true, 1.0e-06 },
    // laea sphere, equatorial: points within 1e-10 * R of the centre came back on the equator
    { "+proj=laea +lat_0=0 +R=6371000", 1e-09, -2e-09, 0.00011119492664455874, -0.00022238985328911749, fwd_xy, true, 1.0e-06 },
    // gnom sphere, oblique: latitude from asin() lost precision near the pole
    { "+proj=gnom +lat_0=45 +R=6371000", 10, 89.9999999, 0.0027306760042264916, 6370999.9780988762, fwd_xy, true, 1.0e-06 },
    // gnom sphere, polar: points within 1e-10 * R of the centre came back at the centre
    { "+proj=gnom +lat_0=90 +R=6371000", 30, 89.999999999, 0.000055597665444302562, -0.000096297981331748516, fwd_xy, true, 1.0e-06 },
    // nsper: latitude from asin() lost precision near the pole
    { "+proj=nsper +lat_0=45 +R=6371000 +h=3000000", 100, 89.9999999, 0.0067512397377098711, 2777408.3169169292, fwd_xy, true, 1.0e-06 },
    // nsper: points within 1e-10 * R of the centre came back at the centre
    { "+proj=nsper +lat_0=45 +R=6371000 +h=3000000", 1e-09, 45.000000001, 0.000078626686662535913, 0.0001111945408016165, fwd_xy, true, 1.0e-06 },
    // ob_tran: the rotated latitude from asin() lost precision near the rotated pole
    { "+proj=ob_tran +o_proj=eqc +o_lat_p=40 +o_lon_p=10 +R=6371000", 180, 39.9999999, -18903137.529574985, 10007543.386890794, fwd_eqc, true, 1.0e-06 },
    // moll: near the poles the Newton iteration stopped early or gave up
    { "+proj=moll +R=6371000", -179, -89.99999, -591.0783918084689, -9009954.6009776294, fwd_xy, true, 2.9e-06 },
    // moll: near the poles the Newton iteration stopped early or gave up
    { "+proj=moll +R=6371000", 100, 89.99, 33021.103249211675, 9009905.5922611956, fwd_xy, true, 1.0e-06 },
    // moll: near the poles the Newton iteration stopped early or gave up
    { "+proj=moll +R=6371000", 45, 89.3, 252308.41375123064, 8995812.5734922552, fwd_xy, true, 1.0e-06 },
    // eck4: near the poles the Newton iteration stopped early
    { "+proj=eck4 +R=6371000", -150, 89.9999, -7042628.2805732957, 8451134.2278927005, fwd_xy, true, 1.8e-03 },
    // eck4: near the poles the Newton iteration stopped early
    { "+proj=eck4 +R=6371000", 60, -89.999, 2817110.4380865138, -8451134.2256175863, fwd_xy, true, 1.8e-04 },
    // putp2: near the poles the Newton iteration stopped early
    { "+proj=putp2 +R=6371000", 100, -89.9999, 26.820261630556856, -9481615.7309176723, fwd_xy, true, 1.0e-06 },
    // putp2: near the poles the Newton iteration stopped early
    { "+proj=putp2 +R=6371000", -60, 89.99, -1609.0354738766009, 9480819.1246850995, fwd_xy, true, 1.0e-06 },
    // vandg: close to the origin the formulas lost the digits; within 1e-10 of the equator or the central meridian y or lon was 0
    { "+proj=vandg +R=6371000", 1e-06, 1e-06, 0.11119492664455873, 0.11119492664455874, fwd_xy, true, 1.0e-06 },
    // vandg: close to the origin the formulas lost the digits; within 1e-10 of the equator or the central meridian y or lon was 0
    { "+proj=vandg +R=6371000", 10, 3e-09, 1111949.2664455874, 0.00033358477993369337, fwd_xy, false, 1.0e-06 },
    // vandg: close to the origin the formulas lost the digits; within 1e-10 of the equator or the central meridian y or lon was 0
    { "+proj=vandg +R=6371000", 1e-08, 0.5, 0.0011119406864577287, 55597.892321672304, fwd_xy, true, 1.0e-06 },
    // wintri: close to the origin the forward lost half the digits (acos) and the inverse failed
    { "+proj=wintri +R=6371000", 1e-06, 4e-07, 0.090991907766723808, 0.044477970657823493, fwd_xy, true, 1.0e-06 },
    // wintri: close to the origin the forward lost half the digits (acos) and the inverse failed
    { "+proj=wintri +R=6371000", 1e-09, 1e-09, 0.000090991907766723819, 0.00011119492664455874, fwd_xy, true, 1.0e-06 },
    // aitoff: close to the origin the forward lost half the digits (acos) and the inverse failed
    { "+proj=aitoff +R=6371000", 1e-06, 4e-07, 0.11119492664455873, 0.044477970657823493, fwd_xy, true, 1.0e-06 },
    // aitoff: close to the origin the forward lost half the digits (acos) and the inverse failed
    { "+proj=aitoff +R=6371000", 1e-09, 1e-09, 0.00011119492664455874, 0.00011119492664455874, fwd_xy, true, 1.0e-06 },
    // hammer: the inverse ignored +M, and lost precision near the poles (asin)
    { "+proj=hammer +R=6371000 +M=2", 30, 40, 5417081.7750169166, 2195294.0700680249, fwd_xy, true, 1.0e-06 },
    // hammer: the inverse ignored +M, and lost precision near the poles (asin)
    { "+proj=hammer +R=6371000 +W=0.25 +M=2", -150, 80, -7143380.5118388616, 4159277.561056562, fwd_xy, true, 1.0e-06 },
    // hammer: the inverse ignored +M, and lost precision near the poles (asin)
    { "+proj=hammer +R=6371000", -100, -89.99999, -2.4092613216098564, -9009954.1004762939, fwd_xy, true, 1.0e-06 }
};

// gnom in equatorial and oblique mode close to 90 degrees from the centre, where the coordinates
// are very large and cos(z) from sqrt(1 - sin(z)^2) lost its digits. (x, y) is the exact forward
// image rounded to double, and (lon, lat) the exact inverse of those doubles (50-digit computation).
struct inverse_case
{
    char const* def;
    double x, y;
    double lon, lat;
};

inverse_case const inverse_cases[] =
{
    { "+proj=gnom +lat_0=0 +R=6371000", -0.011119492664455874, -3650314329490979.0, -1e-07, -89.9999999 },
    { "+proj=gnom +lat_0=0 +R=6371000", 1123379.1940936304, 3706626311913009.5, 10.000000000000002, 89.9999999 },
    { "+proj=gnom +lat_0=0 +R=6371000", 0.0, 36503141116197.78, 0.0, 89.99999 },
    { "+proj=gnom +lat_0=45 +R=6371000", 516232372521323.0, 6371000.0, 90.0, 1e-06 },
    { "+proj=gnom +lat_0=45 +R=6371000", 0.0, -3650314070120549.0, 0.0, -44.9999999 },
    { "+proj=gnom +lat_0=-30 +R=6371000", 0.0, 3650314070120549.0, 0.0, 59.9999999 }
};

double const earth_radius = 6371000.0;
double const tolerance = 1e-6; // metres, for the forward

// great-circle distance in metres between two points given in radians
double distance(double lam1, double phi1, double lam2, double phi2)
{
    double const a = sin((phi2 - phi1) / 2);
    double const b = sin((lam2 - lam1) / 2);
    return 2 * earth_radius * asin(sqrt(a * a + cos(phi1) * cos(phi2) * b * b));
}

void test_case(precision_case const& c)
{
    double const d2r = bg::math::d2r<double>();
    bg::srs::projection<> prj = bg::srs::proj4(c.def);

    xy_t xy;
    prj.forward(ll_t(c.lon, c.lat), xy);
    double const fwd_err = c.fwd == fwd_eqc
        ? distance(bg::get<0>(xy) / earth_radius, bg::get<1>(xy) / earth_radius,
                   c.x / earth_radius, c.y / earth_radius)
        : bg::math::sqrt(bg::math::sqr(bg::get<0>(xy) - c.x) + bg::math::sqr(bg::get<1>(xy) - c.y));
    BOOST_CHECK_MESSAGE(fwd_err < tolerance,
                        std::setprecision(17) << c.def << " forward (" << c.lon << ", " << c.lat
                        << "): (" << bg::get<0>(xy) << ", " << bg::get<1>(xy) << ") is " << fwd_err << " m off");

    if (c.inv)
    {
        ll_t ll;
        prj.inverse(xy_t(c.x, c.y), ll);
        double const inv_err = distance(bg::get<0>(ll) * d2r, bg::get<1>(ll) * d2r,
                                        c.lon * d2r, c.lat * d2r);
        BOOST_CHECK_MESSAGE(inv_err < c.inv_tolerance,
                            std::setprecision(17) << c.def << " inverse (" << c.x << ", " << c.y
                            << "): (" << bg::get<0>(ll) << ", " << bg::get<1>(ll) << ") is " << inv_err << " m off");
    }
}

void test_inverse_case(inverse_case const& c)
{
    double const d2r = bg::math::d2r<double>();
    bg::srs::projection<> prj = bg::srs::proj4(c.def);

    ll_t ll;
    prj.inverse(xy_t(c.x, c.y), ll);
    double const inv_err = distance(bg::get<0>(ll) * d2r, bg::get<1>(ll) * d2r,
                                    c.lon * d2r, c.lat * d2r);
    BOOST_CHECK_MESSAGE(inv_err < tolerance,
                        std::setprecision(17) << c.def << " inverse (" << c.x << ", " << c.y
                        << "): (" << bg::get<0>(ll) << ", " << bg::get<1>(ll) << ") is " << inv_err << " m off");
}

} // namespace

int test_main(int, char*[])
{
    for (precision_case const& c : precision_cases)
    {
        test_case(c);
    }

    for (inverse_case const& c : inverse_cases)
    {
        test_inverse_case(c);
    }

    return 0;
}
