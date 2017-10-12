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
#include <boost/geometry/algorithms/transform.hpp>

#include <boost/geometry/core/coordinate_dimension.hpp>

#include <boost/geometry/srs/projections/ellps.hpp>
#include <boost/geometry/srs/projections/exception.hpp>
#include <boost/geometry/srs/projections/factory.hpp>
#include <boost/geometry/srs/projections/impl/base_dynamic.hpp>
#include <boost/geometry/srs/projections/impl/base_static.hpp>
#include <boost/geometry/srs/projections/impl/pj_init.hpp>
#include <boost/geometry/srs/projections/proj4_params.hpp>

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
    // currently no need because equal dimensions are checked
}


// forward projection of any geometry type
template
<
    typename Geometry,
    typename Tag = typename geometry::tag<Geometry>::type
>
struct forward
{
    template <typename Proj>
    struct forward_strategy
    {
        forward_strategy(Proj const& proj)
            : m_proj(proj)
        {}

        template <typename LL, typename XY>
        inline bool apply(LL const& ll, XY & xy) const
        {
            return forward<LL>::apply(ll, xy, m_proj);
        }

        Proj const& m_proj;
    };

    template <typename LL, typename XY, typename Proj>
    static inline bool apply(LL const& ll,
                             XY & xy,
                             Proj const& proj)
    {
        return resolve_strategy::transform
                ::apply(ll, xy, forward_strategy<Proj>(proj));
    }
};

template <typename Geometry>
struct forward<Geometry, point_tag>
{
    template <typename LL, typename XY, typename Proj>
    static inline bool apply(LL const& ll, XY & xy, Proj const& proj)
    {
        // (Geographic -> Cartesian) will be projected, rest will be copied.
        // So first copy third or higher dimensions
        projections::detail::copy_higher_dimensions<2>(ll, xy);

        return proj.forward(ll, xy);
    }
};


// inverse projection of any geometry type
template
<
    typename Geometry,
    typename Tag = typename geometry::tag<Geometry>::type
>
struct inverse
{
    template <typename Proj>
    struct inverse_strategy
    {
        inverse_strategy(Proj const& proj)
            : m_proj(proj)
        {}

        template <typename XY, typename LL>
        inline bool apply(XY const& xy, LL & ll) const
        {
            return inverse<XY>::apply(xy, ll, m_proj);
        }

        Proj const& m_proj;
    };

    template <typename XY, typename LL, typename Proj>
    static inline bool apply(XY const& xy,
                             LL & ll,
                             Proj const& proj)
    {
        return resolve_strategy::transform
                ::apply(xy, ll, inverse_strategy<Proj>(proj));
    }
};

template <typename Geometry>
struct inverse<Geometry, point_tag>
{
    template <typename XY, typename LL, typename Proj>
    static inline bool apply(XY const& xy, LL & ll, Proj const& proj)
    {
        // (Cartesian -> Geographic) will be projected, rest will be copied.
        // So first copy third or higher dimensions
        projections::detail::copy_higher_dimensions<2>(xy, ll);

        return proj.inverse(xy, ll);
    }
};

} // namespace detail
#endif // DOXYGEN_NO_DETAIL


template <typename Params, typename CT>
struct dynamic_parameters
{
    BOOST_MPL_ASSERT_MSG((false),
                         NOT_IMPLEMENTED_FOR_THESE_PARAMETERS,
                         (Params));
};

template <typename CT>
struct dynamic_parameters<srs::proj4, CT>
{
    static inline projections::parameters<CT> apply(srs::proj4 const& params)
    {
        return projections::detail::pj_init_plus<CT>(srs::dynamic(), params.str);
    }  
};

// proj_wrapper class and its specializations wrapps the internal projection
// representation and implements transparent creation of projection object
template <typename Proj, typename CT>
class proj_wrapper
{
    BOOST_MPL_ASSERT_MSG((false),
                         NOT_IMPLEMENTED_FOR_THIS_PROJECTION,
                         (Proj));
};

template <typename CT>
class proj_wrapper<srs::dynamic, CT>
{
    // Some projections do not work with float -> wrong results
    // select <double> from int/float/double and else selects T
    typedef typename projections::detail::promote_to_double<CT>::type calc_t;

    typedef projections::parameters<calc_t> parameters_type;
    typedef projections::detail::base_v<calc_t, parameters_type> vprj_t;

public:
    template <typename Params>
    proj_wrapper(Params const& params)
        : m_ptr(create(projections::dynamic_parameters<Params, calc_t>::apply(params)))
    {}

    vprj_t const& proj() const { return *m_ptr; }
    vprj_t & mutable_proj() { return *m_ptr; }

private:
    static vprj_t* create(parameters_type const& pj_params)
    {
        static projections::detail::factory<calc_t, parameters_type> fac;

        vprj_t* result = fac.create_new(pj_params);

        if (result == NULL)
        {
            if (pj_params.name.empty())
            {
                BOOST_THROW_EXCEPTION(projection_not_named_exception());
            }
            else
            {
                BOOST_THROW_EXCEPTION(projection_unknown_id_exception(pj_params.name));
            }
        }

        return result;
    }

    boost::shared_ptr<vprj_t> m_ptr;
};

template <typename Proj, typename Model, typename CT>
class proj_wrapper<srs::static_proj4<Proj, Model>, CT>
{
    typedef typename projections::detail::promote_to_double<CT>::type calc_t;

    typedef projections::parameters<calc_t> parameters_type;
    typedef typename projections::detail::static_projection_type
        <
            Proj,
            typename traits::tag<Model>::type,
            calc_t,
            parameters_type
        >::type projection_type;

public:
    proj_wrapper()
        : m_proj(get_parameters(srs::static_proj4<Proj, Model>()))
    {}

    proj_wrapper(srs::static_proj4<Proj, Model> const& params)
        : m_proj(get_parameters(params))
    {}

    projection_type const& proj() const { return m_proj; }
    projection_type & mutable_proj() { return m_proj; }

private:
    static parameters_type get_parameters(srs::static_proj4<Proj, Model> const& params)
    {
        return projections::detail::pj_init_plus<calc_t>(params, params.str);
    }

    projection_type m_proj;
};


// projection class implements transparent forward/inverse projection interface
template <typename Proj, typename CT>
class projection
    : private proj_wrapper<Proj, CT>
{
    typedef proj_wrapper<Proj, CT> base_t;

public:
    projection()
    {}

    template <typename Params>
    projection(Params const& params)
        : base_t(params)
    {}

    /// Forward projection, from Latitude-Longitude to Cartesian
    template <typename LL, typename XY>
    inline bool forward(LL const& ll, XY& xy) const
    {
        concepts::check_concepts_and_equal_dimensions<LL const, XY>();

        return projections::detail::forward<LL>::apply(ll, xy, base_t::proj());
    }

    /// Inverse projection, from Cartesian to Latitude-Longitude
    template <typename XY, typename LL>
    inline bool inverse(XY const& xy, LL& ll) const
    {
        concepts::check_concepts_and_equal_dimensions<XY const, LL>();

        return projections::detail::inverse<XY>::apply(xy, ll, base_t::proj());
    }
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
                         NOT_IMPLEMENTED_FOR_THIS_PROJECTION,
                         (Proj));
};

template <typename CT>
class projection<srs::dynamic, CT>
    : public projections::projection<srs::dynamic, CT>
{
    typedef projections::projection<srs::dynamic, CT> base_t;

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
    template <typename Params>
    projection(Params const& params)
        : base_t(params)
    {}
};

template <typename Proj, typename Model, typename CT>
class projection<srs::static_proj4<Proj, Model>, CT>
    : public projections::projection<srs::static_proj4<Proj, Model>, CT>
{
    typedef projections::projection<srs::static_proj4<Proj, Model>, CT> base_t;

public:
    projection()
    {}

    projection(srs::static_proj4<Proj, Model> const& params)
        : base_t(params)
    {}
};

} // namespace srs


}} // namespace boost::geometry


#endif // BOOST_GEOMETRY_SRS_PROJECTION_HPP
