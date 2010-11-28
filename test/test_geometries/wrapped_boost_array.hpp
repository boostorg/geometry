// Boost.Geometry (aka GGL, Generic Geometry Library) test file
//
// Copyright Barend Gehrels 2010, Geodan, Amsterdam, the Netherlands
// Use, modification and distribution is subject to the Boost Software License,
// Version 1.0. (See accompanying file LICENSE_1_0.txt or copy ar
// http://www.boost.org/LICENSE_1_0.txt)


#ifndef GEOMETRY_TEST_TEST_GEOMETRIES_WRAPPED_BOOST_ARRAY_HPP
#define GEOMETRY_TEST_TEST_GEOMETRIES_WRAPPED_BOOST_ARRAY_HPP

#include <cstddef>

#include <boost/array.hpp>
#include <boost/range.hpp>

#include <boost/geometry/core/container_access.hpp>
#include <boost/geometry/core/tag.hpp>
#include <boost/geometry/core/tags.hpp>


namespace test
{

template <typename Point, std::size_t Count>
struct wrapped_boost_array
{
    inline wrapped_boost_array() : size(0) {}

    boost::array<Point, Count> array;
    int size;
};


} // namespace test


// 1a: adapt to Boost.Range
namespace boost
{
    using namespace test;

    template <typename Point, std::size_t Count>
    struct range_mutable_iterator<wrapped_boost_array<Point, Count> >
        : public range_mutable_iterator<boost::array<Point, Count> >
    {};

    template <typename Point, std::size_t Count>
    struct range_const_iterator<wrapped_boost_array<Point, Count> >
        : public range_const_iterator<boost::array<Point, Count> >
    {};


} // namespace 'boost'


// 1b) adapt to Boost.Range with ADP
namespace test
{
    template <typename Point, std::size_t Count>
    inline typename boost::range_iterator
        <
            wrapped_boost_array<Point, Count>
        >::type range_begin(wrapped_boost_array<Point, Count>& ar)
    {
        return ar.array.begin();
    }

    template <typename Point, std::size_t Count>
    inline typename boost::range_iterator
        <
            wrapped_boost_array<Point, Count> const
        >::type range_begin(wrapped_boost_array<Point, Count> const& ar)
    {
        return ar.array.begin();
    }

    template <typename Point, std::size_t Count>
    inline typename boost::range_iterator
        <
            wrapped_boost_array<Point, Count>
        >::type range_end(wrapped_boost_array<Point, Count>& ar)
    {
        typename boost::range_iterator
            <
                wrapped_boost_array<Point, Count>
            >::type it = ar.array.begin();
        return it + ar.size;
    }

    template <typename Point, std::size_t Count>
    inline typename boost::range_iterator
        <
            wrapped_boost_array<Point, Count> const
        >::type range_end(wrapped_boost_array<Point, Count> const& ar)
    {
        typename boost::range_iterator
            <
                wrapped_boost_array<Point, Count> const
            >::type it = ar.array.begin();
        return it + ar.size;
    }

}


// 2: adapt to Boost.Geometry
namespace boost { namespace geometry { namespace traits
{

    template <typename Point, std::size_t Count>
    struct tag< wrapped_boost_array<Point, Count> >
    {
        typedef linestring_tag type;
    };

    template <typename Point, std::size_t Count>
    struct clear< wrapped_boost_array<Point, Count> >
    {
        static inline void apply(wrapped_boost_array<Point, Count>& ar)
        {
            ar.size = 0;
        }
    };


}}} // namespace bg::traits



namespace std
{

template <typename Point, std::size_t Count>
class back_insert_iterator< test::wrapped_boost_array<Point, Count> >
    : public std::iterator<std::output_iterator_tag, void, void, void, void>
{
public:

    typedef test::wrapped_boost_array<Point, Count> container_type;
    typedef back_insert_iterator<container_type> this_type;

    explicit back_insert_iterator(container_type& ar)
        : m_current(boost::begin(ar) + ar.size)
        , m_array(ar)
    {}

    inline this_type& operator=(Point const& value)
    {
        // Check if not passed beyond
        if (std::size_t(m_array.size) < Count)
        {
            *m_current++ = value;
            m_array.size++;
        }
        return *this;
    }

    // Boiler-plate
    inline this_type& operator*()     { return *this; }
    inline this_type& operator++()    { return *this; }
    inline this_type& operator++(int) { return *this; }

private:
    typename boost::range_iterator<container_type>::type m_current;
    container_type& m_array;
};

} // namespace std


#endif // GEOMETRY_TEST_TEST_GEOMETRIES_WRAPPED_BOOST_ARRAY_HPP
