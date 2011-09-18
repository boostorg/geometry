#ifndef TESTS_RTREE_FUNCTION_HPP
#define TESTS_RTREE_FUNCTION_HPP

#include <boost/geometry/geometry.hpp>

#include <boost/geometry/extensions/index/rtree/rtree.hpp>
#include <boost/geometry/extensions/index/translator/index.hpp>

#include <boost/geometry/extensions/index/rtree/visitors/print.hpp>

#include <boost/range/algorithm.hpp>
#include <boost/foreach.hpp>

#include <map>

template <typename Options>
void tests_rtree_function_box3f_impl()
{
    namespace bg = boost::geometry;
    namespace bgi = bg::index;

    typedef bg::model::point<float, 3, bg::cs::cartesian> P;
    typedef bg::model::box<P> B;
    typedef B val_t;

    bgi::rtree<val_t, Options> t;

    bgi::insert(t, B(P(0, 0, 0), P(1, 1, 1)));
    bgi::insert(t, B(P(2, 2, 2), P(3, 3, 3)));
    bgi::insert(t, B(P(4, 4, 4), P(5, 5, 5)));
    bgi::insert(t, B(P(6, 6, 6), P(7, 7, 7)));
    bgi::insert(t, B(P(8, 8, 8), P(9, 9, 9)));
    
    B g(P(4.5f, 4.5f, 4.5f), P(5.5f, 5.5f, 5.5f));
    B gi(P(4, 4, 4), P(5, 5, 5));
    std::vector<val_t> result;

    size_t f = bgi::query(t, g, std::back_inserter(result));
    BOOST_CHECK( 1 == f && 1 == result.size() && bg::equals(result[0], gi) );

    result.clear();
    f = bgi::query(t, bgi::intersects(g), std::back_inserter(result));
    BOOST_CHECK( 1 == f && 1 == result.size() && bg::equals(result[0], gi) );

    BOOST_CHECK( bg::overlaps(g, gi) );

    result.clear();
    f = bgi::query(t, bgi::overlaps(g), std::back_inserter(result));
    BOOST_CHECK( 1 == f && 1 == result.size() && bg::equals(result[0], gi) );

    result.clear();
    f = bgi::query(t, bgi::within(g), std::back_inserter(result));
    BOOST_CHECK( 0 == f && 0 == result.size() );

    result.clear();
    f = bgi::query(t, bgi::covered_by(g), std::back_inserter(result));
    BOOST_CHECK( 0 == f && 0 == result.size() );

    P p(3.9f, 3.5f, 3.5f);
    val_t result_val;

    f = bgi::nearest(t, p, result_val);
    BOOST_CHECK( 1 == f && bg::equals(result_val, gi) );
}

BOOST_AUTO_TEST_CASE(tests_rtree_function_box3f)
{
    std::cout << "tests/rtree_function_box3f\n";

    namespace bg = boost::geometry;
    namespace bgi = bg::index;

    tests_rtree_function_box3f_impl< bgi::linear<4, 2> >();
    tests_rtree_function_box3f_impl< bgi::quadratic<4, 2> >();
    tests_rtree_function_box3f_impl< bgi::rstar<4, 2> >();
}


  //  std::cout << "Boxes2d\n";
  //  {
  //      typedef bg::model::point<float, 2, bg::cs::cartesian> P;
  //      typedef bg::model::box<P> B;

  //      bgi::rtree<B, Options> t;
  //      bgi::insert(t, B(P(0, 0), P(1, 1)));
  //      bgi::insert(t, B(P(2, 2), P(3, 3)));
  //      bgi::insert(t, B(P(4, 4), P(5, 5)));
  //      bgi::insert(t, B(P(6, 6), P(7, 7)));
  //      bgi::insert(t, B(P(8, 8), P(9, 9)));
  //      std::cerr << t;
  //  }

  //  std::cout << "-------------------------------------------------\n";
  //  std::cout << "-------------------------------------------------\n";

  //  std::cout << "Points\n";
  //  {
  //      typedef bg::model::point<float, 2, bg::cs::cartesian> P;
  //      typedef bg::model::box<P> B;

  //      bgi::rtree<P, Options> t;
  //      bgi::insert(t, P(0, 0));
  //      bgi::insert(t, P(2, 2));
  //      bgi::insert(t, P(4, 4));
  //      bgi::insert(t, P(6, 6));
  //      bgi::insert(t, P(8, 8));
  //      std::cerr << t;
  //  }

  //  std::cout << "-------------------------------------------------\n";
  //  std::cout << "-------------------------------------------------\n";

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
