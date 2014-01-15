#ifndef BOOST_GEOMETRY_MULTI_ALGORITHMS_DETAIL_DISTANCE_RANDOM_INTEGER_GENERATOR_H
#define BOOST_GEOMETRY_MULTI_ALGORITHMS_DETAIL_DISTANCE_RANDOM_INTEGER_GENERATOR_H

#if (__cplusplus > 199711L)
#  include <random>
#  include <cmath>
#else
#  include <cstdlib>
#endif



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


}} // namespace detail::distance



}} // namespace boost::geometry


#endif // BOOST_GEOMETRY_MULTI_ALGORITHMS_DETAIL_DISTANCE_RANDOM_INTEGER_GENERATOR_H
