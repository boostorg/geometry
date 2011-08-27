// Boost.Geometry (aka GGL, Generic Geometry Library)
//
// Boost.SpatialIndex - value to bounding object translation as a getter (object's member function)
//
// Copyright 2011 Adam Wulkiewicz.
// Use, modification and distribution is subject to the Boost Software License,
// Version 1.0. (See accompanying file LICENSE_1_0.txt or copy at
// http://www.boost.org/LICENSE_1_0.txt)

#ifndef BOOST_GEOMETRY_EXTENSIONS_INDEX_TRANSLATOR_GETTER_HPP
#define BOOST_GEOMETRY_EXTENSIONS_INDEX_TRANSLATOR_GETTER_HPP

namespace boost { namespace geometry { namespace index { namespace translator {

template <typename Value, typename Indexable, Indexable const& (Value::*Getter)() const>
struct getter
{
    typedef Indexable indexable_type;

    indexable_type const& operator()(Value const& v) const
    {
        return (v.*Getter)();
    }

    bool equals(Value const& v1, Value const& v2) const
    {
        //return geometry::equals(operator()(v1), operator()(v2));
        return v1 == v2;
    }
};

// TODO: awulkiew - implement specialization for Value* ?

}}}} // namespace boost::geometry::index::translator

#endif // BOOST_GEOMETRY_EXTENSIONS_INDEX_TRANSLATOR_GETTER_HPP
