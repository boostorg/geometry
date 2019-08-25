// Boost.Geometry (aka GGL, Generic Geometry Library)

// Copyright (c) 2019 Tinko Bartels, Berlin, Germany.

// Use, modification and distribution is subject to the Boost Software License,
// Version 1.0. (See accompanying file LICENSE_1_0.txt or copy at
// http://www.boost.org/LICENSE_1_0.txt)

#ifndef BOOST_GEOMETRY_EXTENSIONS_RANDOM_UNIFORM_POINT_DISTRIBUTION_HPP
#define BOOST_GEOMETRY_EXTENSIONS_RANDOM_UNIFORM_POINT_DISTRIBUTION_HPP

#include <boost/geometry/extensions/random/strategies/agnostic/uniform_envelope_rejection.hpp>
#include <boost/geometry/extensions/random/strategies/agnostic/uniform_linear.hpp>
#include <boost/geometry/extensions/random/strategies/agnostic/uniform_point_distribution_discrete.hpp>
#include <boost/geometry/extensions/random/strategies/cartesian/uniform_point_distribution_box.hpp>
#include <boost/geometry/extensions/random/strategies/cartesian/uniform_point_distribution_segment.hpp>
#include <boost/geometry/extensions/random/strategies/spherical/edwilliams_avform_intermediate.hpp>
#include <boost/geometry/extensions/random/strategies/spherical/uniform_inverse_transform_sampling.hpp>

namespace boost { namespace geometry { namespace random
{

template
<
    typename DomainGeometry,
    typename Point = typename geometry::point_type<DomainGeometry>::type,
    typename Strategy =
        typename strategy::uniform_point_distribution::services::default_strategy
            <
                Point,
                DomainGeometry
            >
>
class uniform_point_distribution
{
public:
    typedef Point result_type;
    typedef DomainGeometry domain_type;
    class param_type {
    public:
        param_type(DomainGeometry const& domain) : m_domain(domain) {}
        param_type(param_type const& p) : m_domain(p.m_domain) {}
        DomainGeometry const& domain() const { return m_domain; }
        bool operator==(param_type const& rhs) const
        {
            return equals(m_domain, rhs.m_domain);
        }
    private:
        DomainGeometry m_domain;
    };
    uniform_point_distribution(DomainGeometry const& domain)
        : m_strategy(domain),
          m_param(domain) {}
    uniform_point_distribution()
        : m_strategy(DomainGeometry()),
          m_param(DomainGeometry()) {}
    uniform_point_distribution(param_type const& param)
        : m_strategy(param.domain()),
          m_param(param) {}
    void reset() { m_strategy.reset(); }
    param_type param() const { return m_param; }
    void param(param_type const& p)
    {
        m_param = p;
        m_strategy = Strategy(p.domain());
    }
    bool operator==(uniform_point_distribution const& rhs) const
    {
        return m_strategy.equals(m_param.domain(),
                                 rhs.domain(),
                                 rhs.m_strategy);
    }
    domain_type const& domain() const { return m_param.domain(); }
    template< typename Gen >
    result_type operator()(Gen& g)
    {
        return m_strategy.apply(g, m_param.domain());
    }
    template< typename Gen >
    result_type operator()(Gen& g, param_type const& p)
    {
        Strategy strat(p.domain());
        return strat.apply(g, p.domain());
    }
private:
    Strategy m_strategy;
    param_type m_param;
};

template
<
    typename DomainGeometry,
    typename Point = typename geometry::point_type<DomainGeometry>::type
>
inline uniform_point_distribution<DomainGeometry, Point>
    return_uniform_point_distribution(DomainGeometry const& domain)
{
    return uniform_point_distribution<DomainGeometry, Point>(domain);
}

}}} // namespace boost::geometry::random

template
<
    typename Char,
    typename Traits,
    typename Point,
    typename DomainGeometry,
    typename Strategy
>
inline std::basic_ostream<Char, Traits>& operator<<
    (
        std::basic_ostream<Char, Traits> &os,
        boost::geometry::random::uniform_point_distribution
            <
                DomainGeometry, Point, Strategy
            > const& dist
    )
{
    os << boost::geometry::wkt<DomainGeometry>(dist.domain());
    return os;
}

template
<
    typename Char,
    typename Traits,
    typename Point,
    typename DomainGeometry,
    typename Strategy
>
inline std::basic_istream<Char, Traits>& operator>>
    (
        std::basic_istream<Char, Traits> &is,
        boost::geometry::random::uniform_point_distribution
            <
                DomainGeometry, Point, Strategy
            > & dist
    )
{
    std::basic_string<Char, Traits> line;
    std::getline(is, line);
    DomainGeometry g;
    namespace bg = boost::geometry;
    typedef typename bg::random::uniform_point_distribution
        <
            DomainGeometry, Point, Strategy
        >::param_type param_type;
    bg::read_wkt<DomainGeometry>(line, g);
    dist.param( param_type(g) );
    return is;
}

#endif // BOOST_GEOMETRY_EXTENSIONS_RANDOM_UNIFORM_POINT_DISTRIBUTION_HPP
