// Boost.Geometry (aka GGL, Generic Geometry Library)
// Unit Test

// Copyright (c) 2015 Barend Gehrels, Amsterdam, the Netherlands.

// This file was modified by Oracle on 2017.
// Modifications copyright (c) 2017, Oracle and/or its affiliates.
// Contributed and/or modified by Adam Wulkiewicz, on behalf of Oracle

// Use, modification and distribution is subject to the Boost Software License,
// Version 1.0. (See accompanying file LICENSE_1_0.txt or copy at
// http://www.boost.org/LICENSE_1_0.txt)


#if defined(_MSC_VER)
#pragma warning( disable : 4305 ) // truncation double -> float
#endif // defined(_MSC_VER)

#include <geometry_test_common.hpp>

#include <boost/geometry/extensions/gis/projections/parameters.hpp>
#include <boost/geometry/extensions/gis/projections/projection.hpp>

#include <boost/geometry/extensions/gis/projections/proj/aea.hpp>
#include <boost/geometry/extensions/gis/projections/proj/aeqd.hpp>
#include <boost/geometry/extensions/gis/projections/proj/airy.hpp>
#include <boost/geometry/extensions/gis/projections/proj/aitoff.hpp>
#include <boost/geometry/extensions/gis/projections/proj/august.hpp>
#include <boost/geometry/extensions/gis/projections/proj/bacon.hpp>
#include <boost/geometry/extensions/gis/projections/proj/bipc.hpp>
#include <boost/geometry/extensions/gis/projections/proj/boggs.hpp>
#include <boost/geometry/extensions/gis/projections/proj/bonne.hpp>
#include <boost/geometry/extensions/gis/projections/proj/cass.hpp>
#include <boost/geometry/extensions/gis/projections/proj/cc.hpp>
#include <boost/geometry/extensions/gis/projections/proj/cea.hpp>
#include <boost/geometry/extensions/gis/projections/proj/chamb.hpp>
#include <boost/geometry/extensions/gis/projections/proj/collg.hpp>
#include <boost/geometry/extensions/gis/projections/proj/crast.hpp>
#include <boost/geometry/extensions/gis/projections/proj/denoy.hpp>
#include <boost/geometry/extensions/gis/projections/proj/eck1.hpp>
#include <boost/geometry/extensions/gis/projections/proj/eck2.hpp>
#include <boost/geometry/extensions/gis/projections/proj/eck3.hpp>
#include <boost/geometry/extensions/gis/projections/proj/eck4.hpp>
#include <boost/geometry/extensions/gis/projections/proj/eck5.hpp>
#include <boost/geometry/extensions/gis/projections/proj/eqc.hpp>
#include <boost/geometry/extensions/gis/projections/proj/eqdc.hpp>
#include <boost/geometry/extensions/gis/projections/proj/etmerc.hpp>
#include <boost/geometry/extensions/gis/projections/proj/fahey.hpp>
#include <boost/geometry/extensions/gis/projections/proj/fouc_s.hpp>
#include <boost/geometry/extensions/gis/projections/proj/gall.hpp>
#include <boost/geometry/extensions/gis/projections/proj/geocent.hpp>
#include <boost/geometry/extensions/gis/projections/proj/geos.hpp>
#include <boost/geometry/extensions/gis/projections/proj/gins8.hpp>
#include <boost/geometry/extensions/gis/projections/proj/gn_sinu.hpp>
#include <boost/geometry/extensions/gis/projections/proj/gnom.hpp>
#include <boost/geometry/extensions/gis/projections/proj/goode.hpp>
#include <boost/geometry/extensions/gis/projections/proj/gstmerc.hpp>
#include <boost/geometry/extensions/gis/projections/proj/hammer.hpp>
#include <boost/geometry/extensions/gis/projections/proj/hatano.hpp>
#include <boost/geometry/extensions/gis/projections/proj/healpix.hpp>
#include <boost/geometry/extensions/gis/projections/proj/krovak.hpp>
//#include <boost/geometry/extensions/gis/projections/proj/igh.hpp> -> in combined
#include <boost/geometry/extensions/gis/projections/proj/imw_p.hpp>
#include <boost/geometry/extensions/gis/projections/proj/isea.hpp>
#include <boost/geometry/extensions/gis/projections/proj/laea.hpp>
#include <boost/geometry/extensions/gis/projections/proj/labrd.hpp>
#include <boost/geometry/extensions/gis/projections/proj/lagrng.hpp>
#include <boost/geometry/extensions/gis/projections/proj/larr.hpp>
#include <boost/geometry/extensions/gis/projections/proj/lask.hpp>
#include <boost/geometry/extensions/gis/projections/proj/latlong.hpp>
#include <boost/geometry/extensions/gis/projections/proj/lcc.hpp>
#include <boost/geometry/extensions/gis/projections/proj/lcca.hpp>
#include <boost/geometry/extensions/gis/projections/proj/loxim.hpp>
#include <boost/geometry/extensions/gis/projections/proj/lsat.hpp>
#include <boost/geometry/extensions/gis/projections/proj/mbtfpp.hpp>
#include <boost/geometry/extensions/gis/projections/proj/mbtfpq.hpp>
#include <boost/geometry/extensions/gis/projections/proj/mbt_fps.hpp>
#include <boost/geometry/extensions/gis/projections/proj/merc.hpp>
#include <boost/geometry/extensions/gis/projections/proj/mill.hpp>
#include <boost/geometry/extensions/gis/projections/proj/mod_ster.hpp>
#include <boost/geometry/extensions/gis/projections/proj/moll.hpp>
#include <boost/geometry/extensions/gis/projections/proj/natearth.hpp>
#include <boost/geometry/extensions/gis/projections/proj/nell.hpp>
#include <boost/geometry/extensions/gis/projections/proj/nell_h.hpp>
#include <boost/geometry/extensions/gis/projections/proj/nocol.hpp>
#include <boost/geometry/extensions/gis/projections/proj/nsper.hpp>
#include <boost/geometry/extensions/gis/projections/proj/nzmg.hpp>
//#include <boost/geometry/extensions/gis/projections/proj/ob_tran.hpp> -> in combined
#include <boost/geometry/extensions/gis/projections/proj/ocea.hpp>
#include <boost/geometry/extensions/gis/projections/proj/oea.hpp>
#include <boost/geometry/extensions/gis/projections/proj/omerc.hpp>
#include <boost/geometry/extensions/gis/projections/proj/ortho.hpp>
#include <boost/geometry/extensions/gis/projections/proj/qsc.hpp>
#include <boost/geometry/extensions/gis/projections/proj/poly.hpp>
#include <boost/geometry/extensions/gis/projections/proj/putp2.hpp>
#include <boost/geometry/extensions/gis/projections/proj/putp3.hpp>
#include <boost/geometry/extensions/gis/projections/proj/putp4p.hpp>
#include <boost/geometry/extensions/gis/projections/proj/putp5.hpp>
#include <boost/geometry/extensions/gis/projections/proj/putp6.hpp>
#include <boost/geometry/extensions/gis/projections/proj/robin.hpp>
#include <boost/geometry/extensions/gis/projections/proj/rouss.hpp>
#include <boost/geometry/extensions/gis/projections/proj/rpoly.hpp>
#include <boost/geometry/extensions/gis/projections/proj/sconics.hpp>
#include <boost/geometry/extensions/gis/projections/proj/somerc.hpp>
#include <boost/geometry/extensions/gis/projections/proj/stere.hpp>
#include <boost/geometry/extensions/gis/projections/proj/sterea.hpp>
#include <boost/geometry/extensions/gis/projections/proj/sts.hpp>
#include <boost/geometry/extensions/gis/projections/proj/tcc.hpp>
#include <boost/geometry/extensions/gis/projections/proj/tcea.hpp>
#include <boost/geometry/extensions/gis/projections/proj/tmerc.hpp>
#include <boost/geometry/extensions/gis/projections/proj/tpeqd.hpp>
#include <boost/geometry/extensions/gis/projections/proj/urm5.hpp>
#include <boost/geometry/extensions/gis/projections/proj/urmfps.hpp>
#include <boost/geometry/extensions/gis/projections/proj/vandg.hpp>
#include <boost/geometry/extensions/gis/projections/proj/vandg2.hpp>
#include <boost/geometry/extensions/gis/projections/proj/vandg4.hpp>
#include <boost/geometry/extensions/gis/projections/proj/wag2.hpp>
#include <boost/geometry/extensions/gis/projections/proj/wag3.hpp>
#include <boost/geometry/extensions/gis/projections/proj/wag7.hpp>
#include <boost/geometry/extensions/gis/projections/proj/wink1.hpp>
#include <boost/geometry/extensions/gis/projections/proj/wink2.hpp>


#include <boost/geometry/core/coordinate_type.hpp>
#include <boost/geometry/algorithms/make.hpp>

#include <boost/geometry.hpp>
#include <boost/geometry/geometries/geometries.hpp>
#include <boost/geometry/geometries/point_xy.hpp>
#include <boost/geometry/extensions/gis/latlong/point_ll.hpp>


template <template <typename, typename> class Projection, typename GeoPoint>
void test_forward(GeoPoint const& geo_point1, GeoPoint const& geo_point2,
        std::string const& parameters, int deviation = 1)
{
    typedef typename bg::coordinate_type<GeoPoint>::type coordinate_type;
    typedef bg::model::d2::point_xy<coordinate_type> cartesian_point_type;
    typedef Projection<coordinate_type, bg::projections::parameters> projection_type;

    try
    {
        bg::srs::dynamic bgp; //TODO: TEMP
        bg::projections::parameters par = bg::projections::detail::pj_init_plus(bgp, parameters);

        projection_type prj(par);

        cartesian_point_type xy1, xy2;
        prj.forward(geo_point1, xy1);
        prj.forward(geo_point2, xy2);

        // Calculate distances in KM
        int const distance_expected = static_cast<int>(bg::distance(geo_point1, geo_point2) / 1000.0);
        int const distance_found = static_cast<int>(bg::distance(xy1, xy2) / 1000.0);

        int const difference = std::abs(distance_expected - distance_found);
        BOOST_CHECK_MESSAGE(difference <= 1 || difference == deviation,
                " projection: " << projection_type::get_name()
                << " distance found: " << distance_found
                << " expected: " << distance_expected);

// For debug:
//        std::cout << projection_type::get_name() << " " << distance_expected
//            << " " << distance_found
//            << " " << (difference > 1 && difference != deviation ? " *** WRONG ***" : "")
//            << " " << difference
//            << std::endl;
    }
    catch(bg::proj_exception const& e)
    {
        std::cout << "Exception in " << projection_type::get_name() << " : " << e.code() << std::endl;
    }
    catch(...)
    {
        std::cout << "Exception (unknown) in " << projection_type::get_name() << std::endl;
    }
}

template <typename T>
void test_all()
{
    typedef bg::model::ll::point<bg::degree, T> geo_point_type;

    geo_point_type amsterdam = bg::make<geo_point_type>(4.8925, 52.3731);
    geo_point_type utrecht   = bg::make<geo_point_type>(5.1213, 52.0907);

    test_forward<bg::projections::detail::aea_ellipsoid>(amsterdam, utrecht, "+ellps=WGS84 +units=m +lat_1=55 +lat_2=65");
    test_forward<bg::projections::detail::aeqd_ellipsoid>(amsterdam, utrecht, "+ellps=WGS84 +units=m");
    test_forward<bg::projections::detail::aeqd_spheroid>(amsterdam, utrecht, "+ellps=WGS84 +units=m");

    test_forward<bg::projections::detail::airy_spheroid>(amsterdam, utrecht, "+ellps=WGS84 +units=m", 4);
    test_forward<bg::projections::detail::aitoff_spheroid>(amsterdam, utrecht, "+ellps=WGS84 +units=m", 2);
    test_forward<bg::projections::detail::apian_spheroid>(amsterdam, utrecht, "+ellps=WGS84 +units=m +lon_0=11d32'00E");
    test_forward<bg::projections::detail::august_spheroid>(amsterdam, utrecht, "+ellps=WGS84 +units=m", 14);

    test_forward<bg::projections::detail::bacon_spheroid>(amsterdam, utrecht, "+ellps=WGS84 +units=m +lon_0=11d32'00E", 5);
    test_forward<bg::projections::detail::bipc_spheroid>(amsterdam, utrecht, "+ellps=WGS84 +units=m", 7);
    test_forward<bg::projections::detail::boggs_spheroid>(amsterdam, utrecht, "+ellps=WGS84 +units=m +lon_0=11d32'00E", 2);

    test_forward<bg::projections::detail::bonne_ellipsoid>(amsterdam, utrecht, "+ellps=WGS84 +units=m +lat_1=50");
    test_forward<bg::projections::detail::bonne_spheroid>(amsterdam, utrecht, "+ellps=WGS84 +units=m +lat_1=50", 33);

    test_forward<bg::projections::detail::cass_ellipsoid>(amsterdam, utrecht, "+ellps=WGS84 +units=m");
    test_forward<bg::projections::detail::cass_spheroid>(amsterdam, utrecht, "+ellps=WGS84 +units=m");
    test_forward<bg::projections::detail::cc_spheroid>(amsterdam, utrecht, "+ellps=WGS84 +units=m", 52);

    test_forward<bg::projections::detail::cea_ellipsoid>(amsterdam, utrecht, "+ellps=WGS84 +units=m +lon_0=11d32'00E", 4);
    test_forward<bg::projections::detail::cea_spheroid>(amsterdam, utrecht, "+ellps=WGS84 +units=m +lon_0=11d32'00E", 4);

    test_forward<bg::projections::detail::chamb_spheroid>(amsterdam, utrecht, "+ellps=WGS84 +units=m +lat_1=52 +lon_1=5 +lat_2=30 +lon_2=80 +lat_3=20 +lon_3=-50", 2);
    test_forward<bg::projections::detail::collg_spheroid>(amsterdam, utrecht, "+ellps=WGS84 +units=m", 5);
    test_forward<bg::projections::detail::crast_spheroid>(amsterdam, utrecht, "+ellps=WGS84 +units=m");
    test_forward<bg::projections::detail::denoy_spheroid>(amsterdam, utrecht,  "+ellps=WGS84 +units=m", 2);
    test_forward<bg::projections::detail::eck1_spheroid>(amsterdam, utrecht,  "+ellps=WGS84 +units=m", 2);
    test_forward<bg::projections::detail::eck2_spheroid>(amsterdam, utrecht,  "+ellps=WGS84 +units=m");
    test_forward<bg::projections::detail::eck3_spheroid>(amsterdam, utrecht,  "+ellps=WGS84 +units=m", 2);
    test_forward<bg::projections::detail::eck4_spheroid>(amsterdam, utrecht, "+ellps=WGS84 +units=m", 2);
    test_forward<bg::projections::detail::eck5_spheroid>(amsterdam, utrecht, "+ellps=WGS84 +units=m", 2);

    test_forward<bg::projections::detail::eck6_spheroid>(amsterdam, utrecht,  "+ellps=WGS84 +units=m");

    test_forward<bg::projections::detail::eqc_spheroid>(amsterdam, utrecht,  "+ellps=WGS84 +units=m", 5);
    test_forward<bg::projections::detail::eqdc_ellipsoid>(amsterdam, utrecht,  "+ellps=WGS84 +units=m +lat_1=60 +lat_2=0");
    test_forward<bg::projections::detail::etmerc_ellipsoid>(amsterdam, utrecht,  "+ellps=WGS84 +units=m");
    test_forward<bg::projections::detail::euler_spheroid>(amsterdam, utrecht, "+ellps=WGS84 +units=m +lat_1=60 +lat_2=0");

    test_forward<bg::projections::detail::fahey_spheroid>(amsterdam, utrecht, "+ellps=WGS84 +units=m", 5);
    test_forward<bg::projections::detail::fouc_spheroid>(amsterdam, utrecht, "+ellps=WGS84 +units=m", 6);
    test_forward<bg::projections::detail::fouc_s_spheroid>(amsterdam, utrecht, "+ellps=WGS84 +units=m", 4);
    test_forward<bg::projections::detail::gall_spheroid>(amsterdam, utrecht, "+ellps=WGS84 +units=m", 2);
    test_forward<bg::projections::detail::geocent_other>(amsterdam, utrecht, "+ellps=WGS84 +units=m", 5);
    test_forward<bg::projections::detail::geos_spheroid>(amsterdam, utrecht, "+ellps=WGS84 +units=m +h=40000000", 13);
    test_forward<bg::projections::detail::gins8_spheroid>(amsterdam, utrecht, "+ellps=WGS84 +units=m", 7);

    test_forward<bg::projections::detail::gn_sinu_spheroid>(amsterdam, utrecht, "+ellps=WGS84 +units=m +m=0.5 +n=1.785");

    test_forward<bg::projections::detail::gnom_spheroid>(amsterdam, utrecht, "+ellps=WGS84 +units=m", 50);
    test_forward<bg::projections::detail::goode_spheroid>(amsterdam, utrecht, "+ellps=WGS84 +units=m");
    test_forward<bg::projections::detail::gstmerc_spheroid>(amsterdam, utrecht, "+ellps=WGS84 +units=m");
    test_forward<bg::projections::detail::hammer_spheroid>(amsterdam, utrecht, "+ellps=WGS84 +units=m");
    test_forward<bg::projections::detail::hatano_spheroid>(amsterdam, utrecht, "+ellps=WGS84 +units=m", 2);

    test_forward<bg::projections::detail::healpix_ellipsoid>(amsterdam, utrecht, "+ellps=WGS84 +units=m", 6);
    test_forward<bg::projections::detail::healpix_spheroid>(amsterdam, utrecht, "+ellps=WGS84 +units=m", 6);
    test_forward<bg::projections::detail::rhealpix_ellipsoid>(amsterdam, utrecht, "+ellps=WGS84 +units=m", 6);
    test_forward<bg::projections::detail::rhealpix_spheroid>(amsterdam, utrecht, "+ellps=WGS84 +units=m", 6);

    test_forward<bg::projections::detail::imw_p_ellipsoid>(amsterdam, utrecht, "+ellps=WGS84 +units=m +lat_1=20n +lat_2=60n +lon_1=5");
    test_forward<bg::projections::detail::isea_spheroid>(amsterdam, utrecht, "+ellps=WGS84 +units=m", 2);
    test_forward<bg::projections::detail::kav5_spheroid>(amsterdam, utrecht, "+ellps=WGS84 +units=m", 2);
    test_forward<bg::projections::detail::kav7_spheroid>(amsterdam, utrecht, "+ellps=WGS84 +units=m", 2);
    test_forward<bg::projections::detail::krovak_ellipsoid>(amsterdam, utrecht, "+ellps=WGS84 +units=m");
    test_forward<bg::projections::detail::laea_spheroid>(amsterdam, utrecht, "+ellps=WGS84 +units=m", 2);
    test_forward<bg::projections::detail::lagrng_spheroid>(amsterdam, utrecht, "+ellps=WGS84 +units=m +W=1", 8);
    test_forward<bg::projections::detail::larr_spheroid>(amsterdam, utrecht, "+ellps=WGS84 +units=m", 13);
    test_forward<bg::projections::detail::lask_spheroid>(amsterdam, utrecht, "+ellps=WGS84 +units=m", 5);
    test_forward<bg::projections::detail::lcc_ellipsoid>(amsterdam, utrecht, "+ellps=WGS84 +units=m +lat_1=20n +lat_2=60n", 2);
    test_forward<bg::projections::detail::lcca_ellipsoid>(amsterdam, utrecht, "+ellps=WGS84 +units=m +lat_0=30n +lat_1=55n +lat_2=60n", 2);
    test_forward<bg::projections::detail::leac_ellipsoid>(amsterdam, utrecht, "+ellps=WGS84 +units=m", 8);
    test_forward<bg::projections::detail::loxim_spheroid>(amsterdam, utrecht, "+ellps=WGS84 +units=m", 3);
    test_forward<bg::projections::detail::lsat_ellipsoid>(amsterdam, utrecht, "+ellps=WGS84 +units=m +lsat=1 +path=1", 3);
    test_forward<bg::projections::detail::mbt_fps_spheroid>(amsterdam, utrecht, "+ellps=WGS84 +units=m", 2);
    test_forward<bg::projections::detail::mbt_s_spheroid>(amsterdam, utrecht, "+ellps=WGS84 +units=m", 2);
    test_forward<bg::projections::detail::mbtfpp_spheroid>(amsterdam, utrecht, "+ellps=WGS84 +units=m");
    test_forward<bg::projections::detail::mbtfpq_spheroid>(amsterdam, utrecht, "+ellps=WGS84 +units=m", 2);

    test_forward<bg::projections::detail::mbtfps_spheroid>(amsterdam, utrecht, "+ellps=WGS84 +units=m");

    test_forward<bg::projections::detail::merc_ellipsoid>(amsterdam, utrecht, "+ellps=WGS84 +units=m", 22);
    test_forward<bg::projections::detail::merc_spheroid>(amsterdam, utrecht, "+ellps=WGS84 +units=m", 22);

    test_forward<bg::projections::detail::mil_os_ellipsoid>(amsterdam, utrecht, "+ellps=WGS84 +units=m");
    test_forward<bg::projections::detail::mill_spheroid>(amsterdam, utrecht, "+ellps=WGS84 +units=m", 14);
    test_forward<bg::projections::detail::moll_spheroid>(amsterdam, utrecht, "+ellps=WGS84 +units=m");
    test_forward<bg::projections::detail::murd1_spheroid>(amsterdam, utrecht, "+ellps=WGS84 +units=m +lat_1=20n +lat_2=60n");
    test_forward<bg::projections::detail::murd2_spheroid>(amsterdam, utrecht, "+ellps=WGS84 +units=m +lat_1=20n +lat_2=60n");
    test_forward<bg::projections::detail::murd3_spheroid>(amsterdam, utrecht, "+ellps=WGS84 +units=m +lat_1=20n +lat_2=60n");
    test_forward<bg::projections::detail::natearth_spheroid>(amsterdam, utrecht, "+ellps=WGS84 +units=m");
    test_forward<bg::projections::detail::nell_spheroid>(amsterdam, utrecht, "+ellps=WGS84 +units=m", 4);
    test_forward<bg::projections::detail::nell_h_spheroid>(amsterdam, utrecht, "+ellps=WGS84 +units=m", 3);
    test_forward<bg::projections::detail::nicol_spheroid>(amsterdam, utrecht, "+ellps=WGS84 +units=m");

    test_forward<bg::projections::detail::oea_spheroid>(amsterdam, utrecht, "+ellps=WGS84 +units=m +lat_1=20n +lat_2=60n +lon_1=1e +lon_2=30e +m=1 +n=1", 4);
    test_forward<bg::projections::detail::omerc_ellipsoid>(amsterdam, utrecht, "+ellps=WGS84 +units=m +lat_1=20n +lat_2=60n  +lon_1=1e +lon_2=30e");
    test_forward<bg::projections::detail::ortel_spheroid>(amsterdam, utrecht, "+ellps=WGS84 +units=m");
    test_forward<bg::projections::detail::ortho_spheroid>(amsterdam, utrecht, "+ellps=WGS84 +units=m", 9);
    test_forward<bg::projections::detail::pconic_spheroid>(amsterdam, utrecht, "+ellps=WGS84 +units=m +lat_1=20n +lat_2=60n +lon_0=10E");
    test_forward<bg::projections::detail::qsc_ellipsoid>(amsterdam, utrecht, "+ellps=WGS84 +units=m", 10);
    test_forward<bg::projections::detail::poly_spheroid>(amsterdam, utrecht, "+ellps=WGS84 +units=m");
    test_forward<bg::projections::detail::putp1_spheroid>(amsterdam, utrecht, "+ellps=WGS84 +units=m");
    test_forward<bg::projections::detail::putp2_spheroid>(amsterdam, utrecht, "+ellps=WGS84 +units=m");
    test_forward<bg::projections::detail::putp3_spheroid>(amsterdam, utrecht, "+ellps=WGS84 +units=m", 6);
    test_forward<bg::projections::detail::putp3p_spheroid>(amsterdam, utrecht, "+ellps=WGS84 +units=m", 5);
    test_forward<bg::projections::detail::putp4p_spheroid>(amsterdam, utrecht, "+ellps=WGS84 +units=m");
    test_forward<bg::projections::detail::putp5_spheroid>(amsterdam, utrecht, "+ellps=WGS84 +units=m");
    test_forward<bg::projections::detail::putp5p_spheroid>(amsterdam, utrecht, "+ellps=WGS84 +units=m", 3);
    test_forward<bg::projections::detail::putp6_spheroid>(amsterdam, utrecht, "+ellps=WGS84 +units=m");
    test_forward<bg::projections::detail::putp6p_spheroid>(amsterdam, utrecht, "+ellps=WGS84 +units=m");
    test_forward<bg::projections::detail::qua_aut_spheroid>(amsterdam, utrecht, "+ellps=WGS84 +units=m");
    test_forward<bg::projections::detail::robin_spheroid>(amsterdam, utrecht, "+ellps=WGS84 +units=m");
    test_forward<bg::projections::detail::rouss_ellipsoid>(amsterdam, utrecht, "+ellps=WGS84 +units=m", 8);
    test_forward<bg::projections::detail::rpoly_spheroid>(amsterdam, utrecht, "+ellps=WGS84 +units=m");

    test_forward<bg::projections::detail::sinu_ellipsoid>(amsterdam, utrecht, "+ellps=WGS84 +units=m");
    test_forward<bg::projections::detail::sinu_spheroid>(amsterdam, utrecht, "+ellps=sphere +units=m");

    test_forward<bg::projections::detail::somerc_ellipsoid>(amsterdam, utrecht, "+ellps=WGS84 +units=m", 22);
    test_forward<bg::projections::detail::stere_spheroid>(amsterdam, utrecht, "+ellps=WGS84 +units=m +lat_ts=50n", 8);
    test_forward<bg::projections::detail::sterea_ellipsoid>(amsterdam, utrecht, "+lat_0=52.15616055555555 +lon_0=5.38763888888889 +k=0.9999079 +x_0=155000 +y_0=463000 +ellps=bessel +units=m");
    test_forward<bg::projections::detail::tcc_spheroid>(amsterdam, utrecht, "+ellps=WGS84 +units=m");
    test_forward<bg::projections::detail::tcea_spheroid>(amsterdam, utrecht, "+ellps=WGS84 +units=m");
    test_forward<bg::projections::detail::tissot_spheroid>(amsterdam, utrecht, "+ellps=WGS84 +units=m +lat_1=20n +lat_2=60n", 2);

    test_forward<bg::projections::detail::tmerc_spheroid>(amsterdam, utrecht, "+ellps=sphere +units=m");
    test_forward<bg::projections::detail::tmerc_ellipsoid>(amsterdam, utrecht, "+ellps=WGS84 +units=m");

    test_forward<bg::projections::detail::tpeqd_spheroid>(amsterdam, utrecht, "+ellps=WGS84 +units=m +lat_1=20n +lat_2=60n  +lon_1=0 +lon_2=30e");
    test_forward<bg::projections::detail::tpers_spheroid>(amsterdam, utrecht, "+ellps=WGS84 +units=m +tilt=50 +azi=20 +h=40000000", 14);

    test_forward<bg::projections::detail::ups_spheroid>(amsterdam, utrecht, "+ellps=WGS84 +units=m", 3);
    test_forward<bg::projections::detail::ups_ellipsoid>(amsterdam, utrecht, "+ellps=WGS84 +units=m", 3);

    test_forward<bg::projections::detail::urm5_spheroid>(amsterdam, utrecht, "+ellps=WGS84 +units=m +n=.3 +q=.3 +alpha=10", 4);
    test_forward<bg::projections::detail::urmfps_spheroid>(amsterdam, utrecht, "+ellps=WGS84 +units=m +n=0.50", 4);

    test_forward<bg::projections::detail::utm_spheroid>(amsterdam, utrecht, "+ellps=sphere +units=m +lon_0=11d32'00E");
    test_forward<bg::projections::detail::utm_ellipsoid>(amsterdam, utrecht, "+ellps=WGS84 +units=m +lon_0=11d32'00E");

    test_forward<bg::projections::detail::vandg_spheroid>(amsterdam, utrecht, "+ellps=WGS84 +units=m", 13);
    test_forward<bg::projections::detail::vandg2_spheroid>(amsterdam, utrecht, "+ellps=WGS84 +units=m", 13);
    test_forward<bg::projections::detail::vandg3_spheroid>(amsterdam, utrecht, "+ellps=WGS84 +units=m", 13);
    test_forward<bg::projections::detail::vandg4_spheroid>(amsterdam, utrecht, "+ellps=WGS84 +units=m");
    test_forward<bg::projections::detail::vitk1_spheroid>(amsterdam, utrecht, "+ellps=WGS84 +units=m +lat_1=20n +lat_2=60n");
    test_forward<bg::projections::detail::wag1_spheroid>(amsterdam, utrecht, "+ellps=WGS84 +units=m");
    test_forward<bg::projections::detail::wag2_spheroid>(amsterdam, utrecht, "+ellps=WGS84 +units=m");
    test_forward<bg::projections::detail::wag3_spheroid>(amsterdam, utrecht, "+ellps=WGS84 +units=m", 3);
    test_forward<bg::projections::detail::wag4_spheroid>(amsterdam, utrecht, "+ellps=WGS84 +units=m", 2);
    test_forward<bg::projections::detail::wag5_spheroid>(amsterdam, utrecht, "+ellps=WGS84 +units=m");
    test_forward<bg::projections::detail::wag6_spheroid>(amsterdam, utrecht, "+ellps=WGS84 +units=m");
    test_forward<bg::projections::detail::wag7_spheroid>(amsterdam, utrecht, "+ellps=WGS84 +units=m", 2);
    test_forward<bg::projections::detail::weren_spheroid>(amsterdam, utrecht, "+ellps=WGS84 +units=m", 4);
    test_forward<bg::projections::detail::wink1_spheroid>(amsterdam, utrecht, "+ellps=WGS84 +units=m", 3);
    test_forward<bg::projections::detail::wink2_spheroid>(amsterdam, utrecht, "+ellps=WGS84 +units=m", 4);
    test_forward<bg::projections::detail::wintri_spheroid>(amsterdam, utrecht, "+ellps=WGS84 +units=m");

    //    We SKIP ob_tran because it internally requires the factory and is, in that sense, not a static test
    //    test_forward<bg::projections::detail::ob_tran_oblique>(amsterdam, utrecht, "+ellps=WGS84 +units=m +o_proj=moll +o_lat_p=10 +o_lon_p=90 +o_lon_o=11.50");
    //    test_forward<bg::projections::detail::ob_tran_transverse>(amsterdam, utrecht, "+ellps=WGS84 +units=m +o_proj=moll +o_lat_p=10 +o_lon_p=90 +o_lon_o=11.50");

    // TODO: wrong projections or parameters or input points
//    test_forward<bg::projections::detail::ocea_spheroid>(auckland, wellington, "+ellps=sphere +units=m +lat_1=20s +lat_2=60s  +lon_1=165e +lon_2=175e"); => distance is very large
//    test_forward<bg::projections::detail::nsper_spheroid>(amsterdam, utrecht, "+ellps=WGS84 +units=m +a=10 +h=40000000"); => distance is 0
//    test_forward<bg::projections::detail::lee_os_ellipsoid>(amsterdam, utrecht, "+ellps=WGS84 +units=m"); => distance is 407


    // Alaska
    {
        geo_point_type anchorage = bg::make<geo_point_type>(-149.90, 61.22);
        geo_point_type juneau    = bg::make<geo_point_type>(-134.42, 58.30);
        test_forward<bg::projections::detail::alsk_ellipsoid>(anchorage, juneau, "+ellps=WGS84 +units=m +lon_0=-150W", 1);
    }
    // New Zealand
    {
        geo_point_type auckland   = bg::make<geo_point_type>(174.74, -36.84);
        geo_point_type wellington = bg::make<geo_point_type>(177.78, -41.29);
        test_forward<bg::projections::detail::nzmg_ellipsoid>(auckland, wellington, "+ellps=WGS84 +units=m", 0);
    }

    // US
    {
        geo_point_type aspen  = bg::make<geo_point_type>(-106.84, 39.19);
        geo_point_type denver = bg::make<geo_point_type>(-104.88, 39.76);
        // TODO: test_forward<bg::projections::detail::gs48_ellipsoid>(aspen, denver, "+ellps=WGS84 +units=m +lon1=-48");=> distance is > 1000
        test_forward<bg::projections::detail::gs50_ellipsoid>(aspen, denver, "+ellps=WGS84 +units=m +lon1=-50", 2);
    }

}

int test_main(int, char* [])
{
    test_all<double>();

    return 0;
}
