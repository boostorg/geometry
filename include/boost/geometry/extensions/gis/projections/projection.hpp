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
#include <boost/mpl/if.hpp>
#include <boost/smart_ptr/shared_ptr.hpp>
#include <boost/throw_exception.hpp>
#include <boost/type_traits/is_integral.hpp>
#include <boost/type_traits/is_same.hpp>


namespace boost { namespace geometry { namespace projections
{

namespace detail
{

template <typename CT>
struct promote_to_double
{
    typedef typename boost::mpl::if_c
        <
            boost::is_integral<CT>::value || boost::is_same<CT, float>::value,
            double, CT
        >::type type;
};

} // namespace detail

/*!
    \brief Representation of projection
    \details Either dynamic or static projection representation
    \ingroup projection
    \tparam Proj default_dynamic or static projection parameters
    \tparam CT calculation type used internally
*/
template
<
    typename Proj = default_dynamic,
    typename CT = double
>
class projection
{
    BOOST_MPL_ASSERT_MSG((false),
                         NOT_IMPLEMENTED_FOR_THIS_PROJECTION_TAG,
                         (Proj));
};

template <typename CT>
class projection<default_dynamic, CT>
{
    // Some projections do not work with float -> wrong results
    // select <double> from int/float/double and else selects T
    typedef typename detail::promote_to_double<CT>::type calc_t;

public:
    /*!
    \ingroup projection
    \brief Initializes a projection as a string, using the format with + and =
    \details The projection can be initialized with a string (with the same format as the PROJ4 package) for
      convenient initialization from, for example, the command line
    \par Example
        <tt>+proj=labrd +ellps=intl +lon_0=46d26'13.95E +lat_0=18d54S +azi=18d54 +k_0=.9995 +x_0=400000 +y_0=800000</tt>
        for the Madagascar projection.
    \note Parameters are described in the group
    */
    projection(proj4 const& params)
        : m_ptr(create(detail::pj_init_plus(default_dynamic(), params.str)))
    {}

    projection(epsg const& params)
        : m_ptr(create(detail::pj_init_plus(default_dynamic(),
                                detail::code_to_string(params.code), false)))
    {}

    /// Forward projection, from Latitude-Longitude to Cartesian
    template <typename LL, typename XY>
    bool forward(LL const& lp, XY& xy) const
    {
        return m_ptr->forward(lp, xy);
    }

    /// Inverse projection, from Cartesian to Latitude-Longitude
    template <typename XY, typename LL>
    bool inverse(XY const& xy, LL& lp) const
    {
        return m_ptr->inverse(xy, lp);
    }

private:
    typedef projections::detail::base_v<calc_t, parameters> vprj_t;

    static vprj_t* create(parameters const& pj_params)
    {
        static detail::factory<calc_t, parameters> fac;

        vprj_t* result = fac.create_new(pj_params);

        if (result == NULL)
        {
            BOOST_THROW_EXCEPTION(proj_exception());
        }

        return result;
    }

    boost::shared_ptr<vprj_t> m_ptr;
};

template <typename Proj, typename Model, typename CT>
class projection<static_proj4<Proj, Model>, CT>
{
    typedef typename detail::promote_to_double<CT>::type calc_t;

    typedef typename detail::static_projection_type
        <
            Proj,
            typename traits::tag<Model>::type,
            calc_t,
            parameters
        >::type projection_type;

public:
    projection()
        : m_proj(get_parameters(static_proj4<Proj, Model>()))
    {}

    projection(static_proj4<Proj, Model> const& params)
        : m_proj(get_parameters(params))
    {}

    /// Forward projection, from Latitude-Longitude to Cartesian
    template <typename LL, typename XY>
    bool forward(LL const& lp, XY& xy) const
    {
        return m_proj.forward(lp, xy);
    }

    /// Inverse projection, from Cartesian to Latitude-Longitude
    template <typename XY, typename LL>
    bool inverse(XY const& xy, LL& lp) const
    {
        return m_proj.inverse(xy, lp);
    }

private:
    static projections::parameters get_parameters(static_proj4<Proj, Model> const& params)
    {
        return detail::pj_init_plus(params, params.str);
    }

    projection_type m_proj;
};

template <int Code, typename CT>
class projection<static_epsg<Code>, CT>
{
    typedef typename detail::promote_to_double<CT>::type calc_t;

    typedef detail::epsg_traits<Code> epsg_traits;

    typedef typename detail::static_projection_type
        <
            typename epsg_traits::type,
            typename epsg_traits::srs_tag,
            calc_t,
            parameters
        >::type projection_type;

public:
    projection()
        : m_proj(detail::pj_init_plus(static_epsg<Code>(), epsg_traits::par(), false))
    {}

    /// Forward projection, from Latitude-Longitude to Cartesian
    template <typename LL, typename XY>
    bool forward(LL const& lp, XY& xy) const
    {
        return m_proj.forward(lp, xy);
    }

    /// Inverse projection, from Cartesian to Latitude-Longitude
    template <typename XY, typename LL>
    bool inverse(XY const& xy, LL& lp) const
    {
        return m_proj.inverse(xy, lp);
    }

private:
    projection_type m_proj;
};

}}} // namespace boost::geometry::projections



#endif

