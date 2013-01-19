// Boost.Geometry Index
//
// Translator translating random-access container index value to Indexable object
//
// Copyright (c) 2011-2013 Adam Wulkiewicz, Lodz, Poland.
//
// Use, modification and distribution is subject to the Boost Software License,
// Version 1.0. (See accompanying file LICENSE_1_0.txt or copy at
// http://www.boost.org/LICENSE_1_0.txt)

#ifndef BOOST_GEOMETRY_INDEX_TRANSLATOR_INDEX_HPP
#define BOOST_GEOMETRY_INDEX_TRANSLATOR_INDEX_HPP

#include <boost/geometry/index/translator/def.hpp>

namespace boost { namespace geometry { namespace index { namespace translator {

/*!
\brief The index translator.

This translator translates from index of an element in an external Container to the Indexable.
The container should have operator[](size_type) defined.
Index translator uses translator::def<...> to translate from Container::value_type to the Indexable which
means that it can handle Indexables, std::pairs and boost::tuples stored in an external Container.

\tparam Container   The Container type.
*/
template <typename Container>
class index
{
    typedef typename Container::value_type value_type;
    typedef typename Container::size_type size_type;

public:
    typedef typename translator::def<value_type>::result_type result_type;
    
    /*!
    \brief The constructor.

    \param c    The container which stores indexed values.
    */
    explicit index(Container const& c)
        : m_c(c)
    {}

    result_type operator()(size_type i) const
    {
        return m_tr(m_c[i]);
    }

    bool equals(size_type i1, size_type i2) const
    {
        return m_tr.equals(m_c[i1], m_c[i2]);
    }

private:
    Container const& m_c;
    translator::def<value_type> m_tr;
};

}}}} // namespace boost::geometry::index::translator

#endif // BOOST_GEOMETRY_INDEX_TRANSLATOR_INDEX_HPP
