#ifndef TESTS_TRANSLATORS_HPP
#define TESTS_TRANSLATORS_HPP

#include <boost/geometry/geometry.hpp>
#include <boost/geometry/geometries/polygon.hpp>

#include <boost/geometry/extensions/index/rtree/rtree.hpp>

#include <vector>
#include <map>

#include <boost/shared_ptr.hpp>
#include <boost/scoped_ptr.hpp>

template <typename Indexable>
struct tests_translators_val
{
    tests_translators_val(Indexable const& ii)
        : i(ii)
    {}

	Indexable const& get_box() const
	{
		return i;
	}

	bool operator==(tests_translators_val const& v2)
	{
		return boost::geometry::equals(i, v2.i);
	}

	Indexable i;
};

BOOST_AUTO_TEST_CASE(tests_translators)
{
#ifdef TEST_PRINT_INFO
    std::cout << "tests/translators\n";
#endif

    namespace bg = boost::geometry;
    namespace bgm = bg::model;
    namespace bgi = bg::index;
    namespace bgit = bgi::translator;

	typedef bgm::point<float, 2, bg::cs::cartesian> P;
    typedef bgm::box<P> B;

    bgit::def< P > p;
    bgit::def< P* > pp;
    bgit::def< std::pair<int, P>* > ppip;
    bgit::def< boost::shared_ptr<P> > sp;
    bgit::def< std::vector<P>::iterator > ip;
    bgit::def< std::map<int, P>::iterator > mip;
    bgit::def< std::pair<int, P> > pip;
    bgit::def< boost::shared_ptr< std::pair<int, P> > > spip;
    bgit::def< boost::shared_ptr< std::pair<P, int> > > sppi;
    bgit::def< boost::scoped_ptr< std::pair<P, int> > > scppi;
    bgit::def< boost::scoped_ptr< std::pair<int, P> > > scpip;

    P tmp_p(2, 3);
    boost::shared_ptr<P> tmp_sp(new P(2, 3));
    std::vector<P> tmp_v(1, P(2, 3));
    std::map<int, P> tmp_m;
    tmp_m.insert(std::pair<int, P>(0, P(2, 3)));
    std::pair<int, P> tmp_pip(0, P(2, 3));
    boost::shared_ptr< std::pair<int, P> > tmp_spip(new std::pair<int, P>(0, P(2, 3)));
    boost::shared_ptr< std::pair<P, int> > tmp_sppi(new std::pair<P, int>(P(2, 3), 0));
    boost::scoped_ptr< std::pair<int, P> > tmp_scpip(new std::pair<int, P>(0, P(2, 3)));
    boost::scoped_ptr< std::pair<P, int> > tmp_scppi(new std::pair<P, int>(P(2, 3), 0));

    BOOST_CHECK( bg::equals(tmp_p, p(tmp_p)) );
    BOOST_CHECK( bg::equals(tmp_p, pp(&tmp_p)) );
    BOOST_CHECK( bg::equals(tmp_p, ppip(&tmp_pip)) );
    BOOST_CHECK( bg::equals(tmp_p, sp(tmp_sp)) );
    BOOST_CHECK( bg::equals(tmp_p, ip(tmp_v.begin())) );
    BOOST_CHECK( bg::equals(tmp_p, mip(tmp_m.begin())) );
    BOOST_CHECK( bg::equals(tmp_p, pip(tmp_pip)) );
    BOOST_CHECK( bg::equals(tmp_p, spip(tmp_spip)) );
    BOOST_CHECK( bg::equals(tmp_p, sppi(tmp_sppi)) );
    BOOST_CHECK( bg::equals(tmp_p, scpip(tmp_scpip)) );
    BOOST_CHECK( bg::equals(tmp_p, scppi(tmp_scppi)) );
    
    //bgit::def<int> d;					// error
    //bgit::def< bgm::segment<P> > d;	// error

    B tmp_b(P(2, 3), P(4, 5));
    std::pair<bgm::polygon<P>, B> tmp_ppb =
        std::make_pair(bgm::polygon<P>(), tmp_b);
    std::pair<B, bgm::polygon<P> > tmp_pbp =
        std::make_pair(tmp_b, bgm::polygon<P>());

    bgit::def< std::pair<bgm::polygon<P>, B> > ppb;
    bgit::def< std::pair<B, bgm::polygon<P> > > pbp;

    BOOST_CHECK( bg::equals(tmp_b, ppb(tmp_ppb)) );
    BOOST_CHECK( bg::equals(tmp_b, pbp(tmp_pbp)) );
}

#endif // TESTS_TRANSLATORS_HPP
