// Boost.Geometry (aka GGL, Generic Geometry Library)
//
// Boost.Index - pushable array
//
// Copyright 2011 Adam Wulkiewicz.
// Use, modification and distribution is subject to the Boost Software License,
// Version 1.0. (See accompanying file LICENSE_1_0.txt or copy at
// http://www.boost.org/LICENSE_1_0.txt)

#ifndef BOOST_GEOMETRY_EXTENSIONS_INDEX_PUSHABLE_ARRAY_HPP
#define BOOST_GEOMETRY_EXTENSIONS_INDEX_PUSHABLE_ARRAY_HPP

#include <boost/array.hpp>

#include <boost/geometry/extensions/index/assert.hpp>

namespace boost { namespace geometry { namespace index {

template <typename Element, size_t Capacity>
class pushable_array
{
	typedef typename boost::array<Element, Capacity> array_type;

public:
	typedef typename array_type::value_type value_type;
	typedef typename array_type::size_type size_type;
	typedef typename array_type::iterator iterator;
	typedef typename array_type::const_iterator const_iterator;

	inline pushable_array()
		: m_size(0)
	{}

	inline pushable_array(size_type s, Element const& v)
		: m_size(s)
	{
		BOOST_GEOMETRY_INDEX_ASSERT(s < Capacity, "size too big");
		std::fill(m_array.begin(), m_array.begin() + s, v);
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

	inline void clear()
	{
		m_size = 0;
	}

	inline void erase(iterator it)
	{
		BOOST_GEOMETRY_INDEX_ASSERT(0 < m_size, "there are no elements in the container");
		BOOST_GEOMETRY_INDEX_ASSERT(begin() <= it && it < end(), "iterator points on the element outside the container");
		//std::copy(it + 1, end(), it);
		*it = back();
		--m_size;
	}
	
	inline void push_back(Element const& v)
	{
		BOOST_GEOMETRY_INDEX_ASSERT(m_size < Capacity, "can't further increase the size of the container");
		m_array[m_size++] = v;
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

}}} // namespace boost::geometry::index

#endif // BOOST_GEOMETRY_EXTENSIONS_INDEX_PUSHABLE_ARRAY_HPP
