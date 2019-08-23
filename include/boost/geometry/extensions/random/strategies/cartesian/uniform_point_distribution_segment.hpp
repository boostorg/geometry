// Boost.Geometry (aka GGL, Generic Geometry Library)

// Copyright (c) 2019 Tinko Bartels, Berlin, Germany.

// Use, modification and distribution is subject to the Boost Software License,
// Version 1.0. (See accompanying file LICENSE_1_0.txt or copy at
// http://www.boost.org/LICENSE_1_0.txt)

#ifndef BOOST_GEOMETRY_EXTENSIONS_RANDOM_STRATEGIES_CARTESIAN_UNIFORM_POINT_DISTRIBUTION_SEGMENT_HPP
#define BOOST_GEOMETRY_EXTENSIONS_RANDOM_STRATEGIES_CARTESIAN_UNIFORM_POINT_DISTRIBUTION_SEGMENT_HPP

#include <random>
#include <iterator>

#include <boost/geometry/algorithms/equals.hpp>
#include <boost/geometry/iterators/segment_iterator.hpp>
#include <boost/geometry/arithmetic/arithmetic.hpp>

#include <boost/geometry/extensions/random/strategies/uniform_point_distribution.hpp>

namespace boost { namespace geometry
{

namespace strategy { namespace uniform_point_distribution {

template
<
    typename Point,
    typename DomainGeometry
>
struct segment_linear_mapping_distribution
{
    segment_linear_mapping_distribution(DomainGeometry const& g) {}
    bool equals(DomainGeometry const& l_domain,
                DomainGeometry const& r_domain,
                segment_linear_mapping_distribution const& r_strategy) const
    {
        return boost::geometry::equals(l_domain.domain(), r_domain.domain());
    }

    template
    <
        typename LengthType,
        typename PointIn
    >
    static Point map(PointIn const& p1, PointIn const& p2, LengthType const& l)
    {
        Point r;
        assign(r, p2);
        subtract_point(r, p1);
        multiply_value(r, l);
        add_point(r, p1);
        return r;
    }

    template<typename Gen>
    Point apply(Gen& g, DomainGeometry const& d)
    {
        typedef typename select_most_precise
            <
                typename coordinate_type<DomainGeometry>::type,
                double
            >::type sample_type;
        std::uniform_real_distribution<sample_type> real_dist(0, 1);
        typedef typename default_length_result<DomainGeometry>::type
            length_type;
        boost::geometry::segment_view<DomainGeometry> view(d);
        return map<length_type>(*view.begin(), *(view.begin() + 1), real_dist(g));
    }
    void reset(DomainGeometry const&) {};
};

namespace services {

template
<
    typename Point,
    typename DomainGeometry,
    int Dim
>
struct default_strategy
<
    Point,
    DomainGeometry,
    segment_tag,
    single_tag,
    Dim,
    cartesian_tag
> : public segment_linear_mapping_distribution<Point, DomainGeometry> {
    typedef segment_linear_mapping_distribution<Point, DomainGeometry> base;
    using base::base;
};

} // namespace services

}} // namespace strategy::uniform_point_distribution

}} // namespace boost::geometry

#endif // BOOST_GEOMETRY_EXTENSIONS_RANDOM_STRATEGIES_CARTESIAN_UNIFORM_POINT_DISTRIBUTION_SEGMENT_HPP
