// Boost.Geometry (aka GGL, Generic Geometry Library)

// Copyright (c) 2008-2012 Barend Gehrels, Amsterdam, the Netherlands.

// This file was modified by Oracle on 2017.
// Modifications copyright (c) 2017, Oracle and/or its affiliates.
// Contributed and/or modified by Adam Wulkiewicz, on behalf of Oracle

// Use, modification and distribution is subject to the Boost Software License,
// Version 1.0. (See accompanying file LICENSE_1_0.txt or copy at
// http://www.boost.org/LICENSE_1_0.txt)

#ifndef BOOST_GEOMETRY_PROJECTIONS_PROJECTION_HPP
#define BOOST_GEOMETRY_PROJECTIONS_PROJECTION_HPP


#include <string>

#include <boost/geometry/extensions/gis/projections/ellps.hpp>
#include <boost/geometry/extensions/gis/projections/epsg.hpp>
#include <boost/geometry/extensions/gis/projections/epsg_traits.hpp>
#include <boost/geometry/extensions/gis/projections/factory.hpp>
#include <boost/geometry/extensions/gis/projections/impl/base_dynamic.hpp>
#include <boost/geometry/extensions/gis/projections/impl/base_static.hpp>
#include <boost/geometry/extensions/gis/projections/parameters.hpp>

#include <boost/mpl/assert.hpp>
#include <boost/smart_ptr/shared_ptr.hpp>


namespace boost { namespace geometry { namespace projections
{

struct default_dynamic {};

struct proj4
{
    explicit proj4(std::string const& s)
        : str(s)
    {}

    std::string str;
};

struct epsg
{
    explicit epsg(int c)
        : code(c)
    {}

    int code;
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

    static_proj4(Model const& m, std::string const& s)
        : model(m), str(s)
    {}

    Model model;
    std::string str;
};

template <int Code>
struct static_epsg
{
    static const int code = Code;
};


/*!
    \brief Representation of projection
    \details Either dynamic or static projection representation
    \ingroup projection
    \tparam LL latlong point type
    \tparam XY xy point type
    \tparam Proj default_dynamic or static projection tag
*/
template <typename LL, typename XY, typename Proj = default_dynamic>
class projection
{
    BOOST_MPL_ASSERT_MSG((false),
                         NOT_IMPLEMENTED_FOR_THIS_PROJECTION_TAG,
                         (Proj));
};

template <typename LL, typename XY>
class projection<LL, XY, default_dynamic>
{
public:
    explicit projection(proj4 const& params)
        : m_ptr(factory().create_new(projections::init(params.str)))
    {}

    explicit projection(epsg const& params)
        : m_ptr(factory().create_new(projections::init(params.code)))
    {}

    typedef LL geographic_point_type; ///< latlong point type
    typedef XY cartesian_point_type;  ///< xy point type

    /// Forward projection, from Latitude-Longitude to Cartesian
    bool forward(LL const& lp, XY& xy) const
    {
        return m_ptr->forward(lp, xy);
    }

    /// Inverse projection, from Cartesian to Latitude-Longitude
    bool inverse(XY const& xy, LL& lp) const
    {
        return m_ptr->inverse(xy, lp);
    }

private:
    static detail::factory<LL, XY> const& factory()
    {
        static detail::factory<LL, XY> fac;
        return fac;
    }

    boost::shared_ptr<projections::detail::base_v<LL, XY> > m_ptr;
};

template <typename LL, typename XY, typename Proj, typename Model>
class projection<LL, XY, static_proj4<Proj, Model> >
{
    typedef typename detail::static_projection_type
        <
            Proj,
            typename traits::tag<Model>::type,
            LL,
            XY,
            parameters
        >::type projection_type;

public:
    projection()
        : m_proj(get_parameters(static_proj4<Proj, Model>()))
    {}

    explicit projection(static_proj4<Proj, Model> const& params)
        : m_proj(get_parameters(params))
    {}

    typedef LL geographic_point_type; ///< latlong point type
    typedef XY cartesian_point_type;  ///< xy point type

    /// Forward projection, from Latitude-Longitude to Cartesian
    bool forward(LL const& lp, XY& xy) const
    {
        return m_proj.forward(lp, xy);
    }

    /// Inverse projection, from Cartesian to Latitude-Longitude
    bool inverse(XY const& xy, LL& lp) const
    {
        return m_proj.inverse(xy, lp);
    }

private:
    static projections::parameters get_parameters(static_proj4<Proj, Model> const& params)
    {
        // TODO: temporary, later fill parameters manually
        // Reason: currently init() throws exception
        // when there is no ellipsoid definition in the string
        std::stringstream ss;        
        ss << params.str;
        ss << std::setprecision(12)
           << " +a=" << geometry::get_radius<0>(params.model)
           << " +b=" << geometry::get_radius<2>(params.model);

        return projections::init(ss.str());
    }

    projection_type m_proj;
};

template <typename LL, typename XY, int Code>
class projection<LL, XY, static_epsg<Code> >
{
    typedef detail::epsg_traits<Code> epsg_traits;

    typedef typename detail::static_projection_type
        <
            typename epsg_traits::type,
            typename epsg_traits::srs_tag,
            LL,
            XY,
            parameters
        >::type projection_type;

public:
    projection()
        : m_proj(projections::init(epsg_traits::par()))
    {}

    typedef LL geographic_point_type; ///< latlong point type
    typedef XY cartesian_point_type;  ///< xy point type

    /// Forward projection, from Latitude-Longitude to Cartesian
    bool forward(LL const& lp, XY& xy) const
    {
        return m_proj.forward(lp, xy);
    }

    /// Inverse projection, from Cartesian to Latitude-Longitude
    bool inverse(XY const& xy, LL& lp) const
    {
        return m_proj.inverse(xy, lp);
    }

private:
    projection_type m_proj;
};

}}} // namespace boost::geometry::projections



#endif

