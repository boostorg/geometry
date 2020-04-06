// Boost.Geometry (aka GGL, Generic Geometry Library)

// Copyright (c) 2019 Tinko Bartels, Berlin, Germany.

// Contributed and/or modified by Tinko Bartels,
//   as part of Google Summer of Code 2019 program.

// Use, modification and distribution is subject to the Boost Software License,
// Version 1.0. (See accompanying file LICENSE_1_0.txt or copy at
// http://www.boost.org/LICENSE_1_0.txt)

#ifndef BOOST_GEOMETRY_EXTENSIONS_RANDOM_UNIFORM_POINT_DISTRIBUTION_HPP
#define BOOST_GEOMETRY_EXTENSIONS_RANDOM_UNIFORM_POINT_DISTRIBUTION_HPP

#include <boost/geometry/extensions/random/policies/uniform_linear.hpp>
#include <boost/geometry/extensions/random/policies/uniform_envelope_rejection.hpp>
#include <boost/geometry/extensions/random/policies/uniform_box.hpp>
#include <boost/geometry/extensions/random/policies/uniform_point_discrete.hpp>
#include <boost/geometry/extensions/random/policies/uniform_convex_polygon.hpp>
#include <boost/geometry/extensions/random/policies/uniform_convex_hull_rejection.hpp>

namespace boost { namespace geometry { namespace random
{

template
<
    typename DomainGeometry,
    typename Point = typename geometry::point_type<DomainGeometry>::type,
    typename Policy =
        typename policy::uniform_point_distribution::services::default_policy
            <
                Point,
                DomainGeometry
            >::type
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
    uniform_point_distribution(DomainGeometry const& domain,
                               Policy const& policy)
        : m_policy(policy),
          m_param(domain) {}
    uniform_point_distribution(DomainGeometry const& domain)
        : m_policy(domain),
          m_param(domain) {}
    uniform_point_distribution()
        : m_policy(DomainGeometry()),
          m_param(DomainGeometry()) {}
    uniform_point_distribution(param_type const& param)
        : m_policy(param.domain()),
          m_param(param) {}
    void reset() { m_policy.reset(); }
    param_type param() const { return m_param; }
    void param(param_type const& p)
    {
        m_param = p;
        m_policy = Policy(p.domain());
    }
    bool operator==(uniform_point_distribution const& rhs) const
    {
        return m_policy.equals(m_param.domain(),
                               rhs.domain(),
                               rhs.m_policy);
    }
    domain_type const& domain() const { return m_param.domain(); }
    template< typename Generator >
    result_type operator()(Generator& g)
    {
        return m_policy.apply(g, m_param.domain());
    }
    template< typename Generator >
    result_type operator()(Generator& g, param_type const& p)
    {
        Policy pol(p.domain());
        return pol.apply(g, p.domain());
    }
private:
    Policy m_policy;
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
    typename Policy
>
inline std::basic_ostream<Char, Traits>& operator<<
    (
        std::basic_ostream<Char, Traits> &os,
        boost::geometry::random::uniform_point_distribution
            <
                DomainGeometry, Point, Policy
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
    typename Policy
>
inline std::basic_istream<Char, Traits>& operator>>
    (
        std::basic_istream<Char, Traits> &is,
        boost::geometry::random::uniform_point_distribution
            <
                DomainGeometry, Point, Policy
            > & dist
    )
{
    std::basic_string<Char, Traits> line;
    std::getline(is, line);
    DomainGeometry g;
    namespace bg = boost::geometry;
    typedef typename bg::random::uniform_point_distribution
        <
            DomainGeometry, Point, Policy
        >::param_type param_type;
    bg::read_wkt<DomainGeometry>(line, g);
    dist.param( param_type(g) );
    return is;
}

#endif // BOOST_GEOMETRY_EXTENSIONS_RANDOM_UNIFORM_POINT_DISTRIBUTION_HPP
