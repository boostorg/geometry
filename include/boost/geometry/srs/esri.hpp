// Boost.Geometry

// Copyright (c) 2017, Oracle and/or its affiliates.
// Contributed and/or modified by Adam Wulkiewicz, on behalf of Oracle

// Use, modification and distribution is subject to the Boost Software License,
// Version 1.0. (See accompanying file LICENSE_1_0.txt or copy at
// http://www.boost.org/LICENSE_1_0.txt)

#ifndef BOOST_GEOMETRY_SRS_ESRI_HPP
#define BOOST_GEOMETRY_SRS_ESRI_HPP


#include <boost/geometry/srs/projection.hpp>
#include <boost/geometry/srs/projections/esri.hpp>
#include <boost/geometry/srs/projections/esri_params.hpp>
#include <boost/geometry/srs/projections/esri_traits.hpp>


namespace boost { namespace geometry
{
    
namespace projections
{

template <typename CT>
struct dynamic_parameters<srs::esri, CT>
{
    static inline projections::parameters<CT> apply(srs::esri const& params)
    {
        return projections::detail::pj_init_plus<CT>(
                srs::dynamic(),
                projections::detail::esri_to_string(params.code),
                false);
    }  
};

template <int Code, typename CT>
class proj_wrapper<srs::static_esri<Code>, CT>
{
    typedef typename projections::detail::promote_to_double<CT>::type calc_t;

    typedef projections::detail::esri_traits<Code> esri_traits;

    typedef projections::parameters<calc_t> parameters_type;
    typedef typename projections::detail::static_projection_type
        <
            typename esri_traits::type,
            typename esri_traits::srs_tag,
            calc_t,
            parameters_type
        >::type projection_type;

public:
    proj_wrapper()
        : m_proj(projections::detail::pj_init_plus<calc_t>(
                        srs::static_esri<Code>(),
                        esri_traits::par(), false))
    {}

    projection_type const& proj() const { return m_proj; }
    projection_type & mutable_proj() { return m_proj; }

private:
    projection_type m_proj;
};


} // namespace projections


namespace srs
{


template <int Code, typename CT>
class projection<srs::static_esri<Code>, CT>
    : public projections::projection<srs::static_esri<Code>, CT>
{
    typedef projections::projection<srs::static_esri<Code>, CT> base_t;

public:
    projection()
    {}
};


} // namespace srs


}} // namespace boost::geometry


#endif // BOOST_GEOMETRY_SRS_ESRI_HPP
