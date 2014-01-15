#ifndef BOOST_GEOMETRY_MULTI_ALGORITHMS_DETAIL_BICHROMATIC_CLOSEST_PAIR_HPP
#define BOOST_GEOMETRY_MULTI_ALGORITHMS_DETAIL_BICHROMATIC_CLOSEST_PAIR_HPP

#include <boost/geometry/algorithms/distance.hpp>
#include <boost/geometry/strategies/distance.hpp>
#include <boost/geometry/multi/algorithms/distance.hpp>

#if (__cplusplus > 199711L)
#include <random>
#endif

#include <cmath>
#include <map>
#include <boost/range.hpp>
//#include <cstdlib>



namespace boost { namespace geometry
{

namespace detail { namespace distance
{


#if (__cplusplus > 199711L)
template <typename IntegerType, typename RealType = double>
class random_integer_generator
{
public:
    random_integer_generator(unsigned int seed = 0)
      : generator(), distribution(0, 1)
    {
        generator.seed(seed);
    }

    IntegerType operator()(IntegerType const& low,
                           IntegerType const& high)
    {
        RealType r = distribution(generator);
        r = low + r * (high - low + 1);
        return static_cast<IntegerType>(std::floor(r));
    }

private:
    std::default_random_engine generator;
    std::uniform_real_distribution<RealType> distribution;
};
#else
template <typename IntegerType, typename RealType = double>
class random_integer_generator
{
public:
    random_integer_generator(unsigned int seed = 0)
    {
      srand(seed);
    }

    IntegerType operator()(IntegerType const& low,
                           IntegerType const& high)
    {
        IntegerType i = rand() % (high - low + 1);
        i += low;
        return i;
    }
};
#endif



template
<
    typename MultiPoint1,
    typename MultiPoint2,
    typename PointToPointStrategy = typename detail::distance::default_strategy
    <
        typename point_type<MultiPoint1>::type,
        typename point_type<MultiPoint2>::type
    >::type
>
struct bichromatic_closest_pair
{
    template <typename Pair>
    struct pair_less
    { 
        inline bool operator()(Pair const& pair1, Pair const& pair2) const
        {
            if ( pair1.first != pair2.first )
            {
                return pair1.first < pair2.first;
            }
            return pair1.second < pair2.second;
        }
    };


    typedef PointToPointStrategy Strategy;
    typedef std::size_t size_type;
    typedef std::pair<size_type,size_type> Bin_id;
    typedef pair_less<Bin_id> Bin_id_less;

    template <typename MultiPoint>
    struct Bin {
        Bin_id id;
        MultiPoint mp;
    };


    typedef typename strategy::distance::services::return_type
        <
            Strategy,
            typename point_type<MultiPoint1>::type,
            typename point_type<MultiPoint2>::type
        >::type return_type;


    static inline return_type
    distance_from_random_sample(MultiPoint1 const& multipoint1,
                                MultiPoint2 const& multipoint2,
                                Strategy const& strategy)
    {
        random_integer_generator<size_type> generator;

        size_type num_samples =
            boost::size(multipoint1) + boost::size(multipoint2);
        
        size_type i = generator(0, boost::size(multipoint1) - 1);
        size_type j = generator(0, boost::size(multipoint2) - 1);
        return_type dmin = strategy.apply(*(boost::begin(multipoint1) + i),
                                          *(boost::begin(multipoint2) + j));

#ifdef PRINT_DEBUG
        std::cout << "choosing points: "
                  << geometry::wkt(*(boost::begin(multipoint1) + i))
                  << " "
                  << geometry::wkt(*(boost::begin(multipoint2) + j))
                  << std::endl;
        std::cout << "distance: " << dmin << std::endl;
#endif
        return_type d;
        for (size_type k = 1; k < num_samples; ++k) {
            i = generator(0, boost::size(multipoint1) - 1);
            j = generator(0, boost::size(multipoint2) - 1);
            d = strategy.apply(*(boost::begin(multipoint1) + i),
                               *(boost::begin(multipoint2) + j));
#ifdef PRINT_DEBUG
            std::cout << "choosing points: "
                      << geometry::wkt(*(boost::begin(multipoint1) + i))
                      << " "
                      << geometry::wkt(*(boost::begin(multipoint2) + j))
                      << std::endl;
            std::cout << "distance: " << d << std::endl;
#endif
            if ( d < dmin )
            {
                dmin = d;
            }
        }
        return dmin;
    }

    template <typename Point, int Dimension>
    struct coordinate_less
    {
        bool operator()(Point const& p, Point const& q) const
        {
            return geometry::get<Dimension>(p) < geometry::get<Dimension>(q);
        }
    };

    template <typename MP1, typename MP2>
    static inline typename point_type<MP1>::type
    compute_origin(MP1 const& mp1, MP2 const& mp2)
    {
        typedef typename point_type<MP1>::type P1;
        typedef typename point_type<MP2>::type P2;
        typedef typename coordinate_type<P1>::type C1;
        typedef typename coordinate_type<P2>::type C2;

        C1 xmin1 = geometry::get<0>(*std::min_element(boost::begin(mp1),
                                                      boost::end(mp1),
                                                      coordinate_less<P1,0>()
                                                      )
                                    );
        C2 xmin2 = geometry::get<0>(*std::min_element(boost::begin(mp2),
                                                      boost::end(mp2),
                                                      coordinate_less<P2,0>()
                                                      )
                                    );

        C1 ymin1 = geometry::get<1>(*std::min_element(boost::begin(mp1),
                                                      boost::end(mp1),
                                                      coordinate_less<P1,1>()
                                                      )
                                    );
        C2 ymin2 = geometry::get<1>(*std::min_element(boost::begin(mp2),
                                                      boost::end(mp2),
                                                      coordinate_less<P2,1>()
                                                      )
                                    );

        return P1(std::min(xmin1, xmin2), std::min(ymin1, ymin2));
    }


    template <typename Point, typename Origin, typename Distance>
    static inline Bin_id get_bin_id(Point const& p, Origin const& o,
                                    Distance const& d)
    {
        std::size_t x_id =
            std::floor( (geometry::get<0>(p) - geometry::get<0>(o)) / d );

        std::size_t y_id =
            std::floor( (geometry::get<1>(p) - geometry::get<1>(o)) / d );
        return std::make_pair(x_id, y_id);
    }

    template
    <
        typename MultiPoint, typename Bins,
        typename Origin, typename Distance
    >
    static inline void assign_to_bins(MultiPoint const& mp,
                                      Bins& bins,
                                      Origin const& o,
                                      Distance const& d)
    {
        BOOST_AUTO_TPL(it, boost::begin(mp));
        for (; it != boost::end(mp); ++it)
        {
            Bin_id id = get_bin_id(*it, o, d);
            typename Bins::iterator bin_it = bins.find(id);
            if ( bin_it == bins.end() )
            {
                MultiPoint new_bin;
                new_bin.push_back(*it);
                bins[id] = new_bin;
            } else {
                bin_it->second.push_back(*it);
            }
        }
    }


    static void print_bin_id(Bin_id const& id)
    {
        std::cout << "(" << id.first << "," << id.second << "): ";
    }

    template <typename Bin>
    static void print_bin(Bin const& bin)
    {
        print_bin_id(bin.first);
        std::cout << geometry::wkt(bin.second) << std::endl;
    }

    template <typename Bins>
    static void print_bins(Bins const& bins, std::string const& str)
    {
        std::cout << std::endl;
        std::cout << str << std::endl;
        for (typename Bins::const_iterator it = bins.begin();
             it != bins.end(); ++it)
        {
            print_bin(*it);
        }
    }

    template
    <
        typename RedBin,
        typename BlueBins,
        typename Distance,
        typename Strategy
    >
    static inline void eval_distance_to_blue_bin(RedBin const& red_bin,
                                                 BlueBins const& blue_bins,
                                                 Distance& dmin,
                                                 int xoffset, int yoffset,
                                                 Strategy const& strategy)
    {
        if ( (red_bin.first.first == 0 && xoffset < 0) ||
             (red_bin.first.second == 0 && yoffset < 0) )
        {
            return;
        }

        Bin_id blue_id = std::make_pair(red_bin.first.first + xoffset,
                                        red_bin.first.second + yoffset);
        
        typedef typename BlueBins::const_iterator BBIT;
        BBIT blue_bin = blue_bins.find(blue_id);

#ifdef PRINT_DEBUG
            std::cout << "checking blue bin: ";
            print_bin_id(blue_id);
            std::cout << std::endl;
#endif
        if ( blue_bin != blue_bins.end() )
        {
            Distance d = detail::distance::distance_multi_to_multi
                <
                    MultiPoint1, MultiPoint2, Strategy
                >::apply(red_bin.second, blue_bin->second, strategy);
            if ( d < dmin ) { dmin = d; }
        }
        else
        {
#ifdef PRINT_DEBUG
            std::cout << "bin is empty" << std::endl;
#endif
        }
    }


    static inline return_type apply(MultiPoint1 const& multipoint1,
                                    MultiPoint2 const& multipoint2,
                                    Strategy const& strategy)
    {
        return_type dmin = distance_from_random_sample(multipoint1,
                                                       multipoint2,
                                                       strategy);

        typedef typename std::map<Bin_id, MultiPoint1, Bin_id_less> Red_bins;
        typedef typename std::map<Bin_id, MultiPoint2, Bin_id_less> Blue_bins;
        Red_bins red_bins;
        Blue_bins blue_bins;

        typename point_type<MultiPoint1>::type origin
            = compute_origin(multipoint1, multipoint2);

#ifdef PRINT_DEBUG
        std::cout << "My origin: " << geometry::wkt(origin) << std::endl;
#endif

        assign_to_bins(multipoint1, red_bins, origin, dmin);
        assign_to_bins(multipoint2, blue_bins, origin, dmin);

#ifdef PRINT_DEBUG
        print_bins(red_bins, "Red Bins");
        print_bins(blue_bins, "Blue Bins");

        std::cout << std::endl << std::endl;
#endif
        for (typename Red_bins::const_iterator it = red_bins.begin();
             it != red_bins.end(); ++it)
        {
            eval_distance_to_blue_bin(*it, blue_bins, dmin, -1, -1, strategy);
            eval_distance_to_blue_bin(*it, blue_bins, dmin, -1,  0, strategy);
            eval_distance_to_blue_bin(*it, blue_bins, dmin, -1,  1, strategy);
            eval_distance_to_blue_bin(*it, blue_bins, dmin,  0, -1, strategy);
            eval_distance_to_blue_bin(*it, blue_bins, dmin,  0,  0, strategy);
            eval_distance_to_blue_bin(*it, blue_bins, dmin,  0,  1, strategy);
            eval_distance_to_blue_bin(*it, blue_bins, dmin,  1, -1, strategy);
            eval_distance_to_blue_bin(*it, blue_bins, dmin,  1,  0, strategy);
            eval_distance_to_blue_bin(*it, blue_bins, dmin,  1,  1, strategy);
        }
        return dmin;
    }

    static inline return_type apply(MultiPoint1 const& multipoint1,
                                    MultiPoint2 const& multipoint2)
    {
        typedef typename detail::distance::default_strategy
            <
                typename point_type<MultiPoint1>::type,
                typename point_type<MultiPoint2>::type
            >::type Strategy;
        
        return apply(multipoint1, multipoint2, Strategy());
    }
};






}} // namespace detail::distance

}} // namespace boost::geometry


#endif // BOOST_GEOMETRY_MULTI_ALGORITHMS_DETAIL_BICHROMATIC_CLOSEST_PAIR_HPP
