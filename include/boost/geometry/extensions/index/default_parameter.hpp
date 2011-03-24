// Boost.Geometry (aka GGL, Generic Geometry Library)
//
// Boost.SpatialIndex - default indexes parameters
//
// Copyright 2011 Adam Wulkiewicz.
// Use, modification and distribution is subject to the Boost Software License,
// Version 1.0. (See accompanying file LICENSE_1_0.txt or copy at
// http://www.boost.org/LICENSE_1_0.txt)

#ifndef BOOST_GEOMETRY_EXTENSIONS_INDEX_DEFAULT_PARAMETER_HPP
#define BOOST_GEOMETRY_EXTENSIONS_INDEX_DEFAULT_PARAMETER_HPP

#include <boost/geometry/geometries/point.hpp>
#include <boost/geometry/geometries/box.hpp>
#include <boost/geometry/extensions/index/translator/def.hpp>

namespace boost { namespace geometry { namespace index {

// TODO: awulkiew - move this elsewhere
struct default_parameter {};

namespace detail {

template <typename Indexable>
struct geometry_box_type
{
    typedef geometry::model::box<
        geometry::model::point<
            typename index::traits::coordinate_type<Indexable>::type,
            index::traits::dimension<Indexable>::value,
            typename index::traits::coordinate_system<Indexable>::type
        >
    > type;
};

template <typename Value, typename Translator>
struct default_translator_type
{
    typedef Translator type;
};
template <typename Value>
struct default_translator_type<Value, default_parameter>
{
    typedef translator::def<Value> type;
};

template <typename Indexable, typename Box>
struct default_box_type
{
    typedef Box type;
};
template <typename Indexable>
struct default_box_type<Indexable, default_parameter>
{
    typedef typename detail::geometry_box_type<Indexable>::type type;
};

} // namespace detail

}}} // namespace boost::geometry::index

#endif // BOOST_GEOMETRY_EXTENSIONS_INDEX_DEFAULT_PARAMETER_HPP
