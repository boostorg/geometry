// Boost.Geometry Index
//
// Throwing objects implementation
//
// Copyright (c) 2011-2012 Adam Wulkiewicz, Lodz, Poland.
//
// Use, modification and distribution is subject to the Boost Software License,
// Version 1.0. (See accompanying file LICENSE_1_0.txt or copy at
// http://www.boost.org/LICENSE_1_0.txt)

#ifndef BOOST_GEOMETRY_EXTENSIONS_INDEX_TEST_THROWING_HPP
#define BOOST_GEOMETRY_EXTENSIONS_INDEX_TEST_THROWING_HPP

// value

struct throwing_value_copy_exception : public std::exception
{
    const char * what() const throw() { return "value copy failed."; }
};

struct throwing_value
{
    explicit throwing_value(int v = 0)
        : value(v)
    {}

    bool operator==(throwing_value const& v) const
    {
        return value == v.value;
    }

    throwing_value(throwing_value const& v)
    {
        throw_if_required();

        value = v.value;
    }

    throwing_value & operator=(throwing_value const& v)
    {
        throw_if_required();

        value = v.value;
        return *this;
    }

    void throw_if_required()
    {
        // throw if counter meets max count
        if ( get_max_calls_ref() <= get_calls_counter_ref() )
            throw throwing_value_copy_exception();
        else
            ++get_calls_counter_ref();
    }

    static void reset_calls_counter() { get_calls_counter_ref() = 0; }
    static void set_max_calls(size_t mc) { get_max_calls_ref() = mc; }

    static size_t & get_calls_counter_ref() { static size_t cc = 0; return cc; }
    static size_t & get_max_calls_ref() { static size_t mc = (std::numeric_limits<size_t>::max)(); return mc; }

    int value;
};

template <typename T, typename C>
struct generate_value< std::pair<bg::model::point<T, 2, C>, throwing_value> >
{
    typedef bg::model::point<T, 2, C> P;
    typedef std::pair<P, throwing_value> R;
    static R apply(int x, int y)
    {
        return std::make_pair(P(x, y), throwing_value(x + y * 100));
    }
};

// box
//
//#include <boost/geometry/geometries/register/box.hpp>
//
//struct throwing_box_copy_exception : public std::exception
//{
//    const char * what() const throw() { return "box copy failed."; }
//};
//
//template <typename Point>
//struct throwing_box
//{
//    throwing_box(){}
//
//    throwing_box(Point const& m, Point const& mm)
//        : min_p(m), max_p(mm)
//    {}
//
//    throwing_box(throwing_box const& o)
//    {
//        throw_if_required();
//
//        min_p = o.min_p;
//        max_p = o.max_p;
//    }
//
//    throwing_box & operator=(throwing_box const& o)
//    {
//        throw_if_required();
//
//        min_p = o.min_p;
//        max_p = o.max_p;
//        return *this;
//    }
//
//    void throw_if_required()
//    {
//        // throw if counter meets max count
//        if ( get_max_calls_ref() <= get_calls_counter_ref() )
//            throw throwing_box_copy_exception();
//        else
//            ++get_calls_counter_ref();
//    }
//
//    static void reset_calls_counter() { get_calls_counter_ref() = 0; }
//    static void set_max_calls(size_t mc) { get_max_calls_ref() = mc; }
//
//    static size_t & get_calls_counter_ref() { static size_t cc = 0; return cc; }
//    static size_t & get_max_calls_ref() { static size_t mc = (std::numeric_limits<size_t>::max)(); return mc; }
//
//    Point min_p;
//    Point max_p;
//};
//
//BOOST_GEOMETRY_REGISTER_BOX_TEMPLATED(throwing_box, min_p, max_p)
//
//namespace boost { namespace geometry { namespace index {
//
//template <typename CT, size_t D, typename CS>
//struct default_box_type< bg::model::point<CT, D, CS> >
//{
//    typedef throwing_box<
//        bg::model::point<CT, D, CS>
//    > type;
//};
//
//}}} // namespace boost::geometry::index

#include <boost/geometry/extensions/index/pushable_array.hpp>

struct throwing_pushable_array_exception : public std::exception
{
    const char * what() const throw() { return "pushable array exception."; }
};

struct throwing_pushable_array_settings
{
    static void throw_if_required()
    {
        // throw if counter meets max count
        if ( get_max_calls_ref() <= get_calls_counter_ref() )
            throw throwing_pushable_array_exception();
        else
            ++get_calls_counter_ref();
    }

    static void reset_calls_counter() { get_calls_counter_ref() = 0; }
    static void set_max_calls(size_t mc) { get_max_calls_ref() = mc; }

    static size_t & get_calls_counter_ref() { static size_t cc = 0; return cc; }
    static size_t & get_max_calls_ref() { static size_t mc = (std::numeric_limits<size_t>::max)(); return mc; }
};

template <typename Element, size_t Capacity>
class throwing_pushable_array
{
    typedef typename boost::array<Element, Capacity> array_type;

public:
    typedef typename array_type::value_type value_type;
    typedef typename array_type::size_type size_type;
    typedef typename array_type::iterator iterator;
    typedef typename array_type::const_iterator const_iterator;
    typedef typename array_type::reverse_iterator reverse_iterator;
    typedef typename array_type::const_reverse_iterator const_reverse_iterator;
    typedef typename array_type::reference reference;
    typedef typename array_type::const_reference const_reference;

    inline throwing_pushable_array()
        : m_size(0)
    {}

    //inline explicit throwing_pushable_array(size_type s)
    //    : m_size(s)
    //{
    //    BOOST_GEOMETRY_INDEX_ASSERT(s <= Capacity, "size too big");
    //}

    inline void resize(size_type s)
    {
        BOOST_GEOMETRY_INDEX_ASSERT(s <= Capacity, "size too big");
        throwing_pushable_array_settings::throw_if_required();
        m_size = s;
    }

    inline void reserve(size_type /*s*/)
    {
        //BOOST_GEOMETRY_INDEX_ASSERT(s <= Capacity, "size too big");
        // do nothing
    }

    inline Element & operator[](size_type i)
    {
        BOOST_GEOMETRY_INDEX_ASSERT(i < m_size, "index of the element outside the container");
        return m_array[i];
    }

    inline Element const& operator[](size_type i) const
    {
        BOOST_GEOMETRY_INDEX_ASSERT(i < m_size, "index of the element outside the container");
        return m_array[i];
    }

    inline Element const& front() const
    {
        BOOST_GEOMETRY_INDEX_ASSERT(0 < m_size, "there are no elements in the container");
        return m_array.front();
    }

    inline Element & front()
    {
        BOOST_GEOMETRY_INDEX_ASSERT(0 < m_size, "there are no elements in the container");
        return m_array.front();
    }

    inline Element const& back() const
    {
        BOOST_GEOMETRY_INDEX_ASSERT(0 < m_size, "there are no elements in the container");
        return *(begin() + (m_size - 1));
    }

    inline Element & back()
    {
        BOOST_GEOMETRY_INDEX_ASSERT(0 < m_size, "there are no elements in the container");
        return *(begin() + (m_size - 1));
    }

    inline iterator begin()
    {
        return m_array.begin();
    }

    inline iterator end()
    {
        return m_array.begin() + m_size;
    }

    inline const_iterator begin() const
    {
        return m_array.begin();
    }

    inline const_iterator end() const
    {
        return m_array.begin() + m_size;
    }

    inline reverse_iterator rbegin()
    {
        return reverse_iterator(end());
    }

    inline reverse_iterator rend()
    {
        return reverse_iterator(begin());
    }

    inline const_reverse_iterator rbegin() const
    {
        return const_reverse_iterator(end());
    }

    inline const_reverse_iterator rend() const
    {
        return const_reverse_iterator(begin());
    }

    inline void clear()
    {
        m_size = 0;
    }

    void push_back(Element const& v)
    {
        BOOST_GEOMETRY_INDEX_ASSERT(m_size < Capacity, "can't further increase the size of the container");
        throwing_pushable_array_settings::throw_if_required();
        m_array[m_size] = v;
        ++m_size;
    }

    inline void pop_back()
    {
        BOOST_GEOMETRY_INDEX_ASSERT(0 < m_size, "there are no elements in the container");
        --m_size;
    }

    inline bool empty() const
    {
        return m_size == 0;
    }

    inline size_t size() const
    {
        return m_size;
    }

    inline size_t capacity() const
    {
        return Capacity;
    }

private:
    boost::array<Element, Capacity> m_array;
    size_type m_size;
};

#include <boost/geometry/extensions/index/static_vector.hpp>

struct throwing_static_vector_exception : public std::exception
{
    const char * what() const throw() { return "static vector exception."; }
};

struct throwing_static_vector_settings
{
    static void throw_if_required()
    {
        // throw if counter meets max count
        if ( get_max_calls_ref() <= get_calls_counter_ref() )
            throw throwing_pushable_array_exception();
        else
            ++get_calls_counter_ref();
    }

    static void reset_calls_counter() { get_calls_counter_ref() = 0; }
    static void set_max_calls(size_t mc) { get_max_calls_ref() = mc; }

    static size_t & get_calls_counter_ref() { static size_t cc = 0; return cc; }
    static size_t & get_max_calls_ref() { static size_t mc = (std::numeric_limits<size_t>::max)(); return mc; }
};

template <typename Element, size_t Capacity>
class throwing_static_vector
    : public boost::geometry::index::static_vector<Element, Capacity>
{
    typedef boost::geometry::index::static_vector<Element, Capacity> container;

public:
    typedef typename container::value_type value_type;
    typedef typename container::size_type size_type;
    typedef typename container::iterator iterator;
    typedef typename container::const_iterator const_iterator;
    typedef typename container::reverse_iterator reverse_iterator;
    typedef typename container::const_reverse_iterator const_reverse_iterator;
    typedef typename container::reference reference;
    typedef typename container::const_reference const_reference;

    inline void resize(size_type s)
    {
        throwing_pushable_array_settings::throw_if_required();
        container::resize(s);
    }

    void push_back(Element const& v)
    {
        throwing_pushable_array_settings::throw_if_required();
        container::push_back(v);
    }
};

#endif // BOOST_GEOMETRY_EXTENSIONS_INDEX_TEST_THROWING_HPP
