// Boost.Geometry (aka GGL, Generic Geometry Library)

// Copyright (c) 2008-2012 Barend Gehrels, Amsterdam, the Netherlands.

// This file was modified by Oracle on 2017.
// Modifications copyright (c) 2017, Oracle and/or its affiliates.
// Contributed and/or modified by Adam Wulkiewicz, on behalf of Oracle

// Use, modification and distribution is subject to the Boost Software License,
// Version 1.0. (See accompanying file LICENSE_1_0.txt or copy at
// http://www.boost.org/LICENSE_1_0.txt)

#ifndef BOOST_GEOMETRY_STRATEGY_PROJECT_TRANSFORMER_HPP
#define BOOST_GEOMETRY_STRATEGY_PROJECT_TRANSFORMER_HPP


#include <boost/geometry/core/coordinate_dimension.hpp>
#include <boost/geometry/core/coordinate_system.hpp>
#include <boost/geometry/core/tags.hpp>

#include <boost/geometry/extensions/gis/projections/projection.hpp>

#include <boost/type_traits/is_same.hpp>


namespace boost { namespace geometry
{
    
namespace strategy { namespace transform
{

/*!
    \brief Transformation strategy to do transform using a Map Projection
    \ingroup transform
    \tparam Proj Projection type
    \tparam CT CalculationType used internally
 */
template
<
    typename Proj = srs::dynamic,
    typename CT = double
>
class project_transformer
{
public:
    inline project_transformer()
    {}

    template <typename Parameters>
    inline project_transformer(Parameters const& parameters)
        : m_proj(parameters)
    {}

    template <typename Point1, typename Point2>
    inline bool apply(Point1 const& p1, Point2& p2) const
    {
        typedef typename boost::is_same
            <
                typename geometry::cs_tag<Point2>::type,
                cartesian_tag
            >::type is_forward;

        return apply(p1, p2, is_forward());
    }

private:
    template <typename Point1, typename Point2>
    inline bool apply(Point1 const& p1, Point2& p2, boost::true_type const& ) const
    {
        return m_proj.forward(p1, p2);
    }

    template <typename Point1, typename Point2>
    inline bool apply(Point1 const& p1, Point2& p2, boost::false_type const& ) const
    {
        return m_proj.inverse(p1, p2);
    }

    geometry::projection<Proj, CT> m_proj;
};


/*!
    \brief Transformation strategy to do transform using a Map Projection
    \ingroup transform
    \tparam Proj Projection type
    \tparam CT CalculationType used internally

    See also \link p03_projmap_example.cpp the projmap example \endlink
    where this last one plus a transformation using a projection are used.
 */
template
<
    typename Proj = srs::dynamic,
    typename CT = double
>
class project_forward_transformer
{
public:
    inline project_forward_transformer()
    {}

    template <typename Parameters>
    inline project_forward_transformer(Parameters const& parameters)
        : m_proj(parameters)
    {}

    template <typename LatLong, typename Cartesian>
    inline bool apply(LatLong const& p1, Cartesian& p2) const
    {
        return m_proj.forward(p1, p2);
    }

private:
    geometry::projection<Proj, CT> m_proj;
};


/*!
    \brief Transformation strategy to do transform using a Map Projection
    \ingroup transform
    \tparam Proj Projection type
    \tparam CT CalculationType used internally
 */
template
<
    typename Proj = srs::dynamic,
    typename CT = double
>
class project_inverse_transformer
{
public:
    inline project_inverse_transformer()
    {}

    template <typename Parameters>
    inline project_inverse_transformer(Parameters const& parameters)
        : m_proj(parameters)
    {}

    template <typename Cartesian, typename LatLong>
    inline bool apply(Cartesian const& p1, LatLong& p2) const
    {
        return m_proj.inverse(p1, p2);
    }

private:
    geometry::projection<Proj, CT> m_proj;
};


}} // namespace strategy::transform

}} // namespace boost::geometry


#endif // BOOST_GEOMETRY_STRATEGY_PROJECT_TRANSFORMER_HPP
