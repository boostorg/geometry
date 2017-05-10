// Boost.Geometry (aka GGL, Generic Geometry Library)

// Copyright (c) 2008-2012 Barend Gehrels, Amsterdam, the Netherlands.

// This file was modified by Oracle on 2017.
// Modifications copyright (c) 2017, Oracle and/or its affiliates.
// Contributed and/or modified by Adam Wulkiewicz, on behalf of Oracle

// Use, modification and distribution is subject to the Boost Software License,
// Version 1.0. (See accompanying file LICENSE_1_0.txt or copy at
// http://www.boost.org/LICENSE_1_0.txt)

#ifndef BOOST_GEOMETRY_SRS_PROJECTION_HPP
#define BOOST_GEOMETRY_SRS_PROJECTION_HPP


#include <string>

#include <boost/geometry/algorithms/detail/convert_point_to_point.hpp>

#include <boost/geometry/core/coordinate_dimension.hpp>

#include <boost/geometry/srs/ellps.hpp>
#include <boost/geometry/srs/parameters.hpp>
#include <boost/geometry/srs/projections/epsg.hpp>
#include <boost/geometry/srs/projections/epsg_traits.hpp>
#include <boost/geometry/srs/projections/exception.hpp>
#include <boost/geometry/srs/projections/factory.hpp>
#include <boost/geometry/srs/projections/impl/base_dynamic.hpp>
#include <boost/geometry/srs/projections/impl/base_static.hpp>
#include <boost/geometry/srs/projections/impl/pj_init.hpp>

#include <boost/mpl/assert.hpp>
#include <boost/mpl/if.hpp>
#include <boost/smart_ptr/shared_ptr.hpp>
#include <boost/throw_exception.hpp>
#include <boost/type_traits/is_integral.hpp>
#include <boost/type_traits/is_same.hpp>


namespace boost { namespace geometry
{
    
namespace projections
{

#ifndef DOXYGEN_NO_DETAIL
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

// Copy coordinates of dimensions >= MinDim
template <std::size_t MinDim, typename Point1, typename Point2>
inline void copy_higher_dimensions(Point1 const& point1, Point2 & point2)
{
    static const std::size_t dim1 = geometry::dimension<Point1>::value;
    static const std::size_t dim2 = geometry::dimension<Point2>::value;
    static const std::size_t lesser_dim = dim1 < dim2 ? dim1 : dim2;
    BOOST_MPL_ASSERT_MSG((lesser_dim >= MinDim),
                         THE_DIMENSION_OF_POINTS_IS_TOO_SMALL,
                         (Point1, Point2));

    geometry::detail::conversion::point_to_point
        <
            Point1, Point2, MinDim, lesser_dim
        > ::apply(point1, point2);

    // TODO: fill point2 with zeros if dim1 < dim2 ?
}

/// Forward projection, from Latitude-Longitude to Cartesian
template <typename Proj, typename LL, typename XY>
inline bool forward(Proj const& proj, LL const& ll, XY& xy)
{
    // (Geographic -> Cartesian) will be projected, rest will be copied.
    // So first copy third or higher dimensions
    copy_higher_dimensions<2>(ll, xy);

    return proj.forward(ll, xy);
}

/// Inverse projection, from Cartesian to Latitude-Longitude
template <typename Proj, typename XY, typename LL>
inline bool inverse(Proj const& proj, XY const& xy, LL& ll)
{
    // (Cartesian -> Geographic) will be projected, rest will be copied.
    // So first copy third or higher dimensions
    copy_higher_dimensions<2>(ll, xy);

    return proj.inverse(xy, ll);
}

} // namespace detail
#endif // DOXYGEN_NO_DETAIL

template <typename Proj, typename CT>
class projection
{
    BOOST_MPL_ASSERT_MSG((false),
                         NOT_IMPLEMENTED_FOR_THIS_PROJECTION_TAG,
                         (Proj));
};

template <typename CT>
class projection<srs::dynamic, CT>
{
    // Some projections do not work with float -> wrong results
    // select <double> from int/float/double and else selects T
    typedef typename projections::detail::promote_to_double<CT>::type calc_t;

    typedef projections::detail::base_v<calc_t, projections::parameters> vprj_t;

public:
    projection(srs::proj4 const& params)
        : m_ptr(create(projections::detail::pj_init_plus(srs::dynamic(),
                                params.str)))
    {}

    projection(srs::epsg const& params)
        : m_ptr(create(projections::detail::pj_init_plus(
                            srs::dynamic(),
                            projections::detail::code_to_string(params.code),
                            false)))
    {}

    vprj_t const& proj() const { return *m_ptr; }
    vprj_t & mutable_proj() { return *m_ptr; }

private:
    static vprj_t* create(projections::parameters const& pj_params)
    {
        static projections::detail::factory<calc_t, projections::parameters> fac;

        vprj_t* result = fac.create_new(pj_params);

        if (result == NULL)
        {
            BOOST_THROW_EXCEPTION(proj_exception(-4));
        }

        return result;
    }

    boost::shared_ptr<vprj_t> m_ptr;
};

template <typename Proj, typename Model, typename CT>
class projection<srs::static_proj4<Proj, Model>, CT>
{
    typedef typename projections::detail::promote_to_double<CT>::type calc_t;

    typedef typename projections::detail::static_projection_type
        <
            Proj,
            typename traits::tag<Model>::type,
            calc_t,
            projections::parameters
        >::type projection_type;

public:
    projection()
        : m_proj(get_parameters(srs::static_proj4<Proj, Model>()))
    {}

    projection(srs::static_proj4<Proj, Model> const& params)
        : m_proj(get_parameters(params))
    {}

    projection_type const& proj() const { return m_proj; }
    projection_type & mutable_proj() { return m_proj; }

private:
    static projections::parameters get_parameters(srs::static_proj4<Proj, Model> const& params)
    {
        return projections::detail::pj_init_plus(params, params.str);
    }

    projection_type m_proj;
};

template <int Code, typename CT>
class projection<srs::static_epsg<Code>, CT>
{
    typedef typename projections::detail::promote_to_double<CT>::type calc_t;

    typedef projections::detail::epsg_traits<Code> epsg_traits;

    typedef typename projections::detail::static_projection_type
        <
            typename epsg_traits::type,
            typename epsg_traits::srs_tag,
            calc_t,
            projections::parameters
        >::type projection_type;

public:
    projection()
        : m_proj(projections::detail::pj_init_plus(srs::static_epsg<Code>(),
                                                   epsg_traits::par(), false))
    {}

    projection_type const& proj() const { return m_proj; }
    projection_type & mutable_proj() { return m_proj; }

private:
    projection_type m_proj;
};


} // namespace projections


namespace srs
{


/*!
    \brief Representation of projection
    \details Either dynamic or static projection representation
    \ingroup projection
    \tparam Proj default_dynamic or static projection parameters
    \tparam CT calculation type used internally
*/
template
<
    typename Proj = srs::dynamic,
    typename CT = double
>
class projection
{
    BOOST_MPL_ASSERT_MSG((false),
                         NOT_IMPLEMENTED_FOR_THIS_PROJECTION_TAG,
                         (Proj));
};

template <typename CT>
class projection<srs::dynamic, CT>
{
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
    projection(srs::proj4 const& params)
        : m_projection(params)
    {}

    projection(srs::epsg const& params)
        : m_projection(params)
    {}

    /// Forward projection, from Latitude-Longitude to Cartesian
    template <typename LL, typename XY>
    bool forward(LL const& ll, XY& xy) const
    {
        return projections::detail::forward(m_projection.proj(), ll, xy);
    }

    /// Inverse projection, from Cartesian to Latitude-Longitude
    template <typename XY, typename LL>
    bool inverse(XY const& xy, LL& ll) const
    {
        return projections::detail::inverse(m_projection.proj(), xy, ll);
    }

private:
    projections::projection<srs::dynamic, CT> m_projection;
};

template <typename Proj, typename Model, typename CT>
class projection<srs::static_proj4<Proj, Model>, CT>
{
public:
    projection()
    {}

    projection(srs::static_proj4<Proj, Model> const& params)
        : m_projection(params)
    {}

    /// Forward projection, from Latitude-Longitude to Cartesian
    template <typename LL, typename XY>
    bool forward(LL const& ll, XY& xy) const
    {
        return projections::detail::forward(m_projection.proj(), ll, xy);
    }

    /// Inverse projection, from Cartesian to Latitude-Longitude
    template <typename XY, typename LL>
    bool inverse(XY const& xy, LL& ll) const
    {
        return projections::detail::inverse(m_projection.proj(), xy, ll);
    }

private:
    projections::projection<srs::static_proj4<Proj, Model>, CT> m_projection;
};

template <int Code, typename CT>
class projection<srs::static_epsg<Code>, CT>
{
public:
    projection()
    {}

    /// Forward projection, from Latitude-Longitude to Cartesian
    template <typename LL, typename XY>
    bool forward(LL const& ll, XY& xy) const
    {
        return projections::detail::forward(m_projection.proj(), ll, xy);
    }

    /// Inverse projection, from Cartesian to Latitude-Longitude
    template <typename XY, typename LL>
    bool inverse(XY const& xy, LL& ll) const
    {
        return projections::detail::inverse(m_projection.proj(), xy, ll);
    }

private:
    projections::projection<srs::static_epsg<Code>, CT> m_projection;
};


} // namespace srs


}} // namespace boost::geometry


#endif // BOOST_GEOMETRY_SRS_PROJECTION_HPP
