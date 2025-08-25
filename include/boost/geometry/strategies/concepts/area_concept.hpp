// Boost.Geometry (aka GGL, Generic Geometry Library)

// Copyright (c) 2007-2012 Barend Gehrels, Amsterdam, the Netherlands.
// Copyright (c) 2008-2012 Bruno Lalande, Paris, France.
// Copyright (c) 2009-2012 Mateusz Loskot, London, UK.
// Copyright (c) 2017 Adam Wulkiewicz, Lodz, Poland.

// This file was modified by Oracle on 2018.
// Modifications copyright (c) 2018 Oracle and/or its affiliates.
// Contributed and/or modified by Adam Wulkiewicz, on behalf of Oracle

// Parts of Boost.Geometry are redesigned from Geodan's Geographic Library
// (geolib/GGL), copyright (c) 1995-2010 Geodan, Amsterdam, the Netherlands.

// Use, modification and distribution is subject to the Boost Software License,
// Version 1.0. (See accompanying file LICENSE_1_0.txt or copy at
// http://www.boost.org/LICENSE_1_0.txt)

#ifndef BOOST_GEOMETRY_STRATEGIES_CONCEPTS_AREA_CONCEPT_HPP
#define BOOST_GEOMETRY_STRATEGIES_CONCEPTS_AREA_CONCEPT_HPP

#include <type_traits>

#include <boost/concept_check.hpp>

#include <boost/geometry/geometries/point.hpp>


namespace boost { namespace geometry { namespace concepts
{


/*!
    \brief Checks strategy for area
    \ingroup area
*/
template <typename Geometry, typename Strategy>
class AreaStrategy
{
#ifndef DOXYGEN_NO_CONCEPT_MEMBERS

    // 1) must define state template,
    using state_type = typename Strategy::template state<Geometry>;

    // 2) must define result_type template,
    using return_type = typename Strategy::template result_type<Geometry>::type;

    struct check_methods
    {
        static void apply()
        {
            Strategy const* str = nullptr;
            state_type *st = nullptr;

            // 3) must implement a method apply with the following signature
            geometry::point_type_t<Geometry> const* sp = nullptr;
            str->apply(*sp, *sp, *st);

            // 4) must implement a method result with the following signature
            static_assert(std::is_convertible<return_type, decltype(str->result(*st))>::value,
                          "Strategy::result(state_type&) must be convertible to result_type<Geometry>::type");
        }
    };

public :
    BOOST_CONCEPT_USAGE(AreaStrategy)
    {
        check_methods::apply();
    }

#endif
};


}}} // namespace boost::geometry::concepts


#endif // BOOST_GEOMETRY_STRATEGIES_CONCEPTS_AREA_CONCEPT_HPP
