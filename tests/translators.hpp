#ifndef TESTS_TRANSLATORS_HPP
#define TESTS_TRANSLATORS_HPP

#include <boost/geometry/geometry.hpp>
#include <boost/geometry/extensions/index/rtree/rtree.hpp>

#include <boost/geometry/extensions/index/translator/def.hpp>
#include <boost/geometry/extensions/index/translator/index.hpp>

#include <vector>
#include <map>

#include <boost/shared_ptr.hpp>
#include <boost/scoped_ptr.hpp>

void tests_translators_hpp()
{
	std::cout << "tests/translators.hpp\n";

    typedef boost::geometry::model::point<float, 2, boost::geometry::cs::cartesian> P;
    typedef boost::geometry::model::box<P> B;
    typedef boost::geometry::index::rtree<B> I;

    using namespace boost::geometry;

    index::translator::def<P> p;
    index::translator::def<P*> pp;
    index::translator::def<std::pair<int, P>*> pip;
    index::translator::def< boost::shared_ptr<P> > sp;
    index::translator::def< std::vector<P>::iterator > ip;
    index::translator::def< std::map<int, P>::iterator > mip;
    index::translator::def< std::pair<P, size_t> > ppi;
    index::translator::def< boost::shared_ptr< std::pair<int, P> > > spip;
    index::translator::def< boost::shared_ptr< std::pair<P, int> > > sppi;
    index::translator::def< boost::scoped_ptr< std::pair<P, int> > > scppi;
    index::translator::def< boost::scoped_ptr< std::pair<int, P> > > scpip;

    P tmp_p;
    boost::shared_ptr<P> tmp_sp(new P());
    std::vector<P> tmp_v(1);
    std::map<int, P> tmp_m;
    tmp_m.insert(std::pair<int, P>(0, P()));
    std::pair<int, P> tmp_pip;
    boost::shared_ptr< std::pair<int, P> > tmp_spip(new std::pair<int, P>(0, P()));
    boost::shared_ptr< std::pair<P, int> > tmp_sppi(new std::pair<P, int>(P(), 0));
    boost::scoped_ptr< std::pair<int, P> > tmp_scpip(new std::pair<int, P>(0, P()));
    boost::scoped_ptr< std::pair<P, int> > tmp_scppi(new std::pair<P, int>(P(), 0));

    tmp_p = p(P());
    tmp_p = pp(&tmp_p);
    tmp_p = pip(&tmp_pip);
    tmp_p = sp(tmp_sp);
    tmp_p = ip(tmp_v.begin());
    tmp_p = mip(tmp_m.begin());
    tmp_p = ppi(std::pair<P, size_t>(P(), 0));
    tmp_p = spip(tmp_spip);
    tmp_p = sppi(tmp_sppi);
    tmp_p = scpip(tmp_scpip);
    tmp_p = scppi(tmp_scppi);

    //index::translator::def<int> d;					// error
    //index::translator::def< model::segment<P> > d;	// error

    index::translator::def< std::pair<model::polygon<P>, B> > d;
    index::translator::def< std::pair<B, model::polygon<P> > > dd;

    B tmp_b;
    tmp_b = d( std::pair<model::polygon<P>, B>() );
    tmp_b = dd( std::pair<B, model::polygon<P> >() );
}

#endif // TESTS_TRANSLATORS_HPP
