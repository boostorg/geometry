// Boost.Geometry Index
//
// Copyright (c) 2011-2013 Adam Wulkiewicz, Lodz, Poland.
//
// Use, modification and distribution is subject to the Boost Software License,
// Version 1.0. (See accompanying file LICENSE_1_0.txt or copy at
// http://www.boost.org/LICENSE_1_0.txt)

#ifndef BOOST_GEOMETRY_INDEX_DETAIL_VARRAY_HPP
#define BOOST_GEOMETRY_INDEX_DETAIL_VARRAY_HPP

#include <cstddef>
#include <stdexcept>

#include <boost/move/move.hpp>
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

#include <boost/geometry/index/detail/assert.hpp>

namespace boost { namespace geometry { namespace index { namespace detail {

template <typename V>
struct varray_default_alloc
{
    typedef std::size_t size_type;
    typedef std::ptrdiff_t difference_type;
    typedef V * pointer;
    typedef const V * const_pointer;
};

template <typename Value, size_t Capacity, typename Al = varray_default_alloc<Value> >
class varray
{
    BOOST_MPL_ASSERT_MSG(
        (0 < Capacity),
        INVALID_CAPACITY,
        (varray));

    BOOST_COPYABLE_AND_MOVABLE(varray)

public:
    typedef Value value_type;
    typedef Value& reference;
    typedef Value const& const_reference;
    typedef typename Al::size_type size_type;
    typedef typename Al::difference_type difference_type;
    typedef typename Al::pointer pointer;
    typedef typename Al::const_pointer const_pointer;
    typedef pointer iterator;
    typedef const_pointer const_iterator;
    typedef boost::reverse_iterator<iterator> reverse_iterator;
    typedef boost::reverse_iterator<const_iterator> const_reverse_iterator;

    // nothrow
    varray()
        : m_size(0)
    {}

    // strong
    explicit varray(size_type count)
        : m_size(0)
    {
        resize(count);                                                              // may throw
    }

    // strong
    varray(size_type count, value_type const& value)
        : m_size(0)
    {
        resize(count, value);                                                       // may throw
    }

    // strong
    varray(varray const& other)
        : m_size(other.m_size)
    {
        this->uninitialized_copy(other.begin(), other.end(), this->begin());        // may throw
    }

    // strong
    varray(BOOST_RV_REF(varray) other)
        : m_size(other.m_size)
    {
        this->uninitialized_move(other.begin(), other.end(), this->begin());        // may throw
    }

    // strong
    template <typename Iterator>
    varray(Iterator first, Iterator last)
        : m_size(0)
    {
        assign(first, last);                                                        // may throw
    }

    // basic
    varray & operator=(BOOST_COPY_ASSIGN_REF(varray) other)
    {
        assign(other.begin(), other.end());                                         // may throw

        return *this;
    }

    // basic
    varray & operator=(BOOST_RV_REF(varray) other)
    {
        assign(other.begin(), other.end());                                         // may throw

        return *this;
    }

    // nothrow
    ~varray()
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
            check_capacity(count);

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
            check_capacity(count);
            
            std::uninitialized_fill(this->end(), this->begin() + count, value);     // may throw
        }
        m_size = count; // update end
    }

    // nothrow
    void reserve(size_type count)
    {
        check_capacity(count);
    }

    // strong
    void push_back(value_type const& value)
    {
        check_capacity(m_size + 1);
        
        this->uninitialized_fill(this->end(), value);                               // may throw
        ++m_size; // update end
    }

    void push_back(BOOST_RV_REF(value_type) value)
    {
        check_capacity(m_size + 1);

        this->uninitialized_fill(this->end(), value);   // may throw
        ++m_size; // update end
    }

    // nothrow
    void pop_back()
    {
        check_empty();

        //--m_size; // update end
        //this->destroy(this->end());

        // safer and more intuitive version
        this->destroy(this->end() - 1);
        --m_size; // update end
    }

    // basic
    void insert(iterator position, value_type const& value)
    {
        check_iterator_end_eq(position);
        check_capacity(m_size + 1);

        if ( position == this->end() )
        {
            this->uninitialized_fill(position, value);                              // may throw
            ++m_size; // update end
        }
        else
        {
            // TODO - should following lines check for exception and revert to the old size?

            this->uninitialized_fill(this->end(), *(this->end() - 1));              // may throw
            ++m_size; // update end
            this->move_backward(position, this->end() - 2, this->end() - 1);        // may throw
            this->fill(position, value);                                            // may throw
        }
    }

    // basic
    void insert(iterator position, size_type count, value_type const& value)
    {
        check_iterator_end_eq(position);
        check_capacity(m_size + count);

        if ( position == this->end() )
        {
            std::uninitialized_fill(position, position + count, value);             // may throw
            m_size += count; // update end
        }
        else
        {
            difference_type to_move = std::distance(position, this->end());
            
            // TODO - should following lines check for exception and revert to the old size?

            if ( count < static_cast<size_type>(to_move) )
            {
                this->uninitialized_copy(this->end() - count, this->end(), this->end()); // may throw
                m_size += count; // update end
                this->move_backward(position, position + to_move - count, this->end() - count);  // may throw
                std::fill_n(position, count, value);                                // may throw
            }
            else
            {
                std::uninitialized_fill(this->end(), position + count, value);      // may throw
                m_size += count - to_move; // update end
                this->uninitialized_copy(position, position + to_move, position + count);  // may throw
                m_size += to_move; // update end
                std::fill_n(position, to_move, value);                              // may throw
            }
        }
    }

    // basic
    template <typename Iterator>
    void insert(iterator position, Iterator first, Iterator last)
    {
        typedef typename boost::iterator_traversal<Iterator>::type traversal;
        this->insert_dispatch(position, first, last, traversal());
    }

    // basic
    void erase(iterator position)
    {
        check_iterator_end_neq(position);

        this->move(position + 1, this->end(), position);                            // may throw
        this->destroy(this->end() - 1);
        --m_size;
    }

    // basic
    void erase(iterator first, iterator last)
    {
        check_iterator_end_eq(first);
        check_iterator_end_eq(last);
        
        difference_type n = std::distance(first, last);
        BOOST_ASSERT_MSG(0 <= n, "invalid range");

        this->move(last, this->end(), first);                                       // may throw
        this->destroy(this->end() - n, this->end());
        m_size -= n;
    }

    // basic
    template <typename Iterator>
    void assign(Iterator first, Iterator last)
    {
        typedef typename boost::iterator_traversal<Iterator>::type traversal;
        this->assign_dispatch(first, last, traversal());                            // may throw
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
            check_capacity(count);

            std::fill_n(this->begin(), m_size, value);
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
            throw std::out_of_range("index out of bounds");
        return *(this->begin() + i);
    }

    // strong
    Value const& at(size_type i) const
    {
        if ( m_size <= i )
            throw std::out_of_range("index out of bounds");
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
        check_empty();
        return *(this->begin());
    }

    // nothrow
    Value const& front() const
    {
        check_empty();
        return *(this->begin());
    }

    // nothrow
    Value & back()
    {
        check_empty();
        return *(this->end() - 1);
    }

    // nothrow
    Value const& back() const
    {
        check_empty();
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
    static size_type capacity() { return Capacity; }
    static size_type max_size() { return Capacity; }
    size_type size() const { return m_size; }
    bool empty() const { return 0 == m_size; }

private:

    // insert

    template <typename Iterator>
    void insert_dispatch(iterator position, Iterator first, Iterator last, boost::random_access_traversal_tag const&)
    {
        check_iterator_end_eq(position);
        
        difference_type count = std::distance(first, last);

        check_capacity(m_size + count);

        if ( position == this->end() )
        {
            this->uninitialized_copy(first, last, position);                                     // may throw
            m_size += count; // update end
        }
        else
        {
            this->insert_in_the_middle(position, first, last, count);                            // may throw
        }
    }

    template <typename Iterator, typename Traversal>
    void insert_dispatch(iterator position, Iterator first, Iterator last, Traversal const& /*not_random_access*/)
    {
        check_iterator_end_eq(position);

        if ( position == this->end() )
        {
            std::pair<bool, size_type> copy_data =
                this->uninitialized_copy_checked(first, last, position, std::distance(position, this->begin() + Capacity)); // may throw
            
            check_capacity(copy_data.first ? m_size + copy_data.second : Capacity + 1);

            m_size += copy_data.second;
        }
        else
        {
            difference_type count = std::distance(first, last);
            
            check_capacity(m_size + count);

            this->insert_in_the_middle(position, first, last, count);                             // may throw
        }
    }

    template <typename Iterator>
    void insert_in_the_middle(iterator position, Iterator first, Iterator last, difference_type count)
    {
        difference_type to_move = std::distance(position, this->end());

        // TODO - should following lines check for exception and revert to the old size?

        if ( count < to_move )
        {
            this->uninitialized_copy(this->end() - count, this->end(), this->end());            // may throw
            m_size += count; // update end
            this->move_backward(position, position + to_move - count, this->end() - count);     // may throw
            this->copy(first, last, position);                                                  // may throw
        }
        else
        {
            Iterator middle_iter = first;
            std::advance(middle_iter, to_move);

            this->uninitialized_copy(middle_iter, last, this->end());                           // may throw
            m_size += count - to_move; // update end
            this->uninitialized_copy(position, position + to_move, position + count);           // may throw
            m_size += to_move; // update end
            this->copy(first, middle_iter, position) ;                                          // may throw
        }
    }

    // assign

    template <typename Iterator>
    void assign_dispatch(Iterator first, Iterator last, boost::random_access_traversal_tag const& /*not_random_access*/)
    {
        size_type s = std::distance(first, last);

        check_capacity(s);

        if ( m_size <= s )
        {
            this->copy(first, first + m_size, this->begin());                        // may throw
            // TODO - perform uninitialized_copy first?
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
        size_type s = 0;
        iterator it = this->begin();

        for ( ; it != this->end() && first != last ; ++it, ++first, ++s )
            *it = *first;                                                           // may throw

        this->destroy(it, this->end());

        std::pair<bool, size_type> copy_data =
            this->uninitialized_copy_checked(first, last, it, std::distance(it, this->begin() + Capacity)); // may throw
        s += copy_data.second;

        check_capacity(copy_data.first ? s : Capacity + 1);

        m_size = s; // update end
    }

    // uninitialized_copy_checked

    template <typename Iterator>
    std::pair<bool, size_type> uninitialized_copy_checked(Iterator first, Iterator last, iterator dest, size_type max_count)
    {
        size_type count = 0;
        iterator it = dest;
        try
        {
            for ( ; first != last ; ++it, ++first, ++count )
            {
                if ( max_count <= count )
                    return std::make_pair(false, count);

                this->uninitialized_fill(it, *first);                              // may throw
            }
        }
        catch(...)
        {
            this->destroy(dest, it);
            throw;
        }
        return std::make_pair(true, count);
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
                >,
                mpl::or_<
                    is_same<iterator, value_type *>,
                    is_same<iterator, const value_type *>
                >
            >::type
        use_memcpy;
        
        this->copy_dispatch(first, last, dst, use_memcpy());                        // may throw
    }

    void copy_dispatch(const value_type * first, const value_type * last, value_type * dst,
                       boost::mpl::bool_<true> const& /*use_memcpy*/)
    {
        ::memmove(dst, first, sizeof(value_type) * std::distance(first, last));
    }

    template <typename Iterator>
    void copy_dispatch(Iterator first, Iterator last, iterator dst,
                       boost::mpl::bool_<false> const& /*use_memcpy*/)
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
                >,
                mpl::or_<
                    is_same<iterator, value_type *>,
                    is_same<iterator, const value_type *>
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
    void uninitialized_copy_dispatch(Iterator first, Iterator last, iterator dst,
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
                is_same<V, value_type>
            >::type
        use_memcpy;

        uninitialized_fill_dispatch(dst, v, use_memcpy());                         // may throw
    }

    void uninitialized_fill_dispatch(iterator dst, value_type const& v,
                                     boost::mpl::bool_<true> const& /*use_memcpy*/)
    {
        // TODO - check if value_type has operator& defined and call this version only if it hasn't
        const value_type * vptr = &v;
        ::memcpy(&(*dst), vptr, sizeof(value_type));
    }

    template <typename V>
    void uninitialized_fill_dispatch(iterator dst, V const& v,
                                     boost::mpl::bool_<false> const& /*use_memcpy*/)
    {
        new (&(*dst)) value_type(v);                                                    // may throw
    }

    template <typename V>
    void uninitialized_fill(iterator dst, BOOST_RV_REF(V) v)
    {
        // TODO optimized version if has_trivial_move
        new (&(*dst)) value_type(v);                                                    // may throw
    }

    // move

    template <typename Iterator>
    void move(Iterator first, Iterator last, iterator dst)
    {
        typedef typename
            mpl::and_<
                has_trivial_assign<value_type>,
                mpl::or_<
                    is_same<Iterator, value_type *>,
                    is_same<Iterator, const value_type *>
                >,
                mpl::or_<
                    is_same<iterator, value_type *>,
                    is_same<iterator, const value_type *>
                >
            >::type
        use_memcpy;

        this->move_dispatch(first, last, dst, use_memcpy());    // may throw
    }

    void move_dispatch(const value_type * first, const value_type * last, value_type * dst,
                       boost::mpl::bool_<true> const& /*use_mem*/)
    {
        ::memmove(dst, first, sizeof(value_type) * std::distance(first, last));
    }

    template <typename Iterator>
    void move_dispatch(Iterator first, Iterator last, iterator dst,
                       boost::mpl::bool_<false> const& /*use_mem*/)
    {
        std::copy(first, last, dst);                                                // may throw
    }

    // move_backward

    void move_backward(iterator first, iterator last, iterator dst)
    {
        typedef typename
            mpl::and_<
                has_trivial_assign<value_type>,
                mpl::or_<
                    is_same<iterator, value_type *>,
                    is_same<iterator, const value_type *>
                >
            >::type
        use_memcpy;

        this->move_backward_dispatch(first, last, dst, use_memcpy());    // may throw
    }

    void move_backward_dispatch(value_type * first, value_type * last, value_type * dst,
                                boost::mpl::bool_<true> const& /*has_trivial_assign*/)
    {
        difference_type n = std::distance(first, last);
        ::memmove(dst - n, first, sizeof(value_type) * n);
    }

    void move_backward_dispatch(iterator first, iterator last, iterator dst,
                                boost::mpl::bool_<false> const& /*has_trivial_assign*/)
    {
        std::copy_backward(first, last, dst);                                                // may throw
    }

    // uninitialized_fill

    template <typename V>
    void fill(iterator dst, V const& v)
    {
        fill_dispatch(dst, v, has_trivial_assign<value_type>());                            // may throw
    }

    void fill_dispatch(iterator ptr, value_type const& v,
                       boost::true_type const& /*has_trivial_assign*/)
    {
        // TODO - check if value_type has operator& defined and call this version only if it hasn't
        const value_type * vptr = &v;
        ::memcpy(&(*ptr), vptr, sizeof(value_type));
    }

    template <typename V>
    void fill_dispatch(iterator ptr, V const& v,
                       boost::false_type const& /*has_trivial_assign*/)
    {
        *ptr = v;                                                                           // may throw
    }

    // destroy

    void destroy(iterator first, iterator last)
    {
        this->destroy_dispatch(first, last, has_trivial_destructor<value_type>());
    }

    void destroy_dispatch(iterator /*first*/, iterator /*last*/,
                          boost::true_type const& /*has_trivial_destructor*/)
    {}

    void destroy_dispatch(iterator first, iterator last,
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

    void destroy_dispatch(iterator /*ptr*/,
                          boost::true_type const& /*has_trivial_destructor*/)
    {}

    void destroy_dispatch(iterator ptr,
                          boost::false_type const& /*has_trivial_destructor*/)
    {
        ptr->~value_type();
    }

    // uninitialized_move

    template <typename Iterator>
    void uninitialized_move(Iterator first, Iterator last, iterator dst)
    {
        iterator o = dst;
        try
        {
            for (; first != last; ++first, ++o )
                new (boost::addressof(*o)) value_type(boost::move(*first));
        }
        catch(...)
        {
            destroy(dst, o);
            throw;
        }
    }

    // construct

    void construct(iterator first, iterator last)
    {
        this->construct_dispatch(first, last, has_trivial_constructor<value_type>());   // may throw
    }

    void construct_dispatch(iterator /*first*/, iterator /*last*/,
                            boost::true_type const& /*has_trivial_constructor*/)
    {}

    void construct_dispatch(iterator first, iterator last,
                            boost::false_type const& /*has_trivial_constructor*/)
    {
        iterator it = first;
        try
        {
            for ( ; it != last ; ++it )
                new (&(*it)) value_type();                                                  // may throw
        }
        catch(...)
        {
            this->destroy(first, it);
            throw;
        }
    }

    void check_capacity(size_type BOOST_GEOMETRY_INDEX_ASSERT_UNUSED_PARAM(s)) const
    {
        BOOST_ASSERT_MSG(s <= Capacity, "size can't exceed the capacity");
        //if ( Capacity < s ) throw std::bad_alloc();
    }

    void check_empty() const
    {
        BOOST_ASSERT_MSG(0 < m_size, "the container is empty");
    }

    void check_iterator_end_neq(const_iterator position) const
    {
        BOOST_ASSERT_MSG(this->begin() <= position && position < this->end(), "iterator out of bounds");

        /*BOOST_GEOMETRY_INDEX_ASSERT_UNUSED_PARAM(
            difference_type dist = std::distance(this->begin(), position);
        )
        BOOST_ASSERT_MSG(
            0 <= dist &&
            ( sizeof(dist) <= sizeof(m_size) ?
                (static_cast<size_type>(dist) < m_size) :
                ( dist < static_cast<difference_type>(m_size))
            ), "invalid iterator"
        );*/
    }

    void check_iterator_end_eq(const_iterator position) const
    {
        BOOST_ASSERT_MSG(this->begin() <= position && position <= this->end(), "iterator out of bounds");

        /*BOOST_GEOMETRY_INDEX_ASSERT_UNUSED_PARAM(
            difference_type dist = std::distance(this->begin(), position);
        )
        BOOST_ASSERT_MSG(
        0 <= dist &&
            ( sizeof(dist) <= sizeof(m_size) ?
                (static_cast<size_type>(dist) <= m_size) :
                ( dist <= static_cast<difference_type>(m_size))
            ), "invalid iterator"
        );*/
    }

    pointer ptr()
    {
        return pointer(static_cast<value_type *>(m_storage.address()));
    }

    const_pointer ptr() const
    {
        return const_pointer(static_cast<const value_type *>(m_storage.address()));
    }

    boost::aligned_storage<sizeof(Value[Capacity]), boost::alignment_of<Value[Capacity]>::value> m_storage;
    size_type m_size;
};

}}}} // namespace boost::geometry::index::detail

#endif // BOOST_GEOMETRY_INDEX_DETAIL_VARRAY_HPP
