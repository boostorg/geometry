// Boost.Geometry (aka GGL, Generic Geometry Library)

// Copyright (c) 2007-2012 Barend Gehrels, Amsterdam, the Netherlands.

// This file was modified by Oracle on 2017.
// Modifications copyright (c) 2017, Oracle and/or its affiliates.
// Contributed and/or modified by Adam Wulkiewicz, on behalf of Oracle

// Use, modification and distribution is subject to the Boost Software License,
// Version 1.0. (See accompanying file LICENSE_1_0.txt or copy at
// http://www.boost.org/LICENSE_1_0.txt)

#ifndef BOOST_GEOMETRY_PROJECTIONS_IMPL_BASE_DYNAMIC_HPP
#define BOOST_GEOMETRY_PROJECTIONS_IMPL_BASE_DYNAMIC_HPP

#include <string>

#include <boost/geometry/extensions/gis/projections/impl/projects.hpp>

namespace boost { namespace geometry { namespace projections
{

#ifndef DOXYGEN_NO_DETAIL
namespace detail
{

/*!
    \brief projection virtual base class
    \details class containing virtual methods
    \ingroup projection
    \tparam LL latlong point type
    \tparam XY xy point type
*/
template
<
    typename LL,
    typename XY,
    typename P = parameters // temporary default
>
class base_v
{
    protected :
        // see comment above
        //typedef typename geometry::coordinate_type<LL>::type LL_T;
        //typedef typename geometry::coordinate_type<XY>::type XY_T;
        typedef double LL_T;
        typedef double XY_T;

    public :

        typedef LL geographic_point_type; ///< latlong point type
        typedef XY cartesian_point_type;  ///< xy point type

        /// Forward projection, from Latitude-Longitude to Cartesian
        virtual bool forward(LL const& lp, XY& xy) const = 0;

        /// Inverse projection, from Cartesian to Latitude-Longitude
        virtual bool inverse(XY const& xy, LL& lp) const = 0;

        /// Forward projection using lon / lat and x / y separately
        virtual void fwd(LL_T& lp_lon, LL_T& lp_lat, XY_T& xy_x, XY_T& xy_y) const = 0;

        /// Inverse projection using x / y and lon / lat
        virtual void inv(XY_T& xy_x, XY_T& xy_y, LL_T& lp_lon, LL_T& lp_lat) const = 0;

        /// Returns name of projection
        virtual std::string name() const = 0;

        /// Returns parameters of projection
        virtual P const& params() const = 0;

        /// Returns mutable parameters of projection
        virtual P& mutable_params() = 0;

        virtual ~base_v() {}

};

// Base-virtual-forward
template <typename C, typename LL, typename XY, typename P>
class base_v_f : public base_v<LL, XY, P>
{
protected:

    typedef typename base_v<LL, XY, P>::LL_T LL_T;
    typedef typename base_v<LL, XY, P>::XY_T XY_T;

public:

    base_v_f(P const& params) : m_proj(params) {}

    virtual P const& params() const { return m_proj.params(); }

    virtual P& mutable_params() { return m_proj.mutable_params(); }

    virtual bool forward(LL const& ll, XY& xy) const
    {
        return m_proj.forward(ll, xy);
    }

    virtual void fwd(LL_T& lp_lon, LL_T& lp_lat, XY_T& xy_x, XY_T& xy_y) const
    {
        m_proj.fwd(lp_lon, lp_lat, xy_x, xy_y);
    }

    virtual bool inverse(XY const& , LL& ) const
    {
        // exception?
        return false;
    }
    virtual void inv(XY_T& , XY_T& , LL_T& , LL_T& ) const
    {
        // exception?
    }

    virtual std::string name() const
    {
        return m_proj.name();
    }

protected:

    C m_proj;
};

// Base-virtual-forward/inverse
template <typename C, typename LL, typename XY, typename P>
class base_v_fi : public base_v_f<C, LL, XY, P>
{
private:

    typedef typename base_v_f<C, LL, XY, P>::LL_T LL_T;
    typedef typename base_v_f<C, LL, XY, P>::XY_T XY_T;

public :

    base_v_fi(P const& params) : base_v_f<C, LL, XY, P>(params) {}

    virtual bool inverse(XY const& xy, LL& ll) const
    {
        return this->m_proj.inverse(xy, ll);
    }

    virtual void inv(XY_T& xy_x, XY_T& xy_y, LL_T& lp_lon, LL_T& lp_lat) const
    {
        this->m_proj.inv(xy_x, xy_y, lp_lon, lp_lat);
    }
};

} // namespace detail
#endif // DOXYGEN_NO_DETAIL

}}} // namespace boost::geometry::projections

#endif // BOOST_GEOMETRY_PROJECTIONS_IMPL_BASE_DYNAMIC_HPP
