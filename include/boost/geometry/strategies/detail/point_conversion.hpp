// Boost.Geometry
// Copyright (c) 2026 Tinko Bartels.
// Licensed under the Boost Software License, Version 1.0.
// http://www.boost.org/LICENSE_1_0.txt

#ifndef BOOST_GEOMETRY_STRATEGIES_DETAIL_POINT_CONVERSION_HPP
#define BOOST_GEOMETRY_STRATEGIES_DETAIL_POINT_CONVERSION_HPP

#include <iterator>
#include <type_traits>
#include <boost/geometry/algorithms/convert.hpp>

namespace boost { namespace geometry { namespace strategies { namespace detail
{

namespace point_conversion_detail
{

template <typename Strategy, typename Source, typename Destination>
inline auto apply(Strategy const& strategy, Source const& source,
                  Destination& destination, int)
    -> decltype(strategy.convert_geometry(source, destination), void())
{
    strategy.convert_geometry(source, destination);
}

template <typename Strategy, typename Source, typename Destination>
inline void apply(Strategy const&, Source const& source,
                  Destination& destination, long)
{
    geometry::convert(source, destination);
}

} // namespace point_conversion_detail

template <typename Strategy, typename Source, typename Destination>
inline void convert_geometry(Strategy const& strategy, Source const& source,
                             Destination& destination)
{
    point_conversion_detail::apply(strategy, source, destination, 0);
}

namespace point_conversion_detail
{

template <typename Strategy, typename Iterator, typename Destination>
inline auto assign_iterator(Strategy const& strategy, Iterator const& iterator,
                            Destination& destination, int)
    -> decltype(strategy.section().assign_point(iterator, destination), void())
{
    strategy.section().assign_point(iterator, destination);
}

template <typename Strategy, typename Iterator, typename Destination>
inline void assign_iterator(Strategy const& strategy, Iterator iterator,
                            Destination& destination, long)
{
    strategies::detail::convert_geometry(strategy, *iterator, destination);
}

} // namespace point_conversion_detail

template <typename Strategy, typename Iterator, typename Destination>
inline void assign_point_from_iterator(Strategy const& strategy, Iterator const& iterator,
                                       Destination& destination)
{
    point_conversion_detail::assign_iterator(strategy, iterator, destination, 0);
}

template <typename Geometry, typename Iterator, typename Strategy>
class converting_output_iterator
{
public:
    using iterator_category = std::output_iterator_tag;
    using value_type = void;
    using difference_type = void;
    using pointer = void;
    using reference = void;

    converting_output_iterator(Iterator iterator, Strategy const& strategy)
        : m_iterator(iterator), m_strategy(&strategy)
    {}

    struct proxy
    {
        Iterator iterator;
        Strategy const* strategy;

        template <typename WorkingGeometry>
        proxy& operator=(WorkingGeometry const& source)
        {
            Geometry destination;
            convert_geometry(*strategy, source, destination);
            *iterator = std::move(destination);
            return *this;
        }
    };

    proxy operator*() const { return {m_iterator, m_strategy}; }
    converting_output_iterator& operator++() { ++m_iterator; return *this; }
    converting_output_iterator operator++(int)
    {
        auto previous = *this;
        ++*this;
        return previous;
    }
    Iterator base() const { return m_iterator; }

private:
    Iterator m_iterator;
    Strategy const* m_strategy;
};

template <typename Geometry, typename WorkingGeometry, typename Iterator, typename Strategy>
inline auto make_output_iterator(Iterator iterator, Strategy const&,
    std::enable_if_t<std::is_same<Geometry, WorkingGeometry>::value>* = nullptr)
{
    return iterator;
}

template <typename Geometry, typename WorkingGeometry, typename Iterator, typename Strategy>
inline auto make_output_iterator(Iterator iterator, Strategy const& strategy,
    std::enable_if_t<!std::is_same<Geometry, WorkingGeometry>::value>* = nullptr)
{
    return converting_output_iterator<Geometry, Iterator, Strategy>(iterator, strategy);
}

template <typename Iterator>
inline Iterator unwrap_output_iterator(Iterator iterator) { return iterator; }

template <typename Geometry, typename Iterator, typename Strategy>
inline Iterator unwrap_output_iterator(converting_output_iterator<Geometry, Iterator, Strategy> iterator)
{
    return iterator.base();
}

}}}} // namespace boost::geometry::strategies::detail

#endif
