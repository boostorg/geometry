// Boost.Geometry (aka GGL, Generic Geometry Library)

// Copyright (c) 2020 Tinko Bartels, Berlin, Germany.

// Contributed and/or modified by Tinko Bartels,
//   as part of Google Summer of Code 2019 program.

// Use, modification and distribution is subject to the Boost Software License,
// Version 1.0. (See accompanying file LICENSE_1_0.txt or copy at
// http://www.boost.org/LICENSE_1_0.txt)

#ifndef BOOST_GEOMETRY_EXTENSIONS_RANDOM_POLICIES_UNIFORM_LINEAR_HPP
#define BOOST_GEOMETRY_EXTENSIONS_RANDOM_POLICIES_UNIFORM_LINEAR_HPP

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
#include <boost/geometry/strategies/cartesian/distance_pythagoras.hpp>
#include <boost/geometry/strategies/geographic/distance.hpp>
#include <boost/geometry/strategies/spherical/distance_haversine.hpp>

#include <boost/geometry/extensions/random/policies/uniform_default_policy.hpp>

namespace boost { namespace geometry
{

namespace policy { namespace uniform_point_distribution {

template
<
    typename Point,
    typename DomainGeometry,
    typename InterpolationStrategy = void,
    typename DistanceStrategy = void
>
struct linear_single
{
private:
    typedef typename boost::mpl::if_
        <
            boost::is_void<InterpolationStrategy>,
            typename strategy::line_interpolate::services::default_strategy
                <
                    typename cs_tag<Point>::type
                >::type,
            InterpolationStrategy
        >::type interpolation_strategy;
    typedef typename boost::mpl::if_
        <
            boost::is_void<DistanceStrategy>,
            typename strategy::distance::services::default_strategy
                <
                    point_tag, point_tag, Point
                >::type,
            DistanceStrategy
        >::type distance_strategy;
    interpolation_strategy m_interpolation_strategy;
    distance_strategy m_distance_strategy;
public:
    linear_single(DomainGeometry const& d,
                  interpolation_strategy const& s = interpolation_strategy(),
                  distance_strategy const& l = distance_strategy()) :
	m_interpolation_strategy(s),
	m_distance_strategy(l) {}
    bool equals(DomainGeometry const& l_domain,
                DomainGeometry const& r_domain,
                linear_single const&) const
    {
        return boost::geometry::equals(l_domain, r_domain);
    }
    template<typename Generator>
    Point apply(Generator& g, DomainGeometry const& d)
    {
        typedef typename select_most_precise
            <
                double,
                typename coordinate_type<Point>::type
            >::type sample_type;
        std::uniform_real_distribution<sample_type> dist(0, length(d, m_distance_strategy));
        sample_type r = dist(g);
        Point p;
        boost::geometry::line_interpolate(d, r, p, m_interpolation_strategy);
        return p;
    }
    void reset(DomainGeometry const&) {};
};

template
<
    typename Point,
    typename DomainGeometry,
    typename InterpolationStrategy = void,
    typename DistanceStrategy = void
>
class linear_multi
{
private:
    typedef typename default_length_result<DomainGeometry>::type length_type;
    typedef typename point_type<DomainGeometry>::type domain_point_type;
    typedef typename boost::mpl::if_
        <  
            boost::is_void<InterpolationStrategy>,
            typename strategy::line_interpolate::services::default_strategy
                <
                    typename cs_tag<Point>::type
                >::type,
            InterpolationStrategy
        >::type interpolation_strategy;
    typedef typename boost::mpl::if_
        <
            boost::is_void<DistanceStrategy>,
            typename strategy::distance::services::default_strategy
                <
                    point_tag, point_tag, Point
                >::type,
            DistanceStrategy
        >::type distance_strategy;
    std::vector<std::size_t> skip_list;
    std::vector<domain_point_type> point_cache;
    std::vector<length_type> m_accumulated_lengths;
    interpolation_strategy m_interpolation_strategy;
public:
    linear_multi(DomainGeometry const& d,
                 interpolation_strategy const& in = interpolation_strategy(),
                 distance_strategy const& l = distance_strategy()) :
        m_interpolation_strategy(in)
    {
        std::size_t i = 0;
        point_cache.push_back(*segments_begin(d)->first);
        m_accumulated_lengths.push_back(0);
        for (auto it = segments_begin(d) ; it != segments_end(d) ; ++it)
        {
            m_accumulated_lengths.push_back(
                m_accumulated_lengths.back() + length(*it, l));
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
                linear_multi const& r_policy) const
    {
        if(r_policy.skip_list.size() != skip_list.size()
            || r_policy.point_cache.size() != point_cache.size())
            return false;
        for (std::size_t i = 0; i < skip_list.size(); ++i)
        {
            if (skip_list[i] != r_policy.skip_list[i]) return false;
        }
        for (std::size_t i = 0; i < point_cache.size(); ++i)
        {
            if (!boost::geometry::equals(point_cache[i], r_policy.point_cache[i])) return false;
        }
        return true;
    }
    template<typename Generator>
    Point apply(Generator& g, DomainGeometry const& d)
    {
        typedef typename select_most_precise
            <
                double,
                typename coordinate_type<Point>::type
            >::type sample_type;
        typedef model::segment<domain_point_type> segment;
        std::uniform_real_distribution<sample_type> dist(0, m_accumulated_lengths.back());
        sample_type r = dist(g);
        std::size_t i = std::distance(
            m_accumulated_lengths.begin(),
            std::lower_bound(m_accumulated_lengths.begin(),
                             m_accumulated_lengths.end(),
                             r));
        std::size_t offset = std::distance(
            skip_list.begin(),
            std::lower_bound(skip_list.begin(), skip_list.end(), i));
        Point p;
        boost::geometry::line_interpolate(
            segment(point_cache[ i + offset - 1], point_cache[ i + offset ]),
            r - m_accumulated_lengths[ i - 1 ],
            p,
	    m_interpolation_strategy);
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
struct default_policy
<
    Point,
    DomainGeometry,
    segment_tag,
    single_tag,
    Dim,
    CsTag
>
{
    typedef linear_single<Point, DomainGeometry> type;
};

template
<
    typename Point,
    typename DomainGeometry,
    typename MultiOrSingle,
    int Dim,
    typename CsTag
>
struct default_policy
<
    Point,
    DomainGeometry,
    linear_tag,
    MultiOrSingle,
    Dim,
    CsTag
>
{
    typedef linear_multi<Point, DomainGeometry> type;
};

} // namespace services

}} // namespace policy::uniform_point_distribution

}} // namespace boost::geometry

#endif //  BOOST_GEOMETRY_EXTENSIONS_RANDOM_POLICIES_UNIFORM_LINEAR_HPP
