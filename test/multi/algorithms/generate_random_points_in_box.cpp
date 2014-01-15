#include <iostream>
#include <fstream>
#include <cstdlib>
#include <cassert>
#include <utility>

typedef std::pair<double,double> point;

struct random_point_generator
{
    double fmin, fmax;

    random_point_generator(double fmin = 0, double fmax = 1, int seed = 0)
        : fmin(fmin), fmax(fmax)
    {
        srand(seed);
    }

    double random_coordinate() const
    {
        double f = static_cast<double>(rand()) / RAND_MAX;
        return fmin + f * (fmax - fmin);
    }

    point operator*() const
    {
        double x = random_coordinate();
        double y = random_coordinate();
        return std::make_pair(x, y);
    }
};


int main(int argc, char** argv)
{
    assert( argc >= 4 );
    int n = atoi(argv[1]);
    int m = atoi(argv[2]);

    std::ofstream ofs( argv[3] );
    assert( ofs );
    
    ofs << n << " " << m << std::endl;

    random_point_generator g;
    for (int i = 0; i < n; ++i)
    {
        point p = *g;
        ofs << p.first << " " << p.second << std::endl;
    }

    for (int i = 0; i < m; ++i)
    {
        point p = *g;
        ofs << p.first << " " << p.second << std::endl;
    }

    ofs.close();

    return 0;
}
