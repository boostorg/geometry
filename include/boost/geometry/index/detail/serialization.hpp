// Boost.Geometry Index
//
// Copyright (c) 2011-2013 Adam Wulkiewicz, Lodz, Poland.
//
// Use, modification and distribution is subject to the Boost Software License,
// Version 1.0. (See accompanying file LICENSE_1_0.txt or copy at
// http://www.boost.org/LICENSE_1_0.txt)

#ifndef BOOST_GEOMETRY_INDEX_DETAIL_SERIALIZATION_HPP
#define BOOST_GEOMETRY_INDEX_DETAIL_SERIALIZATION_HPP

//#include <boost/serialization/serialization.hpp>
#include <boost/serialization/split_member.hpp>
#include <boost/serialization/version.hpp>
//#include <boost/serialization/nvp.hpp>

// TODO
// how about using the unsigned type capable of storing Max in compile-time versions?

// TODO - move to index/detail/serialization.hpp
namespace boost { namespace geometry { namespace index { namespace detail {

// TODO - use boost::move?
template<typename T>
class serialization_storage
{
public:
    template <typename Archive>
    serialization_storage(Archive & ar, unsigned int version)
    {
        boost::serialization::load_construct_data_adl(ar, this->address(), version);
    }
    ~serialization_storage()
    {
        this->address()->~T();
    }
    T * address()
    {
        return static_cast<T*>(m_storage.address());
    }
private:
    boost::aligned_storage<sizeof(T), boost::alignment_of<T>::value> m_storage;
};

// TODO - save and load item_version? see: collections_load_imp and collections_save_imp
// this should be done once for the whole container
// versions of all used types should be stored

template <typename T, typename Archive> inline
T serialization_load(const char * name, Archive & ar)
{
    namespace bs = boost::serialization;    
    serialization_storage<T> storage(ar, bs::version<T>::value);        // load_construct_data
    ar >> boost::serialization::make_nvp(name, *storage.address());   // serialize
    //ar >> *storage.address();                                           // serialize
    return *storage.address();
}

template <typename T, typename Archive> inline
void serialization_save(T const& t, const char * name, Archive & ar)
{
    namespace bs = boost::serialization;
    bs::save_construct_data_adl(ar, boost::addressof(t), bs::version<T>::value);  // save_construct_data
    ar << boost::serialization::make_nvp(name, t);                                // serialize
    //ar << t;                                                                      // serialize
}
    
}}}}

// TODO - move to index/serialization.hpp
namespace boost { namespace serialization {

// boost::geometry::index::linear

template<class Archive, size_t Max, size_t Min>
void save_construct_data(Archive & ar, const boost::geometry::index::linear<Max, Min> * params, unsigned int )
{
    namespace bs = boost::serialization;
    size_t max = params->get_max_elements(), min = params->get_min_elements();
    ar << bs::make_nvp("max", max) << bs::make_nvp("min", min);
}
template<class Archive, size_t Max, size_t Min>
void load_construct_data(Archive & ar, boost::geometry::index::linear<Max, Min> * params, unsigned int )
{
    namespace bs = boost::serialization;
    size_t max, min;
    ar >> bs::make_nvp("max", max) >> bs::make_nvp("min", min);
    if ( max != params->get_max_elements() || min != params->get_min_elements() )
        // TODO change exception type
        BOOST_THROW_EXCEPTION(std::runtime_error("parameters not compatible"));
    // the constructor musn't be called for this type
    //::new(params)boost::geometry::index::linear<Max, Min>();
}
template<class Archive, size_t Max, size_t Min> void serialize(Archive &, boost::geometry::index::linear<Max, Min> &, unsigned int) {}

// boost::geometry::index::quadratic

template<class Archive, size_t Max, size_t Min>
void save_construct_data(Archive & ar, const boost::geometry::index::quadratic<Max, Min> * params, unsigned int )
{
    namespace bs = boost::serialization;
    size_t max = params->get_max_elements(), min = params->get_min_elements();
    ar << bs::make_nvp("max", max) << bs::make_nvp("min", min);
}
template<class Archive, size_t Max, size_t Min>
void load_construct_data(Archive & ar, boost::geometry::index::quadratic<Max, Min> * params, unsigned int )
{
    namespace bs = boost::serialization;
    size_t max, min;
    ar >> bs::make_nvp("max", max) >> bs::make_nvp("min", min);
    if ( max != params->get_max_elements() || min != params->get_min_elements() )
        // TODO change exception type
        BOOST_THROW_EXCEPTION(std::runtime_error("parameters not compatible"));
    // the constructor musn't be called for this type
    //::new(params)boost::geometry::index::quadratic<Max, Min>();
}
template<class Archive, size_t Max, size_t Min> void serialize(Archive &, boost::geometry::index::quadratic<Max, Min> &, unsigned int) {}

// boost::geometry::index::rstar

template<class Archive, size_t Max, size_t Min, size_t RE, size_t OCT>
void save_construct_data(Archive & ar, const boost::geometry::index::rstar<Max, Min, RE, OCT> * params, unsigned int )
{
    namespace bs = boost::serialization;
    size_t max = params->get_max_elements()
         , min = params->get_min_elements()
         , re = params->get_reinserted_elements()
         , oct = params->get_overlap_cost_threshold();
    ar << bs::make_nvp("max", max) << bs::make_nvp("min", min) << bs::make_nvp("re", re) << bs::make_nvp("oct", oct);
}
template<class Archive, size_t Max, size_t Min, size_t RE, size_t OCT>
void load_construct_data(Archive & ar, boost::geometry::index::rstar<Max, Min, RE, OCT> * params, unsigned int )
{
    namespace bs = boost::serialization;
    size_t max, min, re, oct;
    ar >> bs::make_nvp("max", max) >> bs::make_nvp("min", min) >> bs::make_nvp("re", re) >> bs::make_nvp("oct", oct);
    if ( max != params->get_max_elements() || min != params->get_min_elements() ||
         re != params->get_reinserted_elements() || oct != params->get_overlap_cost_threshold() )
        // TODO change exception type
        BOOST_THROW_EXCEPTION(std::runtime_error("parameters not compatible"));
    // the constructor musn't be called for this type
    //::new(params)boost::geometry::index::rstar<Max, Min, RE, OCT>();
}
template<class Archive, size_t Max, size_t Min, size_t RE, size_t OCT>
void serialize(Archive &, boost::geometry::index::rstar<Max, Min, RE, OCT> &, unsigned int) {}

// boost::geometry::index::dynamic_linear

template<class Archive>
inline void save_construct_data(Archive & ar, const boost::geometry::index::dynamic_linear * params, unsigned int )
{
    namespace bs = boost::serialization;
    size_t max = params->get_max_elements(), min = params->get_min_elements();
    ar << bs::make_nvp("max", max) << bs::make_nvp("min", min);
}
template<class Archive>
inline void load_construct_data(Archive & ar, boost::geometry::index::dynamic_linear * params, unsigned int )
{
    namespace bs = boost::serialization;
    size_t max, min;
    ar >> bs::make_nvp("max", max) >> bs::make_nvp("min", min);
    ::new(params)boost::geometry::index::dynamic_linear(max, min);
}
template<class Archive> void serialize(Archive &, boost::geometry::index::dynamic_linear &, unsigned int) {}

// boost::geometry::index::dynamic_quadratic

template<class Archive>
inline void save_construct_data(Archive & ar, const boost::geometry::index::dynamic_quadratic * params, unsigned int )
{
    namespace bs = boost::serialization;
    size_t max = params->get_max_elements(), min = params->get_min_elements();
    ar << bs::make_nvp("max", max) << bs::make_nvp("min", min);
}
template<class Archive>
inline void load_construct_data(Archive & ar, boost::geometry::index::dynamic_quadratic * params, unsigned int )
{
    namespace bs = boost::serialization;
    size_t max, min;
    ar >> bs::make_nvp("max", max) >> bs::make_nvp("min", min);
    ::new(params)boost::geometry::index::dynamic_quadratic(max, min);
}
template<class Archive> void serialize(Archive &, boost::geometry::index::dynamic_quadratic &, unsigned int) {}

// boost::geometry::index::dynamic_rstar

template<class Archive>
inline void save_construct_data(Archive & ar, const boost::geometry::index::dynamic_rstar * params, unsigned int )
{
    namespace bs = boost::serialization;
    size_t max = params->get_max_elements()
         , min = params->get_min_elements()
         , re = params->get_reinserted_elements()
         , oct = params->get_overlap_cost_threshold();
    ar << bs::make_nvp("max", max) << bs::make_nvp("min", min) << bs::make_nvp("re", re) << bs::make_nvp("oct", oct);
}
template<class Archive>
inline void load_construct_data(Archive & ar, boost::geometry::index::dynamic_rstar * params, unsigned int )
{
    namespace bs = boost::serialization;
    size_t max, min, re, oct;
    ar >> bs::make_nvp("max", max) >> bs::make_nvp("min", min) >> bs::make_nvp("re", re) >> bs::make_nvp("oct", oct);
    ::new(params)boost::geometry::index::dynamic_rstar(max, min, re, oct);
}
template<class Archive> void serialize(Archive &, boost::geometry::index::dynamic_rstar &, unsigned int) {}

}} // boost::serialization

// TODO - move to index/detail/serialization.hpp or maybe geometry/serialization.hpp
namespace boost { namespace geometry { namespace index { namespace detail {

template <typename P, size_t I = 0, size_t D = traits::dimension<P>::value>
struct serialize_point
{
    template <typename Archive>
    static inline void save(Archive & ar, P const& p, unsigned int version)
    {
        namespace bs = boost::serialization;
        typename coordinate_type<P>::type c = get<I>(p);
        ar << bs::make_nvp("c", c);
        serialize_point<P, I+1, D>::save(ar, p, version);
    }

    template <typename Archive>
    static inline void load(Archive & ar, P & p, unsigned int version)
    {
        namespace bs = boost::serialization;
        typename traits::coordinate_type<P>::type c;
        ar >> bs::make_nvp("c", c);
        set<I>(p, c);
        serialize_point<P, I+1, D>::load(ar, p, version);
    }
};

template <typename P, size_t D>
struct serialize_point<P, D, D>
{
    template <typename Archive> static inline void save(Archive &, P const&, unsigned int) {}
    template <typename Archive> static inline void load(Archive &, P &, unsigned int) {}
};

}}}}

// TODO - move to index/detail/serialization.hpp or maybe geometry/serialization.hpp
namespace boost { namespace serialization {

template<class Archive, typename T, size_t D, typename C>
void save(Archive & ar, boost::geometry::model::point<T, D, C> const& p, unsigned int version)
{
    boost::geometry::index::detail::serialize_point< boost::geometry::model::point<T, D, C> >::save(ar, p, version);
}
template<class Archive, typename T, size_t D, typename C>
void load(Archive & ar, boost::geometry::model::point<T, D, C> & p, unsigned int version)
{
    boost::geometry::index::detail::serialize_point< boost::geometry::model::point<T, D, C> >::load(ar, p, version);
}
template<class Archive, typename T, size_t D, typename C>
inline void serialize(Archive & ar, boost::geometry::model::point<T, D, C> & o, const unsigned int version) { split_free(ar, o, version); }

template<class Archive, typename P>
inline void serialize(Archive & ar, boost::geometry::model::box<P> & b, const unsigned int)
{
    namespace bs = boost::serialization;
    ar & bs::make_nvp("min", b.min_corner());
    ar & bs::make_nvp("max", b.max_corner());
}

}} // boost::serialization

// TODO - move to index/detail/rtree/visitors/save.hpp
namespace boost { namespace geometry { namespace index { namespace detail { namespace rtree { namespace visitors {

// TODO move saving and loading of the rtree outside the rtree, this will require adding some kind of members_view

template <typename Archive, typename Value, typename Options, typename Translator, typename Box, typename Allocators>
class save
    : public rtree::visitor<Value, typename Options::parameters_type, Box, Allocators, typename Options::node_tag, true>::type
{
public:
    typedef typename rtree::node<Value, typename Options::parameters_type, Box, Allocators, typename Options::node_tag>::type node;
    typedef typename rtree::internal_node<Value, typename Options::parameters_type, Box, Allocators, typename Options::node_tag>::type internal_node;
    typedef typename rtree::leaf<Value, typename Options::parameters_type, Box, Allocators, typename Options::node_tag>::type leaf;

    save(Archive & archive, unsigned int version)
        : m_archive(archive), m_version(version)
    {}

    inline void operator()(internal_node const& n)
    {
        namespace bs = boost::serialization;

        typedef typename rtree::elements_type<internal_node>::type elements_type;
        elements_type const& elements = rtree::elements(n);

        // change to elements_type::size_type or size_type?
        size_t s = elements.size();
        m_archive << bs::make_nvp("s", s);

        for (typename elements_type::const_iterator it = elements.begin() ; it != elements.end() ; ++it)
        {
            serialization_save(it->first, "b", m_archive);

            rtree::apply_visitor(*this, *it->second);
        }
    }

    inline void operator()(leaf const& l)
    {
        namespace bs = boost::serialization;

        typedef typename rtree::elements_type<leaf>::type elements_type;
        typedef typename elements_type::size_type elements_size;
        elements_type const& elements = rtree::elements(l);

        // change to elements_type::size_type or size_type?
        size_t s = elements.size();
        m_archive << bs::make_nvp("s", s);

        for (typename elements_type::const_iterator it = elements.begin() ; it != elements.end() ; ++it)
        {
            serialization_save(*it, "v", m_archive);
        }
    }

private:
    Archive & m_archive;
    unsigned int m_version;
};

}}}}}} // boost::geometry::index::detail::rtree::visitors

// TODO - move to index/detail/rtree/load.hpp
namespace boost { namespace geometry { namespace index { namespace detail { namespace rtree {

template <typename Value, typename Options, typename Translator, typename Box, typename Allocators>
class load
{
    typedef typename rtree::node<Value, typename Options::parameters_type, Box, Allocators, typename Options::node_tag>::type node;
    typedef typename rtree::internal_node<Value, typename Options::parameters_type, Box, Allocators, typename Options::node_tag>::type internal_node;
    typedef typename rtree::leaf<Value, typename Options::parameters_type, Box, Allocators, typename Options::node_tag>::type leaf;

    typedef typename Options::parameters_type parameters_type;

    typedef typename Allocators::node_pointer node_pointer;
    typedef rtree::node_auto_ptr<Value, Options, Translator, Box, Allocators> node_auto_ptr;
    typedef typename Allocators::size_type size_type;

public:
    template <typename Archive> inline static
    node_pointer apply(Archive & ar, unsigned int version, size_type leafs_level, size_type & values_count, parameters_type const& parameters, Translator const& translator, Allocators & allocators)
    {
        values_count = 0;
        return raw_apply(ar, version, leafs_level, values_count, parameters, translator, allocators);
    }

private:
    template <typename Archive> inline static
    node_pointer raw_apply(Archive & ar, unsigned int version, size_type leafs_level, size_type & values_count, parameters_type const& parameters, Translator const& translator, Allocators & allocators, size_type current_level = 0)
    {
        namespace bs = boost::serialization;

        //BOOST_GEOMETRY_INDEX_ASSERT(current_level <= leafs_level, "invalid parameter");

        // change to elements_type::size_type or size_type?
        size_t elements_count;
        ar >> bs::make_nvp("s", elements_count);

        if ( elements_count < parameters.get_min_elements() || parameters.get_max_elements() < elements_count )
            BOOST_THROW_EXCEPTION(std::runtime_error("rtree loading error"));

        if ( current_level < leafs_level )
        {
            node_pointer n = rtree::create_node<Allocators, internal_node>::apply(allocators);              // MAY THROW (A)
            node_auto_ptr auto_remover(n, allocators);    
            internal_node & in = rtree::get<internal_node>(*n);

            typedef typename rtree::elements_type<internal_node>::type elements_type;
            typedef typename elements_type::value_type element_type;
            typedef typename elements_type::size_type elements_size;
            elements_type & elements = rtree::elements(in);

            elements.reserve(elements_count);                                                               // MAY THROW (A)

            for ( size_t i = 0 ; i < elements_count ; ++i )
            {
                typedef typename elements_type::value_type::first_type box_type;
                box_type b = serialization_load<box_type>("b", ar);
                node_pointer n = raw_apply(ar, version, leafs_level, values_count, parameters, translator, allocators, current_level+1); // recursive call
                elements.push_back(element_type(b, n));
            }

            auto_remover.release();
            return n;
        }
        else
        {
            BOOST_GEOMETRY_INDEX_ASSERT(current_level == leafs_level, "unexpected value");

            node_pointer n = rtree::create_node<Allocators, leaf>::apply(allocators);                       // MAY THROW (A)
            node_auto_ptr auto_remover(n, allocators);
            leaf & l = rtree::get<leaf>(*n);

            typedef typename rtree::elements_type<leaf>::type elements_type;
            typedef typename elements_type::value_type element_type;
            elements_type & elements = rtree::elements(l);

            values_count += elements_count;

            elements.reserve(elements_count);                                                               // MAY THROW (A)

            for ( size_t i = 0 ; i < elements_count ; ++i )
            {
                element_type el = serialization_load<element_type>("v", ar);                                     // MAY THROW (C)
                elements.push_back(el);                                                                     // MAY THROW (C)
            }

            auto_remover.release();
            return n;
        }
    }
};

}}}}} // boost::geometry::index::detail::rtree

#endif // BOOST_GEOMETRY_INDEX_DETAIL_SERIALIZATION_HPP
