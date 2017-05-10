// Boost.Geometry (aka GGL, Generic Geometry Library)

// Copyright (c) 2017, Oracle and/or its affiliates.
// Contributed and/or modified by Adam Wulkiewicz, on behalf of Oracle

// Use, modification and distribution is subject to the Boost Software License,
// Version 1.0. (See accompanying file LICENSE_1_0.txt or copy at
// http://www.boost.org/LICENSE_1_0.txt)

#ifndef BOOST_GEOMETRY_SRS_TRANSFORMATION_HPP
#define BOOST_GEOMETRY_SRS_TRANSFORMATION_HPP


#include <string>

#include <boost/geometry/algorithms/detail/convert_point_to_point.hpp>

#include <boost/geometry/core/coordinate_dimension.hpp>

#include <boost/geometry/srs/projection.hpp>

#include <boost/geometry/srs/projections/impl/pj_transform.hpp>

#include <boost/mpl/assert.hpp>
#include <boost/mpl/if.hpp>
#include <boost/smart_ptr/shared_ptr.hpp>
#include <boost/throw_exception.hpp>
#include <boost/type_traits/is_integral.hpp>
#include <boost/type_traits/is_same.hpp>


namespace boost { namespace geometry
{

namespace projections { namespace detail
{

template
<
    typename RangeOrPoint,
    typename Tag = typename geometry::tag<RangeOrPoint>::type
>
struct transform
{
    template <typename Proj1, typename Proj2>
    static inline void apply(Proj1 const& proj1, Proj2 const& proj2,
                             RangeOrPoint & range)
    {
        pj_transform(proj1.proj(), proj1.proj().params(),
                     proj2.proj(), proj2.proj().params(),
                     range);
    }
};

template <typename Point>
struct transform<Point, point_tag>
{
    template <typename Proj1, typename Proj2>
    static inline void apply(Proj1 const& proj1, Proj2 const& proj2,
                             Point & point)
    {
        Point * ptr = boost::addressof(point);

        std::pair<Point*, Point*> range = std::make_pair(ptr, ptr + 1);

        pj_transform(proj1.proj(), proj1.proj().params(),
                     proj2.proj(), proj2.proj().params(),
                     range);
    }
};


}} // namespace projections::detail
    
namespace srs
{


/*!
    \brief Representation of projection
    \details Either dynamic or static projection representation
    \ingroup projection
    \tparam Proj1 default_dynamic or static projection parameters
    \tparam Proj2 default_dynamic or static projection parameters
    \tparam CT calculation type used internally
*/
template
<
    typename Proj1 = srs::dynamic,
    typename Proj2 = srs::dynamic,
    typename CT = double
>
class transformation
{
    // TODO: currently the cordinate type of RangeOut is used instead
    typedef typename projections::detail::promote_to_double<CT>::type calc_t;

public:
    transformation()
    {}

    template <typename Parameters1>
    transformation(Parameters1 const& parameters1)
        : m_proj1(parameters1)
    {}

    template <typename Parameters1, typename Parameters2>
    transformation(Parameters1 const& parameters1, Parameters2 const& parameters2)
        : m_proj1(parameters1)
        , m_proj2(parameters2)
    {}

    template <typename RangeOrPointIn, typename RangeOrPointOut>
    bool forward(RangeOrPointIn const& in, RangeOrPointOut & out) const
    {
        if (boost::addressof(in) != boost::addressof(out))
            geometry::convert(in, out);

        try
        {
            projections::detail::transform
                <
                    RangeOrPointOut
                >::apply(m_proj1, m_proj2, out);
            
            return true;
        }
        catch(...)
        {
            return false;
        }
    }

    template <typename RangeOrPointIn, typename RangeOrPointOut>
    bool inverse(RangeOrPointIn const& in, RangeOrPointOut & out) const
    {
        if (boost::addressof(in) != boost::addressof(out))
            geometry::convert(in, out);

        try
        {
            projections::detail::transform
                <
                    RangeOrPointOut
                >::apply(m_proj2, m_proj1, out);

            return true;
        }
        catch(...)
        {
            return false;
        }
    }

private:
    projections::projection<Proj1, CT> m_proj1;
    projections::projection<Proj2, CT> m_proj2;
};

} // namespace srs


}} // namespace boost::geometry


#endif // BOOST_GEOMETRY_SRS_TRANSFORMATION_HPP
