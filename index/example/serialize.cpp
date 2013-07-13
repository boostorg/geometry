// Boost.Geometry Index
// Additional tests

// Copyright (c) 2011-2013 Adam Wulkiewicz, Lodz, Poland.

// Use, modification and distribution is subject to the Boost Software License,
// Version 1.0. (See accompanying file LICENSE_1_0.txt or copy at
// http://www.boost.org/LICENSE_1_0.txt)

// WARNING! This code is not fully functional!

#include <iostream>
#include <fstream>

#define BOOST_GEOMETRY_INDEX_DETAIL_EXPERIMENTAL
#include <boost/geometry/index/rtree.hpp>

#include <boost/archive/binary_oarchive.hpp>
#include <boost/archive/binary_iarchive.hpp>

#include <boost/foreach.hpp>

namespace boost { namespace serialization {



}} // namespace boost::serialization

int main()
{
    namespace bg = boost::geometry;
    namespace bgi = bg::index;

    typedef bg::model::point<double, 2, bg::cs::cartesian> P;
    typedef bg::model::box<P> B;
    typedef bgi::rtree<B, bgi::linear<16, 4> > RT;
    //typedef bgi::rtree<B, bgi::quadratic<8, 3> > RT;
    //typedef bgi::rtree<B, bgi::rstar<8, 3> > RT;

    RT tree;

    //insert values
    {
        for ( double x = 0 ; x < 100 ; x += 10 )
            for ( double y = 0 ; y < 100 ; y += 10 )
                tree.insert(B(P(x, y), P(x+1, y+1)));
    }

    // save
    {
        std::ofstream ofs("filename", std::ios::binary);
        boost::archive::binary_oarchive oa(ofs);
        oa << tree;
    }

    return 0;
}
