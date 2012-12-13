// Boost.Geometry Index
//
// Copyright (c) 2011-2012 Adam Wulkiewicz, Lodz, Poland.
//
// Use, modification and distribution is subject to the Boost Software License,
// Version 1.0. (See accompanying file LICENSE_1_0.txt or copy at
// http://www.boost.org/LICENSE_1_0.txt)

#include <boost/aligned_storage.hpp>
#include <boost/assert.hpp>
#include <boost/iterator/reverse_iterator.hpp>

#ifndef BOOST_GEOMETRY_EXTENSIONS_INDEX_STATIC_VECTOR_HPP
#define BOOST_GEOMETRY_EXTENSIONS_INDEX_STATIC_VECTOR_HPP

namespace boost { namespace geometry { namespace index {

template <typename Value, size_t Capacity>
class static_vector
{
public:
    typedef Value value_type;
    typedef size_t size_type;
    typedef Value& reference;
    typedef Value const& const_reference;
    typedef Value* iterator;
    typedef const Value * const_iterator;
    typedef boost::reverse_iterator<iterator> reverse_iterator;
    typedef boost::reverse_iterator<const_iterator> const_reverse_iterator;

    // nothrow
    static_vector()
        : m_size(0)
    {}

    // strong
    static_vector(size_type s)
        : m_size(0)
    {
        resize(s);                                                      // may throw
    }

    // strong
    static_vector(static_vector const& other)
        : m_size(0)
    {
        //BOOST_ASSERT_MSG(other.m_size <= Capacity, "capacity too small");

        try
        {
            for ( ; m_size < other.m_size ; ++m_size )
                this->construct(m_size, other[m_size]);                 // may throw
        }
        catch(...)
        {
            this->destroy(0, m_size);
            throw;                                                      // rethrow
        }
    }

    // basic
    static_vector & operator=(static_vector const& other)
    {
        //BOOST_ASSERT_MSG(other.m_size <= Capacity, "capacity too small");

        size_t min_size = m_size < other.m_size ? m_size : other.m_size;

        for ( size_t i = 0 ; i < min_size ; ++i )
            (*this)[i] = other[i];                                          // may throw

        for ( ; m_size < other.m_size ; ++m_size )
            this->construct(m_size, other[m_size]);                         // may throw

        for ( size_t i = other.m_size ; i < m_size ; ++i )
            this->destroy(i);

        return *this;
    }

    // nothrow
    ~static_vector()
    {
        this->destroy(0, m_size);
    }

    // nothrow if s <= size(), strong otherwise
    void resize(size_type s)
    {
        if ( s < m_size )
        {
            this->destroy(s, m_size);
            m_size = s;
        }
        else
        {
            size_type i = m_size;
            try
            {
                for ( ; i < s ; ++i )
                    this->construct(i);                                  // may throw
                m_size = s;
            }
            catch(...)
            {
                this->destroy(m_size, i);
                throw;                                                  // rethrow
            }
        }
    }

    // nothrow
    void reserve(size_type s)
    {
        BOOST_ASSERT_MSG(s <= Capacity, "max capacity reached");
    }

    // strong
    void push_back(Value const& value)
    {
        BOOST_ASSERT_MSG(m_size < Capacity, "max capacity reached");
        this->construct(m_size, value);                                 // may throw
        ++m_size;
    }

    // nothrow
    void pop_back()
    {
        BOOST_ASSERT_MSG(0 < m_size, "the container is empty");
        --m_size;
        this->destroy(m_size);
    }

    // nothrow
    void clear()
    {
        this->destroy(0, m_size);
        m_size = 0;
    }

    // nothrow
    Value & operator[](size_t i)
    {
        BOOST_ASSERT_MSG(i < Capacity, "index out of bounds");
        return *(this->ptr(i));
    }

    // nothrow
    Value const& operator[](size_t i) const
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
    iterator begin() { return this->ptr(0); }
    const_iterator begin() const { return this->ptr(0); }
    iterator end() { return this->ptr(m_size); }
    const_iterator end() const { return this->ptr(m_size); }
    // nothrow
    reverse_iterator rbegin() { return reverse_iterator(this->end()); }
    const_reverse_iterator rbegin() const { return reverse_iterator(this->end()); }
    reverse_iterator rend() { return reverse_iterator(this->begin()); }
    const_reverse_iterator rend() const { return reverse_iterator(this->begin()); }

    // nothrow
    size_type capacity() const { return Capacity; }
    size_type size() const { return m_size; }
    bool empty() const { return 0 == m_size; }

private:
    void construct(size_type i)
    {
        new (this->ptr(i)) value_type();                                          // may throw
    }

    void construct(size_type i, Value const& value)
    {
        new (this->ptr(i)) value_type(value);                                     // may throw
    }

    void destroy(size_type i)
    {
        this->ptr(i)->~value_type();
    }

    void destroy(size_type first_i, size_type last_i)
    {
        for ( size_type i = first_i ; i < last_i ; ++i )
            this->destroy(i);
    }

    Value * ptr(size_type i)
    {
        return (reinterpret_cast<Value*>(m_storage.address()) + i);
    }

    const Value * ptr(size_type i) const
    {
        return (reinterpret_cast<const Value*>(m_storage.address()) + i);
    }

    boost::aligned_storage<sizeof(Value[Capacity])> m_storage;
    size_type m_size;
};

}}} // namespace boost::geometry::index

#endif // BOOST_GEOMETRY_EXTENSIONS_INDEX_STATIC_VECTOR_HPP
