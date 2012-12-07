// Boost.Geometry Index
//
// Insert iterator
//
// Copyright (c) 2011-2012 Adam Wulkiewicz, Lodz, Poland.
//
// Use, modification and distribution is subject to the Boost Software License,
// Version 1.0. (See accompanying file LICENSE_1_0.txt or copy at
// http://www.boost.org/LICENSE_1_0.txt)

#ifndef BOOST_GEOMETRY_EXTENSIONS_INDEX_INSERTER_HPP
#define BOOST_GEOMETRY_EXTENSIONS_INDEX_INSERTER_HPP

#include <iterator>

namespace boost { namespace geometry { namespace index {

template <class Container>
class insert_iterator :
    public std::iterator<std::output_iterator_tag, void, void, void, void>
{
public:
    typedef Container container_type;

    inline explicit insert_iterator(Container & c)
        : container(&c)
    {}
    
    insert_iterator & operator=(typename Container::value_type const& value)
    {
        container->insert(value);
        return *this;
    }

    insert_iterator & operator* ()
    {
        return *this;
    }

    insert_iterator & operator++ ()
    {
        return *this;
    }

    insert_iterator operator++(int)
    {
        return *this;
    }

private:
    Container * container;
};

template <typename Container>
insert_iterator<Container> inserter(Container & c)
{
    return insert_iterator<Container>(c);
}

}}} // namespace boost::geometry::index

#endif // BOOST_GEOMETRY_EXTENSIONS_INDEX_INSERTER_HPP
