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

            coord_t c1 = rand() / coord_t(RAND_MAX / m);
            coord_t c2 = rand() / coord_t(RAND_MAX / w);

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

            coord_t c = rand() / coord_t(RAND_MAX / m);

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
                if ( bgi::predicates_check<bgi::detail::rtree::value_predicates_tag>(pred, t.get_translator()(*it)) )
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

        template <typename Indexable>
        bool operator()(Indexable const& i1, Indexable const& i2)
        {
            return boost::geometry::index::mindist(pt, i1) < boost::geometry::index::mindist(pt, i2);
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
                if ( bgi::predicates_check<bgi::detail::rtree::value_predicates_tag>(pred, t.get_translator()(*it)) )
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

template <typename Value, typename Options>
void tests_rtree_function()
{
    namespace bg = boost::geometry;
    namespace bgi = bg::index;

    bgi::rtree<Value, Options> t;
    std::vector<Value> v;

    typedef typename bgi::rtree<Value, Options>::box_type B;
    typedef typename bgi::traits::point_type<B>::type P ;

    helpers::random_insert(t, v, 10, helpers::value_randomizer<Value>(10, 1));

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

BOOST_AUTO_TEST_CASE(tests_rtree_function_box3f)
{
    std::cout << "tests/rtree_function_box3f\n";

    namespace bg = boost::geometry;
    namespace bgi = bg::index;

    typedef bg::model::point<float, 3, bg::cs::cartesian> P;
    typedef bg::model::box<P> B;
    typedef B val_t;

    tests_rtree_function< val_t, bgi::linear<4, 2> >();
    tests_rtree_function< val_t, bgi::quadratic<4, 2> >();
    tests_rtree_function< val_t, bgi::rstar<4, 2> >();
}

BOOST_AUTO_TEST_CASE(tests_rtree_function_box2f)
{
    std::cout << "tests/rtree_function_box2f\n";

    namespace bg = boost::geometry;
    namespace bgi = bg::index;

    typedef bg::model::point<float, 2, bg::cs::cartesian> P;
    typedef bg::model::box<P> B;
    typedef B val_t;

    tests_rtree_function< val_t, bgi::linear<4, 2> >();
    tests_rtree_function< val_t, bgi::quadratic<4, 2> >();
    tests_rtree_function< val_t, bgi::rstar<4, 2> >();
}

//BOOST_AUTO_TEST_CASE(tests_rtree_function_point2f)
//{
//    std::cout << "tests/rtree_function_point2f\n";
//
//    namespace bg = boost::geometry;
//    namespace bgi = bg::index;
//
//    typedef bg::model::point<float, 2, bg::cs::cartesian> P;
//    typedef P val_t;
//
//    tests_rtree_function< val_t, bgi::linear<4, 2> >();
//    tests_rtree_function< val_t, bgi::quadratic<4, 2> >();
//    tests_rtree_function< val_t, bgi::rstar<4, 2> >();
//}


  //  std::cout << "std::pair<Box, int>\n";
  //  {
  //      typedef bg::model::point<float, 2, bg::cs::cartesian> P;
  //      typedef bg::model::box<P> B;
  //      typedef std::pair<B, int> V;

  //      bgi::rtree<V, Options> t;
  //      bgi::insert(t, V(B(P(0, 0), P(1, 1)), 0));
  //      bgi::insert(t, V(B(P(2, 2), P(3, 3)), 1));
  //      bgi::insert(t, V(B(P(4, 4), P(5, 5)), 2));
  //      bgi::insert(t, V(B(P(6, 6), P(7, 7)), 3));
  //      bgi::insert(t, V(B(P(8, 8), P(9, 9)), 4));
  //      std::cerr << t;
  //  }

  //  std::cout << "-------------------------------------------------\n";
  //  std::cout << "-------------------------------------------------\n";

  //  std::cout << "boost::shared_ptr< std::pair<Box, int> >\n";
  //  {
  //      typedef bg::model::point<float, 2, bg::cs::cartesian> P;
  //      typedef bg::model::box<P> B;
  //      
  //      typedef boost::shared_ptr< std::pair<B, int> > V;

  //      V v1( new std::pair<B, int>(B(P(0, 0), P(1, 1)), 0) );
  //      V v2( new std::pair<B, int>(B(P(2, 2), P(3, 3)), 1) );
  //      V v3( new std::pair<B, int>(B(P(4, 4), P(5, 5)), 2) );
  //      V v4( new std::pair<B, int>(B(P(6, 6), P(7, 7)), 3) );
  //      V v5( new std::pair<B, int>(B(P(8, 8), P(9, 9)), 4) );

  //      bgi::rtree<V, Options> t;
  //      bgi::insert(t, v1);
  //      bgi::insert(t, v2);
  //      bgi::insert(t, v3);
  //      bgi::insert(t, v4);
  //      bgi::insert(t, v5);
  //      std::cerr << t;
  //  }

  //  std::cout << "-------------------------------------------------\n";
  //  std::cout << "-------------------------------------------------\n";

  //  std::cout << "std::map<int, Box>::iterator\n";
  //  {
  //      typedef bg::model::point<float, 2, bg::cs::cartesian> P;
  //      typedef bg::model::box<P> B;
  //      
  //      typedef std::map<int, B>::iterator V;

  //      std::map<int, B> m;
  //      m.insert(std::pair<int, B>(0, B(P(0, 0), P(1, 1))));
  //      m.insert(std::pair<int, B>(1, B(P(2, 2), P(3, 3))));
  //      m.insert(std::pair<int, B>(2, B(P(4, 4), P(5, 5))));
  //      m.insert(std::pair<int, B>(3, B(P(6, 6), P(7, 7))));
  //      m.insert(std::pair<int, B>(4, B(P(8, 8), P(9, 9))));

  //      bgi::rtree<V, Options> t;
  //      V vit = m.begin();
  //      bgi::insert(t, vit++);
  //      bgi::insert(t, vit++);
  //      bgi::insert(t, vit++);
  //      bgi::insert(t, vit++);
  //      bgi::insert(t, vit);
  //      std::cerr << t;
  //  }

  //  std::cout << "-------------------------------------------------\n";
  //  std::cout << "-------------------------------------------------\n";

  //  std::cout << "size_t\n";
  //  {
  //      typedef bg::model::point<float, 2, bg::cs::cartesian> P;
  //      typedef bg::model::box<P> B;

  //      typedef size_t V;
  //      typedef bgi::translator::index<std::vector<B> > Tr;

  //      std::vector<B> v;
  //      v.push_back(B(P(0, 0), P(1, 1)));
  //      v.push_back(B(P(2, 2), P(3, 3)));
  //      v.push_back(B(P(4, 4), P(5, 5)));
  //      v.push_back(B(P(6, 6), P(7, 7)));
  //      v.push_back(B(P(8, 8), P(9, 9)));

		//Tr tr(v);
  //      bgi::rtree<V, Options, Tr> t(tr);

  //      bgi::insert(t, 0u);
  //      bgi::insert(t, 1u);
  //      bgi::insert(t, 2u);
  //      bgi::insert(t, 3u);
  //      bgi::insert(t, 4u);
  //      std::cerr << t;
  //  }

#endif // TESTS_RTREE_FUNCTION_HPP
