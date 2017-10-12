// Boost.Geometry

// Copyright (c) 2017, Oracle and/or its affiliates.
// Contributed and/or modified by Adam Wulkiewicz, on behalf of Oracle

// Use, modification and distribution is subject to the Boost Software License,
// Version 1.0. (See accompanying file LICENSE_1_0.txt or copy at
// http://www.boost.org/LICENSE_1_0.txt)

#ifndef BOOST_GEOMETRY_PROJECTIONS_ESRI_TRAITS_HPP
#define BOOST_GEOMETRY_PROJECTIONS_ESRI_TRAITS_HPP


#include <boost/geometry/core/tags.hpp>

#include <boost/geometry/srs/projections/factory.hpp>
#include <boost/geometry/srs/projections/impl/projects.hpp>


namespace boost { namespace geometry { namespace projections
{

#ifndef DOXYGEN_NO_DETAIL
namespace detail
{

/*!
    \brief ESRI traits
    \details With help of the ESRI traits library users can statically use projections
        or coordinate systems specifying an ESRI code. The correct projections for transformations
        are used automically then, still keeping static polymorphism.
    \ingroup projection
    \tparam ESRI esri code
*/
template <size_t ESRI>
struct esri_traits
{
    // Specializations define:
    // - type to get projection type
    // - function par to get parameters
};

#define BOOST_GEOMETRY_PROJECTIONS_DETAIL_ESRI_TRAITS(ESRI, PROJ4_TAG, SRS_TAG, PROJ4_STR) \
template<> \
struct esri_traits<ESRI> \
{ \
    typedef PROJ4_TAG type; \
    typedef SRS_TAG srs_tag; \
    static inline std::string par() \
    { \
        return PROJ4_STR; \
    } \
}; \

BOOST_GEOMETRY_PROJECTIONS_DETAIL_ESRI_TRAITS(37001, srs::proj::longlat, srs_spheroid_tag, "+proj=longlat +ellps=WGS66 +no_defs");
BOOST_GEOMETRY_PROJECTIONS_DETAIL_ESRI_TRAITS(37002, srs::proj::longlat, srs_spheroid_tag, "+proj=longlat +a=6378166 +b=6356784.283607107 +no_defs");
BOOST_GEOMETRY_PROJECTIONS_DETAIL_ESRI_TRAITS(37003, srs::proj::longlat, srs_spheroid_tag, "+proj=longlat +a=6378150 +b=6356768.337244385 +no_defs");
BOOST_GEOMETRY_PROJECTIONS_DETAIL_ESRI_TRAITS(37004, srs::proj::longlat, srs_spheroid_tag, "+proj=longlat +ellps=fschr60m +no_defs");
BOOST_GEOMETRY_PROJECTIONS_DETAIL_ESRI_TRAITS(37005, srs::proj::longlat, srs_spheroid_tag, "+proj=longlat +a=6378270 +b=6356794.343434343 +no_defs");
BOOST_GEOMETRY_PROJECTIONS_DETAIL_ESRI_TRAITS(37006, srs::proj::longlat, srs_spheroid_tag, "+proj=longlat +a=6377295.664 +b=6356094.667915204 +no_defs");
BOOST_GEOMETRY_PROJECTIONS_DETAIL_ESRI_TRAITS(37007, srs::proj::longlat, srs_spheroid_tag, "+proj=longlat +a=6376896 +b=6355834.846687363 +no_defs");
BOOST_GEOMETRY_PROJECTIONS_DETAIL_ESRI_TRAITS(37008, srs::proj::longlat, srs_sphere_tag, "+proj=longlat +a=6370997 +b=6370997 +no_defs");
BOOST_GEOMETRY_PROJECTIONS_DETAIL_ESRI_TRAITS(37201, srs::proj::longlat, srs_spheroid_tag, "+proj=longlat +ellps=intl +no_defs");
BOOST_GEOMETRY_PROJECTIONS_DETAIL_ESRI_TRAITS(37202, srs::proj::longlat, srs_spheroid_tag, "+proj=longlat +a=6377276.345 +b=6356075.41314024 +no_defs");
BOOST_GEOMETRY_PROJECTIONS_DETAIL_ESRI_TRAITS(37203, srs::proj::longlat, srs_spheroid_tag, "+proj=longlat +a=6377301.243 +b=6356100.230165384 +no_defs");
BOOST_GEOMETRY_PROJECTIONS_DETAIL_ESRI_TRAITS(37204, srs::proj::longlat, srs_spheroid_tag, "+proj=longlat +ellps=intl +no_defs");
BOOST_GEOMETRY_PROJECTIONS_DETAIL_ESRI_TRAITS(37205, srs::proj::longlat, srs_spheroid_tag, "+proj=longlat +ellps=intl +no_defs");
BOOST_GEOMETRY_PROJECTIONS_DETAIL_ESRI_TRAITS(37206, srs::proj::longlat, srs_spheroid_tag, "+proj=longlat +ellps=clrk80 +no_defs");
BOOST_GEOMETRY_PROJECTIONS_DETAIL_ESRI_TRAITS(37207, srs::proj::longlat, srs_spheroid_tag, "+proj=longlat +ellps=fschr60m +no_defs");
BOOST_GEOMETRY_PROJECTIONS_DETAIL_ESRI_TRAITS(37208, srs::proj::longlat, srs_spheroid_tag, "+proj=longlat +ellps=clrk80 +no_defs");
BOOST_GEOMETRY_PROJECTIONS_DETAIL_ESRI_TRAITS(37211, srs::proj::longlat, srs_spheroid_tag, "+proj=longlat +ellps=clrk80 +no_defs");
BOOST_GEOMETRY_PROJECTIONS_DETAIL_ESRI_TRAITS(37212, srs::proj::longlat, srs_spheroid_tag, "+proj=longlat +ellps=intl +no_defs");
BOOST_GEOMETRY_PROJECTIONS_DETAIL_ESRI_TRAITS(37213, srs::proj::longlat, srs_spheroid_tag, "+proj=longlat +ellps=intl +no_defs");
BOOST_GEOMETRY_PROJECTIONS_DETAIL_ESRI_TRAITS(37214, srs::proj::longlat, srs_spheroid_tag, "+proj=longlat +ellps=intl +no_defs");
BOOST_GEOMETRY_PROJECTIONS_DETAIL_ESRI_TRAITS(37215, srs::proj::longlat, srs_spheroid_tag, "+proj=longlat +ellps=intl +no_defs");
BOOST_GEOMETRY_PROJECTIONS_DETAIL_ESRI_TRAITS(37216, srs::proj::longlat, srs_spheroid_tag, "+proj=longlat +ellps=intl +no_defs");
BOOST_GEOMETRY_PROJECTIONS_DETAIL_ESRI_TRAITS(37217, srs::proj::longlat, srs_spheroid_tag, "+proj=longlat +ellps=intl +no_defs");
BOOST_GEOMETRY_PROJECTIONS_DETAIL_ESRI_TRAITS(37218, srs::proj::longlat, srs_spheroid_tag, "+proj=longlat +ellps=intl +no_defs");
BOOST_GEOMETRY_PROJECTIONS_DETAIL_ESRI_TRAITS(37219, srs::proj::longlat, srs_spheroid_tag, "+proj=longlat +ellps=intl +no_defs");
BOOST_GEOMETRY_PROJECTIONS_DETAIL_ESRI_TRAITS(37220, srs::proj::longlat, srs_spheroid_tag, "+proj=longlat +ellps=clrk66 +no_defs");
BOOST_GEOMETRY_PROJECTIONS_DETAIL_ESRI_TRAITS(37221, srs::proj::longlat, srs_spheroid_tag, "+proj=longlat +ellps=intl +no_defs");
BOOST_GEOMETRY_PROJECTIONS_DETAIL_ESRI_TRAITS(37222, srs::proj::longlat, srs_spheroid_tag, "+proj=longlat +ellps=intl +no_defs");
BOOST_GEOMETRY_PROJECTIONS_DETAIL_ESRI_TRAITS(37223, srs::proj::longlat, srs_spheroid_tag, "+proj=longlat +a=6378249.2 +b=6356514.999904194 +no_defs");
BOOST_GEOMETRY_PROJECTIONS_DETAIL_ESRI_TRAITS(37224, srs::proj::longlat, srs_spheroid_tag, "+proj=longlat +ellps=intl +no_defs");
BOOST_GEOMETRY_PROJECTIONS_DETAIL_ESRI_TRAITS(37226, srs::proj::longlat, srs_spheroid_tag, "+proj=longlat +ellps=intl +no_defs");
BOOST_GEOMETRY_PROJECTIONS_DETAIL_ESRI_TRAITS(37227, srs::proj::longlat, srs_spheroid_tag, "+proj=longlat +ellps=intl +no_defs");
BOOST_GEOMETRY_PROJECTIONS_DETAIL_ESRI_TRAITS(37228, srs::proj::longlat, srs_spheroid_tag, "+proj=longlat +ellps=clrk80 +no_defs");
BOOST_GEOMETRY_PROJECTIONS_DETAIL_ESRI_TRAITS(37229, srs::proj::longlat, srs_spheroid_tag, "+proj=longlat +a=6378270 +b=6356794.343434343 +no_defs");
BOOST_GEOMETRY_PROJECTIONS_DETAIL_ESRI_TRAITS(37230, srs::proj::longlat, srs_spheroid_tag, "+proj=longlat +ellps=intl +no_defs");
BOOST_GEOMETRY_PROJECTIONS_DETAIL_ESRI_TRAITS(37231, srs::proj::longlat, srs_spheroid_tag, "+proj=longlat +ellps=aust_SA +no_defs");
BOOST_GEOMETRY_PROJECTIONS_DETAIL_ESRI_TRAITS(37232, srs::proj::longlat, srs_spheroid_tag, "+proj=longlat +ellps=intl +no_defs");
BOOST_GEOMETRY_PROJECTIONS_DETAIL_ESRI_TRAITS(37233, srs::proj::longlat, srs_spheroid_tag, "+proj=longlat +ellps=intl +no_defs");
BOOST_GEOMETRY_PROJECTIONS_DETAIL_ESRI_TRAITS(37234, srs::proj::longlat, srs_spheroid_tag, "+proj=longlat +ellps=intl +no_defs");
BOOST_GEOMETRY_PROJECTIONS_DETAIL_ESRI_TRAITS(37235, srs::proj::longlat, srs_spheroid_tag, "+proj=longlat +ellps=intl +no_defs");
BOOST_GEOMETRY_PROJECTIONS_DETAIL_ESRI_TRAITS(37237, srs::proj::longlat, srs_spheroid_tag, "+proj=longlat +ellps=intl +no_defs");
BOOST_GEOMETRY_PROJECTIONS_DETAIL_ESRI_TRAITS(37238, srs::proj::longlat, srs_spheroid_tag, "+proj=longlat +ellps=intl +no_defs");
BOOST_GEOMETRY_PROJECTIONS_DETAIL_ESRI_TRAITS(37239, srs::proj::longlat, srs_spheroid_tag, "+proj=longlat +ellps=clrk66 +no_defs");
BOOST_GEOMETRY_PROJECTIONS_DETAIL_ESRI_TRAITS(37240, srs::proj::longlat, srs_spheroid_tag, "+proj=longlat +ellps=clrk80 +no_defs");
BOOST_GEOMETRY_PROJECTIONS_DETAIL_ESRI_TRAITS(37241, srs::proj::longlat, srs_spheroid_tag, "+proj=longlat +ellps=intl +no_defs");
BOOST_GEOMETRY_PROJECTIONS_DETAIL_ESRI_TRAITS(37242, srs::proj::longlat, srs_spheroid_tag, "+proj=longlat +ellps=intl +no_defs");
BOOST_GEOMETRY_PROJECTIONS_DETAIL_ESRI_TRAITS(37243, srs::proj::longlat, srs_spheroid_tag, "+proj=longlat +ellps=clrk66 +no_defs");
BOOST_GEOMETRY_PROJECTIONS_DETAIL_ESRI_TRAITS(37245, srs::proj::longlat, srs_spheroid_tag, "+proj=longlat +ellps=intl +no_defs");
BOOST_GEOMETRY_PROJECTIONS_DETAIL_ESRI_TRAITS(37246, srs::proj::longlat, srs_spheroid_tag, "+proj=longlat +ellps=intl +no_defs");
BOOST_GEOMETRY_PROJECTIONS_DETAIL_ESRI_TRAITS(37247, srs::proj::longlat, srs_spheroid_tag, "+proj=longlat +ellps=intl +no_defs");
BOOST_GEOMETRY_PROJECTIONS_DETAIL_ESRI_TRAITS(37249, srs::proj::longlat, srs_spheroid_tag, "+proj=longlat +ellps=intl +no_defs");
BOOST_GEOMETRY_PROJECTIONS_DETAIL_ESRI_TRAITS(37250, srs::proj::longlat, srs_spheroid_tag, "+proj=longlat +ellps=intl +no_defs");
BOOST_GEOMETRY_PROJECTIONS_DETAIL_ESRI_TRAITS(37251, srs::proj::longlat, srs_spheroid_tag, "+proj=longlat +ellps=intl +no_defs");
BOOST_GEOMETRY_PROJECTIONS_DETAIL_ESRI_TRAITS(37252, srs::proj::longlat, srs_spheroid_tag, "+proj=longlat +ellps=clrk66 +no_defs");
BOOST_GEOMETRY_PROJECTIONS_DETAIL_ESRI_TRAITS(37253, srs::proj::longlat, srs_spheroid_tag, "+proj=longlat +ellps=intl +no_defs");
BOOST_GEOMETRY_PROJECTIONS_DETAIL_ESRI_TRAITS(37254, srs::proj::longlat, srs_spheroid_tag, "+proj=longlat +ellps=clrk80 +no_defs");
BOOST_GEOMETRY_PROJECTIONS_DETAIL_ESRI_TRAITS(37255, srs::proj::longlat, srs_spheroid_tag, "+proj=longlat +ellps=bessel +no_defs");
BOOST_GEOMETRY_PROJECTIONS_DETAIL_ESRI_TRAITS(37257, srs::proj::longlat, srs_spheroid_tag, "+proj=longlat +ellps=krass +no_defs");
BOOST_GEOMETRY_PROJECTIONS_DETAIL_ESRI_TRAITS(37259, srs::proj::longlat, srs_spheroid_tag, "+proj=longlat +ellps=intl +no_defs");
BOOST_GEOMETRY_PROJECTIONS_DETAIL_ESRI_TRAITS(37260, srs::proj::longlat, srs_spheroid_tag, "+proj=longlat +ellps=clrk66 +no_defs");
//BOOST_GEOMETRY_PROJECTIONS_DETAIL_ESRI_TRAITS(53001, );
BOOST_GEOMETRY_PROJECTIONS_DETAIL_ESRI_TRAITS(53002, srs::proj::eqc, srs_sphere_tag, "+proj=eqc +lat_ts=60 +lat_0=0 +lon_0=0 +x_0=0 +y_0=0 +a=6371000 +b=6371000 +units=m +no_defs");
BOOST_GEOMETRY_PROJECTIONS_DETAIL_ESRI_TRAITS(53003, srs::proj::mill, srs_sphere_tag, "+proj=mill +lat_0=0 +lon_0=0 +x_0=0 +y_0=0 +R_A +a=6371000 +b=6371000 +units=m +no_defs");
BOOST_GEOMETRY_PROJECTIONS_DETAIL_ESRI_TRAITS(53004, srs::proj::merc, srs_sphere_tag, "+proj=merc +lon_0=0 +k=1 +x_0=0 +y_0=0 +a=6371000 +b=6371000 +units=m +no_defs");
BOOST_GEOMETRY_PROJECTIONS_DETAIL_ESRI_TRAITS(53008, srs::proj::sinu, srs_sphere_tag, "+proj=sinu +lon_0=0 +x_0=0 +y_0=0 +a=6371000 +b=6371000 +units=m +no_defs");
BOOST_GEOMETRY_PROJECTIONS_DETAIL_ESRI_TRAITS(53009, srs::proj::moll, srs_sphere_tag, "+proj=moll +lon_0=0 +x_0=0 +y_0=0 +a=6371000 +b=6371000 +units=m +no_defs");
BOOST_GEOMETRY_PROJECTIONS_DETAIL_ESRI_TRAITS(53010, srs::proj::eck6, srs_sphere_tag, "+proj=eck6 +lon_0=0 +x_0=0 +y_0=0 +a=6371000 +b=6371000 +units=m +no_defs");
//BOOST_GEOMETRY_PROJECTIONS_DETAIL_ESRI_TRAITS(53011, );
BOOST_GEOMETRY_PROJECTIONS_DETAIL_ESRI_TRAITS(53012, srs::proj::eck4, srs_sphere_tag, "+proj=eck4 +lon_0=0 +x_0=0 +y_0=0 +a=6371000 +b=6371000 +units=m +no_defs");
//BOOST_GEOMETRY_PROJECTIONS_DETAIL_ESRI_TRAITS(53013, );
//BOOST_GEOMETRY_PROJECTIONS_DETAIL_ESRI_TRAITS(53014, );
//BOOST_GEOMETRY_PROJECTIONS_DETAIL_ESRI_TRAITS(53015, );
BOOST_GEOMETRY_PROJECTIONS_DETAIL_ESRI_TRAITS(53016, srs::proj::gall, srs_sphere_tag, "+proj=gall +lon_0=0 +x_0=0 +y_0=0 +a=6371000 +b=6371000 +units=m +no_defs");
//BOOST_GEOMETRY_PROJECTIONS_DETAIL_ESRI_TRAITS(53017, );
//BOOST_GEOMETRY_PROJECTIONS_DETAIL_ESRI_TRAITS(53018, );
//BOOST_GEOMETRY_PROJECTIONS_DETAIL_ESRI_TRAITS(53019, );
BOOST_GEOMETRY_PROJECTIONS_DETAIL_ESRI_TRAITS(53021, srs::proj::poly, srs_sphere_tag, "+proj=poly +lat_0=0 +lon_0=0 +x_0=0 +y_0=0 +a=6371000 +b=6371000 +units=m +no_defs");
//BOOST_GEOMETRY_PROJECTIONS_DETAIL_ESRI_TRAITS(53022, );
//BOOST_GEOMETRY_PROJECTIONS_DETAIL_ESRI_TRAITS(53023, );
BOOST_GEOMETRY_PROJECTIONS_DETAIL_ESRI_TRAITS(53024, srs::proj::bonne, srs_sphere_tag, "+proj=bonne +lon_0=0 +lat_1=60 +x_0=0 +y_0=0 +a=6371000 +b=6371000 +units=m +no_defs");
BOOST_GEOMETRY_PROJECTIONS_DETAIL_ESRI_TRAITS(53025, srs::proj::omerc, srs_sphere_tag, "+proj=omerc +lat_0=40 +lon_1=0 +lat_1=0 +lon_2=0 +lat_2=0 +k=1 +x_0=0 +y_0=0 +a=6371000 +b=6371000 +units=m +no_defs");
BOOST_GEOMETRY_PROJECTIONS_DETAIL_ESRI_TRAITS(53026, srs::proj::stere, srs_sphere_tag, "+proj=stere +lat_0=0 +lon_0=0 +k=1 +x_0=0 +y_0=0 +a=6371000 +b=6371000 +units=m +no_defs");
BOOST_GEOMETRY_PROJECTIONS_DETAIL_ESRI_TRAITS(53027, srs::proj::eqdc, srs_sphere_tag, "+proj=eqdc +lat_0=0 +lon_0=0 +lat_1=60 +lat_2=60 +x_0=0 +y_0=0 +a=6371000 +b=6371000 +units=m +no_defs");
BOOST_GEOMETRY_PROJECTIONS_DETAIL_ESRI_TRAITS(53028, srs::proj::cass, srs_sphere_tag, "+proj=cass +lat_0=0 +lon_0=0 +x_0=0 +y_0=0 +a=6371000 +b=6371000 +units=m +no_defs");
BOOST_GEOMETRY_PROJECTIONS_DETAIL_ESRI_TRAITS(53029, srs::proj::vandg, srs_sphere_tag, "+proj=vandg +lon_0=0 +x_0=0 +y_0=0 +R_A +a=6371000 +b=6371000 +units=m +no_defs");
BOOST_GEOMETRY_PROJECTIONS_DETAIL_ESRI_TRAITS(53030, srs::proj::robin, srs_sphere_tag, "+proj=robin +lon_0=0 +x_0=0 +y_0=0 +a=6371000 +b=6371000 +units=m +no_defs");
BOOST_GEOMETRY_PROJECTIONS_DETAIL_ESRI_TRAITS(53031, srs::proj::tpeqd, srs_sphere_tag, "+proj=tpeqd +lat_1=0 +lon_1=0 +lat_2=60 +lon_2=60 +x_0=0 +y_0=0 +a=6371000 +b=6371000 +units=m +no_defs");
BOOST_GEOMETRY_PROJECTIONS_DETAIL_ESRI_TRAITS(53032, srs::proj::aeqd, srs_sphere_tag, "+proj=aeqd +lat_0=0 +lon_0=0 +x_0=0 +y_0=0 +a=6371000 +b=6371000 +units=m +no_defs");
//BOOST_GEOMETRY_PROJECTIONS_DETAIL_ESRI_TRAITS(54001, );
BOOST_GEOMETRY_PROJECTIONS_DETAIL_ESRI_TRAITS(54002, srs::proj::eqc, srs_spheroid_tag, "+proj=eqc +lat_ts=60 +lat_0=0 +lon_0=0 +x_0=0 +y_0=0 +ellps=WGS84 +datum=WGS84 +units=m +no_defs");
BOOST_GEOMETRY_PROJECTIONS_DETAIL_ESRI_TRAITS(54003, srs::proj::mill, srs_spheroid_tag, "+proj=mill +lat_0=0 +lon_0=0 +x_0=0 +y_0=0 +R_A +ellps=WGS84 +datum=WGS84 +units=m +no_defs");
BOOST_GEOMETRY_PROJECTIONS_DETAIL_ESRI_TRAITS(54004, srs::proj::merc, srs_spheroid_tag, "+proj=merc +lon_0=0 +k=1 +x_0=0 +y_0=0 +ellps=WGS84 +datum=WGS84 +units=m +no_defs");
BOOST_GEOMETRY_PROJECTIONS_DETAIL_ESRI_TRAITS(54008, srs::proj::sinu, srs_spheroid_tag, "+proj=sinu +lon_0=0 +x_0=0 +y_0=0 +ellps=WGS84 +datum=WGS84 +units=m +no_defs");
BOOST_GEOMETRY_PROJECTIONS_DETAIL_ESRI_TRAITS(54009, srs::proj::moll, srs_spheroid_tag, "+proj=moll +lon_0=0 +x_0=0 +y_0=0 +ellps=WGS84 +datum=WGS84 +units=m +no_defs");
BOOST_GEOMETRY_PROJECTIONS_DETAIL_ESRI_TRAITS(54010, srs::proj::eck6, srs_spheroid_tag, "+proj=eck6 +lon_0=0 +x_0=0 +y_0=0 +ellps=WGS84 +datum=WGS84 +units=m +no_defs");
//BOOST_GEOMETRY_PROJECTIONS_DETAIL_ESRI_TRAITS(54011, );
BOOST_GEOMETRY_PROJECTIONS_DETAIL_ESRI_TRAITS(54012, srs::proj::eck4, srs_spheroid_tag, "+proj=eck4 +lon_0=0 +x_0=0 +y_0=0 +ellps=WGS84 +datum=WGS84 +units=m +no_defs");
//BOOST_GEOMETRY_PROJECTIONS_DETAIL_ESRI_TRAITS(54013, );
//BOOST_GEOMETRY_PROJECTIONS_DETAIL_ESRI_TRAITS(54014, );
//BOOST_GEOMETRY_PROJECTIONS_DETAIL_ESRI_TRAITS(54015, );
BOOST_GEOMETRY_PROJECTIONS_DETAIL_ESRI_TRAITS(54016, srs::proj::gall, srs_spheroid_tag, "+proj=gall +lon_0=0 +x_0=0 +y_0=0 +ellps=WGS84 +datum=WGS84 +units=m +no_defs");
//BOOST_GEOMETRY_PROJECTIONS_DETAIL_ESRI_TRAITS(54017, );
//BOOST_GEOMETRY_PROJECTIONS_DETAIL_ESRI_TRAITS(54018, );
//BOOST_GEOMETRY_PROJECTIONS_DETAIL_ESRI_TRAITS(54019, );
BOOST_GEOMETRY_PROJECTIONS_DETAIL_ESRI_TRAITS(54021, srs::proj::poly, srs_spheroid_tag, "+proj=poly +lat_0=0 +lon_0=0 +x_0=0 +y_0=0 +ellps=WGS84 +datum=WGS84 +units=m +no_defs");
//BOOST_GEOMETRY_PROJECTIONS_DETAIL_ESRI_TRAITS(54022, );
//BOOST_GEOMETRY_PROJECTIONS_DETAIL_ESRI_TRAITS(54023, );
BOOST_GEOMETRY_PROJECTIONS_DETAIL_ESRI_TRAITS(54024, srs::proj::bonne, srs_spheroid_tag, "+proj=bonne +lon_0=0 +lat_1=60 +x_0=0 +y_0=0 +ellps=WGS84 +datum=WGS84 +units=m +no_defs");
BOOST_GEOMETRY_PROJECTIONS_DETAIL_ESRI_TRAITS(54025, srs::proj::omerc, srs_spheroid_tag, "+proj=omerc +lat_0=40 +lon_1=0 +lat_1=0 +lon_2=0 +lat_2=0 +k=1 +x_0=0 +y_0=0 +ellps=WGS84 +datum=WGS84 +units=m +no_defs");
BOOST_GEOMETRY_PROJECTIONS_DETAIL_ESRI_TRAITS(54026, srs::proj::stere, srs_spheroid_tag, "+proj=stere +lat_0=0 +lon_0=0 +k=1 +x_0=0 +y_0=0 +ellps=WGS84 +datum=WGS84 +units=m +no_defs");
BOOST_GEOMETRY_PROJECTIONS_DETAIL_ESRI_TRAITS(54027, srs::proj::eqdc, srs_spheroid_tag, "+proj=eqdc +lat_0=0 +lon_0=0 +lat_1=60 +lat_2=60 +x_0=0 +y_0=0 +ellps=WGS84 +datum=WGS84 +units=m +no_defs");
BOOST_GEOMETRY_PROJECTIONS_DETAIL_ESRI_TRAITS(54028, srs::proj::cass, srs_spheroid_tag, "+proj=cass +lat_0=0 +lon_0=0 +x_0=0 +y_0=0 +ellps=WGS84 +datum=WGS84 +units=m +no_defs");
BOOST_GEOMETRY_PROJECTIONS_DETAIL_ESRI_TRAITS(54029, srs::proj::vandg, srs_spheroid_tag, "+proj=vandg +lon_0=0 +x_0=0 +y_0=0 +R_A +ellps=WGS84 +datum=WGS84 +units=m +no_defs");
BOOST_GEOMETRY_PROJECTIONS_DETAIL_ESRI_TRAITS(54030, srs::proj::robin, srs_spheroid_tag, "+proj=robin +lon_0=0 +x_0=0 +y_0=0 +ellps=WGS84 +datum=WGS84 +units=m +no_defs");
BOOST_GEOMETRY_PROJECTIONS_DETAIL_ESRI_TRAITS(54031, srs::proj::tpeqd, srs_spheroid_tag, "+proj=tpeqd +lat_1=0 +lon_1=0 +lat_2=60 +lon_2=60 +x_0=0 +y_0=0 +ellps=WGS84 +datum=WGS84 +units=m +no_defs");
BOOST_GEOMETRY_PROJECTIONS_DETAIL_ESRI_TRAITS(54032, srs::proj::aeqd, srs_spheroid_tag, "+proj=aeqd +lat_0=0 +lon_0=0 +x_0=0 +y_0=0 +ellps=WGS84 +datum=WGS84 +units=m +no_defs");
BOOST_GEOMETRY_PROJECTIONS_DETAIL_ESRI_TRAITS(65061, srs::proj::poly, srs_spheroid_tag, "+proj=poly +lat_0=13.47246635277778 +lon_0=-144.7487507055556 +x_0=50000.00000000001 +y_0=50000.00000000001 +ellps=clrk66 +datum=NAD27 +to_meter=0.3048006096012192 +no_defs");
BOOST_GEOMETRY_PROJECTIONS_DETAIL_ESRI_TRAITS(65161, srs::proj::poly, srs_spheroid_tag, "+proj=poly +lat_0=13.47246635277778 +lon_0=-144.7487507055556 +x_0=50000 +y_0=50000 +ellps=GRS80 +datum=NAD83 +units=m +no_defs");
BOOST_GEOMETRY_PROJECTIONS_DETAIL_ESRI_TRAITS(102001, srs::proj::aea, srs_spheroid_tag, "+proj=aea +lat_1=50 +lat_2=70 +lat_0=40 +lon_0=-96 +x_0=0 +y_0=0 +ellps=GRS80 +datum=NAD83 +units=m +no_defs");
BOOST_GEOMETRY_PROJECTIONS_DETAIL_ESRI_TRAITS(102002, srs::proj::lcc, srs_spheroid_tag, "+proj=lcc +lat_1=50 +lat_2=70 +lat_0=40 +lon_0=-96 +x_0=0 +y_0=0 +ellps=GRS80 +datum=NAD83 +units=m +no_defs");
BOOST_GEOMETRY_PROJECTIONS_DETAIL_ESRI_TRAITS(102003, srs::proj::aea, srs_spheroid_tag, "+proj=aea +lat_1=29.5 +lat_2=45.5 +lat_0=37.5 +lon_0=-96 +x_0=0 +y_0=0 +ellps=GRS80 +datum=NAD83 +units=m +no_defs");
BOOST_GEOMETRY_PROJECTIONS_DETAIL_ESRI_TRAITS(102004, srs::proj::lcc, srs_spheroid_tag, "+proj=lcc +lat_1=33 +lat_2=45 +lat_0=39 +lon_0=-96 +x_0=0 +y_0=0 +ellps=GRS80 +datum=NAD83 +units=m +no_defs");
BOOST_GEOMETRY_PROJECTIONS_DETAIL_ESRI_TRAITS(102005, srs::proj::eqdc, srs_spheroid_tag, "+proj=eqdc +lat_0=0 +lon_0=0 +lat_1=33 +lat_2=45 +x_0=0 +y_0=0 +ellps=GRS80 +datum=NAD83 +units=m +no_defs");
BOOST_GEOMETRY_PROJECTIONS_DETAIL_ESRI_TRAITS(102006, srs::proj::aea, srs_spheroid_tag, "+proj=aea +lat_1=55 +lat_2=65 +lat_0=50 +lon_0=-154 +x_0=0 +y_0=0 +ellps=GRS80 +datum=NAD83 +units=m +no_defs");
BOOST_GEOMETRY_PROJECTIONS_DETAIL_ESRI_TRAITS(102007, srs::proj::aea, srs_spheroid_tag, "+proj=aea +lat_1=8 +lat_2=18 +lat_0=13 +lon_0=-157 +x_0=0 +y_0=0 +ellps=GRS80 +datum=NAD83 +units=m +no_defs");
BOOST_GEOMETRY_PROJECTIONS_DETAIL_ESRI_TRAITS(102008, srs::proj::aea, srs_spheroid_tag, "+proj=aea +lat_1=20 +lat_2=60 +lat_0=40 +lon_0=-96 +x_0=0 +y_0=0 +ellps=GRS80 +datum=NAD83 +units=m +no_defs");
BOOST_GEOMETRY_PROJECTIONS_DETAIL_ESRI_TRAITS(102009, srs::proj::lcc, srs_spheroid_tag, "+proj=lcc +lat_1=20 +lat_2=60 +lat_0=40 +lon_0=-96 +x_0=0 +y_0=0 +ellps=GRS80 +datum=NAD83 +units=m +no_defs");
BOOST_GEOMETRY_PROJECTIONS_DETAIL_ESRI_TRAITS(102010, srs::proj::eqdc, srs_spheroid_tag, "+proj=eqdc +lat_0=0 +lon_0=0 +lat_1=20 +lat_2=60 +x_0=0 +y_0=0 +ellps=GRS80 +datum=NAD83 +units=m +no_defs");
BOOST_GEOMETRY_PROJECTIONS_DETAIL_ESRI_TRAITS(102011, srs::proj::sinu, srs_spheroid_tag, "+proj=sinu +lon_0=0 +x_0=0 +y_0=0 +ellps=WGS84 +datum=WGS84 +units=m +no_defs");
BOOST_GEOMETRY_PROJECTIONS_DETAIL_ESRI_TRAITS(102012, srs::proj::lcc, srs_spheroid_tag, "+proj=lcc +lat_1=30 +lat_2=62 +lat_0=0 +lon_0=105 +x_0=0 +y_0=0 +ellps=WGS84 +datum=WGS84 +units=m +no_defs");
BOOST_GEOMETRY_PROJECTIONS_DETAIL_ESRI_TRAITS(102013, srs::proj::aea, srs_spheroid_tag, "+proj=aea +lat_1=43 +lat_2=62 +lat_0=30 +lon_0=10 +x_0=0 +y_0=0 +ellps=intl +units=m +no_defs");
BOOST_GEOMETRY_PROJECTIONS_DETAIL_ESRI_TRAITS(102014, srs::proj::lcc, srs_spheroid_tag, "+proj=lcc +lat_1=43 +lat_2=62 +lat_0=30 +lon_0=10 +x_0=0 +y_0=0 +ellps=intl +units=m +no_defs");
BOOST_GEOMETRY_PROJECTIONS_DETAIL_ESRI_TRAITS(102015, srs::proj::lcc, srs_spheroid_tag, "+proj=lcc +lat_1=-5 +lat_2=-42 +lat_0=-32 +lon_0=-60 +x_0=0 +y_0=0 +ellps=aust_SA +units=m +no_defs");
BOOST_GEOMETRY_PROJECTIONS_DETAIL_ESRI_TRAITS(102016, srs::proj::aeqd, srs_spheroid_tag, "+proj=aeqd +lat_0=90 +lon_0=0 +x_0=0 +y_0=0 +ellps=WGS84 +datum=WGS84 +units=m +no_defs");
BOOST_GEOMETRY_PROJECTIONS_DETAIL_ESRI_TRAITS(102017, srs::proj::laea, srs_spheroid_tag, "+proj=laea +lat_0=90 +lon_0=0 +x_0=0 +y_0=0 +ellps=WGS84 +datum=WGS84 +units=m +no_defs");
BOOST_GEOMETRY_PROJECTIONS_DETAIL_ESRI_TRAITS(102018, srs::proj::stere, srs_spheroid_tag, "+proj=stere +lat_0=90 +lon_0=0 +k=1 +x_0=0 +y_0=0 +ellps=WGS84 +datum=WGS84 +units=m +no_defs");
BOOST_GEOMETRY_PROJECTIONS_DETAIL_ESRI_TRAITS(102019, srs::proj::aeqd, srs_spheroid_tag, "+proj=aeqd +lat_0=-90 +lon_0=0 +x_0=0 +y_0=0 +ellps=WGS84 +datum=WGS84 +units=m +no_defs");
BOOST_GEOMETRY_PROJECTIONS_DETAIL_ESRI_TRAITS(102020, srs::proj::laea, srs_spheroid_tag, "+proj=laea +lat_0=-90 +lon_0=0 +x_0=0 +y_0=0 +ellps=WGS84 +datum=WGS84 +units=m +no_defs");
BOOST_GEOMETRY_PROJECTIONS_DETAIL_ESRI_TRAITS(102021, srs::proj::stere, srs_spheroid_tag, "+proj=stere +lat_0=-90 +lon_0=0 +k=1 +x_0=0 +y_0=0 +ellps=WGS84 +datum=WGS84 +units=m +no_defs");
BOOST_GEOMETRY_PROJECTIONS_DETAIL_ESRI_TRAITS(102022, srs::proj::aea, srs_spheroid_tag, "+proj=aea +lat_1=20 +lat_2=-23 +lat_0=0 +lon_0=25 +x_0=0 +y_0=0 +ellps=WGS84 +datum=WGS84 +units=m +no_defs");
BOOST_GEOMETRY_PROJECTIONS_DETAIL_ESRI_TRAITS(102023, srs::proj::eqdc, srs_spheroid_tag, "+proj=eqdc +lat_0=0 +lon_0=0 +lat_1=20 +lat_2=-23 +x_0=0 +y_0=0 +ellps=WGS84 +datum=WGS84 +units=m +no_defs");
BOOST_GEOMETRY_PROJECTIONS_DETAIL_ESRI_TRAITS(102024, srs::proj::lcc, srs_spheroid_tag, "+proj=lcc +lat_1=20 +lat_2=-23 +lat_0=0 +lon_0=25 +x_0=0 +y_0=0 +ellps=WGS84 +datum=WGS84 +units=m +no_defs");
BOOST_GEOMETRY_PROJECTIONS_DETAIL_ESRI_TRAITS(102025, srs::proj::aea, srs_spheroid_tag, "+proj=aea +lat_1=15 +lat_2=65 +lat_0=30 +lon_0=95 +x_0=0 +y_0=0 +ellps=WGS84 +datum=WGS84 +units=m +no_defs");
BOOST_GEOMETRY_PROJECTIONS_DETAIL_ESRI_TRAITS(102026, srs::proj::eqdc, srs_spheroid_tag, "+proj=eqdc +lat_0=0 +lon_0=0 +lat_1=15 +lat_2=65 +x_0=0 +y_0=0 +ellps=WGS84 +datum=WGS84 +units=m +no_defs");
BOOST_GEOMETRY_PROJECTIONS_DETAIL_ESRI_TRAITS(102027, srs::proj::lcc, srs_spheroid_tag, "+proj=lcc +lat_1=15 +lat_2=65 +lat_0=30 +lon_0=95 +x_0=0 +y_0=0 +ellps=WGS84 +datum=WGS84 +units=m +no_defs");
BOOST_GEOMETRY_PROJECTIONS_DETAIL_ESRI_TRAITS(102028, srs::proj::aea, srs_spheroid_tag, "+proj=aea +lat_1=7 +lat_2=-32 +lat_0=-15 +lon_0=125 +x_0=0 +y_0=0 +ellps=WGS84 +datum=WGS84 +units=m +no_defs");
BOOST_GEOMETRY_PROJECTIONS_DETAIL_ESRI_TRAITS(102029, srs::proj::eqdc, srs_spheroid_tag, "+proj=eqdc +lat_0=0 +lon_0=0 +lat_1=7 +lat_2=-32 +x_0=0 +y_0=0 +ellps=WGS84 +datum=WGS84 +units=m +no_defs");
BOOST_GEOMETRY_PROJECTIONS_DETAIL_ESRI_TRAITS(102030, srs::proj::lcc, srs_spheroid_tag, "+proj=lcc +lat_1=7 +lat_2=-32 +lat_0=-15 +lon_0=125 +x_0=0 +y_0=0 +ellps=WGS84 +datum=WGS84 +units=m +no_defs");
BOOST_GEOMETRY_PROJECTIONS_DETAIL_ESRI_TRAITS(102031, srs::proj::eqdc, srs_spheroid_tag, "+proj=eqdc +lat_0=0 +lon_0=0 +lat_1=43 +lat_2=62 +x_0=0 +y_0=0 +ellps=intl +units=m +no_defs");
BOOST_GEOMETRY_PROJECTIONS_DETAIL_ESRI_TRAITS(102032, srs::proj::eqdc, srs_spheroid_tag, "+proj=eqdc +lat_0=0 +lon_0=0 +lat_1=-5 +lat_2=-42 +x_0=0 +y_0=0 +ellps=aust_SA +units=m +no_defs");
BOOST_GEOMETRY_PROJECTIONS_DETAIL_ESRI_TRAITS(102033, srs::proj::aea, srs_spheroid_tag, "+proj=aea +lat_1=-5 +lat_2=-42 +lat_0=-32 +lon_0=-60 +x_0=0 +y_0=0 +ellps=aust_SA +units=m +no_defs");
BOOST_GEOMETRY_PROJECTIONS_DETAIL_ESRI_TRAITS(102065, srs::proj::krovak, srs_spheroid_tag, "+proj=krovak +lat_0=49.5 +lon_0=24.83333333333333 +alpha=30.28813975277778 +k=0.9999 +x_0=0 +y_0=0 +ellps=bessel +units=m +no_defs");
BOOST_GEOMETRY_PROJECTIONS_DETAIL_ESRI_TRAITS(102066, srs::proj::krovak, srs_spheroid_tag, "+proj=krovak +lat_0=49.5 +lon_0=42.5 +alpha=30.28813975277778 +k=0.9999 +x_0=0 +y_0=0 +ellps=bessel +pm=-17.66666666666667 +units=m +no_defs");
BOOST_GEOMETRY_PROJECTIONS_DETAIL_ESRI_TRAITS(102067, srs::proj::krovak, srs_spheroid_tag, "+proj=krovak +lat_0=49.5 +lon_0=24.83333333333333 +alpha=30.28813975277778 +k=0.9999 +x_0=0 +y_0=0 +ellps=bessel +units=m +no_defs");
BOOST_GEOMETRY_PROJECTIONS_DETAIL_ESRI_TRAITS(102091, srs::proj::tmerc, srs_spheroid_tag, "+proj=tmerc +lat_0=0 +lon_0=9 +k=0.9996 +x_0=1500000 +y_0=0 +ellps=intl +units=m +no_defs");
BOOST_GEOMETRY_PROJECTIONS_DETAIL_ESRI_TRAITS(102092, srs::proj::tmerc, srs_spheroid_tag, "+proj=tmerc +lat_0=0 +lon_0=15 +k=0.9996 +x_0=2520000 +y_0=0 +ellps=intl +units=m +no_defs");
BOOST_GEOMETRY_PROJECTIONS_DETAIL_ESRI_TRAITS(102101, srs::proj::tmerc, srs_spheroid_tag, "+proj=tmerc +lat_0=58 +lon_0=6.05625 +k=1 +x_0=0 +y_0=0 +a=6377492.018 +b=6356173.508712696 +units=m +no_defs");
BOOST_GEOMETRY_PROJECTIONS_DETAIL_ESRI_TRAITS(102102, srs::proj::tmerc, srs_spheroid_tag, "+proj=tmerc +lat_0=58 +lon_0=8.389583333333333 +k=1 +x_0=0 +y_0=0 +a=6377492.018 +b=6356173.508712696 +units=m +no_defs");
BOOST_GEOMETRY_PROJECTIONS_DETAIL_ESRI_TRAITS(102103, srs::proj::tmerc, srs_spheroid_tag, "+proj=tmerc +lat_0=58 +lon_0=10.72291666666667 +k=1 +x_0=0 +y_0=0 +a=6377492.018 +b=6356173.508712696 +units=m +no_defs");
BOOST_GEOMETRY_PROJECTIONS_DETAIL_ESRI_TRAITS(102104, srs::proj::tmerc, srs_spheroid_tag, "+proj=tmerc +lat_0=58 +lon_0=13.22291666666667 +k=1 +x_0=0 +y_0=0 +a=6377492.018 +b=6356173.508712696 +units=m +no_defs");
BOOST_GEOMETRY_PROJECTIONS_DETAIL_ESRI_TRAITS(102105, srs::proj::tmerc, srs_spheroid_tag, "+proj=tmerc +lat_0=58 +lon_0=16.88958333333333 +k=1 +x_0=0 +y_0=0 +a=6377492.018 +b=6356173.508712696 +units=m +no_defs");
BOOST_GEOMETRY_PROJECTIONS_DETAIL_ESRI_TRAITS(102106, srs::proj::tmerc, srs_spheroid_tag, "+proj=tmerc +lat_0=58 +lon_0=20.88958333333333 +k=1 +x_0=0 +y_0=0 +a=6377492.018 +b=6356173.508712696 +units=m +no_defs");
BOOST_GEOMETRY_PROJECTIONS_DETAIL_ESRI_TRAITS(102107, srs::proj::tmerc, srs_spheroid_tag, "+proj=tmerc +lat_0=58 +lon_0=24.88958333333333 +k=1 +x_0=0 +y_0=0 +a=6377492.018 +b=6356173.508712696 +units=m +no_defs");
BOOST_GEOMETRY_PROJECTIONS_DETAIL_ESRI_TRAITS(102108, srs::proj::tmerc, srs_spheroid_tag, "+proj=tmerc +lat_0=58 +lon_0=29.05625 +k=1 +x_0=0 +y_0=0 +a=6377492.018 +b=6356173.508712696 +units=m +no_defs");
BOOST_GEOMETRY_PROJECTIONS_DETAIL_ESRI_TRAITS(102110, srs::proj::lcc, srs_spheroid_tag, "+proj=lcc +lat_1=44 +lat_2=49 +lat_0=46.5 +lon_0=3 +x_0=700000 +y_0=6600000 +ellps=GRS80 +units=m +no_defs");
BOOST_GEOMETRY_PROJECTIONS_DETAIL_ESRI_TRAITS(102114, srs::proj::utm, srs_spheroid_tag, "+proj=utm +zone=4 +ellps=clrk66 +units=m +no_defs");
BOOST_GEOMETRY_PROJECTIONS_DETAIL_ESRI_TRAITS(102115, srs::proj::utm, srs_spheroid_tag, "+proj=utm +zone=5 +ellps=clrk66 +units=m +no_defs");
BOOST_GEOMETRY_PROJECTIONS_DETAIL_ESRI_TRAITS(102120, srs::proj::omerc, srs_spheroid_tag, "+proj=omerc +lat_0=45.30916666666666 +lonc=-86 +alpha=337.255555555556 +k=0.9996 +x_0=2546731.495961392 +y_0=-4354009.816002033 +ellps=clrk66 +datum=NAD27 +to_meter=0.3048006096012192 +no_defs");
BOOST_GEOMETRY_PROJECTIONS_DETAIL_ESRI_TRAITS(102121, srs::proj::omerc, srs_spheroid_tag, "+proj=omerc +lat_0=45.30916666666666 +lonc=-86 +alpha=337.255555555556 +k=0.9996 +x_0=2546731.495961392 +y_0=-4354009.816002033 +ellps=GRS80 +datum=NAD83 +to_meter=0.3048006096012192 +no_defs");
BOOST_GEOMETRY_PROJECTIONS_DETAIL_ESRI_TRAITS(102122, srs::proj::omerc, srs_spheroid_tag, "+proj=omerc +lat_0=45.30916666666666 +lonc=-86 +alpha=337.255555555556 +k=0.9996 +x_0=2546731.496 +y_0=-4354009.816 +ellps=clrk66 +datum=NAD27 +units=m +no_defs");
BOOST_GEOMETRY_PROJECTIONS_DETAIL_ESRI_TRAITS(102123, srs::proj::omerc, srs_spheroid_tag, "+proj=omerc +lat_0=45.30916666666666 +lonc=-86 +alpha=337.255555555556 +k=0.9996 +x_0=2546731.496 +y_0=-4354009.816 +ellps=GRS80 +datum=NAD83 +units=m +no_defs");
BOOST_GEOMETRY_PROJECTIONS_DETAIL_ESRI_TRAITS(102132, srs::proj::utm, srs_spheroid_tag, "+proj=utm +zone=32 +a=6377492.018 +b=6356173.508712696 +units=m +no_defs");
BOOST_GEOMETRY_PROJECTIONS_DETAIL_ESRI_TRAITS(102133, srs::proj::utm, srs_spheroid_tag, "+proj=utm +zone=33 +a=6377492.018 +b=6356173.508712696 +units=m +no_defs");
BOOST_GEOMETRY_PROJECTIONS_DETAIL_ESRI_TRAITS(102134, srs::proj::utm, srs_spheroid_tag, "+proj=utm +zone=34 +a=6377492.018 +b=6356173.508712696 +units=m +no_defs");
BOOST_GEOMETRY_PROJECTIONS_DETAIL_ESRI_TRAITS(102135, srs::proj::utm, srs_spheroid_tag, "+proj=utm +zone=35 +a=6377492.018 +b=6356173.508712696 +units=m +no_defs");
BOOST_GEOMETRY_PROJECTIONS_DETAIL_ESRI_TRAITS(102140, srs::proj::tmerc, srs_spheroid_tag, "+proj=tmerc +lat_0=22.31213333333334 +lon_0=114.1785555555556 +k=1 +x_0=836694.05 +y_0=819069.8 +ellps=intl +units=m +no_defs");
BOOST_GEOMETRY_PROJECTIONS_DETAIL_ESRI_TRAITS(102141, srs::proj::utm, srs_spheroid_tag, "+proj=utm +zone=49 +ellps=intl +units=m +no_defs");
BOOST_GEOMETRY_PROJECTIONS_DETAIL_ESRI_TRAITS(102142, srs::proj::utm, srs_spheroid_tag, "+proj=utm +zone=50 +ellps=intl +units=m +no_defs");
BOOST_GEOMETRY_PROJECTIONS_DETAIL_ESRI_TRAITS(102151, srs::proj::utm, srs_spheroid_tag, "+proj=utm +zone=51 +ellps=bessel +units=m +no_defs");
BOOST_GEOMETRY_PROJECTIONS_DETAIL_ESRI_TRAITS(102152, srs::proj::utm, srs_spheroid_tag, "+proj=utm +zone=52 +ellps=bessel +units=m +no_defs");
BOOST_GEOMETRY_PROJECTIONS_DETAIL_ESRI_TRAITS(102153, srs::proj::utm, srs_spheroid_tag, "+proj=utm +zone=53 +ellps=bessel +units=m +no_defs");
BOOST_GEOMETRY_PROJECTIONS_DETAIL_ESRI_TRAITS(102154, srs::proj::utm, srs_spheroid_tag, "+proj=utm +zone=54 +ellps=bessel +units=m +no_defs");
BOOST_GEOMETRY_PROJECTIONS_DETAIL_ESRI_TRAITS(102155, srs::proj::utm, srs_spheroid_tag, "+proj=utm +zone=55 +ellps=bessel +units=m +no_defs");
BOOST_GEOMETRY_PROJECTIONS_DETAIL_ESRI_TRAITS(102156, srs::proj::utm, srs_spheroid_tag, "+proj=utm +zone=56 +ellps=bessel +units=m +no_defs");
BOOST_GEOMETRY_PROJECTIONS_DETAIL_ESRI_TRAITS(102160, srs::proj::tmerc, srs_spheroid_tag, "+proj=tmerc +lat_0=39.66666666666666 +lon_0=-8.131906111111112 +k=1 +x_0=200180.598 +y_0=299913.01 +ellps=intl +units=m +no_defs");
BOOST_GEOMETRY_PROJECTIONS_DETAIL_ESRI_TRAITS(102161, srs::proj::tmerc, srs_spheroid_tag, "+proj=tmerc +lat_0=39.66666666666666 +lon_0=-8.131906111111112 +k=1 +x_0=180.598 +y_0=-86.98999999999999 +ellps=intl +units=m +no_defs");
BOOST_GEOMETRY_PROJECTIONS_DETAIL_ESRI_TRAITS(102162, srs::proj::utm, srs_spheroid_tag, "+proj=utm +zone=26 +ellps=intl +units=m +no_defs");
BOOST_GEOMETRY_PROJECTIONS_DETAIL_ESRI_TRAITS(102163, srs::proj::bonne, srs_spheroid_tag, "+proj=bonne +lon_0=-8.131906111111112 +lat_1=39.66666666666666 +x_0=0 +y_0=0 +ellps=bessel +units=m +no_defs");
BOOST_GEOMETRY_PROJECTIONS_DETAIL_ESRI_TRAITS(102164, srs::proj::tmerc, srs_spheroid_tag, "+proj=tmerc +lat_0=39.66666666666666 +lon_0=-8.131906111111112 +k=1 +x_0=200000 +y_0=300000 +ellps=intl +units=m +no_defs");
BOOST_GEOMETRY_PROJECTIONS_DETAIL_ESRI_TRAITS(102165, srs::proj::tmerc, srs_spheroid_tag, "+proj=tmerc +lat_0=39.66666666666666 +lon_0=-8.131906111111112 +k=1 +x_0=0 +y_0=0 +ellps=intl +units=m +no_defs");
BOOST_GEOMETRY_PROJECTIONS_DETAIL_ESRI_TRAITS(102166, srs::proj::utm, srs_spheroid_tag, "+proj=utm +zone=25 +ellps=intl +units=m +no_defs");
BOOST_GEOMETRY_PROJECTIONS_DETAIL_ESRI_TRAITS(102167, srs::proj::utm, srs_spheroid_tag, "+proj=utm +zone=28 +ellps=intl +units=m +no_defs");
BOOST_GEOMETRY_PROJECTIONS_DETAIL_ESRI_TRAITS(102168, srs::proj::utm, srs_spheroid_tag, "+proj=utm +zone=26 +ellps=intl +units=m +no_defs");
BOOST_GEOMETRY_PROJECTIONS_DETAIL_ESRI_TRAITS(102169, srs::proj::utm, srs_spheroid_tag, "+proj=utm +zone=28 +ellps=intl +units=m +no_defs");
BOOST_GEOMETRY_PROJECTIONS_DETAIL_ESRI_TRAITS(102191, srs::proj::lcc, srs_spheroid_tag, "+proj=lcc +lat_1=33.3 +lat_0=33.3 +lon_0=-5.4 +k_0=0.999625769 +x_0=500000 +y_0=300000 +a=6378249.2 +b=6356514.999904194 +units=m +no_defs");
BOOST_GEOMETRY_PROJECTIONS_DETAIL_ESRI_TRAITS(102192, srs::proj::lcc, srs_spheroid_tag, "+proj=lcc +lat_1=29.7 +lat_0=29.7 +lon_0=-5.4 +k_0=0.9996155960000001 +x_0=500000 +y_0=300000 +a=6378249.2 +b=6356514.999904194 +units=m +no_defs");
BOOST_GEOMETRY_PROJECTIONS_DETAIL_ESRI_TRAITS(102193, srs::proj::lcc, srs_spheroid_tag, "+proj=lcc +lat_1=26.1 +lat_0=26.1 +lon_0=-5.4 +k_0=0.9996 +x_0=1200000 +y_0=400000 +a=6378249.2 +b=6356514.999904194 +units=m +no_defs");
BOOST_GEOMETRY_PROJECTIONS_DETAIL_ESRI_TRAITS(102229, srs::proj::tmerc, srs_spheroid_tag, "+proj=tmerc +lat_0=30.5 +lon_0=-85.83333333333333 +k=0.99996 +x_0=200000 +y_0=0 +ellps=GRS80 +units=m +no_defs");
BOOST_GEOMETRY_PROJECTIONS_DETAIL_ESRI_TRAITS(102230, srs::proj::tmerc, srs_spheroid_tag, "+proj=tmerc +lat_0=30 +lon_0=-87.5 +k=0.9999333333333333 +x_0=600000 +y_0=0 +ellps=GRS80 +units=m +no_defs");
BOOST_GEOMETRY_PROJECTIONS_DETAIL_ESRI_TRAITS(102241, srs::proj::lcc, srs_spheroid_tag, "+proj=lcc +lat_1=40 +lat_2=41.66666666666666 +lat_0=39.33333333333334 +lon_0=-122 +x_0=2000000 +y_0=500000 +ellps=GRS80 +units=m +no_defs");
BOOST_GEOMETRY_PROJECTIONS_DETAIL_ESRI_TRAITS(102242, srs::proj::lcc, srs_spheroid_tag, "+proj=lcc +lat_1=38.33333333333334 +lat_2=39.83333333333334 +lat_0=37.66666666666666 +lon_0=-122 +x_0=2000000 +y_0=500000 +ellps=GRS80 +units=m +no_defs");
BOOST_GEOMETRY_PROJECTIONS_DETAIL_ESRI_TRAITS(102243, srs::proj::lcc, srs_spheroid_tag, "+proj=lcc +lat_1=37.06666666666667 +lat_2=38.43333333333333 +lat_0=36.5 +lon_0=-120.5 +x_0=2000000 +y_0=500000 +ellps=GRS80 +units=m +no_defs");
BOOST_GEOMETRY_PROJECTIONS_DETAIL_ESRI_TRAITS(102244, srs::proj::lcc, srs_spheroid_tag, "+proj=lcc +lat_1=36 +lat_2=37.25 +lat_0=35.33333333333334 +lon_0=-119 +x_0=2000000 +y_0=500000 +ellps=GRS80 +units=m +no_defs");
BOOST_GEOMETRY_PROJECTIONS_DETAIL_ESRI_TRAITS(102245, srs::proj::lcc, srs_spheroid_tag, "+proj=lcc +lat_1=34.03333333333333 +lat_2=35.46666666666667 +lat_0=33.5 +lon_0=-118 +x_0=2000000 +y_0=500000 +ellps=GRS80 +units=m +no_defs");
BOOST_GEOMETRY_PROJECTIONS_DETAIL_ESRI_TRAITS(102246, srs::proj::lcc, srs_spheroid_tag, "+proj=lcc +lat_1=32.78333333333333 +lat_2=33.88333333333333 +lat_0=32.16666666666666 +lon_0=-116.25 +x_0=2000000 +y_0=500000 +ellps=GRS80 +units=m +no_defs");
BOOST_GEOMETRY_PROJECTIONS_DETAIL_ESRI_TRAITS(102248, srs::proj::tmerc, srs_spheroid_tag, "+proj=tmerc +lat_0=31 +lon_0=-110.1666666666667 +k=0.9999 +x_0=213360 +y_0=0 +ellps=GRS80 +units=m +no_defs");
BOOST_GEOMETRY_PROJECTIONS_DETAIL_ESRI_TRAITS(102249, srs::proj::tmerc, srs_spheroid_tag, "+proj=tmerc +lat_0=31 +lon_0=-111.9166666666667 +k=0.9999 +x_0=213360 +y_0=0 +ellps=GRS80 +units=m +no_defs");
BOOST_GEOMETRY_PROJECTIONS_DETAIL_ESRI_TRAITS(102250, srs::proj::tmerc, srs_spheroid_tag, "+proj=tmerc +lat_0=31 +lon_0=-113.75 +k=0.9999333333333333 +x_0=213360 +y_0=0 +ellps=GRS80 +units=m +no_defs");
BOOST_GEOMETRY_PROJECTIONS_DETAIL_ESRI_TRAITS(102251, srs::proj::lcc, srs_spheroid_tag, "+proj=lcc +lat_1=34.93333333333333 +lat_2=36.23333333333333 +lat_0=34.33333333333334 +lon_0=-92 +x_0=400000 +y_0=0 +ellps=GRS80 +units=m +no_defs");
BOOST_GEOMETRY_PROJECTIONS_DETAIL_ESRI_TRAITS(102252, srs::proj::lcc, srs_spheroid_tag, "+proj=lcc +lat_1=33.3 +lat_2=34.76666666666667 +lat_0=32.66666666666666 +lon_0=-92 +x_0=400000 +y_0=400000 +ellps=GRS80 +units=m +no_defs");
BOOST_GEOMETRY_PROJECTIONS_DETAIL_ESRI_TRAITS(102253, srs::proj::lcc, srs_spheroid_tag, "+proj=lcc +lat_1=39.71666666666667 +lat_2=40.78333333333333 +lat_0=39.33333333333334 +lon_0=-105.5 +x_0=914401.8289 +y_0=304800.6096 +ellps=GRS80 +units=m +no_defs");
BOOST_GEOMETRY_PROJECTIONS_DETAIL_ESRI_TRAITS(102254, srs::proj::lcc, srs_spheroid_tag, "+proj=lcc +lat_1=38.45 +lat_2=39.75 +lat_0=37.83333333333334 +lon_0=-105.5 +x_0=914401.8289 +y_0=304800.6096 +ellps=GRS80 +units=m +no_defs");
BOOST_GEOMETRY_PROJECTIONS_DETAIL_ESRI_TRAITS(102255, srs::proj::lcc, srs_spheroid_tag, "+proj=lcc +lat_1=37.23333333333333 +lat_2=38.43333333333333 +lat_0=36.66666666666666 +lon_0=-105.5 +x_0=914401.8289 +y_0=304800.6096 +ellps=GRS80 +units=m +no_defs");
BOOST_GEOMETRY_PROJECTIONS_DETAIL_ESRI_TRAITS(102256, srs::proj::lcc, srs_spheroid_tag, "+proj=lcc +lat_1=41.2 +lat_2=41.86666666666667 +lat_0=40.83333333333334 +lon_0=-72.75 +x_0=304800.6096 +y_0=152400.3048 +ellps=GRS80 +units=m +no_defs");
BOOST_GEOMETRY_PROJECTIONS_DETAIL_ESRI_TRAITS(102257, srs::proj::tmerc, srs_spheroid_tag, "+proj=tmerc +lat_0=38 +lon_0=-75.41666666666667 +k=0.999995 +x_0=200000 +y_0=0 +ellps=GRS80 +units=m +no_defs");
BOOST_GEOMETRY_PROJECTIONS_DETAIL_ESRI_TRAITS(102258, srs::proj::tmerc, srs_spheroid_tag, "+proj=tmerc +lat_0=24.33333333333333 +lon_0=-81 +k=0.9999411764705882 +x_0=200000 +y_0=0 +ellps=GRS80 +units=m +no_defs");
BOOST_GEOMETRY_PROJECTIONS_DETAIL_ESRI_TRAITS(102259, srs::proj::tmerc, srs_spheroid_tag, "+proj=tmerc +lat_0=24.33333333333333 +lon_0=-82 +k=0.9999411764705882 +x_0=200000 +y_0=0 +ellps=GRS80 +units=m +no_defs");
BOOST_GEOMETRY_PROJECTIONS_DETAIL_ESRI_TRAITS(102260, srs::proj::lcc, srs_spheroid_tag, "+proj=lcc +lat_1=29.58333333333333 +lat_2=30.75 +lat_0=29 +lon_0=-84.5 +x_0=600000 +y_0=0 +ellps=GRS80 +units=m +no_defs");
BOOST_GEOMETRY_PROJECTIONS_DETAIL_ESRI_TRAITS(102261, srs::proj::tmerc, srs_spheroid_tag, "+proj=tmerc +lat_0=18.83333333333333 +lon_0=-155.5 +k=0.9999666666666667 +x_0=500000 +y_0=0 +ellps=GRS80 +units=m +no_defs");
BOOST_GEOMETRY_PROJECTIONS_DETAIL_ESRI_TRAITS(102262, srs::proj::tmerc, srs_spheroid_tag, "+proj=tmerc +lat_0=20.33333333333333 +lon_0=-156.6666666666667 +k=0.9999666666666667 +x_0=500000 +y_0=0 +ellps=GRS80 +units=m +no_defs");
BOOST_GEOMETRY_PROJECTIONS_DETAIL_ESRI_TRAITS(102263, srs::proj::tmerc, srs_spheroid_tag, "+proj=tmerc +lat_0=21.16666666666667 +lon_0=-158 +k=0.99999 +x_0=500000 +y_0=0 +ellps=GRS80 +units=m +no_defs");
BOOST_GEOMETRY_PROJECTIONS_DETAIL_ESRI_TRAITS(102264, srs::proj::tmerc, srs_spheroid_tag, "+proj=tmerc +lat_0=21.83333333333333 +lon_0=-159.5 +k=0.99999 +x_0=500000 +y_0=0 +ellps=GRS80 +units=m +no_defs");
BOOST_GEOMETRY_PROJECTIONS_DETAIL_ESRI_TRAITS(102265, srs::proj::tmerc, srs_spheroid_tag, "+proj=tmerc +lat_0=21.66666666666667 +lon_0=-160.1666666666667 +k=1 +x_0=500000 +y_0=0 +ellps=GRS80 +units=m +no_defs");
BOOST_GEOMETRY_PROJECTIONS_DETAIL_ESRI_TRAITS(102266, srs::proj::tmerc, srs_spheroid_tag, "+proj=tmerc +lat_0=30 +lon_0=-82.16666666666667 +k=0.9999 +x_0=200000 +y_0=0 +ellps=GRS80 +units=m +no_defs");
BOOST_GEOMETRY_PROJECTIONS_DETAIL_ESRI_TRAITS(102267, srs::proj::tmerc, srs_spheroid_tag, "+proj=tmerc +lat_0=30 +lon_0=-84.16666666666667 +k=0.9999 +x_0=700000 +y_0=0 +ellps=GRS80 +units=m +no_defs");
BOOST_GEOMETRY_PROJECTIONS_DETAIL_ESRI_TRAITS(102268, srs::proj::tmerc, srs_spheroid_tag, "+proj=tmerc +lat_0=41.66666666666666 +lon_0=-112.1666666666667 +k=0.9999473684210526 +x_0=200000 +y_0=0 +ellps=GRS80 +units=m +no_defs");
BOOST_GEOMETRY_PROJECTIONS_DETAIL_ESRI_TRAITS(102269, srs::proj::tmerc, srs_spheroid_tag, "+proj=tmerc +lat_0=41.66666666666666 +lon_0=-114 +k=0.9999473684210526 +x_0=500000 +y_0=0 +ellps=GRS80 +units=m +no_defs");
BOOST_GEOMETRY_PROJECTIONS_DETAIL_ESRI_TRAITS(102270, srs::proj::tmerc, srs_spheroid_tag, "+proj=tmerc +lat_0=41.66666666666666 +lon_0=-115.75 +k=0.9999333333333333 +x_0=800000 +y_0=0 +ellps=GRS80 +units=m +no_defs");
BOOST_GEOMETRY_PROJECTIONS_DETAIL_ESRI_TRAITS(102271, srs::proj::tmerc, srs_spheroid_tag, "+proj=tmerc +lat_0=36.66666666666666 +lon_0=-88.33333333333333 +k=0.9999749999999999 +x_0=300000 +y_0=0 +ellps=GRS80 +units=m +no_defs");
BOOST_GEOMETRY_PROJECTIONS_DETAIL_ESRI_TRAITS(102272, srs::proj::tmerc, srs_spheroid_tag, "+proj=tmerc +lat_0=36.66666666666666 +lon_0=-90.16666666666667 +k=0.9999411764705882 +x_0=700000 +y_0=0 +ellps=GRS80 +units=m +no_defs");
BOOST_GEOMETRY_PROJECTIONS_DETAIL_ESRI_TRAITS(102273, srs::proj::tmerc, srs_spheroid_tag, "+proj=tmerc +lat_0=37.5 +lon_0=-85.66666666666667 +k=0.9999666666666667 +x_0=100000 +y_0=250000 +ellps=GRS80 +units=m +no_defs");
BOOST_GEOMETRY_PROJECTIONS_DETAIL_ESRI_TRAITS(102274, srs::proj::tmerc, srs_spheroid_tag, "+proj=tmerc +lat_0=37.5 +lon_0=-87.08333333333333 +k=0.9999666666666667 +x_0=900000 +y_0=250000 +ellps=GRS80 +units=m +no_defs");
BOOST_GEOMETRY_PROJECTIONS_DETAIL_ESRI_TRAITS(102277, srs::proj::lcc, srs_spheroid_tag, "+proj=lcc +lat_1=38.71666666666667 +lat_2=39.78333333333333 +lat_0=38.33333333333334 +lon_0=-98 +x_0=400000 +y_0=0 +ellps=GRS80 +units=m +no_defs");
BOOST_GEOMETRY_PROJECTIONS_DETAIL_ESRI_TRAITS(102278, srs::proj::lcc, srs_spheroid_tag, "+proj=lcc +lat_1=37.26666666666667 +lat_2=38.56666666666667 +lat_0=36.66666666666666 +lon_0=-98.5 +x_0=400000 +y_0=400000 +ellps=GRS80 +units=m +no_defs");
BOOST_GEOMETRY_PROJECTIONS_DETAIL_ESRI_TRAITS(102279, srs::proj::lcc, srs_spheroid_tag, "+proj=lcc +lat_1=37.96666666666667 +lat_2=38.96666666666667 +lat_0=37.5 +lon_0=-84.25 +x_0=500000 +y_0=0 +ellps=GRS80 +units=m +no_defs");
BOOST_GEOMETRY_PROJECTIONS_DETAIL_ESRI_TRAITS(102280, srs::proj::lcc, srs_spheroid_tag, "+proj=lcc +lat_1=36.73333333333333 +lat_2=37.93333333333333 +lat_0=36.33333333333334 +lon_0=-85.75 +x_0=500000 +y_0=500000 +ellps=GRS80 +units=m +no_defs");
BOOST_GEOMETRY_PROJECTIONS_DETAIL_ESRI_TRAITS(102281, srs::proj::lcc, srs_spheroid_tag, "+proj=lcc +lat_1=31.16666666666667 +lat_2=32.66666666666666 +lat_0=30.5 +lon_0=-92.5 +x_0=1000000 +y_0=0 +ellps=GRS80 +units=m +no_defs");
BOOST_GEOMETRY_PROJECTIONS_DETAIL_ESRI_TRAITS(102282, srs::proj::lcc, srs_spheroid_tag, "+proj=lcc +lat_1=29.3 +lat_2=30.7 +lat_0=28.5 +lon_0=-91.33333333333333 +x_0=1000000 +y_0=0 +ellps=GRS80 +units=m +no_defs");
BOOST_GEOMETRY_PROJECTIONS_DETAIL_ESRI_TRAITS(102283, srs::proj::tmerc, srs_spheroid_tag, "+proj=tmerc +lat_0=43.66666666666666 +lon_0=-68.5 +k=0.9999 +x_0=300000 +y_0=0 +ellps=GRS80 +units=m +no_defs");
BOOST_GEOMETRY_PROJECTIONS_DETAIL_ESRI_TRAITS(102284, srs::proj::tmerc, srs_spheroid_tag, "+proj=tmerc +lat_0=42.83333333333334 +lon_0=-70.16666666666667 +k=0.9999666666666667 +x_0=900000 +y_0=0 +ellps=GRS80 +units=m +no_defs");
BOOST_GEOMETRY_PROJECTIONS_DETAIL_ESRI_TRAITS(102285, srs::proj::lcc, srs_spheroid_tag, "+proj=lcc +lat_1=38.3 +lat_2=39.45 +lat_0=37.66666666666666 +lon_0=-77 +x_0=400000 +y_0=0 +ellps=GRS80 +units=m +no_defs");
BOOST_GEOMETRY_PROJECTIONS_DETAIL_ESRI_TRAITS(102286, srs::proj::lcc, srs_spheroid_tag, "+proj=lcc +lat_1=41.71666666666667 +lat_2=42.68333333333333 +lat_0=41 +lon_0=-71.5 +x_0=200000 +y_0=750000 +ellps=GRS80 +units=m +no_defs");
BOOST_GEOMETRY_PROJECTIONS_DETAIL_ESRI_TRAITS(102287, srs::proj::lcc, srs_spheroid_tag, "+proj=lcc +lat_1=41.28333333333333 +lat_2=41.48333333333333 +lat_0=41 +lon_0=-70.5 +x_0=500000 +y_0=0 +ellps=GRS80 +units=m +no_defs");
BOOST_GEOMETRY_PROJECTIONS_DETAIL_ESRI_TRAITS(102288, srs::proj::lcc, srs_spheroid_tag, "+proj=lcc +lat_1=45.48333333333333 +lat_2=47.08333333333334 +lat_0=44.78333333333333 +lon_0=-87 +x_0=8000000 +y_0=0 +ellps=GRS80 +units=m +no_defs");
BOOST_GEOMETRY_PROJECTIONS_DETAIL_ESRI_TRAITS(102289, srs::proj::lcc, srs_spheroid_tag, "+proj=lcc +lat_1=44.18333333333333 +lat_2=45.7 +lat_0=43.31666666666667 +lon_0=-84.36666666666666 +x_0=6000000 +y_0=0 +ellps=GRS80 +units=m +no_defs");
BOOST_GEOMETRY_PROJECTIONS_DETAIL_ESRI_TRAITS(102290, srs::proj::lcc, srs_spheroid_tag, "+proj=lcc +lat_1=42.1 +lat_2=43.66666666666666 +lat_0=41.5 +lon_0=-84.36666666666666 +x_0=4000000 +y_0=0 +ellps=GRS80 +units=m +no_defs");
BOOST_GEOMETRY_PROJECTIONS_DETAIL_ESRI_TRAITS(102291, srs::proj::lcc, srs_spheroid_tag, "+proj=lcc +lat_1=47.03333333333333 +lat_2=48.63333333333333 +lat_0=46.5 +lon_0=-93.09999999999999 +x_0=800000 +y_0=100000 +ellps=GRS80 +units=m +no_defs");
BOOST_GEOMETRY_PROJECTIONS_DETAIL_ESRI_TRAITS(102292, srs::proj::lcc, srs_spheroid_tag, "+proj=lcc +lat_1=45.61666666666667 +lat_2=47.05 +lat_0=45 +lon_0=-94.25 +x_0=800000 +y_0=100000 +ellps=GRS80 +units=m +no_defs");
BOOST_GEOMETRY_PROJECTIONS_DETAIL_ESRI_TRAITS(102293, srs::proj::lcc, srs_spheroid_tag, "+proj=lcc +lat_1=43.78333333333333 +lat_2=45.21666666666667 +lat_0=43 +lon_0=-94 +x_0=800000 +y_0=100000 +ellps=GRS80 +units=m +no_defs");
BOOST_GEOMETRY_PROJECTIONS_DETAIL_ESRI_TRAITS(102294, srs::proj::tmerc, srs_spheroid_tag, "+proj=tmerc +lat_0=29.5 +lon_0=-88.83333333333333 +k=0.99995 +x_0=300000 +y_0=0 +ellps=GRS80 +units=m +no_defs");
BOOST_GEOMETRY_PROJECTIONS_DETAIL_ESRI_TRAITS(102295, srs::proj::tmerc, srs_spheroid_tag, "+proj=tmerc +lat_0=29.5 +lon_0=-90.33333333333333 +k=0.99995 +x_0=700000 +y_0=0 +ellps=GRS80 +units=m +no_defs");
BOOST_GEOMETRY_PROJECTIONS_DETAIL_ESRI_TRAITS(102296, srs::proj::tmerc, srs_spheroid_tag, "+proj=tmerc +lat_0=35.83333333333334 +lon_0=-90.5 +k=0.9999333333333333 +x_0=250000 +y_0=0 +ellps=GRS80 +units=m +no_defs");
BOOST_GEOMETRY_PROJECTIONS_DETAIL_ESRI_TRAITS(102297, srs::proj::tmerc, srs_spheroid_tag, "+proj=tmerc +lat_0=35.83333333333334 +lon_0=-92.5 +k=0.9999333333333333 +x_0=500000 +y_0=0 +ellps=GRS80 +units=m +no_defs");
BOOST_GEOMETRY_PROJECTIONS_DETAIL_ESRI_TRAITS(102298, srs::proj::tmerc, srs_spheroid_tag, "+proj=tmerc +lat_0=36.16666666666666 +lon_0=-94.5 +k=0.9999411764705882 +x_0=850000 +y_0=0 +ellps=GRS80 +units=m +no_defs");
BOOST_GEOMETRY_PROJECTIONS_DETAIL_ESRI_TRAITS(102300, srs::proj::lcc, srs_spheroid_tag, "+proj=lcc +lat_1=45 +lat_2=49 +lat_0=44.25 +lon_0=-109.5 +x_0=600000 +y_0=0 +ellps=GRS80 +units=m +no_defs");
BOOST_GEOMETRY_PROJECTIONS_DETAIL_ESRI_TRAITS(102304, srs::proj::lcc, srs_spheroid_tag, "+proj=lcc +lat_1=40 +lat_2=43 +lat_0=39.83333333333334 +lon_0=-100 +x_0=500000 +y_0=0 +ellps=GRS80 +units=m +no_defs");
BOOST_GEOMETRY_PROJECTIONS_DETAIL_ESRI_TRAITS(102307, srs::proj::tmerc, srs_spheroid_tag, "+proj=tmerc +lat_0=34.75 +lon_0=-115.5833333333333 +k=0.9999 +x_0=200000 +y_0=8000000 +ellps=GRS80 +units=m +no_defs");
BOOST_GEOMETRY_PROJECTIONS_DETAIL_ESRI_TRAITS(102308, srs::proj::tmerc, srs_spheroid_tag, "+proj=tmerc +lat_0=34.75 +lon_0=-116.6666666666667 +k=0.9999 +x_0=500000 +y_0=6000000 +ellps=GRS80 +units=m +no_defs");
BOOST_GEOMETRY_PROJECTIONS_DETAIL_ESRI_TRAITS(102309, srs::proj::tmerc, srs_spheroid_tag, "+proj=tmerc +lat_0=34.75 +lon_0=-118.5833333333333 +k=0.9999 +x_0=800000 +y_0=4000000 +ellps=GRS80 +units=m +no_defs");
BOOST_GEOMETRY_PROJECTIONS_DETAIL_ESRI_TRAITS(102310, srs::proj::tmerc, srs_spheroid_tag, "+proj=tmerc +lat_0=42.5 +lon_0=-71.66666666666667 +k=0.9999666666666667 +x_0=300000 +y_0=0 +ellps=GRS80 +units=m +no_defs");
BOOST_GEOMETRY_PROJECTIONS_DETAIL_ESRI_TRAITS(102311, srs::proj::tmerc, srs_spheroid_tag, "+proj=tmerc +lat_0=38.83333333333334 +lon_0=-74.5 +k=0.9999 +x_0=150000 +y_0=0 +ellps=GRS80 +units=m +no_defs");
BOOST_GEOMETRY_PROJECTIONS_DETAIL_ESRI_TRAITS(102312, srs::proj::tmerc, srs_spheroid_tag, "+proj=tmerc +lat_0=31 +lon_0=-104.3333333333333 +k=0.9999090909090909 +x_0=165000 +y_0=0 +ellps=GRS80 +units=m +no_defs");
BOOST_GEOMETRY_PROJECTIONS_DETAIL_ESRI_TRAITS(102313, srs::proj::tmerc, srs_spheroid_tag, "+proj=tmerc +lat_0=31 +lon_0=-106.25 +k=0.9999 +x_0=500000 +y_0=0 +ellps=GRS80 +units=m +no_defs");
BOOST_GEOMETRY_PROJECTIONS_DETAIL_ESRI_TRAITS(102314, srs::proj::tmerc, srs_spheroid_tag, "+proj=tmerc +lat_0=31 +lon_0=-107.8333333333333 +k=0.9999166666666667 +x_0=830000 +y_0=0 +ellps=GRS80 +units=m +no_defs");
BOOST_GEOMETRY_PROJECTIONS_DETAIL_ESRI_TRAITS(102315, srs::proj::tmerc, srs_spheroid_tag, "+proj=tmerc +lat_0=38.83333333333334 +lon_0=-74.5 +k=0.9999 +x_0=150000 +y_0=0 +ellps=GRS80 +units=m +no_defs");
BOOST_GEOMETRY_PROJECTIONS_DETAIL_ESRI_TRAITS(102316, srs::proj::tmerc, srs_spheroid_tag, "+proj=tmerc +lat_0=40 +lon_0=-76.58333333333333 +k=0.9999375 +x_0=250000 +y_0=0 +ellps=GRS80 +units=m +no_defs");
BOOST_GEOMETRY_PROJECTIONS_DETAIL_ESRI_TRAITS(102317, srs::proj::tmerc, srs_spheroid_tag, "+proj=tmerc +lat_0=40 +lon_0=-78.58333333333333 +k=0.9999375 +x_0=350000 +y_0=0 +ellps=GRS80 +units=m +no_defs");
BOOST_GEOMETRY_PROJECTIONS_DETAIL_ESRI_TRAITS(102318, srs::proj::lcc, srs_spheroid_tag, "+proj=lcc +lat_1=40.66666666666666 +lat_2=41.03333333333333 +lat_0=40.16666666666666 +lon_0=-74 +x_0=300000 +y_0=0 +ellps=GRS80 +units=m +no_defs");
BOOST_GEOMETRY_PROJECTIONS_DETAIL_ESRI_TRAITS(102320, srs::proj::lcc, srs_spheroid_tag, "+proj=lcc +lat_1=47.43333333333333 +lat_2=48.73333333333333 +lat_0=47 +lon_0=-100.5 +x_0=600000 +y_0=0 +ellps=GRS80 +units=m +no_defs");
BOOST_GEOMETRY_PROJECTIONS_DETAIL_ESRI_TRAITS(102321, srs::proj::lcc, srs_spheroid_tag, "+proj=lcc +lat_1=46.18333333333333 +lat_2=47.48333333333333 +lat_0=45.66666666666666 +lon_0=-100.5 +x_0=600000 +y_0=0 +ellps=GRS80 +units=m +no_defs");
BOOST_GEOMETRY_PROJECTIONS_DETAIL_ESRI_TRAITS(102322, srs::proj::lcc, srs_spheroid_tag, "+proj=lcc +lat_1=40.43333333333333 +lat_2=41.7 +lat_0=39.66666666666666 +lon_0=-82.5 +x_0=600000 +y_0=0 +ellps=GRS80 +units=m +no_defs");
BOOST_GEOMETRY_PROJECTIONS_DETAIL_ESRI_TRAITS(102323, srs::proj::lcc, srs_spheroid_tag, "+proj=lcc +lat_1=38.73333333333333 +lat_2=40.03333333333333 +lat_0=38 +lon_0=-82.5 +x_0=600000 +y_0=0 +ellps=GRS80 +units=m +no_defs");
BOOST_GEOMETRY_PROJECTIONS_DETAIL_ESRI_TRAITS(102324, srs::proj::lcc, srs_spheroid_tag, "+proj=lcc +lat_1=35.56666666666667 +lat_2=36.76666666666667 +lat_0=35 +lon_0=-98 +x_0=600000 +y_0=0 +ellps=GRS80 +units=m +no_defs");
BOOST_GEOMETRY_PROJECTIONS_DETAIL_ESRI_TRAITS(102325, srs::proj::lcc, srs_spheroid_tag, "+proj=lcc +lat_1=33.93333333333333 +lat_2=35.23333333333333 +lat_0=33.33333333333334 +lon_0=-98 +x_0=600000 +y_0=0 +ellps=GRS80 +units=m +no_defs");
BOOST_GEOMETRY_PROJECTIONS_DETAIL_ESRI_TRAITS(102326, srs::proj::lcc, srs_spheroid_tag, "+proj=lcc +lat_1=44.33333333333334 +lat_2=46 +lat_0=43.66666666666666 +lon_0=-120.5 +x_0=2500000 +y_0=0 +ellps=GRS80 +units=m +no_defs");
BOOST_GEOMETRY_PROJECTIONS_DETAIL_ESRI_TRAITS(102327, srs::proj::lcc, srs_spheroid_tag, "+proj=lcc +lat_1=42.33333333333334 +lat_2=44 +lat_0=41.66666666666666 +lon_0=-120.5 +x_0=1500000 +y_0=0 +ellps=GRS80 +units=m +no_defs");
BOOST_GEOMETRY_PROJECTIONS_DETAIL_ESRI_TRAITS(102330, srs::proj::tmerc, srs_spheroid_tag, "+proj=tmerc +lat_0=41.08333333333334 +lon_0=-71.5 +k=0.99999375 +x_0=100000 +y_0=0 +ellps=GRS80 +units=m +no_defs");
BOOST_GEOMETRY_PROJECTIONS_DETAIL_ESRI_TRAITS(102334, srs::proj::lcc, srs_spheroid_tag, "+proj=lcc +lat_1=44.41666666666666 +lat_2=45.68333333333333 +lat_0=43.83333333333334 +lon_0=-100 +x_0=600000 +y_0=0 +ellps=GRS80 +units=m +no_defs");
BOOST_GEOMETRY_PROJECTIONS_DETAIL_ESRI_TRAITS(102335, srs::proj::lcc, srs_spheroid_tag, "+proj=lcc +lat_1=42.83333333333334 +lat_2=44.4 +lat_0=42.33333333333334 +lon_0=-100.3333333333333 +x_0=600000 +y_0=0 +ellps=GRS80 +units=m +no_defs");
BOOST_GEOMETRY_PROJECTIONS_DETAIL_ESRI_TRAITS(102336, srs::proj::lcc, srs_spheroid_tag, "+proj=lcc +lat_1=35.25 +lat_2=36.41666666666666 +lat_0=34.33333333333334 +lon_0=-86 +x_0=600000 +y_0=0 +ellps=GRS80 +units=m +no_defs");
BOOST_GEOMETRY_PROJECTIONS_DETAIL_ESRI_TRAITS(102337, srs::proj::lcc, srs_spheroid_tag, "+proj=lcc +lat_1=34.65 +lat_2=36.18333333333333 +lat_0=34 +lon_0=-101.5 +x_0=200000 +y_0=1000000 +ellps=GRS80 +units=m +no_defs");
BOOST_GEOMETRY_PROJECTIONS_DETAIL_ESRI_TRAITS(102338, srs::proj::lcc, srs_spheroid_tag, "+proj=lcc +lat_1=32.13333333333333 +lat_2=33.96666666666667 +lat_0=31.66666666666667 +lon_0=-98.5 +x_0=600000 +y_0=2000000 +ellps=GRS80 +units=m +no_defs");
BOOST_GEOMETRY_PROJECTIONS_DETAIL_ESRI_TRAITS(102339, srs::proj::lcc, srs_spheroid_tag, "+proj=lcc +lat_1=30.11666666666667 +lat_2=31.88333333333333 +lat_0=29.66666666666667 +lon_0=-100.3333333333333 +x_0=700000 +y_0=3000000 +ellps=GRS80 +units=m +no_defs");
BOOST_GEOMETRY_PROJECTIONS_DETAIL_ESRI_TRAITS(102340, srs::proj::lcc, srs_spheroid_tag, "+proj=lcc +lat_1=28.38333333333333 +lat_2=30.28333333333334 +lat_0=27.83333333333333 +lon_0=-99 +x_0=600000 +y_0=4000000 +ellps=GRS80 +units=m +no_defs");
BOOST_GEOMETRY_PROJECTIONS_DETAIL_ESRI_TRAITS(102341, srs::proj::lcc, srs_spheroid_tag, "+proj=lcc +lat_1=26.16666666666667 +lat_2=27.83333333333333 +lat_0=25.66666666666667 +lon_0=-98.5 +x_0=300000 +y_0=5000000 +ellps=GRS80 +units=m +no_defs");
BOOST_GEOMETRY_PROJECTIONS_DETAIL_ESRI_TRAITS(102342, srs::proj::lcc, srs_spheroid_tag, "+proj=lcc +lat_1=40.71666666666667 +lat_2=41.78333333333333 +lat_0=40.33333333333334 +lon_0=-111.5 +x_0=500000 +y_0=1000000 +ellps=GRS80 +units=m +no_defs");
BOOST_GEOMETRY_PROJECTIONS_DETAIL_ESRI_TRAITS(102343, srs::proj::lcc, srs_spheroid_tag, "+proj=lcc +lat_1=39.01666666666667 +lat_2=40.65 +lat_0=38.33333333333334 +lon_0=-111.5 +x_0=500000 +y_0=2000000 +ellps=GRS80 +units=m +no_defs");
BOOST_GEOMETRY_PROJECTIONS_DETAIL_ESRI_TRAITS(102344, srs::proj::lcc, srs_spheroid_tag, "+proj=lcc +lat_1=37.21666666666667 +lat_2=38.35 +lat_0=36.66666666666666 +lon_0=-111.5 +x_0=500000 +y_0=3000000 +ellps=GRS80 +units=m +no_defs");
BOOST_GEOMETRY_PROJECTIONS_DETAIL_ESRI_TRAITS(102345, srs::proj::tmerc, srs_spheroid_tag, "+proj=tmerc +lat_0=42.5 +lon_0=-72.5 +k=0.9999642857142857 +x_0=500000 +y_0=0 +ellps=GRS80 +units=m +no_defs");
BOOST_GEOMETRY_PROJECTIONS_DETAIL_ESRI_TRAITS(102346, srs::proj::lcc, srs_spheroid_tag, "+proj=lcc +lat_1=38.03333333333333 +lat_2=39.2 +lat_0=37.66666666666666 +lon_0=-78.5 +x_0=3500000 +y_0=2000000 +ellps=GRS80 +units=m +no_defs");
BOOST_GEOMETRY_PROJECTIONS_DETAIL_ESRI_TRAITS(102347, srs::proj::lcc, srs_spheroid_tag, "+proj=lcc +lat_1=36.76666666666667 +lat_2=37.96666666666667 +lat_0=36.33333333333334 +lon_0=-78.5 +x_0=3500000 +y_0=1000000 +ellps=GRS80 +units=m +no_defs");
BOOST_GEOMETRY_PROJECTIONS_DETAIL_ESRI_TRAITS(102348, srs::proj::lcc, srs_spheroid_tag, "+proj=lcc +lat_1=47.5 +lat_2=48.73333333333333 +lat_0=47 +lon_0=-120.8333333333333 +x_0=500000 +y_0=0 +ellps=GRS80 +units=m +no_defs");
BOOST_GEOMETRY_PROJECTIONS_DETAIL_ESRI_TRAITS(102349, srs::proj::lcc, srs_spheroid_tag, "+proj=lcc +lat_1=45.83333333333334 +lat_2=47.33333333333334 +lat_0=45.33333333333334 +lon_0=-120.5 +x_0=500000 +y_0=0 +ellps=GRS80 +units=m +no_defs");
BOOST_GEOMETRY_PROJECTIONS_DETAIL_ESRI_TRAITS(102350, srs::proj::lcc, srs_spheroid_tag, "+proj=lcc +lat_1=39 +lat_2=40.25 +lat_0=38.5 +lon_0=-79.5 +x_0=600000 +y_0=0 +ellps=GRS80 +units=m +no_defs");
BOOST_GEOMETRY_PROJECTIONS_DETAIL_ESRI_TRAITS(102351, srs::proj::lcc, srs_spheroid_tag, "+proj=lcc +lat_1=37.48333333333333 +lat_2=38.88333333333333 +lat_0=37 +lon_0=-81 +x_0=600000 +y_0=0 +ellps=GRS80 +units=m +no_defs");
BOOST_GEOMETRY_PROJECTIONS_DETAIL_ESRI_TRAITS(102352, srs::proj::lcc, srs_spheroid_tag, "+proj=lcc +lat_1=45.56666666666667 +lat_2=46.76666666666667 +lat_0=45.16666666666666 +lon_0=-90 +x_0=600000 +y_0=0 +ellps=GRS80 +units=m +no_defs");
BOOST_GEOMETRY_PROJECTIONS_DETAIL_ESRI_TRAITS(102353, srs::proj::lcc, srs_spheroid_tag, "+proj=lcc +lat_1=44.25 +lat_2=45.5 +lat_0=43.83333333333334 +lon_0=-90 +x_0=600000 +y_0=0 +ellps=GRS80 +units=m +no_defs");
BOOST_GEOMETRY_PROJECTIONS_DETAIL_ESRI_TRAITS(102354, srs::proj::lcc, srs_spheroid_tag, "+proj=lcc +lat_1=42.73333333333333 +lat_2=44.06666666666667 +lat_0=42 +lon_0=-90 +x_0=600000 +y_0=0 +ellps=GRS80 +units=m +no_defs");
BOOST_GEOMETRY_PROJECTIONS_DETAIL_ESRI_TRAITS(102355, srs::proj::tmerc, srs_spheroid_tag, "+proj=tmerc +lat_0=40.5 +lon_0=-105.1666666666667 +k=0.9999375 +x_0=200000 +y_0=0 +ellps=GRS80 +units=m +no_defs");
BOOST_GEOMETRY_PROJECTIONS_DETAIL_ESRI_TRAITS(102356, srs::proj::tmerc, srs_spheroid_tag, "+proj=tmerc +lat_0=40.5 +lon_0=-107.3333333333333 +k=0.9999375 +x_0=400000 +y_0=100000 +ellps=GRS80 +units=m +no_defs");
BOOST_GEOMETRY_PROJECTIONS_DETAIL_ESRI_TRAITS(102357, srs::proj::tmerc, srs_spheroid_tag, "+proj=tmerc +lat_0=40.5 +lon_0=-108.75 +k=0.9999375 +x_0=600000 +y_0=0 +ellps=GRS80 +units=m +no_defs");
BOOST_GEOMETRY_PROJECTIONS_DETAIL_ESRI_TRAITS(102358, srs::proj::tmerc, srs_spheroid_tag, "+proj=tmerc +lat_0=40.5 +lon_0=-110.0833333333333 +k=0.9999375 +x_0=800000 +y_0=100000 +ellps=GRS80 +units=m +no_defs");
BOOST_GEOMETRY_PROJECTIONS_DETAIL_ESRI_TRAITS(102361, srs::proj::lcc, srs_spheroid_tag, "+proj=lcc +lat_1=18.03333333333334 +lat_2=18.43333333333333 +lat_0=17.83333333333333 +lon_0=-66.43333333333334 +x_0=200000 +y_0=200000 +ellps=GRS80 +units=m +no_defs");
BOOST_GEOMETRY_PROJECTIONS_DETAIL_ESRI_TRAITS(102491, srs::proj::lcc, srs_spheroid_tag, "+proj=lcc +lat_1=36 +lat_0=36 +lon_0=2.7 +k_0=0.999625544 +x_0=500000 +y_0=300000 +a=6378249.2 +b=6356514.999904194 +units=m +no_defs");
BOOST_GEOMETRY_PROJECTIONS_DETAIL_ESRI_TRAITS(102492, srs::proj::lcc, srs_spheroid_tag, "+proj=lcc +lat_1=33.3 +lat_0=33.3 +lon_0=2.7 +k_0=0.999625769 +x_0=500000 +y_0=300000 +a=6378249.2 +b=6356514.999904194 +units=m +no_defs");
BOOST_GEOMETRY_PROJECTIONS_DETAIL_ESRI_TRAITS(102581, srs::proj::lcc, srs_spheroid_tag, "+proj=lcc +lat_1=49.5 +lat_0=49.5 +lon_0=2.337229166666667 +k_0=0.999877341 +x_0=600000 +y_0=1200000 +a=6378249.2 +b=6356514.999904194 +units=m +no_defs");
BOOST_GEOMETRY_PROJECTIONS_DETAIL_ESRI_TRAITS(102582, srs::proj::lcc, srs_spheroid_tag, "+proj=lcc +lat_1=46.8 +lat_0=46.8 +lon_0=2.337229166666667 +k_0=0.99987742 +x_0=600000 +y_0=2200000 +a=6378249.2 +b=6356514.999904194 +units=m +no_defs");
BOOST_GEOMETRY_PROJECTIONS_DETAIL_ESRI_TRAITS(102583, srs::proj::lcc, srs_spheroid_tag, "+proj=lcc +lat_1=44.1 +lat_0=44.1 +lon_0=2.337229166666667 +k_0=0.999877499 +x_0=600000 +y_0=3200000 +a=6378249.2 +b=6356514.999904194 +units=m +no_defs");
BOOST_GEOMETRY_PROJECTIONS_DETAIL_ESRI_TRAITS(102584, srs::proj::lcc, srs_spheroid_tag, "+proj=lcc +lat_1=42.165 +lat_0=42.165 +lon_0=2.337229166666667 +k_0=0.99994471 +x_0=234.358 +y_0=4185861.369 +a=6378249.2 +b=6356514.999904194 +units=m +no_defs");
BOOST_GEOMETRY_PROJECTIONS_DETAIL_ESRI_TRAITS(102591, srs::proj::lcc, srs_spheroid_tag, "+proj=lcc +lat_1=36 +lat_0=36 +lon_0=2.7 +k_0=0.999625544 +x_0=500135 +y_0=300090 +ellps=clrk80 +units=m +no_defs");
BOOST_GEOMETRY_PROJECTIONS_DETAIL_ESRI_TRAITS(102592, srs::proj::lcc, srs_spheroid_tag, "+proj=lcc +lat_1=33.3 +lat_0=33.3 +lon_0=2.7 +k_0=0.999625769 +x_0=500135 +y_0=300090 +ellps=clrk80 +units=m +no_defs");
BOOST_GEOMETRY_PROJECTIONS_DETAIL_ESRI_TRAITS(102629, srs::proj::tmerc, srs_spheroid_tag, "+proj=tmerc +lat_0=30.5 +lon_0=-85.83333333333333 +k=0.99996 +x_0=200000 +y_0=0 +ellps=GRS80 +datum=NAD83 +to_meter=0.3048006096012192 +no_defs");
BOOST_GEOMETRY_PROJECTIONS_DETAIL_ESRI_TRAITS(102630, srs::proj::tmerc, srs_spheroid_tag, "+proj=tmerc +lat_0=30 +lon_0=-87.5 +k=0.9999333333333333 +x_0=600000.0000000001 +y_0=0 +ellps=GRS80 +datum=NAD83 +to_meter=0.3048006096012192 +no_defs");
BOOST_GEOMETRY_PROJECTIONS_DETAIL_ESRI_TRAITS(102631, srs::proj::omerc, srs_spheroid_tag, "+proj=omerc +lat_0=57 +lonc=-133.6666666666667 +alpha=-36.86989764583333 +k=0.9999 +x_0=4999999.999999999 +y_0=-4999999.999999999 +ellps=GRS80 +datum=NAD83 +to_meter=0.3048006096012192 +no_defs");
BOOST_GEOMETRY_PROJECTIONS_DETAIL_ESRI_TRAITS(102632, srs::proj::tmerc, srs_spheroid_tag, "+proj=tmerc +lat_0=54 +lon_0=-142 +k=0.9999 +x_0=500000.0000000002 +y_0=0 +ellps=GRS80 +datum=NAD83 +to_meter=0.3048006096012192 +no_defs");
BOOST_GEOMETRY_PROJECTIONS_DETAIL_ESRI_TRAITS(102633, srs::proj::tmerc, srs_spheroid_tag, "+proj=tmerc +lat_0=54 +lon_0=-146 +k=0.9999 +x_0=500000.0000000002 +y_0=0 +ellps=GRS80 +datum=NAD83 +to_meter=0.3048006096012192 +no_defs");
BOOST_GEOMETRY_PROJECTIONS_DETAIL_ESRI_TRAITS(102634, srs::proj::tmerc, srs_spheroid_tag, "+proj=tmerc +lat_0=54 +lon_0=-150 +k=0.9999 +x_0=500000.0000000002 +y_0=0 +ellps=GRS80 +datum=NAD83 +to_meter=0.3048006096012192 +no_defs");
BOOST_GEOMETRY_PROJECTIONS_DETAIL_ESRI_TRAITS(102635, srs::proj::tmerc, srs_spheroid_tag, "+proj=tmerc +lat_0=54 +lon_0=-154 +k=0.9999 +x_0=500000.0000000002 +y_0=0 +ellps=GRS80 +datum=NAD83 +to_meter=0.3048006096012192 +no_defs");
BOOST_GEOMETRY_PROJECTIONS_DETAIL_ESRI_TRAITS(102636, srs::proj::tmerc, srs_spheroid_tag, "+proj=tmerc +lat_0=54 +lon_0=-158 +k=0.9999 +x_0=500000.0000000002 +y_0=0 +ellps=GRS80 +datum=NAD83 +to_meter=0.3048006096012192 +no_defs");
BOOST_GEOMETRY_PROJECTIONS_DETAIL_ESRI_TRAITS(102637, srs::proj::tmerc, srs_spheroid_tag, "+proj=tmerc +lat_0=54 +lon_0=-162 +k=0.9999 +x_0=500000.0000000002 +y_0=0 +ellps=GRS80 +datum=NAD83 +to_meter=0.3048006096012192 +no_defs");
BOOST_GEOMETRY_PROJECTIONS_DETAIL_ESRI_TRAITS(102638, srs::proj::tmerc, srs_spheroid_tag, "+proj=tmerc +lat_0=54 +lon_0=-166 +k=0.9999 +x_0=500000.0000000002 +y_0=0 +ellps=GRS80 +datum=NAD83 +to_meter=0.3048006096012192 +no_defs");
BOOST_GEOMETRY_PROJECTIONS_DETAIL_ESRI_TRAITS(102639, srs::proj::tmerc, srs_spheroid_tag, "+proj=tmerc +lat_0=54 +lon_0=-170 +k=0.9999 +x_0=500000.0000000002 +y_0=0 +ellps=GRS80 +datum=NAD83 +to_meter=0.3048006096012192 +no_defs");
BOOST_GEOMETRY_PROJECTIONS_DETAIL_ESRI_TRAITS(102640, srs::proj::lcc, srs_spheroid_tag, "+proj=lcc +lat_1=51.83333333333334 +lat_2=53.83333333333334 +lat_0=51 +lon_0=-176 +x_0=1000000 +y_0=0 +ellps=GRS80 +datum=NAD83 +to_meter=0.3048006096012192 +no_defs");
BOOST_GEOMETRY_PROJECTIONS_DETAIL_ESRI_TRAITS(102641, srs::proj::lcc, srs_spheroid_tag, "+proj=lcc +lat_1=40 +lat_2=41.66666666666666 +lat_0=39.33333333333334 +lon_0=-122 +x_0=2000000 +y_0=500000.0000000002 +ellps=GRS80 +datum=NAD83 +to_meter=0.3048006096012192 +no_defs");
BOOST_GEOMETRY_PROJECTIONS_DETAIL_ESRI_TRAITS(102642, srs::proj::lcc, srs_spheroid_tag, "+proj=lcc +lat_1=38.33333333333334 +lat_2=39.83333333333334 +lat_0=37.66666666666666 +lon_0=-122 +x_0=2000000 +y_0=500000.0000000002 +ellps=GRS80 +datum=NAD83 +to_meter=0.3048006096012192 +no_defs");
BOOST_GEOMETRY_PROJECTIONS_DETAIL_ESRI_TRAITS(102643, srs::proj::lcc, srs_spheroid_tag, "+proj=lcc +lat_1=37.06666666666667 +lat_2=38.43333333333333 +lat_0=36.5 +lon_0=-120.5 +x_0=2000000 +y_0=500000.0000000002 +ellps=GRS80 +datum=NAD83 +to_meter=0.3048006096012192 +no_defs");
BOOST_GEOMETRY_PROJECTIONS_DETAIL_ESRI_TRAITS(102644, srs::proj::lcc, srs_spheroid_tag, "+proj=lcc +lat_1=36 +lat_2=37.25 +lat_0=35.33333333333334 +lon_0=-119 +x_0=2000000 +y_0=500000.0000000002 +ellps=GRS80 +datum=NAD83 +to_meter=0.3048006096012192 +no_defs");
BOOST_GEOMETRY_PROJECTIONS_DETAIL_ESRI_TRAITS(102645, srs::proj::lcc, srs_spheroid_tag, "+proj=lcc +lat_1=34.03333333333333 +lat_2=35.46666666666667 +lat_0=33.5 +lon_0=-118 +x_0=2000000 +y_0=500000.0000000002 +ellps=GRS80 +datum=NAD83 +to_meter=0.3048006096012192 +no_defs");
BOOST_GEOMETRY_PROJECTIONS_DETAIL_ESRI_TRAITS(102646, srs::proj::lcc, srs_spheroid_tag, "+proj=lcc +lat_1=32.78333333333333 +lat_2=33.88333333333333 +lat_0=32.16666666666666 +lon_0=-116.25 +x_0=2000000 +y_0=500000.0000000002 +ellps=GRS80 +datum=NAD83 +to_meter=0.3048006096012192 +no_defs");
BOOST_GEOMETRY_PROJECTIONS_DETAIL_ESRI_TRAITS(102648, srs::proj::tmerc, srs_spheroid_tag, "+proj=tmerc +lat_0=31 +lon_0=-110.1666666666667 +k=0.9999 +x_0=213360 +y_0=0 +ellps=GRS80 +datum=NAD83 +to_meter=0.3048006096012192 +no_defs");
BOOST_GEOMETRY_PROJECTIONS_DETAIL_ESRI_TRAITS(102649, srs::proj::tmerc, srs_spheroid_tag, "+proj=tmerc +lat_0=31 +lon_0=-111.9166666666667 +k=0.9999 +x_0=213360 +y_0=0 +ellps=GRS80 +datum=NAD83 +to_meter=0.3048006096012192 +no_defs");
BOOST_GEOMETRY_PROJECTIONS_DETAIL_ESRI_TRAITS(102650, srs::proj::tmerc, srs_spheroid_tag, "+proj=tmerc +lat_0=31 +lon_0=-113.75 +k=0.9999333333333333 +x_0=213360 +y_0=0 +ellps=GRS80 +datum=NAD83 +to_meter=0.3048006096012192 +no_defs");
BOOST_GEOMETRY_PROJECTIONS_DETAIL_ESRI_TRAITS(102651, srs::proj::lcc, srs_spheroid_tag, "+proj=lcc +lat_1=34.93333333333333 +lat_2=36.23333333333333 +lat_0=34.33333333333334 +lon_0=-92 +x_0=399999.9999999999 +y_0=0 +ellps=GRS80 +datum=NAD83 +to_meter=0.3048006096012192 +no_defs");
BOOST_GEOMETRY_PROJECTIONS_DETAIL_ESRI_TRAITS(102652, srs::proj::lcc, srs_spheroid_tag, "+proj=lcc +lat_1=33.3 +lat_2=34.76666666666667 +lat_0=32.66666666666666 +lon_0=-92 +x_0=399999.9999999999 +y_0=399999.9999999999 +ellps=GRS80 +datum=NAD83 +to_meter=0.3048006096012192 +no_defs");
BOOST_GEOMETRY_PROJECTIONS_DETAIL_ESRI_TRAITS(102653, srs::proj::lcc, srs_spheroid_tag, "+proj=lcc +lat_1=39.71666666666667 +lat_2=40.78333333333333 +lat_0=39.33333333333334 +lon_0=-105.5 +x_0=914401.8289 +y_0=304800.6096 +ellps=GRS80 +datum=NAD83 +to_meter=0.3048006096012192 +no_defs");
BOOST_GEOMETRY_PROJECTIONS_DETAIL_ESRI_TRAITS(102654, srs::proj::lcc, srs_spheroid_tag, "+proj=lcc +lat_1=38.45 +lat_2=39.75 +lat_0=37.83333333333334 +lon_0=-105.5 +x_0=914401.8289 +y_0=304800.6096 +ellps=GRS80 +datum=NAD83 +to_meter=0.3048006096012192 +no_defs");
BOOST_GEOMETRY_PROJECTIONS_DETAIL_ESRI_TRAITS(102655, srs::proj::lcc, srs_spheroid_tag, "+proj=lcc +lat_1=37.23333333333333 +lat_2=38.43333333333333 +lat_0=36.66666666666666 +lon_0=-105.5 +x_0=914401.8289 +y_0=304800.6096 +ellps=GRS80 +datum=NAD83 +to_meter=0.3048006096012192 +no_defs");
BOOST_GEOMETRY_PROJECTIONS_DETAIL_ESRI_TRAITS(102656, srs::proj::lcc, srs_spheroid_tag, "+proj=lcc +lat_1=41.2 +lat_2=41.86666666666667 +lat_0=40.83333333333334 +lon_0=-72.75 +x_0=304800.6096 +y_0=152400.3048 +ellps=GRS80 +datum=NAD83 +to_meter=0.3048006096012192 +no_defs");
BOOST_GEOMETRY_PROJECTIONS_DETAIL_ESRI_TRAITS(102657, srs::proj::tmerc, srs_spheroid_tag, "+proj=tmerc +lat_0=38 +lon_0=-75.41666666666667 +k=0.999995 +x_0=200000 +y_0=0 +ellps=GRS80 +datum=NAD83 +to_meter=0.3048006096012192 +no_defs");
BOOST_GEOMETRY_PROJECTIONS_DETAIL_ESRI_TRAITS(102658, srs::proj::tmerc, srs_spheroid_tag, "+proj=tmerc +lat_0=24.33333333333333 +lon_0=-81 +k=0.9999411764705882 +x_0=200000 +y_0=0 +ellps=GRS80 +datum=NAD83 +to_meter=0.3048006096012192 +no_defs");
BOOST_GEOMETRY_PROJECTIONS_DETAIL_ESRI_TRAITS(102659, srs::proj::tmerc, srs_spheroid_tag, "+proj=tmerc +lat_0=24.33333333333333 +lon_0=-82 +k=0.9999411764705882 +x_0=200000 +y_0=0 +ellps=GRS80 +datum=NAD83 +to_meter=0.3048006096012192 +no_defs");
BOOST_GEOMETRY_PROJECTIONS_DETAIL_ESRI_TRAITS(102660, srs::proj::lcc, srs_spheroid_tag, "+proj=lcc +lat_1=29.58333333333333 +lat_2=30.75 +lat_0=29 +lon_0=-84.5 +x_0=600000.0000000001 +y_0=0 +ellps=GRS80 +datum=NAD83 +to_meter=0.3048006096012192 +no_defs");
BOOST_GEOMETRY_PROJECTIONS_DETAIL_ESRI_TRAITS(102661, srs::proj::tmerc, srs_spheroid_tag, "+proj=tmerc +lat_0=18.83333333333333 +lon_0=-155.5 +k=0.9999666666666667 +x_0=500000.0000000002 +y_0=0 +ellps=GRS80 +datum=NAD83 +to_meter=0.3048006096012192 +no_defs");
BOOST_GEOMETRY_PROJECTIONS_DETAIL_ESRI_TRAITS(102662, srs::proj::tmerc, srs_spheroid_tag, "+proj=tmerc +lat_0=20.33333333333333 +lon_0=-156.6666666666667 +k=0.9999666666666667 +x_0=500000.0000000002 +y_0=0 +ellps=GRS80 +datum=NAD83 +to_meter=0.3048006096012192 +no_defs");
BOOST_GEOMETRY_PROJECTIONS_DETAIL_ESRI_TRAITS(102663, srs::proj::tmerc, srs_spheroid_tag, "+proj=tmerc +lat_0=21.16666666666667 +lon_0=-158 +k=0.99999 +x_0=500000.0000000002 +y_0=0 +ellps=GRS80 +datum=NAD83 +to_meter=0.3048006096012192 +no_defs");
BOOST_GEOMETRY_PROJECTIONS_DETAIL_ESRI_TRAITS(102664, srs::proj::tmerc, srs_spheroid_tag, "+proj=tmerc +lat_0=21.83333333333333 +lon_0=-159.5 +k=0.99999 +x_0=500000.0000000002 +y_0=0 +ellps=GRS80 +datum=NAD83 +to_meter=0.3048006096012192 +no_defs");
BOOST_GEOMETRY_PROJECTIONS_DETAIL_ESRI_TRAITS(102665, srs::proj::tmerc, srs_spheroid_tag, "+proj=tmerc +lat_0=21.66666666666667 +lon_0=-160.1666666666667 +k=1 +x_0=500000.0000000002 +y_0=0 +ellps=GRS80 +datum=NAD83 +to_meter=0.3048006096012192 +no_defs");
BOOST_GEOMETRY_PROJECTIONS_DETAIL_ESRI_TRAITS(102666, srs::proj::tmerc, srs_spheroid_tag, "+proj=tmerc +lat_0=30 +lon_0=-82.16666666666667 +k=0.9999 +x_0=200000 +y_0=0 +ellps=GRS80 +datum=NAD83 +to_meter=0.3048006096012192 +no_defs");
BOOST_GEOMETRY_PROJECTIONS_DETAIL_ESRI_TRAITS(102667, srs::proj::tmerc, srs_spheroid_tag, "+proj=tmerc +lat_0=30 +lon_0=-84.16666666666667 +k=0.9999 +x_0=700000 +y_0=0 +ellps=GRS80 +datum=NAD83 +to_meter=0.3048006096012192 +no_defs");
BOOST_GEOMETRY_PROJECTIONS_DETAIL_ESRI_TRAITS(102668, srs::proj::tmerc, srs_spheroid_tag, "+proj=tmerc +lat_0=41.66666666666666 +lon_0=-112.1666666666667 +k=0.9999473684210526 +x_0=200000 +y_0=0 +ellps=GRS80 +datum=NAD83 +to_meter=0.3048006096012192 +no_defs");
BOOST_GEOMETRY_PROJECTIONS_DETAIL_ESRI_TRAITS(102669, srs::proj::tmerc, srs_spheroid_tag, "+proj=tmerc +lat_0=41.66666666666666 +lon_0=-114 +k=0.9999473684210526 +x_0=500000.0000000002 +y_0=0 +ellps=GRS80 +datum=NAD83 +to_meter=0.3048006096012192 +no_defs");
BOOST_GEOMETRY_PROJECTIONS_DETAIL_ESRI_TRAITS(102670, srs::proj::tmerc, srs_spheroid_tag, "+proj=tmerc +lat_0=41.66666666666666 +lon_0=-115.75 +k=0.9999333333333333 +x_0=799999.9999999999 +y_0=0 +ellps=GRS80 +datum=NAD83 +to_meter=0.3048006096012192 +no_defs");
BOOST_GEOMETRY_PROJECTIONS_DETAIL_ESRI_TRAITS(102671, srs::proj::tmerc, srs_spheroid_tag, "+proj=tmerc +lat_0=36.66666666666666 +lon_0=-88.33333333333333 +k=0.9999749999999999 +x_0=300000 +y_0=0 +ellps=GRS80 +datum=NAD83 +to_meter=0.3048006096012192 +no_defs");
BOOST_GEOMETRY_PROJECTIONS_DETAIL_ESRI_TRAITS(102672, srs::proj::tmerc, srs_spheroid_tag, "+proj=tmerc +lat_0=36.66666666666666 +lon_0=-90.16666666666667 +k=0.9999411764705882 +x_0=700000 +y_0=0 +ellps=GRS80 +datum=NAD83 +to_meter=0.3048006096012192 +no_defs");
BOOST_GEOMETRY_PROJECTIONS_DETAIL_ESRI_TRAITS(102673, srs::proj::tmerc, srs_spheroid_tag, "+proj=tmerc +lat_0=37.5 +lon_0=-85.66666666666667 +k=0.9999666666666667 +x_0=100000 +y_0=250000 +ellps=GRS80 +datum=NAD83 +to_meter=0.3048006096012192 +no_defs");
BOOST_GEOMETRY_PROJECTIONS_DETAIL_ESRI_TRAITS(102674, srs::proj::tmerc, srs_spheroid_tag, "+proj=tmerc +lat_0=37.5 +lon_0=-87.08333333333333 +k=0.9999666666666667 +x_0=900000.0000000001 +y_0=250000 +ellps=GRS80 +datum=NAD83 +to_meter=0.3048006096012192 +no_defs");
BOOST_GEOMETRY_PROJECTIONS_DETAIL_ESRI_TRAITS(102675, srs::proj::lcc, srs_spheroid_tag, "+proj=lcc +lat_1=42.06666666666667 +lat_2=43.26666666666667 +lat_0=41.5 +lon_0=-93.5 +x_0=1500000 +y_0=1000000 +ellps=GRS80 +datum=NAD83 +to_meter=0.3048006096012192 +no_defs");
BOOST_GEOMETRY_PROJECTIONS_DETAIL_ESRI_TRAITS(102676, srs::proj::lcc, srs_spheroid_tag, "+proj=lcc +lat_1=40.61666666666667 +lat_2=41.78333333333333 +lat_0=40 +lon_0=-93.5 +x_0=500000.0000000002 +y_0=0 +ellps=GRS80 +datum=NAD83 +to_meter=0.3048006096012192 +no_defs");
BOOST_GEOMETRY_PROJECTIONS_DETAIL_ESRI_TRAITS(102677, srs::proj::lcc, srs_spheroid_tag, "+proj=lcc +lat_1=38.71666666666667 +lat_2=39.78333333333333 +lat_0=38.33333333333334 +lon_0=-98 +x_0=399999.9999999999 +y_0=0 +ellps=GRS80 +datum=NAD83 +to_meter=0.3048006096012192 +no_defs");
BOOST_GEOMETRY_PROJECTIONS_DETAIL_ESRI_TRAITS(102678, srs::proj::lcc, srs_spheroid_tag, "+proj=lcc +lat_1=37.26666666666667 +lat_2=38.56666666666667 +lat_0=36.66666666666666 +lon_0=-98.5 +x_0=399999.9999999999 +y_0=399999.9999999999 +ellps=GRS80 +datum=NAD83 +to_meter=0.3048006096012192 +no_defs");
BOOST_GEOMETRY_PROJECTIONS_DETAIL_ESRI_TRAITS(102679, srs::proj::lcc, srs_spheroid_tag, "+proj=lcc +lat_1=37.96666666666667 +lat_2=38.96666666666667 +lat_0=37.5 +lon_0=-84.25 +x_0=500000.0000000002 +y_0=0 +ellps=GRS80 +datum=NAD83 +to_meter=0.3048006096012192 +no_defs");
BOOST_GEOMETRY_PROJECTIONS_DETAIL_ESRI_TRAITS(102680, srs::proj::lcc, srs_spheroid_tag, "+proj=lcc +lat_1=36.73333333333333 +lat_2=37.93333333333333 +lat_0=36.33333333333334 +lon_0=-85.75 +x_0=500000.0000000002 +y_0=500000.0000000002 +ellps=GRS80 +datum=NAD83 +to_meter=0.3048006096012192 +no_defs");
BOOST_GEOMETRY_PROJECTIONS_DETAIL_ESRI_TRAITS(102681, srs::proj::lcc, srs_spheroid_tag, "+proj=lcc +lat_1=31.16666666666667 +lat_2=32.66666666666666 +lat_0=30.5 +lon_0=-92.5 +x_0=1000000 +y_0=0 +ellps=GRS80 +datum=NAD83 +to_meter=0.3048006096012192 +no_defs");
BOOST_GEOMETRY_PROJECTIONS_DETAIL_ESRI_TRAITS(102682, srs::proj::lcc, srs_spheroid_tag, "+proj=lcc +lat_1=29.3 +lat_2=30.7 +lat_0=28.5 +lon_0=-91.33333333333333 +x_0=1000000 +y_0=0 +ellps=GRS80 +datum=NAD83 +to_meter=0.3048006096012192 +no_defs");
BOOST_GEOMETRY_PROJECTIONS_DETAIL_ESRI_TRAITS(102683, srs::proj::tmerc, srs_spheroid_tag, "+proj=tmerc +lat_0=43.66666666666666 +lon_0=-68.5 +k=0.9999 +x_0=300000 +y_0=0 +ellps=GRS80 +datum=NAD83 +to_meter=0.3048006096012192 +no_defs");
BOOST_GEOMETRY_PROJECTIONS_DETAIL_ESRI_TRAITS(102684, srs::proj::tmerc, srs_spheroid_tag, "+proj=tmerc +lat_0=42.83333333333334 +lon_0=-70.16666666666667 +k=0.9999666666666667 +x_0=900000.0000000001 +y_0=0 +ellps=GRS80 +datum=NAD83 +to_meter=0.3048006096012192 +no_defs");
BOOST_GEOMETRY_PROJECTIONS_DETAIL_ESRI_TRAITS(102685, srs::proj::lcc, srs_spheroid_tag, "+proj=lcc +lat_1=38.3 +lat_2=39.45 +lat_0=37.66666666666666 +lon_0=-77 +x_0=399999.9999999999 +y_0=0 +ellps=GRS80 +datum=NAD83 +to_meter=0.3048006096012192 +no_defs");
BOOST_GEOMETRY_PROJECTIONS_DETAIL_ESRI_TRAITS(102686, srs::proj::lcc, srs_spheroid_tag, "+proj=lcc +lat_1=41.71666666666667 +lat_2=42.68333333333333 +lat_0=41 +lon_0=-71.5 +x_0=200000 +y_0=750000.0000000001 +ellps=GRS80 +datum=NAD83 +to_meter=0.3048006096012192 +no_defs");
BOOST_GEOMETRY_PROJECTIONS_DETAIL_ESRI_TRAITS(102687, srs::proj::lcc, srs_spheroid_tag, "+proj=lcc +lat_1=41.28333333333333 +lat_2=41.48333333333333 +lat_0=41 +lon_0=-70.5 +x_0=500000.0000000002 +y_0=0 +ellps=GRS80 +datum=NAD83 +to_meter=0.3048006096012192 +no_defs");
BOOST_GEOMETRY_PROJECTIONS_DETAIL_ESRI_TRAITS(102688, srs::proj::lcc, srs_spheroid_tag, "+proj=lcc +lat_1=45.48333333333333 +lat_2=47.08333333333334 +lat_0=44.78333333333333 +lon_0=-87 +x_0=7999999.999999999 +y_0=0 +ellps=GRS80 +datum=NAD83 +to_meter=0.3048006096012192 +no_defs");
BOOST_GEOMETRY_PROJECTIONS_DETAIL_ESRI_TRAITS(102689, srs::proj::lcc, srs_spheroid_tag, "+proj=lcc +lat_1=44.18333333333333 +lat_2=45.7 +lat_0=43.31666666666667 +lon_0=-84.36666666666666 +x_0=6000000.000000001 +y_0=0 +ellps=GRS80 +datum=NAD83 +to_meter=0.3048006096012192 +no_defs");
BOOST_GEOMETRY_PROJECTIONS_DETAIL_ESRI_TRAITS(102690, srs::proj::lcc, srs_spheroid_tag, "+proj=lcc +lat_1=42.1 +lat_2=43.66666666666666 +lat_0=41.5 +lon_0=-84.36666666666666 +x_0=4000000 +y_0=0 +ellps=GRS80 +datum=NAD83 +to_meter=0.3048006096012192 +no_defs");
BOOST_GEOMETRY_PROJECTIONS_DETAIL_ESRI_TRAITS(102691, srs::proj::lcc, srs_spheroid_tag, "+proj=lcc +lat_1=47.03333333333333 +lat_2=48.63333333333333 +lat_0=46.5 +lon_0=-93.09999999999999 +x_0=799999.9999999999 +y_0=100000 +ellps=GRS80 +datum=NAD83 +to_meter=0.3048006096012192 +no_defs");
BOOST_GEOMETRY_PROJECTIONS_DETAIL_ESRI_TRAITS(102692, srs::proj::lcc, srs_spheroid_tag, "+proj=lcc +lat_1=45.61666666666667 +lat_2=47.05 +lat_0=45 +lon_0=-94.25 +x_0=799999.9999999999 +y_0=100000 +ellps=GRS80 +datum=NAD83 +to_meter=0.3048006096012192 +no_defs");
BOOST_GEOMETRY_PROJECTIONS_DETAIL_ESRI_TRAITS(102693, srs::proj::lcc, srs_spheroid_tag, "+proj=lcc +lat_1=43.78333333333333 +lat_2=45.21666666666667 +lat_0=43 +lon_0=-94 +x_0=799999.9999999999 +y_0=100000 +ellps=GRS80 +datum=NAD83 +to_meter=0.3048006096012192 +no_defs");
BOOST_GEOMETRY_PROJECTIONS_DETAIL_ESRI_TRAITS(102694, srs::proj::tmerc, srs_spheroid_tag, "+proj=tmerc +lat_0=29.5 +lon_0=-88.83333333333333 +k=0.99995 +x_0=300000 +y_0=0 +ellps=GRS80 +datum=NAD83 +to_meter=0.3048006096012192 +no_defs");
BOOST_GEOMETRY_PROJECTIONS_DETAIL_ESRI_TRAITS(102695, srs::proj::tmerc, srs_spheroid_tag, "+proj=tmerc +lat_0=29.5 +lon_0=-90.33333333333333 +k=0.99995 +x_0=700000 +y_0=0 +ellps=GRS80 +datum=NAD83 +to_meter=0.3048006096012192 +no_defs");
BOOST_GEOMETRY_PROJECTIONS_DETAIL_ESRI_TRAITS(102696, srs::proj::tmerc, srs_spheroid_tag, "+proj=tmerc +lat_0=35.83333333333334 +lon_0=-90.5 +k=0.9999333333333333 +x_0=250000 +y_0=0 +ellps=GRS80 +datum=NAD83 +to_meter=0.3048006096012192 +no_defs");
BOOST_GEOMETRY_PROJECTIONS_DETAIL_ESRI_TRAITS(102697, srs::proj::tmerc, srs_spheroid_tag, "+proj=tmerc +lat_0=35.83333333333334 +lon_0=-92.5 +k=0.9999333333333333 +x_0=500000.0000000002 +y_0=0 +ellps=GRS80 +datum=NAD83 +to_meter=0.3048006096012192 +no_defs");
BOOST_GEOMETRY_PROJECTIONS_DETAIL_ESRI_TRAITS(102698, srs::proj::tmerc, srs_spheroid_tag, "+proj=tmerc +lat_0=36.16666666666666 +lon_0=-94.5 +k=0.9999411764705882 +x_0=850000 +y_0=0 +ellps=GRS80 +datum=NAD83 +to_meter=0.3048006096012192 +no_defs");
BOOST_GEOMETRY_PROJECTIONS_DETAIL_ESRI_TRAITS(102700, srs::proj::lcc, srs_spheroid_tag, "+proj=lcc +lat_1=45 +lat_2=49 +lat_0=44.25 +lon_0=-109.5 +x_0=600000.0000000001 +y_0=0 +ellps=GRS80 +datum=NAD83 +to_meter=0.3048006096012192 +no_defs");
BOOST_GEOMETRY_PROJECTIONS_DETAIL_ESRI_TRAITS(102704, srs::proj::lcc, srs_spheroid_tag, "+proj=lcc +lat_1=40 +lat_2=43 +lat_0=39.83333333333334 +lon_0=-100 +x_0=500000.0000000002 +y_0=0 +ellps=GRS80 +datum=NAD83 +to_meter=0.3048006096012192 +no_defs");
BOOST_GEOMETRY_PROJECTIONS_DETAIL_ESRI_TRAITS(102707, srs::proj::tmerc, srs_spheroid_tag, "+proj=tmerc +lat_0=34.75 +lon_0=-115.5833333333333 +k=0.9999 +x_0=200000 +y_0=7999999.999999999 +ellps=GRS80 +datum=NAD83 +to_meter=0.3048006096012192 +no_defs");
BOOST_GEOMETRY_PROJECTIONS_DETAIL_ESRI_TRAITS(102708, srs::proj::tmerc, srs_spheroid_tag, "+proj=tmerc +lat_0=34.75 +lon_0=-116.6666666666667 +k=0.9999 +x_0=500000.0000000002 +y_0=6000000.000000001 +ellps=GRS80 +datum=NAD83 +to_meter=0.3048006096012192 +no_defs");
BOOST_GEOMETRY_PROJECTIONS_DETAIL_ESRI_TRAITS(102709, srs::proj::tmerc, srs_spheroid_tag, "+proj=tmerc +lat_0=34.75 +lon_0=-118.5833333333333 +k=0.9999 +x_0=799999.9999999999 +y_0=4000000 +ellps=GRS80 +datum=NAD83 +to_meter=0.3048006096012192 +no_defs");
BOOST_GEOMETRY_PROJECTIONS_DETAIL_ESRI_TRAITS(102710, srs::proj::tmerc, srs_spheroid_tag, "+proj=tmerc +lat_0=42.5 +lon_0=-71.66666666666667 +k=0.9999666666666667 +x_0=300000 +y_0=0 +ellps=GRS80 +datum=NAD83 +to_meter=0.3048006096012192 +no_defs");
BOOST_GEOMETRY_PROJECTIONS_DETAIL_ESRI_TRAITS(102711, srs::proj::tmerc, srs_spheroid_tag, "+proj=tmerc +lat_0=38.83333333333334 +lon_0=-74.5 +k=0.9999 +x_0=150000 +y_0=0 +ellps=GRS80 +datum=NAD83 +to_meter=0.3048006096012192 +no_defs");
BOOST_GEOMETRY_PROJECTIONS_DETAIL_ESRI_TRAITS(102712, srs::proj::tmerc, srs_spheroid_tag, "+proj=tmerc +lat_0=31 +lon_0=-104.3333333333333 +k=0.9999090909090909 +x_0=165000 +y_0=0 +ellps=GRS80 +datum=NAD83 +to_meter=0.3048006096012192 +no_defs");
BOOST_GEOMETRY_PROJECTIONS_DETAIL_ESRI_TRAITS(102713, srs::proj::tmerc, srs_spheroid_tag, "+proj=tmerc +lat_0=31 +lon_0=-106.25 +k=0.9999 +x_0=500000.0000000002 +y_0=0 +ellps=GRS80 +datum=NAD83 +to_meter=0.3048006096012192 +no_defs");
BOOST_GEOMETRY_PROJECTIONS_DETAIL_ESRI_TRAITS(102714, srs::proj::tmerc, srs_spheroid_tag, "+proj=tmerc +lat_0=31 +lon_0=-107.8333333333333 +k=0.9999166666666667 +x_0=829999.9999999999 +y_0=0 +ellps=GRS80 +datum=NAD83 +to_meter=0.3048006096012192 +no_defs");
BOOST_GEOMETRY_PROJECTIONS_DETAIL_ESRI_TRAITS(102715, srs::proj::tmerc, srs_spheroid_tag, "+proj=tmerc +lat_0=38.83333333333334 +lon_0=-74.5 +k=0.9999 +x_0=150000 +y_0=0 +ellps=GRS80 +datum=NAD83 +to_meter=0.3048006096012192 +no_defs");
BOOST_GEOMETRY_PROJECTIONS_DETAIL_ESRI_TRAITS(102716, srs::proj::tmerc, srs_spheroid_tag, "+proj=tmerc +lat_0=40 +lon_0=-76.58333333333333 +k=0.9999375 +x_0=250000 +y_0=0 +ellps=GRS80 +datum=NAD83 +to_meter=0.3048006096012192 +no_defs");
BOOST_GEOMETRY_PROJECTIONS_DETAIL_ESRI_TRAITS(102717, srs::proj::tmerc, srs_spheroid_tag, "+proj=tmerc +lat_0=40 +lon_0=-78.58333333333333 +k=0.9999375 +x_0=350000.0000000001 +y_0=0 +ellps=GRS80 +datum=NAD83 +to_meter=0.3048006096012192 +no_defs");
BOOST_GEOMETRY_PROJECTIONS_DETAIL_ESRI_TRAITS(102718, srs::proj::lcc, srs_spheroid_tag, "+proj=lcc +lat_1=40.66666666666666 +lat_2=41.03333333333333 +lat_0=40.16666666666666 +lon_0=-74 +x_0=300000 +y_0=0 +ellps=GRS80 +datum=NAD83 +to_meter=0.3048006096012192 +no_defs");
BOOST_GEOMETRY_PROJECTIONS_DETAIL_ESRI_TRAITS(102719, srs::proj::lcc, srs_spheroid_tag, "+proj=lcc +lat_1=34.33333333333334 +lat_2=36.16666666666666 +lat_0=33.75 +lon_0=-79 +x_0=609601.2199999999 +y_0=0 +ellps=GRS80 +datum=NAD83 +to_meter=0.3048006096012192 +no_defs");
BOOST_GEOMETRY_PROJECTIONS_DETAIL_ESRI_TRAITS(102720, srs::proj::lcc, srs_spheroid_tag, "+proj=lcc +lat_1=47.43333333333333 +lat_2=48.73333333333333 +lat_0=47 +lon_0=-100.5 +x_0=600000.0000000001 +y_0=0 +ellps=GRS80 +datum=NAD83 +to_meter=0.3048006096012192 +no_defs");
BOOST_GEOMETRY_PROJECTIONS_DETAIL_ESRI_TRAITS(102721, srs::proj::lcc, srs_spheroid_tag, "+proj=lcc +lat_1=46.18333333333333 +lat_2=47.48333333333333 +lat_0=45.66666666666666 +lon_0=-100.5 +x_0=600000.0000000001 +y_0=0 +ellps=GRS80 +datum=NAD83 +to_meter=0.3048006096012192 +no_defs");
BOOST_GEOMETRY_PROJECTIONS_DETAIL_ESRI_TRAITS(102722, srs::proj::lcc, srs_spheroid_tag, "+proj=lcc +lat_1=40.43333333333333 +lat_2=41.7 +lat_0=39.66666666666666 +lon_0=-82.5 +x_0=600000.0000000001 +y_0=0 +ellps=GRS80 +datum=NAD83 +to_meter=0.3048006096012192 +no_defs");
BOOST_GEOMETRY_PROJECTIONS_DETAIL_ESRI_TRAITS(102723, srs::proj::lcc, srs_spheroid_tag, "+proj=lcc +lat_1=38.73333333333333 +lat_2=40.03333333333333 +lat_0=38 +lon_0=-82.5 +x_0=600000.0000000001 +y_0=0 +ellps=GRS80 +datum=NAD83 +to_meter=0.3048006096012192 +no_defs");
BOOST_GEOMETRY_PROJECTIONS_DETAIL_ESRI_TRAITS(102724, srs::proj::lcc, srs_spheroid_tag, "+proj=lcc +lat_1=35.56666666666667 +lat_2=36.76666666666667 +lat_0=35 +lon_0=-98 +x_0=600000.0000000001 +y_0=0 +ellps=GRS80 +datum=NAD83 +to_meter=0.3048006096012192 +no_defs");
BOOST_GEOMETRY_PROJECTIONS_DETAIL_ESRI_TRAITS(102725, srs::proj::lcc, srs_spheroid_tag, "+proj=lcc +lat_1=33.93333333333333 +lat_2=35.23333333333333 +lat_0=33.33333333333334 +lon_0=-98 +x_0=600000.0000000001 +y_0=0 +ellps=GRS80 +datum=NAD83 +to_meter=0.3048006096012192 +no_defs");
BOOST_GEOMETRY_PROJECTIONS_DETAIL_ESRI_TRAITS(102726, srs::proj::lcc, srs_spheroid_tag, "+proj=lcc +lat_1=44.33333333333334 +lat_2=46 +lat_0=43.66666666666666 +lon_0=-120.5 +x_0=2500000 +y_0=0 +ellps=GRS80 +datum=NAD83 +to_meter=0.3048006096012192 +no_defs");
BOOST_GEOMETRY_PROJECTIONS_DETAIL_ESRI_TRAITS(102727, srs::proj::lcc, srs_spheroid_tag, "+proj=lcc +lat_1=42.33333333333334 +lat_2=44 +lat_0=41.66666666666666 +lon_0=-120.5 +x_0=1500000 +y_0=0 +ellps=GRS80 +datum=NAD83 +to_meter=0.3048006096012192 +no_defs");
BOOST_GEOMETRY_PROJECTIONS_DETAIL_ESRI_TRAITS(102728, srs::proj::lcc, srs_spheroid_tag, "+proj=lcc +lat_1=40.88333333333333 +lat_2=41.95 +lat_0=40.16666666666666 +lon_0=-77.75 +x_0=600000.0000000001 +y_0=0 +ellps=GRS80 +datum=NAD83 +to_meter=0.3048006096012192 +no_defs");
BOOST_GEOMETRY_PROJECTIONS_DETAIL_ESRI_TRAITS(102729, srs::proj::lcc, srs_spheroid_tag, "+proj=lcc +lat_1=39.93333333333333 +lat_2=40.96666666666667 +lat_0=39.33333333333334 +lon_0=-77.75 +x_0=600000.0000000001 +y_0=0 +ellps=GRS80 +datum=NAD83 +to_meter=0.3048006096012192 +no_defs");
BOOST_GEOMETRY_PROJECTIONS_DETAIL_ESRI_TRAITS(102730, srs::proj::tmerc, srs_spheroid_tag, "+proj=tmerc +lat_0=41.08333333333334 +lon_0=-71.5 +k=0.99999375 +x_0=100000 +y_0=0 +ellps=GRS80 +datum=NAD83 +to_meter=0.3048006096012192 +no_defs");
BOOST_GEOMETRY_PROJECTIONS_DETAIL_ESRI_TRAITS(102733, srs::proj::lcc, srs_spheroid_tag, "+proj=lcc +lat_1=32.5 +lat_2=34.83333333333334 +lat_0=31.83333333333333 +lon_0=-81 +x_0=609600.0000000001 +y_0=0 +ellps=GRS80 +datum=NAD83 +to_meter=0.3048006096012192 +no_defs");
BOOST_GEOMETRY_PROJECTIONS_DETAIL_ESRI_TRAITS(102734, srs::proj::lcc, srs_spheroid_tag, "+proj=lcc +lat_1=44.41666666666666 +lat_2=45.68333333333333 +lat_0=43.83333333333334 +lon_0=-100 +x_0=600000.0000000001 +y_0=0 +ellps=GRS80 +datum=NAD83 +to_meter=0.3048006096012192 +no_defs");
BOOST_GEOMETRY_PROJECTIONS_DETAIL_ESRI_TRAITS(102735, srs::proj::lcc, srs_spheroid_tag, "+proj=lcc +lat_1=42.83333333333334 +lat_2=44.4 +lat_0=42.33333333333334 +lon_0=-100.3333333333333 +x_0=600000.0000000001 +y_0=0 +ellps=GRS80 +datum=NAD83 +to_meter=0.3048006096012192 +no_defs");
BOOST_GEOMETRY_PROJECTIONS_DETAIL_ESRI_TRAITS(102736, srs::proj::lcc, srs_spheroid_tag, "+proj=lcc +lat_1=35.25 +lat_2=36.41666666666666 +lat_0=34.33333333333334 +lon_0=-86 +x_0=600000.0000000001 +y_0=0 +ellps=GRS80 +datum=NAD83 +to_meter=0.3048006096012192 +no_defs");
BOOST_GEOMETRY_PROJECTIONS_DETAIL_ESRI_TRAITS(102737, srs::proj::lcc, srs_spheroid_tag, "+proj=lcc +lat_1=34.65 +lat_2=36.18333333333333 +lat_0=34 +lon_0=-101.5 +x_0=200000 +y_0=1000000 +ellps=GRS80 +datum=NAD83 +to_meter=0.3048006096012192 +no_defs");
BOOST_GEOMETRY_PROJECTIONS_DETAIL_ESRI_TRAITS(102738, srs::proj::lcc, srs_spheroid_tag, "+proj=lcc +lat_1=32.13333333333333 +lat_2=33.96666666666667 +lat_0=31.66666666666667 +lon_0=-98.5 +x_0=600000.0000000001 +y_0=2000000 +ellps=GRS80 +datum=NAD83 +to_meter=0.3048006096012192 +no_defs");
BOOST_GEOMETRY_PROJECTIONS_DETAIL_ESRI_TRAITS(102739, srs::proj::lcc, srs_spheroid_tag, "+proj=lcc +lat_1=30.11666666666667 +lat_2=31.88333333333333 +lat_0=29.66666666666667 +lon_0=-100.3333333333333 +x_0=700000 +y_0=3000000 +ellps=GRS80 +datum=NAD83 +to_meter=0.3048006096012192 +no_defs");
BOOST_GEOMETRY_PROJECTIONS_DETAIL_ESRI_TRAITS(102740, srs::proj::lcc, srs_spheroid_tag, "+proj=lcc +lat_1=28.38333333333333 +lat_2=30.28333333333334 +lat_0=27.83333333333333 +lon_0=-99 +x_0=600000.0000000001 +y_0=4000000 +ellps=GRS80 +datum=NAD83 +to_meter=0.3048006096012192 +no_defs");
BOOST_GEOMETRY_PROJECTIONS_DETAIL_ESRI_TRAITS(102741, srs::proj::lcc, srs_spheroid_tag, "+proj=lcc +lat_1=26.16666666666667 +lat_2=27.83333333333333 +lat_0=25.66666666666667 +lon_0=-98.5 +x_0=300000 +y_0=4999999.999999999 +ellps=GRS80 +datum=NAD83 +to_meter=0.3048006096012192 +no_defs");
BOOST_GEOMETRY_PROJECTIONS_DETAIL_ESRI_TRAITS(102742, srs::proj::lcc, srs_spheroid_tag, "+proj=lcc +lat_1=40.71666666666667 +lat_2=41.78333333333333 +lat_0=40.33333333333334 +lon_0=-111.5 +x_0=500000.0000000002 +y_0=1000000 +ellps=GRS80 +datum=NAD83 +to_meter=0.3048006096012192 +no_defs");
BOOST_GEOMETRY_PROJECTIONS_DETAIL_ESRI_TRAITS(102743, srs::proj::lcc, srs_spheroid_tag, "+proj=lcc +lat_1=39.01666666666667 +lat_2=40.65 +lat_0=38.33333333333334 +lon_0=-111.5 +x_0=500000.0000000002 +y_0=2000000 +ellps=GRS80 +datum=NAD83 +to_meter=0.3048006096012192 +no_defs");
BOOST_GEOMETRY_PROJECTIONS_DETAIL_ESRI_TRAITS(102744, srs::proj::lcc, srs_spheroid_tag, "+proj=lcc +lat_1=37.21666666666667 +lat_2=38.35 +lat_0=36.66666666666666 +lon_0=-111.5 +x_0=500000.0000000002 +y_0=3000000 +ellps=GRS80 +datum=NAD83 +to_meter=0.3048006096012192 +no_defs");
BOOST_GEOMETRY_PROJECTIONS_DETAIL_ESRI_TRAITS(102745, srs::proj::tmerc, srs_spheroid_tag, "+proj=tmerc +lat_0=42.5 +lon_0=-72.5 +k=0.9999642857142857 +x_0=500000.0000000002 +y_0=0 +ellps=GRS80 +datum=NAD83 +to_meter=0.3048006096012192 +no_defs");
BOOST_GEOMETRY_PROJECTIONS_DETAIL_ESRI_TRAITS(102746, srs::proj::lcc, srs_spheroid_tag, "+proj=lcc +lat_1=38.03333333333333 +lat_2=39.2 +lat_0=37.66666666666666 +lon_0=-78.5 +x_0=3499999.999999999 +y_0=2000000 +ellps=GRS80 +datum=NAD83 +to_meter=0.3048006096012192 +no_defs");
BOOST_GEOMETRY_PROJECTIONS_DETAIL_ESRI_TRAITS(102747, srs::proj::lcc, srs_spheroid_tag, "+proj=lcc +lat_1=36.76666666666667 +lat_2=37.96666666666667 +lat_0=36.33333333333334 +lon_0=-78.5 +x_0=3499999.999999999 +y_0=1000000 +ellps=GRS80 +datum=NAD83 +to_meter=0.3048006096012192 +no_defs");
BOOST_GEOMETRY_PROJECTIONS_DETAIL_ESRI_TRAITS(102748, srs::proj::lcc, srs_spheroid_tag, "+proj=lcc +lat_1=47.5 +lat_2=48.73333333333333 +lat_0=47 +lon_0=-120.8333333333333 +x_0=500000.0000000002 +y_0=0 +ellps=GRS80 +datum=NAD83 +to_meter=0.3048006096012192 +no_defs");
BOOST_GEOMETRY_PROJECTIONS_DETAIL_ESRI_TRAITS(102749, srs::proj::lcc, srs_spheroid_tag, "+proj=lcc +lat_1=45.83333333333334 +lat_2=47.33333333333334 +lat_0=45.33333333333334 +lon_0=-120.5 +x_0=500000.0000000002 +y_0=0 +ellps=GRS80 +datum=NAD83 +to_meter=0.3048006096012192 +no_defs");
BOOST_GEOMETRY_PROJECTIONS_DETAIL_ESRI_TRAITS(102750, srs::proj::lcc, srs_spheroid_tag, "+proj=lcc +lat_1=39 +lat_2=40.25 +lat_0=38.5 +lon_0=-79.5 +x_0=600000.0000000001 +y_0=0 +ellps=GRS80 +datum=NAD83 +to_meter=0.3048006096012192 +no_defs");
BOOST_GEOMETRY_PROJECTIONS_DETAIL_ESRI_TRAITS(102751, srs::proj::lcc, srs_spheroid_tag, "+proj=lcc +lat_1=37.48333333333333 +lat_2=38.88333333333333 +lat_0=37 +lon_0=-81 +x_0=600000.0000000001 +y_0=0 +ellps=GRS80 +datum=NAD83 +to_meter=0.3048006096012192 +no_defs");
BOOST_GEOMETRY_PROJECTIONS_DETAIL_ESRI_TRAITS(102752, srs::proj::lcc, srs_spheroid_tag, "+proj=lcc +lat_1=45.56666666666667 +lat_2=46.76666666666667 +lat_0=45.16666666666666 +lon_0=-90 +x_0=600000.0000000001 +y_0=0 +ellps=GRS80 +datum=NAD83 +to_meter=0.3048006096012192 +no_defs");
BOOST_GEOMETRY_PROJECTIONS_DETAIL_ESRI_TRAITS(102753, srs::proj::lcc, srs_spheroid_tag, "+proj=lcc +lat_1=44.25 +lat_2=45.5 +lat_0=43.83333333333334 +lon_0=-90 +x_0=600000.0000000001 +y_0=0 +ellps=GRS80 +datum=NAD83 +to_meter=0.3048006096012192 +no_defs");
BOOST_GEOMETRY_PROJECTIONS_DETAIL_ESRI_TRAITS(102754, srs::proj::lcc, srs_spheroid_tag, "+proj=lcc +lat_1=42.73333333333333 +lat_2=44.06666666666667 +lat_0=42 +lon_0=-90 +x_0=600000.0000000001 +y_0=0 +ellps=GRS80 +datum=NAD83 +to_meter=0.3048006096012192 +no_defs");
BOOST_GEOMETRY_PROJECTIONS_DETAIL_ESRI_TRAITS(102755, srs::proj::tmerc, srs_spheroid_tag, "+proj=tmerc +lat_0=40.5 +lon_0=-105.1666666666667 +k=0.9999375 +x_0=200000 +y_0=0 +ellps=GRS80 +datum=NAD83 +to_meter=0.3048006096012192 +no_defs");
BOOST_GEOMETRY_PROJECTIONS_DETAIL_ESRI_TRAITS(102756, srs::proj::tmerc, srs_spheroid_tag, "+proj=tmerc +lat_0=40.5 +lon_0=-107.3333333333333 +k=0.9999375 +x_0=399999.9999999999 +y_0=100000 +ellps=GRS80 +datum=NAD83 +to_meter=0.3048006096012192 +no_defs");
BOOST_GEOMETRY_PROJECTIONS_DETAIL_ESRI_TRAITS(102757, srs::proj::tmerc, srs_spheroid_tag, "+proj=tmerc +lat_0=40.5 +lon_0=-108.75 +k=0.9999375 +x_0=600000.0000000001 +y_0=0 +ellps=GRS80 +datum=NAD83 +to_meter=0.3048006096012192 +no_defs");
BOOST_GEOMETRY_PROJECTIONS_DETAIL_ESRI_TRAITS(102758, srs::proj::tmerc, srs_spheroid_tag, "+proj=tmerc +lat_0=40.5 +lon_0=-110.0833333333333 +k=0.9999375 +x_0=799999.9999999999 +y_0=100000 +ellps=GRS80 +datum=NAD83 +to_meter=0.3048006096012192 +no_defs");
BOOST_GEOMETRY_PROJECTIONS_DETAIL_ESRI_TRAITS(102761, srs::proj::lcc, srs_spheroid_tag, "+proj=lcc +lat_1=18.03333333333334 +lat_2=18.43333333333333 +lat_0=17.83333333333333 +lon_0=-66.43333333333334 +x_0=200000 +y_0=200000 +ellps=GRS80 +datum=NAD83 +to_meter=0.3048006096012192 +no_defs");
BOOST_GEOMETRY_PROJECTIONS_DETAIL_ESRI_TRAITS(102766, srs::proj::poly, srs_spheroid_tag, "+proj=poly +lat_0=13.47246635277778 +lon_0=-144.7487507055556 +x_0=49999.99999999999 +y_0=49999.99999999999 +ellps=GRS80 +datum=NAD83 +to_meter=0.3048006096012192 +no_defs");
BOOST_GEOMETRY_PROJECTIONS_DETAIL_ESRI_TRAITS(103300, srs::proj::lcc, srs_spheroid_tag, "+proj=lcc +lat_1=49.8333339 +lat_2=51.16666733333333 +lat_0=90 +lon_0=4.367486666666666 +x_0=150000.01256 +y_0=5400088.4378 +ellps=intl +units=m +no_defs");
BOOST_GEOMETRY_PROJECTIONS_DETAIL_ESRI_TRAITS(104000, srs::proj::longlat, srs_spheroid_tag, "+proj=longlat +ellps=clrk66 +datum=NAD27 +no_defs");
BOOST_GEOMETRY_PROJECTIONS_DETAIL_ESRI_TRAITS(104101, srs::proj::longlat, srs_spheroid_tag, "+proj=longlat +ellps=bessel +no_defs");
BOOST_GEOMETRY_PROJECTIONS_DETAIL_ESRI_TRAITS(104102, srs::proj::longlat, srs_spheroid_tag, "+proj=longlat +ellps=bessel +no_defs");
BOOST_GEOMETRY_PROJECTIONS_DETAIL_ESRI_TRAITS(104103, srs::proj::longlat, srs_spheroid_tag, "+proj=longlat +ellps=clrk80 +no_defs");
BOOST_GEOMETRY_PROJECTIONS_DETAIL_ESRI_TRAITS(104104, srs::proj::longlat, srs_spheroid_tag, "+proj=longlat +ellps=intl +no_defs");
BOOST_GEOMETRY_PROJECTIONS_DETAIL_ESRI_TRAITS(104105, srs::proj::longlat, srs_spheroid_tag, "+proj=longlat +ellps=bessel +no_defs");
BOOST_GEOMETRY_PROJECTIONS_DETAIL_ESRI_TRAITS(104106, srs::proj::longlat, srs_spheroid_tag, "+proj=longlat +ellps=intl +no_defs");
BOOST_GEOMETRY_PROJECTIONS_DETAIL_ESRI_TRAITS(104107, srs::proj::longlat, srs_spheroid_tag, "+proj=longlat +ellps=GRS80 +no_defs");
BOOST_GEOMETRY_PROJECTIONS_DETAIL_ESRI_TRAITS(104108, srs::proj::longlat, srs_spheroid_tag, "+proj=longlat +ellps=GRS80 +no_defs");
BOOST_GEOMETRY_PROJECTIONS_DETAIL_ESRI_TRAITS(104261, srs::proj::longlat, srs_spheroid_tag, "+proj=longlat +a=6378249.2 +b=6356514.999904194 +no_defs");
BOOST_GEOMETRY_PROJECTIONS_DETAIL_ESRI_TRAITS(104304, srs::proj::longlat, srs_spheroid_tag, "+proj=longlat +a=6378249.2 +b=6356514.999904194 +no_defs");
BOOST_GEOMETRY_PROJECTIONS_DETAIL_ESRI_TRAITS(104305, srs::proj::longlat, srs_spheroid_tag, "+proj=longlat +ellps=clrk80 +no_defs");

} // namespace detail
#endif // DOXYGEN_NO_DETAIL

}}} // namespace boost::geometry::projections


#endif

