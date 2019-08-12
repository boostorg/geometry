// Boost.Geometry (aka GGL, Generic Geometry Library)

// Copyright (c) 2019 Tinko Bartels, Berlin, Germany.

// Use, modification and distribution is subject to the Boost Software License,
// Version 1.0. (See accompanying file LICENSE_1_0.txt or copy at
// http://www.boost.org/LICENSE_1_0.txt)

#ifndef BOOST_GEOMETRY_EXTENSIONS_RANDOM_DETAIL_UNIFORM_POINT_DISTRIBUTION_HPP
#define BOOST_GEOMETRY_EXTENSIONS_RANDOM_DETAIL_UNIFORM_POINT_DISTRIBUTION_HPP

#include <boost/range.hpp>
#include <boost/type_traits/is_integral.hpp>

#include <boost/geometry/core/access.hpp>
#include <boost/geometry/core/assert.hpp>
#include <boost/geometry/util/for_each_coordinate.hpp>
#include <boost/geometry/util/select_most_precise.hpp>
#include <boost/geometry/views/box_view.hpp>
#include <boost/geometry/views/segment_view.hpp>
#include <boost/geometry/geometries/box.hpp>
#include <boost/geometry/geometries/multi_point.hpp>
#include <boost/geometry/algorithms/append.hpp>
#include <boost/geometry/algorithms/envelope.hpp>

#include <algorithm>
#include <iterator>
#include <random>
#include <vector>

namespace boost { namespace geometry { namespace random { namespace detail
{

template<typename Point, typename DomainGeometry>
class uniform_point_distribution_base
{
public:
    typedef Point result_type;
    class param_type {
    public:
        param_type(DomainGeometry const& domain):_domain(domain) {}
        param_type(param_type const& p):_domain(p._domain) {}
        DomainGeometry const& domain() const {return _domain;}
        bool operator==(param_type const& rhs) const
        {
            return equals(_domain,rhs._domain);
        }
    private:
        DomainGeometry _domain;
    };
    uniform_point_distribution_base():_param(DomainGeometry()) {}
    uniform_point_distribution_base(param_type const& param):_param(param) {}
    uniform_point_distribution_base(DomainGeometry const& domain) :
        _param(param_type(domain)) {}
    void reset() {}
    param_type param() const {return _param;}
    DomainGeometry const& domain() const {return _param.domain();}
    bool operator==(uniform_point_distribution_base const& rhs) const
    {
        return _param==rhs._param;
    }
    void param(param_type const& p) {_param = p;}
protected:
   param_type _param;
};

template
<
    typename Point,
    typename DomainGeometry,
    typename DomainTag,
    typename SingleOrMulti,
    typename CSTag,
    std::size_t dimension
>
class uniform_point_distribution {};

template
<
    typename Point,
    typename DomainGeometry,
    typename CSTag,
    std::size_t dimension
>
class uniform_point_distribution
<
    Point,
    DomainGeometry,
    pointlike_tag,
    single_tag,
    CSTag,
    dimension
> : public uniform_point_distribution_base<Point, DomainGeometry>
{
    typedef uniform_point_distribution_base<Point, DomainGeometry> base;
public:
    using base::base;
    template<typename Generator>
    Point operator()(Generator& gen, typename base::param_type const& p)
    { return this->domain(); }
    template<typename Generator>
    Point operator()(Generator& gen) {return (*this)(gen, this->_param);}
};

template
<
    typename Point,
    typename DomainGeometry,
    typename CSTag,
    std::size_t dimension
>
class uniform_point_distribution
<
    Point,
    DomainGeometry,
    pointlike_tag,
    multi_tag,
    CSTag,
    dimension
> : public uniform_point_distribution_base<Point, DomainGeometry>
{
    typedef uniform_point_distribution_base<Point, DomainGeometry> base;
public:
    using base::base;
    template<typename Generator>
    Point operator()(Generator& gen, typename base::param_type const& p)
    {
        std::size_t count = num_points(p.domain());
        std::uniform_int_distribution<std::size_t> dis(0, count-1);
        return *(boost::begin(p.domain())+dis(gen));
    }
    template<typename Generator>
    Point operator()(Generator& gen) {return (*this)(gen, this->_param);}
    bool operator==(
        uniform_point_distribution
            <
                Point,
                DomainGeometry,
                pointlike_tag,
                multi_tag,
                CSTag,
                dimension
            >& rhs)
    {
        DomainGeometry const& lhs_domain = this->domain();
        DomainGeometry const& rhs_domain = rhs.domain();
        if(num_points(lhs_domain) != num_points(rhs_domain)) return false;
        typedef typename range_iterator<const DomainGeometry>::type cit;
        cit lhs_it = boost::begin(lhs_domain),
            lhs_end = boost::end(lhs_domain),
            rhs_it = boost::begin(rhs_domain);
        while(lhs_it != lhs_end) {
            if(!equals(*lhs_it++, *rhs_it++)) return false;
        }
        return true;
    }
};

template
<
    typename Point,
    typename Box,
    typename Generator,
    bool isIntegral =
        boost::is_integral<typename coordinate_type<Point>::type>::type::value
>
struct interval_sample {};

template<typename Point, typename Box, typename Generator>
struct interval_sample<Point, Box, Generator, true>
{
    Box const& b;
    Generator& gen;
    inline interval_sample(Box const& b, Generator& gen):b(b),gen(gen) {}
    template <typename PointDst, std::size_t Index>
    inline void apply(PointDst& point_dst) const
    {
        std::uniform_int_distribution<typename coordinate_type<Point>::type>
            dist(get<min_corner,Index>(b),get<max_corner,Index>(b));
        set<Index>(point_dst,dist(gen));
    }
};

template<typename Point, typename Box, typename Generator>
struct interval_sample<Point, Box, Generator, false>
{
    Box const& b;
    Generator& gen;
    inline interval_sample(Box const& b, Generator& gen):b(b),gen(gen) {}

    template <typename PointDst, std::size_t Index>
    inline void apply(PointDst& point_dst) const
    {
        std::uniform_real_distribution<typename coordinate_type<Point>::type>
            dist(get<min_corner,Index>(b),get<max_corner,Index>(b));
        set<Index>(point_dst,dist(gen));
    }
};

template<typename Point, typename DomainGeometry, typename Generator>
inline Point sample_spherical_box_2d(DomainGeometry const& b, Generator& gen)
{
    typedef typename coordinate_type<Point>::type coordinate_type;
    typedef typename select_most_precise
    <
        coordinate_type,
        double
    >::type computation_type;

    Point out;
    std::uniform_real_distribution<computation_type> lon_dist(
        get_as_radian<0, 0>(b),
        get_as_radian<1, 0>(b));
    set_from_radian<0>(out, lon_dist(gen));

    coordinate_type lat1 = get_as_radian<0, 1>(b);
    coordinate_type lat2 = get_as_radian<1, 1>(b);
    coordinate_type x1   = (1.0 - std::cos(lat1))/2,
                    x2   = (1.0 - std::cos(lat2))/2;
    std::uniform_real_distribution<computation_type> x_dist(
        std::min(x1, x2),
        std::max(x1, x2));
    coordinate_type x = x_dist(gen);
    set_from_radian<1>(out, std::acos(1.0 - 2.0 * x));
    return out;
}

template<typename Point, typename DomainGeometry>
class uniform_point_distribution
<
    Point,
    DomainGeometry,
    box_tag,
    single_tag,
    spherical_tag,
    2
> : public uniform_point_distribution_base<Point, DomainGeometry>
{
    typedef uniform_point_distribution_base<Point, DomainGeometry> base;
public:
    using base::base;
    template<typename Generator>
    Point operator()(Generator& gen, typename base::param_type const& p)
    {
        return sample_spherical_box_2d<Point>(p.domain(), gen);
    }
    template<typename Generator>
    Point operator()(Generator& gen)
    {
        return (*this)(gen, this->_param);
    }
};

template<typename Point, typename DomainGeometry>
class uniform_point_distribution
<
    Point,
    DomainGeometry,
    box_tag,
    single_tag,
    spherical_tag,
    3
> : public uniform_point_distribution_base<Point, DomainGeometry>
{
    typedef uniform_point_distribution_base<Point, DomainGeometry> base;
public:
    using base::base;
    template<typename Generator>
    Point operator()(Generator& gen, typename base::param_type const& p)
    {
        typedef typename coordinate_type<Point>::type coordinate_type;
        typedef typename select_most_precise
        <
            coordinate_type,
            double
        >::type computation_type;

        Point out = sample_spherical_box_2d<Point>(p.domain(), gen);
        coordinate_type r1 = get<0, 2>(p.domain());
        coordinate_type r2 = get<1, 2>(p.domain());
        std::uniform_real_distribution<computation_type>
            r_dist( r1 * r1 * r1 , r2 * r2 * r2 );
        set<2>(out, std::cbrt(r_dist(gen)));
        return out;
    }
    template<typename Generator>
    Point operator()(Generator& gen)
    {
        return (*this)(gen, this->_param);
    }
};

template<typename Point, typename DomainGeometry, std::size_t dimension>
class uniform_point_distribution
<
    Point,
    DomainGeometry,
    box_tag,
    single_tag,
    cartesian_tag,
    dimension
> : public uniform_point_distribution_base<Point, DomainGeometry>
{
    typedef uniform_point_distribution_base<Point, DomainGeometry> base;
public:
    using base::base;
    template<typename Generator>
    Point operator()(Generator& gen, typename base::param_type const& p)
    {
        model::box<Point> cached_box;
        assign(cached_box, p.domain());
        Point out;
        for_each_coordinate(out,
            interval_sample<Point, DomainGeometry, Generator>(p.domain(), gen));
        return out;
    }
    template<typename Generator>
    Point operator()(Generator& gen)
    {
        return (*this)(gen, this->_param);
    }
};

template
<
    typename Point,
    typename CSTag,
    std::size_t dimension,
    typename PointIn,
    typename LengthType
>
struct sample_segment;

template
<
    typename Point,
    std::size_t dimension,
    typename PointIn,
    typename LengthType
>
struct sample_segment
<
    Point,
    cartesian_tag,
    dimension,
    PointIn,
    LengthType
>
{
    static Point apply(PointIn const& p1, PointIn const& p2,
        LengthType const& r)
    {
        Point out;
        assign(out,p2);
        subtract_point(out, p1);
        multiply_value(out, r);
        add_point(out, p1);
        return out;
    }
};

template<typename Point, typename PointIn, typename LengthType>
inline Point sample_segment_spherical_2d(PointIn const& p1, PointIn const& p2,
    LengthType const& f)
{
    Point out;
    const auto lat1 = get_as_radian<1>(p1);
    const auto lon1 = get_as_radian<0>(p1);
    const auto lat2 = get_as_radian<1>(p2);
    const auto lon2 = get_as_radian<0>(p2);
    LengthType const d = std::acos(
          std::sin(lat1) * std::sin(lat2)
        + std::cos(lat1) * std::cos(lat2) * std::cos(lon1 - lon2));
    LengthType const A = std::sin( ( 1 - f ) * d ) / std::sin(d);
    LengthType const B = std::sin( f * d ) / std::sin( d );
    LengthType const x = A * std::cos(lat1) * std::cos(lon1)
                       + B * std::cos(lat2) * std::cos(lon2);
    LengthType const y = A * std::cos(lat1) * std::sin(lon1)
                       + B * std::cos(lat2) * std::sin(lon2);
    LengthType const z = A * std::sin(lat1) + B * std::sin(lat2);
    LengthType const lat = std::atan2(z, std::sqrt(x * x + y * y));
    LengthType const lon = std::atan2(y, x);
    set_from_radian<1>(out, lat);
    set_from_radian<0>(out, lon);
    return out;
}

template<typename Point, typename PointIn, typename LengthType>
struct sample_segment<Point, spherical_tag, 2, PointIn, LengthType> {
    static Point apply(PointIn const& p1, PointIn const& p2,
        LengthType const& f)
    {
        return sample_segment_spherical_2d<Point>(p1, p2, f);
    }
};

template
<
    typename Point,
    typename PointVec,
    typename IndexVec,
    typename LengthVec,
    typename LengthType
>
inline Point sample_multi_line(PointVec const& point_cache,
    IndexVec const& skip_list, LengthVec const& accumulated_lengths,
    LengthType const& r)
{
    std::size_t i = std::distance(
        accumulated_lengths.begin(),
        std::lower_bound(
            accumulated_lengths.begin(), accumulated_lengths.end(), r));
    std::size_t offset = std::distance(skip_list.begin(),
        std::lower_bound(skip_list.begin(), skip_list.end(), i));
    return sample_segment
        <
            Point,
            typename tag_cast
                <
                    typename cs_tag<Point>::type,
                    spherical_tag
                >::type,
            dimension<Point>::type::value,
            typename PointVec::value_type,
            LengthType
        >::apply(point_cache[ i + offset - 1 ], point_cache[ i + offset ],
                ( r - accumulated_lengths[ i - 1 ]) /
                ( accumulated_lengths[ i ] - accumulated_lengths[ i - 1 ] ));
}

template
<
    typename Point,
    typename DomainGeometry,
    typename SingleOrMulti,
    typename CSTag,
    std::size_t dimension
>
class uniform_point_distribution
<
    Point,
    DomainGeometry,
    areal_tag,
    SingleOrMulti,
    CSTag,
    dimension
> : public uniform_point_distribution_base<Point, DomainGeometry>
{
    typedef uniform_point_distribution_base<Point, DomainGeometry> base;
public:
    typedef typename uniform_point_distribution_base<Point, DomainGeometry>
        ::param_type param_type;
    using base::base;
    using base::param;
    uniform_point_distribution() {}
    uniform_point_distribution(DomainGeometry const& g) : base(g)
    {
        envelope(g,_cached_box);
    }
    void param(param_type const& p)
    {
        this->_param = p;
        envelope(p.domain(), _cached_box);
    }
    template<typename Generator>
    Point operator()(Generator& gen, typename base::param_type const& p)
    {
        model::box<Point> _cached_box;
        envelope(p.domain(), _cached_box);
        uniform_point_distribution
        <
            Point,
            model::box<Point>,
            box_tag,
            single_tag,
            CSTag,
            dimension
        > box_dist(_cached_box);
        Point out = box_dist(gen);
        while(!within(out, p.domain()))
            out = box_dist(gen);
        return out;
    }
    template<typename Generator>
    Point operator()(Generator& gen)
    {
        uniform_point_distribution
        <
            Point,
            model::box<Point>,
            box_tag,
            single_tag,
            CSTag,
            dimension
        > box_dist(_cached_box);
        Point out = box_dist(gen);
        while(!within(out, this->_param.domain()))
            out = box_dist(gen);
        return out;
    }
private:
    model::box<Point> _cached_box;
};

template
<
    typename Point,
    typename DomainGeometry,
    typename SingleOrMulti,
    typename CSTag,
    std::size_t dimension
>
class uniform_point_distribution
<
    Point,
    DomainGeometry,
    linear_tag,
    SingleOrMulti,
    CSTag,
    dimension
> : public uniform_point_distribution_base<Point, DomainGeometry>
{
    typedef uniform_point_distribution_base<Point, DomainGeometry> base;
    typedef typename default_length_result<DomainGeometry>::type length_type;
    typedef typename point_type<DomainGeometry>::type domain_point_type;
public:
    using base::base;
    using base::param;
    bool operator==(uniform_point_distribution const& rhs) const
    {
        if(rhs.skip_list.size() != skip_list.size()
            || rhs.point_cache.size() != point_cache.size())
            return false;
        for(std::size_t i = 0; i < skip_list.size(); ++i)
            if(skip_list[i] != rhs.skip_list[i]) return false;
        for(std::size_t i = 0; i < point_cache.size(); ++i)
            if(!equals(point_cache[i], rhs.point_cache[i])) return false;
        return true;
    }
    uniform_point_distribution(DomainGeometry const& g) : base(g)
    {
        init(skip_list, point_cache, accumulated_lengths, this->_param);
    }
    void init(std::vector<std::size_t>& skip_list,
        std::vector<domain_point_type>& point_cache,
        std::vector<length_type>& accumulated_lengths,
        typename base::param_type const& p)
    {
        std::size_t i = 0;
        point_cache.push_back(*segments_begin(p.domain())->first);
        accumulated_lengths.push_back(0);
        for(auto it = segments_begin(p.domain()); it!=segments_end(p.domain());
            ++it) {
            accumulated_lengths.push_back(
                accumulated_lengths.back() + length(*it));
            if(!equals(point_cache.back(),*it->first)) {
                point_cache.push_back(*it->first);
                skip_list.push_back(i);
            }
            point_cache.push_back(*it->second);
            ++i;
        }
    }
    void param(typename base::param_type const& p)
    {
        this->_param = p;
        skip_list.clear();
        point_cache.clear();
        accumulated_lengths.clear();
        init(skip_list, point_cache, accumulated_lengths, p);
    }
    template<typename Generator>
    Point operator()(Generator& gen, typename base::param_type const& p)
    {
        std::vector<std::size_t> skip_list;
        std::vector<domain_point_type> point_cache;
        std::vector<length_type> accumulated_lengths;
        init(skip_list, point_cache, accumulated_lengths, p);
        typedef typename select_most_precise
            <
                double,
                typename coordinate_type<Point>::type
            >::type sample_type;
        std::uniform_real_distribution<sample_type> dist(0, 1);
        return sample_multi_line<Point>(
            point_cache,
            skip_list,
            accumulated_lengths,
            dist(gen)*accumulated_lengths.back());
    }
    template<typename Generator>
    Point operator()(Generator& gen)
    {
        typedef typename select_most_precise
            <
                double,
                typename coordinate_type<Point>::type
            >::type sample_type;
        std::uniform_real_distribution<sample_type> dist(0, 1);
        return sample_multi_line<Point>(
            point_cache,
            skip_list,
            accumulated_lengths,
            dist(gen)*accumulated_lengths.back());
    }
private:
    std::vector<std::size_t> skip_list;
    std::vector<domain_point_type> point_cache;
    std::vector<length_type> accumulated_lengths;
};

template
<
    typename Point,
    typename DomainGeometry,
    typename SingleOrMulti,
    typename CSTag,
    std::size_t dimension
>
class uniform_point_distribution
<
    Point,
    DomainGeometry,
    segment_tag,
    SingleOrMulti,
    CSTag,
    dimension
> : public uniform_point_distribution_base<Point, DomainGeometry>
{
    typedef uniform_point_distribution_base<Point, DomainGeometry> base;
    typedef uniform_point_distribution<Point, segment_view<DomainGeometry>,
        linear_tag, SingleOrMulti, CSTag, dimension> delegate_dist;
public:
    using base::base;
    using base::param;
    uniform_point_distribution(DomainGeometry const& g) :
        base(g),
        d(segment_view<DomainGeometry>(g)) {}
    void param(typename base::param_type const& p) {
        d = delegate_dist(segment_view<DomainGeometry>(p.domain()));
    }
    template<typename Generator>
        Point operator()(Generator& gen, typename base::param_type const& p)
        {
            delegate_dist d(segment_view<DomainGeometry>(p.domain()));
            return d(gen);
        }
        template<typename Generator>
        Point operator()(Generator& gen)
        {
            return d(gen);
        }
    private:
        delegate_dist d;
    };

}}}} // namespace boost::geometry::random::detail

#endif // BOOST_GEOMETRY_EXTENSIONS_RANDOM_DETAIL_UNIFORM_POINT_DISTRIBUTION_HPP
