// Boost.Geometry
// Copyright (c) 2026 Tinko Bartels.
// Distributed under the Boost Software License, Version 1.0.

#ifndef BOOST_GEOMETRY_VIEWS_DETAIL_ITERATOR_POINT_VIEW_HPP
#define BOOST_GEOMETRY_VIEWS_DETAIL_ITERATOR_POINT_VIEW_HPP

#include <cstddef>
#include <iterator>
#include <type_traits>
#include <utility>

#include <boost/geometry/core/access.hpp>
#include <boost/geometry/core/coordinate_dimension.hpp>
#include <boost/geometry/core/coordinate_system.hpp>
#include <boost/geometry/core/coordinate_type.hpp>
#include <boost/geometry/core/tag.hpp>
#include <boost/geometry/core/tags.hpp>

namespace boost { namespace geometry { namespace detail
{

// The source range and any iterator-owning views must outlive this accessor.
template <typename Iterator>
class iterator_point_view
{
public:
    using point_type = typename std::iterator_traits<Iterator>::value_type;
    using source_reference = decltype(*std::declval<Iterator&>());
    using point_reference = std::conditional_t<
        std::is_same<geometry::tag_t<source_reference>, point_tag>::value,
        source_reference, point_type>;
    using reference = std::conditional_t<std::is_lvalue_reference<point_reference>::value,
        std::add_lvalue_reference_t<std::add_const_t<std::remove_reference_t<point_reference>>>,
        point_reference>;

    iterator_point_view() = default;
    explicit iterator_point_view(Iterator const& iterator) : m_iterator(iterator) {}

    Iterator const& base() const { return m_iterator; }
    reference read() const { return *m_iterator; }

private:
    mutable Iterator m_iterator{};
};

}}} // namespace boost::geometry::detail

namespace boost { namespace geometry { namespace traits
{

template <typename Iterator>
struct tag<geometry::detail::iterator_point_view<Iterator>>
{
    using type = point_tag;
};

template <typename Iterator>
struct coordinate_type<geometry::detail::iterator_point_view<Iterator>>
    : geometry::coordinate_type<typename std::iterator_traits<Iterator>::value_type>
{};

template <typename Iterator>
struct coordinate_system<geometry::detail::iterator_point_view<Iterator>>
    : geometry::coordinate_system<typename std::iterator_traits<Iterator>::value_type>
{};

template <typename Iterator>
struct dimension<geometry::detail::iterator_point_view<Iterator>>
    : geometry::dimension<typename std::iterator_traits<Iterator>::value_type>
{};

template <typename Iterator, std::size_t Dimension>
struct access<geometry::detail::iterator_point_view<Iterator>, Dimension>
{
    using point = geometry::detail::iterator_point_view<Iterator>;

    static geometry::coordinate_type_t<point> get(point const& view)
    {
        return geometry::get<Dimension>(view.read());
    }
};

}}} // namespace boost::geometry::traits

#endif
