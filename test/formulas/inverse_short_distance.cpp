// Short-distance accuracy test for inverse formulas.
//
// Reference for the regression in Andoyer: https://github.com/boostorg/geometry/issues/1217
// Fix proposed in PR #1461.
//
// On develop the pr_1461 case fails: andoyer returns ~9 cm for a true
// distance of ~0.67 mm. After the PR, andoyer agrees with the reference values
// from GeographicLib within 1% across the whole range tested here.

#include <geometry_test_common.hpp>

#include <boost/geometry/formulas/andoyer_inverse.hpp>
#include <boost/geometry/formulas/karney_inverse.hpp>
//#include <boost/geometry/formulas/thomas_inverse.hpp>
#include <boost/geometry/formulas/vincenty_inverse.hpp>
#include <boost/geometry/srs/spheroid.hpp>
#include <boost/geometry/util/math.hpp>

namespace
{

struct short_case
{
    char const* name;
    double lon1, lat1, lon2, lat2;
    double distance_expected;
};

void test_short(short_case const& c)
{
    double const d2r = bg::math::d2r<double>();
    bg::srs::spheroid<double> const spheroid; // WGS84 by default

    using andoyer_t = bg::formula::andoyer_inverse<double, true, false, false, false, false>;
    //using thomas_t = bg::formula::thomas_inverse<double, true, false, false, false, false>;
    using vincenty_t = bg::formula::vincenty_inverse<double, true, false, false, false, false>;
    //using karney_t = bg::formula::karney_inverse<double, true, false, false, false, false>;

    double const lon1r = c.lon1 * d2r;
    double const lat1r = c.lat1 * d2r;
    double const lon2r = c.lon2 * d2r;
    double const lat2r = c.lat2 * d2r;

    double const distance_andoyer = andoyer_t::apply(lon1r, lat1r, lon2r, lat2r, spheroid).distance;
    //double const distance_thomas = thomas_t::apply(lon1r, lat1r, lon2r, lat2r, spheroid).distance;
    double const distance_vincenty = vincenty_t::apply(lon1r, lat1r, lon2r, lat2r, spheroid).distance;
    //double const distance_karney = karney_t::apply(lon1r, lat1r, lon2r, lat2r, spheroid).distance;

    double const percent_tolerance = 1.0;  // allow error of 1%
    BOOST_TEST_INFO_SCOPE(c.name);
    BOOST_CHECK_CLOSE(distance_andoyer, c.distance_expected, percent_tolerance);
    //BOOST_CHECK_CLOSE(distance_thomas, c.distance_expected, percent_tolerance); // TODO: Thomas is very inaccurate
    BOOST_CHECK_CLOSE(distance_vincenty, c.distance_expected, percent_tolerance);
    //BOOST_CHECK_CLOSE(distance_karney, c.distance_expected, percent_tolerance); // TODO: Karney is inaccurate at short distances (returns 0 for short meridian steps)
}

} // namespace

int test_main(int, char*[])
{
    // reference expected distance values obtained with GeodSolve/GeographicLib

    // Marquee case from PR #1461: ~0.67 mm true distance at mid-latitude.
    // Develop returns ~9 cm here with Andoyer (the regression the PR fixes).
    test_short({"pr_1461",         8.81, 53.08, 8.81000001, 53.08, 0.0006701306 });

    // East-west steps at the equator, sweeping sub-mm to ~10 m.
    test_short({"sub_mm_equator",  0.0,  0.0, 1.0e-9, 0.0, 0.0001113195 });
    test_short({"mm_equator",      0.0,  0.0, 1.0e-8, 0.0, 0.0011131949 });
    test_short({"cm_equator",      0.0,  0.0, 1.0e-7, 0.0, 0.0111319491 });
    test_short({"m_equator",       0.0,  0.0, 1.0e-5, 0.0, 1.1131949079 });
    test_short({"10m_equator",     0.0,  0.0, 1.0e-4, 0.0, 11.1319490793 });

    // North-south steps along a meridian.
    test_short({"sub_mm_meridian", 0.0, 45.0, 0.0, 45.0 + 1.0e-9, 0.0001111315 });
    test_short({"mm_meridian",     0.0, 45.0, 0.0, 45.0 + 1.0e-8, 0.0011113192 });
    test_short({"cm_meridian",     0.0, 45.0, 0.0, 45.0 + 1.0e-7, 0.0111131792 });
    test_short({"m_meridian",      0.0, 45.0, 0.0, 45.0 + 1.0e-5, 1.1113177758 });
    test_short({"10mm_meridian",   0.0, 45.0, 0.0, 45.0 + 1.0e-4, 11.113177840 });

    // High-latitude oblique step (cos(lat) is small, so longitude differences shrink).
    test_short({"oblique_70N",     10.0, 70.0, 10.0 + 1.0e-7, 70.0 + 1.0e-7, 0.0117916474 });

    return 0;
}
