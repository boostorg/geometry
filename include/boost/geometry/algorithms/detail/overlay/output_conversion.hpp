// Boost.Geometry
// Distributed under the Boost Software License, Version 1.0.

#ifndef BOOST_GEOMETRY_ALGORITHMS_DETAIL_OVERLAY_OUTPUT_CONVERSION_HPP
#define BOOST_GEOMETRY_ALGORITHMS_DETAIL_OVERLAY_OUTPUT_CONVERSION_HPP

#include <iterator>
#include <type_traits>
#include <utility>
#include <boost/range/value_type.hpp>
#include <boost/geometry/core/mutable_range.hpp>
#include <boost/geometry/strategies/detail/point_conversion.hpp>
#include <boost/geometry/util/tuples.hpp>
#include <boost/geometry/util/type_traits.hpp>

namespace boost { namespace geometry { namespace detail { namespace overlay
{

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
            strategies::detail::convert_geometry(*strategy, source, destination);
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
    Iterator& base_reference() { return m_iterator; }

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

template <typename Iterator>
inline Iterator& output_iterator_base(Iterator& iterator) { return iterator; }

template <typename Geometry, typename Iterator, typename Strategy>
inline Iterator& output_iterator_base(converting_output_iterator<Geometry, Iterator, Strategy>& iterator)
{
    return iterator.base_reference();
}

template <typename Geometry, typename Iterator, typename Strategy>
inline Iterator unwrap_output_iterator(converting_output_iterator<Geometry, Iterator, Strategy> iterator)
{
    return iterator.base();
}

template <typename Src, typename Dst, typename Convert>
void append_converted_output(Src const& src, Dst& dst, Convert const& convert);

template <typename Src, typename Dst, typename Convert, std::size_t... I>
void append_converted_output(Src const& src, Dst& dst, Convert const& convert, std::index_sequence<I...>)
{ (append_converted_output(tuples::get<I>(src), tuples::get<I>(dst), convert), ...); }

template <typename Src, typename Dst, typename Convert>
void append_converted_output(Src const& src, Dst& dst, Convert const& convert)
{
    if constexpr (tuples::is_tuple<Dst>::value)
        append_converted_output(src, dst, convert, std::make_index_sequence<tuples::size<Dst>::value>());
    else if constexpr (util::is_box<Dst>::value)
        convert(src, dst, dst);
    else
        for (auto const& value : src)
        {
            typename boost::range_value<Dst>::type converted;
            convert(value, converted, dst);
            traits::push_back<Dst>::apply(dst, std::move(converted));
        }
}

}}}} // namespace boost::geometry::detail::overlay

#endif
