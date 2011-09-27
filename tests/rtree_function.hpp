#ifndef TESTS_RTREE_FUNCTION_HPP
#define TESTS_RTREE_FUNCTION_HPP

#include <boost/geometry/geometry.hpp>

#include <boost/geometry/extensions/index/rtree/rtree.hpp>
#include <boost/geometry/extensions/index/translator/index.hpp>

#include <boost/geometry/extensions/index/rtree/visitors/print.hpp>

#include <boost/range/algorithm.hpp>
#include <boost/foreach.hpp>

#include <map>

namespace helpers
{
    template <typename Indexable, size_t DI, typename Tag>
    struct value_randomizer_impl_set {};

    template <typename Box, size_t DI>
    struct value_randomizer_impl_set<Box, DI, boost::geometry::box_tag>
    {
        inline static void apply(
            Box & b,
            typename boost::geometry::index::traits::coordinate_type<Box>::type m,
            typename boost::geometry::index::traits::coordinate_type<Box>::type w)
        {
            namespace bg = boost::geometry;
            typedef typename bg::index::traits::coordinate_type<Box>::type coord_t;

            coord_t c1 = ::rand() / coord_t(RAND_MAX / m);
            coord_t c2 = ::rand() / coord_t(RAND_MAX / w);

            bg::set<bg::min_corner, DI>(b, c1 - c2);
            bg::set<bg::max_corner, DI>(b, c1 + c2);
        }
    };

    template <typename Point, size_t DI>
    struct value_randomizer_impl_set<Point, DI, boost::geometry::point_tag>
    {
        inline static void apply(
            Point & p,
            typename boost::geometry::index::traits::coordinate_type<Point>::type m,
            typename boost::geometry::index::traits::coordinate_type<Point>::type)
        {
            namespace bg = boost::geometry;
            typedef typename bg::index::traits::coordinate_type<Point>::type coord_t;

            coord_t c = ::rand() / coord_t(RAND_MAX / m);

            bg::set<DI>(p, c);
        }
    };

    template <typename Indexable, size_t D>
    struct value_randomizer_impl
    {
        inline static void apply(
            Indexable & i,
            typename boost::geometry::index::traits::coordinate_type<Indexable>::type m,
            typename boost::geometry::index::traits::coordinate_type<Indexable>::type w)
        {
            value_randomizer_impl<Indexable, D - 1>::apply(i, m, w);
            value_randomizer_impl_set<
                Indexable,
                D - 1,
                typename boost::geometry::index::traits::tag<Indexable>::type
            >::apply(i, m, w);
        }
    };

    template <typename Indexable>
    struct value_randomizer_impl<Indexable, 1>
    {
        inline static void apply(
            Indexable & i,
            typename boost::geometry::index::traits::coordinate_type<Indexable>::type m,
            typename boost::geometry::index::traits::coordinate_type<Indexable>::type w)
        {
            value_randomizer_impl_set<
                Indexable,
                0,
                typename boost::geometry::index::traits::tag<Indexable>::type
            >::apply(i, m, w);
        }
    };

    template <typename Indexable>
    struct value_randomizer
    {
        typedef Indexable value_type;

        typedef typename boost::geometry::index::traits::coordinate_type<Indexable>::type coord_t;
        
        inline value_randomizer(coord_t mm, coord_t ww)
            : m(mm), w(ww)
        {}

        inline Indexable operator()() const
        {
            namespace bg = boost::geometry;
            namespace bgi = bg::index;

            Indexable i;
            value_randomizer_impl<Indexable, bgi::traits::dimension<Indexable>::value>::apply(i, m, w);
            return i;
        }

        coord_t m, w;
    };

    template <typename Rtree, typename Cont, typename Randomizer>
    void random_insert(Rtree & t, Cont & c, size_t n, Randomizer r)
    {
        namespace bg = boost::geometry;
        namespace bgi = bg::index;

        for ( size_t i = 0 ; i < n ; ++i )
        {
            typename Randomizer::value_type v = r();
            bgi::insert(t, v);
            c.push_back(v);
        }
    }

    template <typename Cont, typename Translator>
    bool results_comp(Cont const& c1, Cont const& c2, Translator const& tr)
    {
        if ( c1.size() != c2.size() )
            return false;

        for ( typename Cont::const_iterator it = c1.begin() ; it != c1.end() ; ++it )
        {
            bool found = false;
            for ( typename Cont::const_iterator it2 = c2.begin() ; it2 != c2.end() ; ++it2 )
                if ( tr.equals(*it, *it2) )
                {
                    found = true;
                    break;
                }

                if ( !found )
                    return false;
        }

        return true;
    }

    template <typename Predicate, typename Rtree, typename Cont, typename Randomizer>
    void random_query_check(Rtree const& t, Cont const& c, size_t n, Randomizer r)
    {
        namespace bg = boost::geometry;
        namespace bgi = bg::index;

        for ( size_t i = 0 ; i < n ; ++i )
        {
            Predicate pred = Predicate(r());

            std::vector<typename Rtree::value_type> res1, res2;

            bgi::query(t, pred, std::back_inserter(res1));

            for ( typename Cont::const_iterator it = c.begin() ; it != c.end() ; ++it )
            {
                if ( bgi::predicates_check<bgi::detail::rtree::value_tag>(pred, t.get_translator()(*it)) )
                    res2.push_back(*it);
            }

            BOOST_CHECK( helpers::results_comp(res1, res2, t.get_translator()) );
        }
    }

    template <typename Point, typename Translator>
    struct val_mindist_cmp
    {
        val_mindist_cmp(Point const& p, Translator const& t)
            : pt(p), tr(t) 
        {}

        template <typename Value>
        bool operator()(Value const& v1, Value const& v2)
        {
            return boost::geometry::index::comparable_distance_near(pt, tr(v1))
                < boost::geometry::index::comparable_distance_near(pt, tr(v2));
        }

        Point const& pt;
        Translator const& tr;
    };

    template <typename Predicate, typename Rtree, typename Cont, typename PointRandomizer, typename PredicateRandomizer>
    void random_nearest_check(
        Rtree const& t,
        Cont const& c,
        size_t n,
        PointRandomizer const& pr,
        size_t k,
        PredicateRandomizer const& r)
    {
        namespace bg = boost::geometry;
        namespace bgi = bg::index;

        for ( size_t i = 0 ; i < n ; ++i )
        {
            typename PointRandomizer::value_type pt = pr();
            Predicate pred = Predicate(r());

            std::vector<typename Rtree::value_type> res1, res2;

            bgi::nearest(t, pt, k, pred, std::back_inserter(res1));

            for ( typename Cont::const_iterator it = c.begin() ; it != c.end() ; ++it )
            {
                if ( bgi::predicates_check<bgi::detail::rtree::value_tag>(pred, t.get_translator()(*it)) )
                    res2.push_back(*it);
            }
            std::sort(
                res2.begin(),
                res2.end(),
                val_mindist_cmp<
                    typename PointRandomizer::value_type,
                    typename Rtree::translator_type
                >(pt, t.get_translator())
            );
            if ( k < res2.size() )
                res2.resize(k);

            BOOST_CHECK( helpers::results_comp(res1, res2, t.get_translator()) );
        }
    }
}

template <typename P, typename B, typename Tag>
struct tests_rtree_function_queries {};

template <typename P, typename B>
struct tests_rtree_function_queries<P, B, boost::geometry::point_tag>
{
    template <typename Rtree, typename Cont>
    inline static void apply(Rtree const& t, Cont const& v)
    {
        namespace bgi = boost::geometry::index;

        helpers::random_query_check<B>(t, v, 5, helpers::value_randomizer<B>(10, 5));
        helpers::random_query_check<bgi::detail::intersects<B> >(t, v, 5, helpers::value_randomizer<B>(10, 5));
        helpers::random_query_check<bgi::detail::within<B> >(t, v, 5, helpers::value_randomizer<B>(10, 5));
        helpers::random_query_check<bgi::detail::covered_by<B> >(t, v, 5, helpers::value_randomizer<B>(10, 5));

        helpers::random_nearest_check<bgi::detail::empty>(t, v, 5, helpers::value_randomizer<P>(10, 0), 3, bgi::empty);
        helpers::random_nearest_check<B>(t, v, 5, helpers::value_randomizer<P>(10, 0), 3, helpers::value_randomizer<B>(10, 5));
        helpers::random_nearest_check<bgi::detail::intersects<B> >(t, v, 5, helpers::value_randomizer<P>(10, 0), 3, helpers::value_randomizer<B>(10, 5));
        helpers::random_nearest_check<bgi::detail::within<B> >(t, v, 5, helpers::value_randomizer<P>(10, 0), 3, helpers::value_randomizer<B>(10, 5));
        helpers::random_nearest_check<bgi::detail::covered_by<B> >(t, v, 5, helpers::value_randomizer<P>(10, 0), 3, helpers::value_randomizer<B>(10, 5));
    }
};

template <typename P, typename B>
struct tests_rtree_function_queries<P, B, boost::geometry::box_tag>
{
    template <typename Rtree, typename Cont>
    inline static void apply(Rtree const& t, Cont const& v)
    {
        namespace bgi = boost::geometry::index;

        helpers::random_query_check<B>(t, v, 5, helpers::value_randomizer<B>(10, 5));
        helpers::random_query_check<bgi::detail::intersects<B> >(t, v, 5, helpers::value_randomizer<B>(10, 5));
        helpers::random_query_check<bgi::detail::overlaps<B> >(t, v, 5, helpers::value_randomizer<B>(10, 5));
        helpers::random_query_check<bgi::detail::within<B> >(t, v, 5, helpers::value_randomizer<B>(10, 5));
        helpers::random_query_check<bgi::detail::covered_by<B> >(t, v, 5, helpers::value_randomizer<B>(10, 5));

        helpers::random_nearest_check<bgi::detail::empty>(t, v, 5, helpers::value_randomizer<P>(10, 0), 3, bgi::empty);
        helpers::random_nearest_check<B>(t, v, 5, helpers::value_randomizer<P>(10, 0), 3, helpers::value_randomizer<B>(10, 5));
        helpers::random_nearest_check<bgi::detail::intersects<B> >(t, v, 5, helpers::value_randomizer<P>(10, 0), 3, helpers::value_randomizer<B>(10, 5));
        helpers::random_nearest_check<bgi::detail::overlaps<B> >(t, v, 5, helpers::value_randomizer<P>(10, 0), 3, helpers::value_randomizer<B>(10, 5));
        helpers::random_nearest_check<bgi::detail::within<B> >(t, v, 5, helpers::value_randomizer<P>(10, 0), 3, helpers::value_randomizer<B>(10, 5));
        helpers::random_nearest_check<bgi::detail::covered_by<B> >(t, v, 5, helpers::value_randomizer<P>(10, 0), 3, helpers::value_randomizer<B>(10, 5));
    }
};

template <typename Value, typename Options, typename Translator>
void tests_rtree_function(Translator const& tr = Translator())
{
    namespace bg = boost::geometry;
    namespace bgi = bg::index;

    bgi::rtree<Value, Options, Translator> t(tr);
    std::vector<Value> v;

    typedef typename bgi::rtree<Value, Options, Translator>::indexable_type I;
    typedef typename bgi::rtree<Value, Options, Translator>::box_type B;
    typedef typename bgi::traits::point_type<B>::type P ;

    helpers::random_insert(t, v, 10, helpers::value_randomizer<Value>(10, 1));

    tests_rtree_function_queries<P, B, bgi::traits::tag<I>::type>::apply(t, v);
}

BOOST_AUTO_TEST_CASE(tests_rtree_function_box3f)
{
#ifdef TEST_PRINT_INFO
    std::cout << "tests/rtree_function_box3f\n";
#endif

    namespace bg = boost::geometry;
    namespace bgi = bg::index;

    typedef bg::model::point<float, 3, bg::cs::cartesian> P;
    typedef bg::model::box<P> B;
    typedef B V;

    tests_rtree_function< V, bgi::linear<4, 2>, bgi::translator::def<V> >();
    tests_rtree_function< V, bgi::quadratic<4, 2>, bgi::translator::def<V> >();
    tests_rtree_function< V, bgi::rstar<4, 2>, bgi::translator::def<V> >();
}

BOOST_AUTO_TEST_CASE(tests_rtree_function_box2f)
{
#ifdef TEST_PRINT_INFO
    std::cout << "tests/rtree_function_box2f\n";
#endif

    namespace bg = boost::geometry;
    namespace bgi = bg::index;

    typedef bg::model::point<float, 2, bg::cs::cartesian> P;
    typedef bg::model::box<P> B;
    typedef B V;

    tests_rtree_function< V, bgi::linear<4, 2>, bgi::translator::def<V> >();
    tests_rtree_function< V, bgi::quadratic<4, 2>, bgi::translator::def<V> >();
    tests_rtree_function< V, bgi::rstar<4, 2>, bgi::translator::def<V> >();
}

BOOST_AUTO_TEST_CASE(tests_rtree_function_point2f)
{
#ifdef TEST_PRINT_INFO
    std::cout << "tests/rtree_function_point2f\n";
#endif

    namespace bg = boost::geometry;
    namespace bgi = bg::index;

    typedef bg::model::point<float, 2, bg::cs::cartesian> P;
    typedef P V;

    tests_rtree_function< V, bgi::linear<4, 2>, bgi::translator::def<V> >();
    tests_rtree_function< V, bgi::quadratic<4, 2>, bgi::translator::def<V> >();
    tests_rtree_function< V, bgi::rstar<4, 2>, bgi::translator::def<V> >();
}

namespace helpers {

template <typename Indexable>
struct value_randomizer< std::pair<Indexable, int> >
{
    typedef std::pair<Indexable, int> value_type;

    typedef typename boost::geometry::index::traits::coordinate_type<Indexable>::type coord_t;

    inline value_randomizer(coord_t mm, coord_t ww)
        : r(mm, ww)
    {}

    inline value_type operator()() const
    {
        return std::make_pair(r(), ::rand());
    }

    value_randomizer<Indexable> r;
};

} // namespace helpers

BOOST_AUTO_TEST_CASE(tests_rtree_function_pair_box2f_int)
{
#ifdef TEST_PRINT_INFO
    std::cout << "tests/rtree_function_pair_box2f_int\n";
#endif

    namespace bg = boost::geometry;
    namespace bgi = bg::index;

    typedef bg::model::point<float, 2, bg::cs::cartesian> P;
    typedef bg::model::box<P> B;
    typedef std::pair<B, int> V;

    tests_rtree_function< V, bgi::linear<4, 2>, bgi::translator::def<V> >();
    tests_rtree_function< V, bgi::quadratic<4, 2>, bgi::translator::def<V> >();
    tests_rtree_function< V, bgi::rstar<4, 2>, bgi::translator::def<V> >();
}

namespace helpers {

template <typename Indexable>
struct value_randomizer< boost::shared_ptr< std::pair<Indexable, int> > >
{
    typedef boost::shared_ptr< std::pair<Indexable, int> > value_type;

    typedef typename boost::geometry::index::traits::coordinate_type<Indexable>::type coord_t;

    inline value_randomizer(coord_t mm, coord_t ww)
        : r(mm, ww)
    {}

    inline value_type operator()() const
    {
        return value_type(new std::pair<Indexable, int>(r(), ::rand()));
    }

    value_randomizer<Indexable> r;
};

} // namespace helpers

BOOST_AUTO_TEST_CASE(tests_rtree_function_shared_ptr_pair_box2f_int)
{
#ifdef TEST_PRINT_INFO
    std::cout << "tests/rtree_function_shared_ptr_pair_box2f_int\n";
#endif

    namespace bg = boost::geometry;
    namespace bgi = bg::index;

    typedef bg::model::point<float, 2, bg::cs::cartesian> P;
    typedef bg::model::box<P> B;
    typedef boost::shared_ptr< std::pair<B, int> > V;

    tests_rtree_function< V, bgi::linear<4, 2>, bgi::translator::def<V> >();
    tests_rtree_function< V, bgi::quadratic<4, 2>, bgi::translator::def<V> >();
    tests_rtree_function< V, bgi::rstar<4, 2>, bgi::translator::def<V> >();
}

#endif // TESTS_RTREE_FUNCTION_HPP
