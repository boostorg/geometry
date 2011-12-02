
#include <iostream>
#include <deque>

#include <boost/geometry/extensions/index/rtree/rtree.hpp>
#include <boost/geometry/extensions/index/inserter.hpp>

#include <boost/geometry/geometries/point.hpp>


typedef boost::geometry::model::point<double, 2, boost::geometry::cs::cartesian>
        BGCartesianPoint;
typedef boost::geometry::model::box<BGCartesianPoint> 
        BGCartesianBox;
typedef boost::geometry::model::segment<BGCartesianPoint>
        BGCartesianSegment;
namespace bgi = boost::geometry::index;


struct myTranslator
{
  typedef BGCartesianBox result_type;

  result_type operator()(BGCartesianPoint const& seg) const
  {
    result_type envelope; 
    boost::geometry::envelope(seg, envelope);
    return envelope;
  }

  bool equals(BGCartesianPoint const& go1, BGCartesianPoint const& go2) const
  {
    return (go1.get<0>() == go2.get<0>()) && 
           (go1.get<1>() == go2.get<1>());
  }
};

typedef bgi::rtree
         <BGCartesianPoint,
          bgi::rstar<32, 8>,
          myTranslator>
        BGRTree;

int main()
{
  BGRTree rTree;
  for (int i = 0; i < 50; ++i)
  {
    BGCartesianPoint p(i, i);
    rTree.insert(p);

    BGCartesianBox envelope(BGCartesianPoint(i-0.5, i-0.5), 
                            BGCartesianPoint(i+0.5, i+0.5));

    std::deque<BGCartesianPoint> dq;

    if (rTree.query(envelope, std::back_inserter(dq)))
      std::cout << "Retrieved inserted object nr. " << i << "\n";
    else
      std::cout << "Could not retrieve inserted object nr. " << i << "\n";
  }

  std::cout << "Total objects: " << rTree.size();
  std::cin.get();
  return 0;
}