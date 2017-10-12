// Boost.Geometry

// Copyright (c) 2017, Oracle and/or its affiliates.
// Contributed and/or modified by Adam Wulkiewicz, on behalf of Oracle

// Use, modification and distribution is subject to the Boost Software License,
// Version 1.0. (See accompanying file LICENSE_1_0.txt or copy at
// http://www.boost.org/LICENSE_1_0.txt)

#ifndef BOOST_GEOMETRY_SRS_PROJECTIONS_PROJ4_PARAMS_HPP
#define BOOST_GEOMETRY_SRS_PROJECTIONS_PROJ4_PARAMS_HPP


#include <string>

#include <boost/geometry/srs/spheroid.hpp>


namespace boost { namespace geometry { namespace srs
{

struct dynamic {};


struct proj4
{
    explicit proj4(const char* s)
        : str(s)
    {}

    explicit proj4(std::string const& s)
        : str(s)
    {}

    std::string str;
};


template <typename Proj, typename Model = srs::spheroid<double> >
struct static_proj4
{
    typedef Proj proj_type;
    typedef Model model_type;

    static_proj4()
    {}

    explicit static_proj4(Model const& m)
        : model(m)
    {}

    explicit static_proj4(std::string const& s)
        : str(s)
    {}

    explicit static_proj4(const char* s)
        : str(s)
    {}

    static_proj4(Model const& m, std::string const& s)
        : model(m), str(s)
    {}

    static_proj4(Model const& m, const char* s)
        : model(m), str(s)
    {}

    Model model;
    std::string str;
};


}}} // namespace boost::geometry::srs

#endif // BOOST_GEOMETRY_SRS_PROJECTIONS_PROJ4_PARAMS_HPP
