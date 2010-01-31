// Boost.Geometry (aka GGL, Generic Geometry Library)
//
// Copyright Barend Gehrels 2007-2009, Geodan, Amsterdam, the Netherlands.
// Copyright Bruno Lalande 2008, 2009
// Use, modification and distribution is subject to the Boost Software License,
// Version 1.0. (See accompanying file LICENSE_1_0.txt or copy at
// http://www.boost.org/LICENSE_1_0.txt)

#ifndef BOOST_GEOMETRY_ALGORITHMS_INTERSECTION_POINT_HPP
#define BOOST_GEOMETRY_ALGORITHMS_INTERSECTION_POINT_HPP


#include <vector>


#include <boost/geometry/core/access.hpp>
#include <boost/geometry/core/coordinate_dimension.hpp>

#include <boost/geometry/strategies/distance.hpp>
#include <boost/geometry/strategies/distance_result.hpp>

#include <boost/geometry/algorithms/overlay/segment_identifier.hpp>


#ifdef BOOST_GEOMETRY_USE_MSM
#  include <boost/geometry/algorithms/overlay/msm_state.hpp>
#endif


namespace boost { namespace geometry
{

#ifndef DOXYGEN_NO_DETAIL
namespace detail { namespace intersection {


#if ! defined(BOOST_GEOMETRY_USE_MSM)

class visit_info
{
private :
    static const int NONE = 0;
    static const int STARTED = 1;
    static const int VISITED = 2;
    static const int FINISHED = 3;
    //static const int WITHIN = 4;

    int visit_code;

public:
    inline visit_info()
        : visit_code(0)
    {}

    void set_visited() { visit_code = VISITED; }
    void set_started() { visit_code = STARTED; }
    void set_finished() { visit_code = FINISHED; }

    bool none() const { return visit_code == NONE; }
    bool visited() const { return visit_code == VISITED; }

#ifdef BOOST_GEOMETRY_DEBUG_INTERSECTION
    friend std::ostream& operator<<(std::ostream &os, visit_info const& v)
    {
        if (v.visit_code != 0)
        {
            os << " VIS: " << int(v.visit_code);
        }
        return os;
    }
#endif

};

#else

class visit_info
{
private :
#ifndef USE_MSM_MINI
    mutable
#endif
        traverse_state state;

public :
    inline visit_info()
    {
        state.start();
    }

    void set_visited() { state.process_event(visit()); }
    void set_started() { state.process_event(starting()); }
    void set_finished() { state.process_event(finish()); }

#ifdef USE_MSM_MINI
    bool none() const { return state.flag_none(); }
    bool visited() const { return state.flag_visited(); }
#else
    bool none() const { return state.is_flag_active<is_init>(); }
    bool visited() const { return state.is_flag_active<is_visited>(); }
#endif

#ifdef BOOST_GEOMETRY_DEBUG_INTERSECTION
    friend std::ostream& operator<<(std::ostream &os, visit_info const& v)
    {
        return os;
    }
#endif
};
#endif



template<typename P>
struct intersection_info
{
    typedef P point_type;
    typedef typename distance_result<P, P>::type distance_type;

    inline intersection_info()
        : travels_to_vertex_index(-1)
        , travels_to_ip_index(-1)
        , next_ip_index(-1)
        , distance(geometry::make_distance_result<distance_type>(0))
        , direction(0)
        , how('?')
        , arrival(0)
        , opposite(false)
        , flagged(false)
    {}

    // Point to which the segment from IP is directing (TO-point)
    // If they intersect on their "arrival" points, it is the FROM-point.
    P other_point;

    // Identifier of this segment (source,segment,ring,multi)
    segment_identifier seg_id;

    // Identify the segment where it was intersected with to form this IP
    segment_identifier other_id;


    // vertex to which is free travel after this IP,
    // so from "segment_index+1" to "travels_to_vertex_index", without IP-s,
    // can be -1
    int travels_to_vertex_index;

    // same but now IP index, so "next IP index" but not on THIS segment
    int travels_to_ip_index;

    // index of next IP on this segment, -1 if there is no one
    int next_ip_index;

    distance_type distance; // distance-measurement from segment.first to IP

    // 1: left, -1: right, 0: collinear
    int direction;

    // Information about how intersection is done
    char how;

    // 1: arrived at IP, -1: departs from IP, 0: crosses IP
    int arrival;

    bool opposite;

    visit_info visit_state;
    side_info sides;

    bool flagged; // flagged for deletion


#ifdef BOOST_GEOMETRY_DEBUG_INTERSECTION

        static inline std::string dir(int d)
        {
            return d == 0 ? "-" : (d == 1 ? "L" : d == -1 ? "R" : "#");
        }
        static inline std::string how_str(int h)
        {
            return h == 0 ? "-" : (h == 1 ? "A" : "D");
        }

        friend std::ostream& operator<<(std::ostream &os, intersection_info<P> const& info)
        {
            typename geometry::coordinate_type<P>::type d = info.distance;
            os  << "\t"
                << " src " << info.seg_id.source_index
                << " seg " << info.seg_id.segment_index
                << " (// " << info.other_id.source_index
                    << "." << info.other_id.segment_index << ")"
                << " how " << info.how
                    << "[" << how_str(info.arrival)
                    << " " << dir(info.direction)
                    << (info.opposite ? " o" : "")
                    << "]"
                << " sd "
                    << dir(info.sides.get<0,0>())
                    << dir(info.sides.get<0,1>())
                    << dir(info.sides.get<1,0>())
                    << dir(info.sides.get<1,1>())
                << " nxt seg " << info.travels_to_vertex_index
                << " , ip " << info.travels_to_ip_index
                << " , or " << info.next_ip_index
                << " dst " << double(d)
                << info.visit_state;
            if (info.flagged)
            {
                os << " FLAGGED";
            }
            return os;
        }
#endif
};


template<typename P>
struct intersection_point
{
    public :
        inline intersection_point()
            : trivial(true)
            , shared(false)
            , flagged(false)
        {
        }


#ifdef BOOST_GEOMETRY_DEBUG_INTERSECTION
        friend std::ostream& operator<<(std::ostream &os, intersection_point<P> const& p)
        {
            // Convert them to double to make them comparable in DEBUG-files
            typedef double casted;

            os
                << std::setprecision(6)
                << "IP (" << casted(geometry::get<0>(p.point))
                << "," << casted(geometry::get<1>(p.point)) << ")"
                << p.visit_state
                << (p.shared ? " SHARED" : "")
                << (p.flagged ? " FLAGGED" : "")
                << std::endl;

            for (unsigned int i = 0; i < p.info.size(); i++)
            {
                os << p.info[i] << std::endl;
            }
            if (! p.report.empty())
            {
                os << p.report << std::endl;
            }
            return os;
        }
        std::string report;
#endif
        typedef intersection_info<P> traversal_type;
        typedef std::vector<traversal_type> traversal_vector;

        typedef P point_type;

        P point;

        visit_info visit_state;

        bool trivial; // FALSE if there is an collinearity, touch or so.
        bool shared; // shared with more IP's
        bool flagged; // flagged for deletion afterwards

        // info about the two intersecting segments
        // usually two, but often more if IP's are merged
        traversal_vector info;

        inline void clone_except_info(intersection_point& other) const
        {
            other.point = point;
            other.trivial = trivial;
            other.shared = shared;
            other.flagged = flagged;
            // Probably not necessary:
            other.visit_state = visit_state;
        }
};




}} // namespace detail::intersection
#endif //DOXYGEN_NO_DETAIL


// Register the intersection point as being a point fulfilling the Point Concept
namespace traits
{

    template <typename P>
    struct coordinate_type<geometry::detail::intersection::intersection_point<P> >
    {
        typedef typename geometry::coordinate_type<P>::type type;
    };

    template <typename P>
    struct coordinate_system<geometry::detail::intersection::intersection_point<P> >
    {
        typedef typename geometry::coordinate_system<P>::type type;
    };

    template <typename P>
    struct dimension<geometry::detail::intersection::intersection_point<P> >
        : geometry::dimension<P>
    {};

    template <typename P>
    struct tag<geometry::detail::intersection::intersection_point<P> >
    {
        typedef point_tag type;
    };

    template <typename P, std::size_t Dimension>
    struct access<geometry::detail::intersection::intersection_point<P>, Dimension>
    {
        static inline typename coordinate_type<P>::type get(
                geometry::detail::intersection::intersection_point<P> const& p)
        {
            return geometry::get<Dimension>(p.point);
        }

        static inline void set(geometry::detail::intersection::intersection_point<P>& p,
                typename coordinate_type<P>::type const& value)
        {
            geometry::set<Dimension>(p.point, value);
        }
    };

}


#ifdef BOOST_GEOMETRY_DEBUG_INTERSECTION

template <typename V>
inline void report_ip(V const& intersection_points)
{
    typedef typename V::const_iterator iterator_type;

    for (iterator_type it = intersection_points.begin();
         it != intersection_points.end();
         ++it)
    {
        if (! it->flagged)
        {
            std::cout << *it;
        }
    }
}
#endif // BOOST_GEOMETRY_DEBUG_INTERSECTION


}} // namespace boost::geometry

#endif // BOOST_GEOMETRY_ALGORITHMS_INTERSECTION_POINT_HPP
