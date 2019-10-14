// Boost.Geometry (aka GGL, Generic Geometry Library)

// Copyright (c) 2019 Tinko Bartels, Berlin, Germany.

// Use, modification and distribution is subject to the Boost Software License,
// Version 1.0. (See accompanying file LICENSE_1_0.txt or copy at
// http://www.boost.org/LICENSE_1_0.txt)

#ifndef BOOST_GEOMETRY_EXTENSIONS_RANDOM_STRATEGIES_AGNOSTIC_UNIFORM_LINEAR_HPP
#define BOOST_GEOMETRY_EXTENSIONS_RANDOM_STRATEGIES_AGNOSTIC_UNIFORM_LINEAR_HPP

#include <random>
#include <iterator>
#include <algorithm>

#include <boost/geometry/algorithms/equals.hpp>
#include <boost/geometry/core/point_type.hpp>
#include <boost/geometry/core/cs.hpp>
#include <boost/geometry/algorithms/length.hpp>
#include <boost/geometry/algorithms/line_interpolate.hpp>
#include <boost/geometry/strategies/cartesian/line_interpolate.hpp>
#include <boost/geometry/strategies/geographic/line_interpolate.hpp>
#include <boost/geometry/strategies/spherical/line_interpolate.hpp>

#include <boost/geometry/extensions/random/strategies/uniform_point_distribution.hpp>

namespace boost { namespace geometry
{

namespace strategy { namespace uniform_point_distribution {

template
<
    typename Point,
    typename DomainGeometry,
    typename InterpolationStrategy =
        typename strategy::line_interpolate::services::default_strategy
            <
                typename cs_tag<DomainGeometry>::type
            >::type
>
struct uniform_linear_single
{
    uniform_linear_single(DomainGeometry const& g) {}
    bool equals(DomainGeometry const& l_domain,
                DomainGeometry const& r_domain,
                uniform_linear_single const& r_strategy) const
    {
        return boost::geometry::equals(l_domain, r_domain);
    }
    
    template<typename Gen>
    Point apply(Gen& g, DomainGeometry const& d)
    {
        typedef typename select_most_precise
            <
                double,
                typename coordinate_type<Point>::type
            >::type sample_type;
        std::uniform_real_distribution<sample_type> dist(0, length(d));
        sample_type r = dist(g);
        Point p;
        boost::geometry::line_interpolate(d, r, p);
        return p;
    }
    void reset(DomainGeometry const&) {};
};

template
<
    typename Point,
    typename DomainGeometry,
    typename InterpolationStrategy =
        typename strategy::line_interpolate::services::default_strategy
            <
                typename cs_tag<DomainGeometry>::type
            >::type
>
class uniform_linear_multi
{
private:
    typedef typename default_length_result<DomainGeometry>::type length_type;
    typedef typename point_type<DomainGeometry>::type domain_point_type;
    std::vector<std::size_t> skip_list;
    std::vector<domain_point_type> point_cache;
    std::vector<length_type> accumulated_lengths;
public:
    uniform_linear_multi(DomainGeometry const& g)
    {
        std::size_t i = 0;
        point_cache.push_back(*segments_begin(g)->first);
        accumulated_lengths.push_back(0);
        for (auto it = segments_begin(g) ; it != segments_end(g) ; ++it)
        {
            accumulated_lengths.push_back(
                accumulated_lengths.back() + length(*it));
            if (!boost::geometry::equals(point_cache.back(), *it->first))
            {
                point_cache.push_back(*it->first);
                skip_list.push_back(i);
            }
            point_cache.push_back(*it->second);
            ++i;
        }
    }
    bool equals(DomainGeometry const& l_domain,
                DomainGeometry const& r_domain,
                uniform_linear_multi const& r_strategy) const
    {
        if(r_strategy.skip_list.size() != skip_list.size()
            || r_strategy.point_cache.size() != point_cache.size())
            return false;
        for (std::size_t i = 0; i < skip_list.size(); ++i)
        {
            if (skip_list[i] != r_strategy.skip_list[i]) return false;
        }
        for (std::size_t i = 0; i < point_cache.size(); ++i)
        {
            if (!boost::geometry::equals(point_cache[i], r_strategy.point_cache[i])) return false;
        }
        return true;
    }
    template<typename Gen>
    Point apply(Gen& g, DomainGeometry const& d)
    {
        typedef typename select_most_precise
            <
                double,
                typename coordinate_type<Point>::type
            >::type sample_type;
        typedef model::segment<domain_point_type> segment;
        std::uniform_real_distribution<sample_type> dist(0, accumulated_lengths.back());
        sample_type r = dist(g);
        std::size_t i = std::distance(
            accumulated_lengths.begin(),
            std::lower_bound(accumulated_lengths.begin(),
                             accumulated_lengths.end(),
                             r));
        std::size_t offset = std::distance(
            skip_list.begin(),
            std::lower_bound(skip_list.begin(), skip_list.end(), i));
        Point p;
        boost::geometry::line_interpolate(
            segment(point_cache[ i + offset - 1], point_cache[ i + offset ]),
            r - accumulated_lengths[ i - 1 ],
            p);
        return p;
    }
    void reset(DomainGeometry const&) {};
};

namespace services {

template
<
    typename Point,
    typename DomainGeometry,
    int Dim,
    typename CsTag
>
struct default_strategy
<
    Point,
    DomainGeometry,
    segment_tag,
    single_tag,
    Dim,
    CsTag
> : public uniform_linear_single<Point, DomainGeometry> {
    typedef uniform_linear_single<Point, DomainGeometry> base;
    using base::base;
};

template
<
    typename Point,
    typename DomainGeometry,
    typename MultiOrSingle,
    int Dim,
    typename CsTag
>
struct default_strategy
<
    Point,
    DomainGeometry,
    linear_tag,
    MultiOrSingle,
    Dim,
    CsTag
> : public uniform_linear_multi<Point, DomainGeometry> {
    typedef uniform_linear_multi<Point, DomainGeometry> base;
    using base::base;
};

} // namespace services

}} // namespace strategy::uniform_point_distribution

}} // namespace boost::geometry

#endif //  BOOST_GEOMETRY_EXTENSIONS_RANDOM_STRATEGIES_AGNOSTIC_UNIFORM_LINEAR_HPP
