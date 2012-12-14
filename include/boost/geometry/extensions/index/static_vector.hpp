// Boost.Geometry Index
//
// Copyright (c) 2011-2012 Adam Wulkiewicz, Lodz, Poland.
//
// Use, modification and distribution is subject to the Boost Software License,
// Version 1.0. (See accompanying file LICENSE_1_0.txt or copy at
// http://www.boost.org/LICENSE_1_0.txt)

#include <stdexcept>
#include <boost/mpl/assert.hpp>
#include <boost/aligned_storage.hpp>
#include <boost/iterator/reverse_iterator.hpp>

#include <boost/type_traits/alignment_of.hpp>
#include <boost/type_traits/aligned_storage.hpp>
#include <boost/type_traits/has_trivial_assign.hpp>
#include <boost/type_traits/has_trivial_copy.hpp>
#include <boost/type_traits/has_trivial_constructor.hpp>
#include <boost/type_traits/has_trivial_destructor.hpp>

#include <boost/geometry/extensions/index/assert.hpp>

#ifndef BOOST_GEOMETRY_EXTENSIONS_INDEX_STATIC_VECTOR_HPP
#define BOOST_GEOMETRY_EXTENSIONS_INDEX_STATIC_VECTOR_HPP

namespace boost { namespace geometry { namespace index {

template <typename Value, size_t Capacity>
class static_vector
{
    BOOST_MPL_ASSERT_MSG(
        (0 < Capacity),
        INVALID_CAPACITY,
        (static_vector));

public:
    typedef Value value_type;
    typedef size_t size_type;
    typedef ptrdiff_t difference_type;
    typedef Value& reference;
    typedef Value const& const_reference;
    typedef Value * pointer;
    typedef const Value* const_pointer;
    typedef Value* iterator;
    typedef const Value * const_iterator;
    typedef boost::reverse_iterator<iterator> reverse_iterator;
    typedef boost::reverse_iterator<const_iterator> const_reverse_iterator;

    // nothrow
    static_vector()
        : m_size(0)
    {}

    // strong
    explicit static_vector(size_type s)
        : m_size(0)
    {
        resize(s);                                                              // may throw
    }

    // strong
    static_vector(size_type s, value_type const& value)
        : m_size(0)
    {
        resize(s, value);                                                      // may throw
    }

    // strong
    static_vector(static_vector const& other)
    {
        //BOOST_ASSERT_MSG(other.m_size <= Capacity, "capacity too small");

        this->uninitialized_copy(other.ptr(0), other.ptr(other.m_size), this->ptr(0),
                                 boost::has_trivial_copy<value_type>());
        m_size = other.m_size;
    }

    // basic
    static_vector & operator=(static_vector const& other)
    {
        //BOOST_ASSERT_MSG(other.m_size <= Capacity, "capacity too small");

        assign(other.ptr(0), other.ptr(other.m_size));

        return *this;
    }

    // nothrow
    ~static_vector()
    {
        this->destroy(this->ptr(0), this->ptr(m_size),
                      boost::has_trivial_destructor<value_type>());
    }

    // strong
    void resize(size_type s)
    {
        if ( s < m_size )
        {
            this->destroy(this->ptr(s), this->ptr(m_size),
                          boost::has_trivial_destructor<value_type>());
            m_size = s;
        }
        else
        {
            BOOST_ASSERT_MSG(s <= Capacity, "size can't exceed the capacity");
            this->construct(this->ptr(m_size), this->ptr(s),
                            boost::has_trivial_constructor<value_type>());      // may throw
            m_size = s;
        }
    }

    // strong
    void resize(size_type s, value_type const& value)
    {
        if ( s < m_size )
        {
            this->destroy(this->ptr(s), this->ptr(m_size),
                          boost::has_trivial_destructor<value_type>());
            m_size = s;
        }
        else
        {
            BOOST_ASSERT_MSG(s <= Capacity, "size can't exceed the capacity");
            std::uninitialized_fill(this->ptr(m_size), this->ptr(s), value);    // may throw
            m_size = s;
        }
    }

    // nothrow
    void reserve(size_type BOOST_GEOMETRY_INDEX_ASSERT_UNUSED_PARAM(s))
    {
        BOOST_ASSERT_MSG(s <= Capacity, "max capacity reached");
    }

    // strong
    void push_back(Value const& value)
    {
        BOOST_ASSERT_MSG(m_size < Capacity, "max capacity reached");
        this->uninitialized_copy(this->ptr(m_size), value,
                                 boost::has_trivial_copy<value_type>());        // may throw
        ++m_size;
    }

    // nothrow
    void pop_back()
    {
        BOOST_ASSERT_MSG(0 < m_size, "the container is empty");
        --m_size;
        this->destroy(this->ptr(m_size), boost::has_trivial_destructor<value_type>());
    }

    // basic
    void assign(const value_type * first, const value_type * last)
    {
        size_type s = std::distance(first, last);

        BOOST_ASSERT_MSG(s <= Capacity, "max capacity reached");

        if ( m_size <= s )
        {
            this->copy(first, first + m_size, this->ptr(0),
                       boost::has_trivial_assign<value_type>());                    // may throw

            this->uninitialized_copy(first + m_size, last, this->ptr(m_size),
                                     boost::has_trivial_copy<value_type>());        // may throw
            m_size = s;
        }
        else
        {
            this->copy(first, last, this->ptr(0),
                       boost::has_trivial_assign<value_type>());                    // may throw

            this->destroy(this->ptr(s), this->ptr(m_size),
                          boost::has_trivial_destructor<value_type>());
            m_size = s;
        }
    }

    // nothrow
    void clear()
    {
        this->destroy(this->ptr(0), this->ptr(m_size),
                      boost::has_trivial_destructor<value_type>());
        m_size = 0;
    }

    // strong
    Value & at(size_type i)
    {
        if ( m_size <= i )
            throw std::out_of_range("static_vector element index out of bounds");
        return *(this->ptr(i));
    }

    // strong
    Value const& at(size_type i) const
    {
        if ( m_size <= i )
            throw std::out_of_range("static_vector element index out of bounds");
        return *(this->ptr(i));
    }

    // nothrow
    Value & operator[](size_type i)
    {
        BOOST_ASSERT_MSG(i < Capacity, "index out of bounds");
        return *(this->ptr(i));
    }

    // nothrow
    Value const& operator[](size_type i) const
    {
        BOOST_ASSERT_MSG(i < Capacity, "index out of bounds");
        return *(this->ptr(i));
    }

    // nothrow
    Value & front()
    {
        BOOST_ASSERT_MSG(0 < m_size, "the container is empty");
        return *(this->ptr(0));
    }

    // nothrow
    Value const& front() const
    {
        BOOST_ASSERT_MSG(0 < m_size, "the container is empty");
        return *(this->ptr(0));
    }

    // nothrow
    Value & back()
    {
        BOOST_ASSERT_MSG(0 < m_size, "the container is empty");
        return *(this->ptr(m_size - 1));
    }

    // nothrow
    Value const& back() const
    {
        BOOST_ASSERT_MSG(0 < m_size, "the container is empty");
        return *(this->ptr(m_size - 1));
    }

    // nothrow
    Value * data() { return this->ptr(0); }
    const Value * data() const { return this->ptr(0); }

    // nothrow
    iterator begin() { return this->ptr(0); }
    const_iterator begin() const { return this->ptr(0); }
    const_iterator cbegin() const { return this->ptr(0); }
    iterator end() { return this->ptr(m_size); }
    const_iterator end() const { return this->ptr(m_size); }
    const_iterator cend() const { return this->ptr(m_size); }
    // nothrow
    reverse_iterator rbegin() { return reverse_iterator(this->end()); }
    const_reverse_iterator rbegin() const { return reverse_iterator(this->end()); }
    const_reverse_iterator crbegin() const { return reverse_iterator(this->end()); }
    reverse_iterator rend() { return reverse_iterator(this->begin()); }
    const_reverse_iterator rend() const { return reverse_iterator(this->begin()); }
    const_reverse_iterator crend() const { return reverse_iterator(this->begin()); }

    // nothrow
    size_type capacity() const { return Capacity; }
    size_type max_size() const { return Capacity; }
    size_type size() const { return m_size; }
    bool empty() const { return 0 == m_size; }

private:
    void copy(const value_type * first, const value_type * last, value_type * dst,
              boost::true_type const& /*has_trivial_assign*/)
    {
        ::memcpy(dst, first, sizeof(value_type) * std::distance(first, last));
    }

    void copy(const value_type * first, const value_type * last, value_type * dst,
              boost::false_type const& /*has_trivial_assign*/)
    {
        std::copy(first, last, dst);                                            // may throw
    }

    void uninitialized_copy(const value_type * first, const value_type * last, value_type * dst,
                            boost::true_type const& /*has_trivial_copy*/)
    {
        ::memcpy(dst, first, sizeof(value_type) * std::distance(first, last));
    }

    void uninitialized_copy(const value_type * first, const value_type * last, value_type * dst,
                            boost::false_type const& /*has_trivial_copy*/)
    {
        std::uninitialized_copy(first, last, dst);                              // may throw
    }

    void uninitialized_copy(value_type * ptr, value_type const& v,
                            boost::true_type const& /*has_trivial_copy*/)
    {
        ::memcpy(ptr, &v, sizeof(value_type));
    }

    void uninitialized_copy(value_type * ptr, value_type const& v,
                            boost::false_type const& /*has_trivial_copy*/)
    {
        new (ptr) value_type(v);                                                    // may throw
    }

    void destroy(const value_type * /*first*/, const value_type * /*last*/,
                 boost::true_type const& /*has_trivial_destructor*/)
    {}

    void destroy(const value_type * first, const value_type * last,
                 boost::false_type const& /*has_trivial_destructor*/)
    {
        for ( ; first != last ; ++first )
            first->~value_type();
    }

    void destroy(const value_type * /*ptr*/,
                 boost::true_type const& /*has_trivial_destructor*/)
    {}

    void destroy(const value_type * ptr, boost::false_type const& /*has_trivial_destructor*/)
    {
        ptr->~value_type();
    }

    void construct(value_type * first, value_type * last,
                   boost::true_type const& /*has_trivial_constructor*/)
    {
        value_type * it = first;
        try
        {
            for ( ; it != last ; ++it )
                new (it) value_type();                                              // may throw
        }
        catch(...)
        {
            this->destroy(first, it, boost::has_trivial_destructor<value_type>());
            throw;
        }
    }

    void construct(value_type * /*first*/, value_type * /*last*/,
                   boost::false_type const& /*has_trivial_constructor*/)
    {}

    Value * ptr(size_type i)
    {
        return (reinterpret_cast<Value*>(m_storage.address()) + i);
    }

    const Value * ptr(size_type i) const
    {
        return (reinterpret_cast<const Value*>(m_storage.address()) + i);
    }

    boost::aligned_storage<sizeof(Value[Capacity]), boost::alignment_of<Value[Capacity]>::value> m_storage;
    size_type m_size;
};

}}} // namespace boost::geometry::index

#endif // BOOST_GEOMETRY_EXTENSIONS_INDEX_STATIC_VECTOR_HPP
