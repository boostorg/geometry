// Boost.Geometry Index
//
// Copyright (c) 2011-2012 Adam Wulkiewicz, Lodz, Poland.
//
// Use, modification and distribution is subject to the Boost Software License,
// Version 1.0. (See accompanying file LICENSE_1_0.txt or copy at
// http://www.boost.org/LICENSE_1_0.txt)

#include <stdexcept>

#include <boost/aligned_storage.hpp>
#include <boost/iterator/reverse_iterator.hpp>

#include <boost/mpl/assert.hpp>
#include <boost/mpl/if.hpp>
#include <boost/mpl/and.hpp>
#include <boost/mpl/or.hpp>

#include <boost/type_traits/is_same.hpp>
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
    explicit static_vector(size_type count)
        : m_size(0)
    {
        resize(count);                                                              // may throw
    }

    // strong
    static_vector(size_type count, value_type const& value)
        : m_size(0)
    {
        resize(count, value);                                                       // may throw
    }

    // strong
    static_vector(static_vector const& other)
        : m_size(other.m_size)
    {
        //BOOST_ASSERT_MSG(other.m_size <= Capacity, "size can't exceed the capacity");
        //if ( Capacity <= other.m_size ) throw std::bad_alloc();

        this->uninitialized_copy(other.begin(), other.end(), this->begin());        // may throw
    }

    // strong
    template <typename Iterator>
    static_vector(Iterator first, Iterator last)
        : m_size(0)
    {
        assign(first, last);                                                        // may throw
    }

    // basic
    static_vector & operator=(static_vector const& other)
    {
        assign(other.begin(), other.end());                                         // may throw

        return *this;
    }

    // nothrow
    ~static_vector()
    {
        this->destroy(this->begin(), this->end());
    }

    // strong
    void resize(size_type count)
    {
        if ( count < m_size )
        {
            this->destroy(this->begin() + count, this->end());
        }
        else
        {
            BOOST_ASSERT_MSG(count <= Capacity, "size can't exceed the capacity");
            //if ( Capacity <= count ) throw std::bad_alloc();
            this->construct(this->end(), this->begin() + count);                    // may throw
        }
        m_size = count; // update end
    }

    // strong
    void resize(size_type count, value_type const& value)
    {
        if ( count < m_size )
        {
            this->destroy(this->begin() + count, this->end());
        }
        else
        {
            BOOST_ASSERT_MSG(count <= Capacity, "size can't exceed the capacity");
            //if ( Capacity <= count ) throw std::bad_alloc();
            std::uninitialized_fill(this->end(), this->begin() + count, value);     // may throw
        }
        m_size = count; // update end
    }

    // nothrow
    void reserve(size_type BOOST_GEOMETRY_INDEX_ASSERT_UNUSED_PARAM(count))
    {
        BOOST_ASSERT_MSG(count <= Capacity, "size can't exceed the capacity");
        //if ( Capacity <= count ) throw std::bad_alloc();
    }

    // strong
    void push_back(value_type const& value)
    {
        BOOST_ASSERT_MSG(m_size < Capacity, "size can't exceed the capacity");
        //if ( Capacity <= m_size ) throw std::bad_alloc();
        this->uninitialized_fill(this->end(), value);                               // may throw
        ++m_size; // update end
    }

    // nothrow
    void pop_back()
    {
        BOOST_ASSERT_MSG(0 < m_size, "the container is empty");
        //--m_size; // update end
        //this->destroy(this->end());
        // safer and more intuitive version
        this->destroy(this->end() - 1);
        --m_size; // update end
    }

    void erase(iterator position)
    {
        // TODO change name of this macro
        BOOST_GEOMETRY_INDEX_ASSERT_UNUSED_PARAM(difference_type dist = std::distance(this->begin(), position));
        BOOST_ASSERT_MSG(0 <= dist && (sizeof(dist)<=sizeof(m_size)?((size_type)dist<m_size):(dist<(difference_type)m_size)), "invalid iterator");

        this->move(position + 1, this->end(), position);                            // may throw
        this->destroy(this->end() - 1);
        --m_size;
    }

    void erase(iterator first, iterator last)
    {
        // TODO change name of this macro
        BOOST_GEOMETRY_INDEX_ASSERT_UNUSED_PARAM(difference_type distf = std::distance(this->begin(), first));
        BOOST_GEOMETRY_INDEX_ASSERT_UNUSED_PARAM(difference_type distl = std::distance(this->begin(), last));
        BOOST_ASSERT_MSG(0 <= distf && (sizeof(distf)<=sizeof(m_size)?((size_type)distf<m_size):(distf<(difference_type)m_size)), "invalid iterator");
        BOOST_ASSERT_MSG(0 <= distl && (sizeof(distl)<=sizeof(m_size)?((size_type)distl<m_size):(distl<(difference_type)m_size)), "invalid iterator");

        difference_type n = std::distance(first, last);
        BOOST_ASSERT_MSG(0 <= n, "invalid iterator");

        this->move(last, this->end(), first);                                       // may throw
        this->destroy(this->end() - n, this->end());
        m_size -= n;
    }

    // basic
    template <typename Iterator>
    void assign(Iterator first, Iterator last)
    {
        typedef typename boost::iterator_traversal<Iterator>::type traversal;
        assign_dispatch(first, last, traversal());                                  // may throw
    }

    // basic
    void assign(size_type count, value_type const& value)
    {
        if ( count < m_size )
        {
            std::fill_n(this->begin(), count, value);
            this->destroy(this->begin() + count, this->end());
        }
        else
        {
            std::fill_n(this->begin(), m_size, value);
            BOOST_ASSERT_MSG(count <= Capacity, "size can't exceed the capacity");
            //if ( Capacity <= count ) throw std::bad_alloc();
            std::uninitialized_fill(this->end(), this->begin() + count, value);     // may throw
        }
        m_size = count; // update end
    }

    // nothrow
    void clear()
    {
        this->destroy(this->begin(), this->end());
        m_size = 0; // update end
    }

    // strong
    Value & at(size_type i)
    {
        if ( m_size <= i )
            throw std::out_of_range("static_vector element index out of bounds");
        return *(this->begin() + i);
    }

    // strong
    Value const& at(size_type i) const
    {
        if ( m_size <= i )
            throw std::out_of_range("static_vector element index out of bounds");
        return *(this->begin() + i);
    }

    // nothrow
    Value & operator[](size_type i)
    {
        BOOST_ASSERT_MSG(i < m_size, "index out of bounds");
        return *(this->begin() + i);
    }

    // nothrow
    Value const& operator[](size_type i) const
    {
        BOOST_ASSERT_MSG(i < m_size, "index out of bounds");
        return *(this->begin() + i);
    }

    // nothrow
    Value & front()
    {
        BOOST_ASSERT_MSG(0 < m_size, "the container is empty");
        return *(this->begin());
    }

    // nothrow
    Value const& front() const
    {
        BOOST_ASSERT_MSG(0 < m_size, "the container is empty");
        return *(this->begin());
    }

    // nothrow
    Value & back()
    {
        BOOST_ASSERT_MSG(0 < m_size, "the container is empty");
        return *(this->end() - 1);
    }

    // nothrow
    Value const& back() const
    {
        BOOST_ASSERT_MSG(0 < m_size, "the container is empty");
        return *(this->end() - 1);
    }

    // nothrow
    Value * data() { return this->ptr(); }
    const Value * data() const { return this->ptr(); }

    // nothrow
    iterator begin() { return this->ptr(); }
    const_iterator begin() const { return this->ptr(); }
    const_iterator cbegin() const { return this->ptr(); }
    iterator end() { return this->begin() + m_size; }
    const_iterator end() const { return this->begin() + m_size; }
    const_iterator cend() const { return this->cbegin() + m_size; }
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

    // assign

    template <typename Iterator>
    void assign_dispatch(Iterator first, Iterator last, boost::random_access_traversal_tag const&)
    {
        size_type s = std::distance(first, last);

        BOOST_ASSERT_MSG(s <= Capacity, "size can't exceed the capacity");
        //if ( Capacity <= m_size ) throw std::bad_alloc();

        if ( m_size <= s )
        {
            this->copy(first, first + m_size, this->begin());                        // may throw
            this->uninitialized_copy(first + m_size, last, this->end());             // may throw
        }
        else
        {
            this->copy(first, last, this->begin());                                  // may throw
            this->destroy(this->begin() + s, this->end());
        }
        m_size = s; // update end
    }

    template <typename Iterator, typename Traversal>
    void assign_dispatch(Iterator first, Iterator last, Traversal const& /*not_random_access*/)
    {
        size_t s = 0;
        iterator it = this->begin();

        for ( ; it != this->end() && first != last ; ++it, ++first, ++s )
            *it = *first;                                                           // may throw

        this->destroy(it, this->end());

        try
        {
            for ( ; first != last ; ++it, ++first, ++s )
            {
                BOOST_ASSERT_MSG(s < Capacity, "size can't exceed the capacity");
                this->uninitialized_fill(it, *first);                               // may throw
            }
            m_size = s; // update end
        }
        catch(...)
        {
            this->destroy(this->begin() + m_size, it);
            throw;
        }
    }

    // copy

    template <typename Iterator>
    void copy(Iterator first, Iterator last, iterator dst)
    {
        typedef typename
            mpl::and_<
                has_trivial_assign<value_type>,
                mpl::or_<
                    is_same<Iterator, value_type *>,
                    is_same<Iterator, const value_type *>
                >
            >::type
        use_memcpy;
        
        this->copy_dispatch(first, last, dst, use_memcpy());                        // may throw
    }

    void copy_dispatch(const value_type * first, const value_type * last, value_type * dst,
                       boost::mpl::bool_<true> const& /*use_memcpy*/)
    {
        ::memcpy(dst, first, sizeof(value_type) * std::distance(first, last));
    }

    template <typename Iterator>
    void copy_dispatch(Iterator first, Iterator last, value_type * dst,
                       boost::mpl::bool_<false> const& /*use_memcpy*/)
    {
        std::copy(first, last, dst);                                                // may throw
    }

    // move

    void move(iterator first, iterator last, iterator dst)
    {
        this->move_dispatch(first, last, dst, has_trivial_assign<value_type>());    // may throw
    }

    void move_dispatch(value_type * first, value_type * last, value_type * dst,
                       boost::true_type const& /*has_trivial_assign*/)
    {
        ::memmove(dst, first, sizeof(value_type) * std::distance(first, last));
    }

    void move_dispatch(value_type * first, value_type * last, value_type * dst,
                       boost::false_type const& /*has_trivial_assign*/)
    {
        std::copy(first, last, dst);                                                // may throw
    }

    // uninitialized_copy

    template <typename Iterator>
    void uninitialized_copy(Iterator first, Iterator last, iterator dst)
    {
        typedef typename
            mpl::and_<
                has_trivial_copy<value_type>,
                mpl::or_<
                    is_same<Iterator, value_type *>,
                    is_same<Iterator, const value_type *>
                >
            >::type
        use_memcpy;

        this->uninitialized_copy_dispatch(first, last, dst, use_memcpy());          // may throw
    }

    void uninitialized_copy_dispatch(const value_type * first, const value_type * last, value_type * dst,
                                     boost::mpl::bool_<true> const& /*use_memcpy*/)
    {
        ::memcpy(dst, first, sizeof(value_type) * std::distance(first, last));
    }

    template <typename Iterator>
    void uninitialized_copy_dispatch(Iterator first, Iterator last, value_type * dst,
                                     boost::mpl::bool_<false> const& /*use_memcpy*/)
    {
        std::uninitialized_copy(first, last, dst);                                  // may throw
    }

    // uninitialized_fill

    template <typename V>
    void uninitialized_fill(iterator dst, V const& v)
    {
        typedef typename
            mpl::and_<
                has_trivial_copy<value_type>,
                is_same<Value, value_type>
            >::type
        use_memcpy;

        uninitialized_fill_dispatch(dst, v, use_memcpy());                         // may throw
    }

    void uninitialized_fill_dispatch(value_type * ptr, value_type const& v,
                                     boost::mpl::bool_<true> const& /*use_memcpy*/)
    {
        ::memcpy(ptr, &v, sizeof(value_type));
    }

    template <typename V>
    void uninitialized_fill_dispatch(value_type * ptr, V const& v,
                                     boost::mpl::bool_<false> const& /*use_memcpy*/)
    {
        new (ptr) value_type(v);                                                    // may throw
    }

    // destroy

    void destroy(iterator first, iterator last)
    {
        this->destroy_dispatch(first, last, has_trivial_destructor<value_type>());
    }

    void destroy_dispatch(value_type * /*first*/, value_type * /*last*/,
                          boost::true_type const& /*has_trivial_destructor*/)
    {}

    void destroy_dispatch(value_type * first, value_type * last,
                          boost::false_type const& /*has_trivial_destructor*/)
    {
        for ( ; first != last ; ++first )
            first->~value_type();
    }

    // destroy

    void destroy(iterator it)
    {
        this->destroy_dispatch(it, has_trivial_destructor<value_type>());
    }

    void destroy_dispatch(value_type * /*ptr*/,
                          boost::true_type const& /*has_trivial_destructor*/)
    {}

    void destroy_dispatch(value_type * ptr,
                          boost::false_type const& /*has_trivial_destructor*/)
    {
        ptr->~value_type();
    }

    // construct

    void construct(iterator first, iterator last)
    {
        this->construct_dispatch(first, last, has_trivial_constructor<value_type>());   // may throw
    }

    void construct_dispatch(value_type * /*first*/, value_type * /*last*/,
                            boost::true_type const& /*has_trivial_constructor*/)
    {}

    void construct_dispatch(value_type * first, value_type * last,
                            boost::false_type const& /*has_trivial_constructor*/)
    {
        value_type * it = first;
        try
        {
            for ( ; it != last ; ++it )
                new (it) value_type();                                                  // may throw
        }
        catch(...)
        {
            this->destroy(first, it);
            throw;
        }
    }

    Value * ptr()
    {
        return (reinterpret_cast<Value*>(m_storage.address()));
    }

    const Value * ptr() const
    {
        return (reinterpret_cast<const Value*>(m_storage.address()));
    }

    boost::aligned_storage<sizeof(Value[Capacity]), boost::alignment_of<Value[Capacity]>::value> m_storage;
    size_type m_size;
};

}}} // namespace boost::geometry::index

#endif // BOOST_GEOMETRY_EXTENSIONS_INDEX_STATIC_VECTOR_HPP
