#include <iostream>
#include <fstream>

#include <boost/geometry/extensions/index/rtree/rtree.hpp>

namespace bg = boost::geometry;
namespace bgi = bg::index;

typedef bg::model::point<double, 2, bg::cs::cartesian> P;
typedef bg::model::box<P> B;
typedef bgi::rtree<
    B,
    bgi::linear<4, 2>
> RTB;
typedef bgi::rtree<
    P,
    bgi::linear<4, 2>
> RTP;

int main()
{
    RTB rtb;
    rtb.insert(B(P(0,0),P(1,1)));
    rtb.insert(B(P(1,0),P(2,1)));
    rtb.insert(B(P(2,0),P(3,1)));
    rtb.insert(B(P(2,2),P(3,3)));
    rtb.insert(B(P(4,4),P(5,5)));

    std::vector<B> res;
    B bb = rtb.box();
    rtb.query(bb, std::back_inserter(res));

    RTP rtp;
    rtp.insert(P(0,0));
    rtp.insert(P(1,0));
    rtp.insert(P(2,0));
    rtp.insert(P(2,2));
    rtp.insert(P(4,4));

    std::vector<P> resp;
    B bp = rtp.box();
    rtp.query(bp, std::back_inserter(resp));

    int a = 10;
}
