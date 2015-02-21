// Boost.Geometry (aka GGL, Generic Geometry Library)

// Copyright (c) 2015, Oracle and/or its affiliates.

// Contributed and/or modified by Menelaos Karavelas, on behalf of Oracle

// Licensed under the Boost Software License version 1.0.
// http://www.boost.org/users/license.html

#ifndef BOOST_GEOMETRY_POLICIES_IS_VALID_FAILING_REASON_POLICY_HPP
#define BOOST_GEOMETRY_POLICIES_IS_VALID_FAILING_REASON_POLICY_HPP

#include <sstream>

#include <boost/geometry/io/dsv/write.hpp>
#include <boost/geometry/util/range.hpp>
#include <boost/geometry/algorithms/validity_failure_type.hpp>


namespace boost { namespace geometry
{


inline char const* validity_failure_type_message(validity_failure_type failure)
{
    switch (failure)
    {
    case no_failure:
        return "Geometry is valid";
    case failure_few_points:
        return "Geometry has too few points";
    case failure_wrong_topological_dimension:
        return "Geometry has wrong topological dimension";
    case failure_not_closed:
        return "Geometry is defined as closed but is open";
    case failure_spikes:
        return "Geometry has spikes";
    case failure_self_intersections:
        return "Geometry has invalid self-intersections";
    case failure_wrong_orientation:
        return "Geometry has wrong orientation";
    case failure_interior_rings_outside:
        return "Geometry has interior rings defined outside the outer boundary";
    case failure_nested_interior_rings:
        return "Geometry has nested interior rings";
    case failure_disconnected_interior:
        return "Geometry has disconnected interior";
    case failure_intersecting_interiors:
        return "Multi-polygon has intersecting interiors";
    case failure_duplicate_points:
        return "Geometry has duplicate (consecutive) points";
    case failure_wrong_corner_order:
        return "Box has corners in wrong order";
    default: // to avoid -Wreturn-type warning
        return "";
    }
}


class failing_reason_policy
{
private:
    inline void update_status(validity_failure_type failure)
    {
        m_failure = failure;
        m_oss.str("");
        m_oss.clear();
        m_oss << validity_failure_type_message(m_failure);
    }

    template <validity_failure_type Failure, typename Data>
    struct process_data
    {
        static inline void apply(std::ostringstream&, Data const&)
        {
        }
    };

    template <typename SpikePoint>
    struct process_data<failure_spikes, SpikePoint>
    {
        static inline void apply(std::ostringstream& oss,
                                 SpikePoint const& spike_point)
        {
            oss << ". A spike point was found with apex at "
                << geometry::dsv(spike_point);
        }
    };

    template <typename Turns>
    struct process_data<failure_self_intersections, Turns>
    {
        static inline void apply(std::ostringstream& oss,
                                 Turns const& turns)
        {
            oss << ". A self-intersection point was found at "
                << geometry::dsv(range::front(turns).point);
        }
    };

    template <typename Point>
    struct process_data<failure_duplicate_points, Point>
    {
        static inline void apply(std::ostringstream& oss,
                                 Point const& point)
        {
            oss << ". Duplicate points were found near point "
                << geometry::dsv(point);
        }
    };

public:
    failing_reason_policy(std::ostringstream& oss)
        : m_failure(no_failure)
        , m_oss(oss)
    {}

    template <validity_failure_type Failure>
    inline void apply()
    {
        update_status(Failure);
    }

    template <validity_failure_type Failure, typename Data>
    inline void apply(Data const& data)
    {
        update_status(Failure);
        process_data<Failure, Data>::apply(m_oss, data);
    }

    template <validity_failure_type Failure, typename Data1, typename Data2>
    inline void apply(Data1 const&, Data2 const&)
    {
        update_status(Failure);
    }

    inline validity_failure_type get_failure_type() const
    {
        return m_failure;
    }

private:
    validity_failure_type m_failure;
    std::ostringstream& m_oss;
};


}} // namespace boost::geometry

#endif // BOOST_GEOMETRY_POLICIES_IS_VALID_FAILING_REASON_POLICY_HPP
