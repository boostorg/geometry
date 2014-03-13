// Boost.Geometry (aka GGL, Generic Geometry Library)

// Copyright (c) 2007-2012 Barend Gehrels, Amsterdam, the Netherlands.

// This file was modified by Oracle on 2013, 2014.
// Modifications copyright (c) 2013-2014 Oracle and/or its affiliates.

// Use, modification and distribution is subject to the Boost Software License,
// Version 1.0. (See accompanying file LICENSE_1_0.txt or copy at
// http://www.boost.org/LICENSE_1_0.txt)

// Contributed and/or modified by Adam Wulkiewicz, on behalf of Oracle

#ifndef BOOST_GEOMETRY_ALGORITHMS_DETAIL_RELATE_FOLLOW_HELPERS_HPP
#define BOOST_GEOMETRY_ALGORITHMS_DETAIL_RELATE_FOLLOW_HELPERS_HPP

namespace boost { namespace geometry
{

#ifndef DOXYGEN_NO_DETAIL
namespace detail { namespace relate {

// TODO:
// For 1-point linestrings or with all equal points turns won't be generated!
// Check for those degenerated cases may be connected with this one!

template <std::size_t OpId,
          typename Geometry,
          typename Tag = typename geometry::tag<Geometry>::type,
          bool IsMulti = boost::is_base_of<multi_tag, Tag>::value
>
struct for_each_disjoint_geometry_if
    : public not_implemented<Tag>
{};

template <std::size_t OpId, typename Geometry, typename Tag>
struct for_each_disjoint_geometry_if<OpId, Geometry, Tag, false>
{
    template <typename TurnIt, typename Pred>
    static inline bool apply(TurnIt first, TurnIt last,
                             Geometry const& geometry,
                             Pred & pred)
    {
        if ( first != last )
            return false;
        pred(geometry);
        return true;
    }
};

template <std::size_t OpId, typename Geometry, typename Tag>
struct for_each_disjoint_geometry_if<OpId, Geometry, Tag, true>
{
    template <typename TurnIt, typename Pred>
    static inline bool apply(TurnIt first, TurnIt last,
                             Geometry const& geometry,
                             Pred & pred)
    {
        BOOST_ASSERT(first != last);

        const std::size_t count = boost::size(geometry);
        boost::ignore_unused_variable_warning(count);

        // O(I)
        // gather info about turns generated for contained geometries
        std::vector<bool> detected_intersections(count, false);
        for ( TurnIt it = first ; it != last ; ++it )
        {
            int multi_index = it->operations[OpId].seg_id.multi_index;
            BOOST_ASSERT(multi_index >= 0);
            std::size_t index = static_cast<std::size_t>(multi_index);
            BOOST_ASSERT(index < count);
            detected_intersections[index] = true;
        }

        bool found = false;

        // O(N)
        // check predicate for each contained geometry without generated turn
        for ( std::vector<bool>::iterator it = detected_intersections.begin() ;
              it != detected_intersections.end() ; ++it )
        {
            // if there were no intersections for this multi_index
            if ( *it == false )
            {
                found = true;
                bool cont = pred(
                                *(boost::begin(geometry)
                                    + std::distance(detected_intersections.begin(), it)));
                if ( !cont )
                    break;
            }
        }
        
        return found;
    }
};

// TODO: rename to point_id_ref?
template <typename Point>
class point_identifier
{
public:
    point_identifier() : sid_ptr(0), pt_ptr(0) {}
    point_identifier(segment_identifier const& sid, Point const& pt)
        : sid_ptr(boost::addressof(sid))
        , pt_ptr(boost::addressof(pt))
    {}
    segment_identifier const& seg_id() const
    {
        BOOST_ASSERT(sid_ptr);
        return *sid_ptr;
    }
    Point const& point() const
    {
        BOOST_ASSERT(pt_ptr);
        return *pt_ptr;
    }

    //friend bool operator==(point_identifier const& l, point_identifier const& r)
    //{
    //    return l.seg_id() == r.seg_id()
    //        && detail::equals::equals_point_point(l.point(), r.point());
    //}

private:
    const segment_identifier * sid_ptr;
    const Point * pt_ptr;
};

class same_multi_index
{
public:
    same_multi_index(segment_identifier const& sid)
        : sid_ptr(boost::addressof(sid))
    {}

    bool operator()(segment_identifier const& sid) const
    {
        return sid.multi_index == sid_ptr->multi_index;                
    }

    template <typename Point>
    bool operator()(point_identifier<Point> const& pid) const
    {
        return operator()(pid.seg_id());
    }

private:
    const segment_identifier * sid_ptr;
};

class segment_watcher
{
public:
    segment_watcher()
        : m_seg_id_ptr(0)
    {}

    bool update(segment_identifier const& seg_id)
    {
        bool result = m_seg_id_ptr == 0 || !same_multi_index(*m_seg_id_ptr)(seg_id);
        m_seg_id_ptr = boost::addressof(seg_id);
        return result;
    }

private:
    const segment_identifier * m_seg_id_ptr;
};

template <typename Point>
class exit_watcher
{
    typedef point_identifier<Point> point_info;

public:
    exit_watcher()
        : exit_operation(overlay::operation_none)
    {}

    void enter(Point const& point, segment_identifier const& other_id)
    {
        other_entry_points.push_back(point_info(other_id, point));
    }

    void exit(Point const& point,
              segment_identifier const& other_id,
              overlay::operation_type exit_op)
    {
        typedef typename std::vector<point_info>::iterator point_iterator;
        // search for the entry point in the same range of other geometry
        point_iterator entry_it = std::find_if(other_entry_points.begin(),
                                               other_entry_points.end(),
                                               same_multi_index(other_id));

        // this end point has corresponding entry point
        if ( entry_it != other_entry_points.end() )
        {
            // here we know that we possibly left LS
            // we must still check if we didn't get back on the same point
            exit_operation = exit_op;
            exit_id = point_info(other_id, point);

            // erase the corresponding entry point
            other_entry_points.erase(entry_it);
        }
    }

    bool is_outside() const
    {
        // if we didn't entered anything in the past, we're outside
        return other_entry_points.empty();
    }

    overlay::operation_type get_exit_operation() const
    {
        return exit_operation;
    }

    Point const& get_exit_point() const
    {
        BOOST_ASSERT(exit_operation != overlay::operation_none);
        return exit_id.point();
    }

    void reset_detected_exit()
    {
        exit_operation = overlay::operation_none;
    }

private:
    overlay::operation_type exit_operation;
    point_info exit_id;
    std::vector<point_info> other_entry_points; // TODO: use map here or sorted vector?
};

template <boundary_query BoundaryQuery,
          typename Point,
          typename BoundaryChecker>
static inline bool is_endpoint_on_boundary(Point const& pt,
                                           BoundaryChecker & boundary_checker)
{
    return boundary_checker.template is_endpoint_boundary<BoundaryQuery>(pt);
}

template <boundary_query BoundaryQuery,
          typename IntersectionPoint,
          typename OperationInfo,
          typename BoundaryChecker>
static inline bool is_ip_on_boundary(IntersectionPoint const& ip,
                                     OperationInfo const& operation_info,
                                     BoundaryChecker & boundary_checker,
                                     segment_identifier const& seg_id)
{
    boost::ignore_unused_variable_warning(seg_id);

    bool res = false;

    // IP on the last point of the linestring
    if ( (BoundaryQuery == boundary_back || BoundaryQuery == boundary_any)
        && operation_info.operation == overlay::operation_blocked )
    {
        BOOST_ASSERT(operation_info.position == overlay::position_back);
        // check if this point is a boundary
        res = boundary_checker.template is_endpoint_boundary<boundary_back>(ip);

#ifdef BOOST_GEOMETRY_DEBUG_RELATE
        BOOST_ASSERT(res == boundary_checker.template is_boundary<boundary_back>(ip, seg_id));
#endif
    }
    // IP on the last point of the linestring
    else if ( (BoundaryQuery == boundary_front || BoundaryQuery == boundary_any)
            && operation_info.position == overlay::position_front )
    {
        // check if this point is a boundary
        res = boundary_checker.template is_endpoint_boundary<boundary_front>(ip);

#ifdef BOOST_GEOMETRY_DEBUG_RELATE
        BOOST_ASSERT(res == boundary_checker.template is_boundary<boundary_front>(ip, seg_id));
#endif
    }
    // IP somewhere in the interior
    else
    {
#ifdef BOOST_GEOMETRY_DEBUG_RELATE
        BOOST_ASSERT(res == boundary_checker.template is_boundary<boundary_any>(ip, seg_id));
#endif
    }
            
    return res;
}

}} // namespace detail::relate
#endif // DOXYGEN_NO_DETAIL

}} // namespace boost::geometry

#endif // BOOST_GEOMETRY_ALGORITHMS_DETAIL_RELATE_FOLLOW_HELPERS_HPP
