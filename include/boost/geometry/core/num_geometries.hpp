// Boost.Geometry (aka GGL, Generic Geometry Library)
//
// Copyright Barend Gehrels 2010, Geodan, Amsterdam, the Netherlands.
// Use, modification and distribution is subject to the Boost Software License,
// Version 1.0. (See accompanying file LICENSE_1_0.txt or copy at
// http://www.boost.org/LICENSE_1_0.txt)


#ifndef BOOST_GEOMETRY_NUM_GEOMETRIES_HPP
#define BOOST_GEOMETRY_NUM_GEOMETRIES_HPP

#include <cstddef>

#include <boost/geometry/core/tag.hpp>
#include <boost/geometry/core/tags.hpp>
#include <boost/geometry/core/is_multi.hpp>

#include <boost/geometry/geometries/concepts/check.hpp>


namespace boost { namespace geometry
{


#ifndef DOXYGEN_NO_DISPATCH
namespace core_dispatch
{


template <typename Tag, bool IsMulti, typename Geometry>
struct num_geometries
{
    static inline std::size_t apply(Geometry const&)
    {
        return 1;
    }
};



} // namespace core_dispatch
#endif


/*!
\brief \brief_calc{number of geometries}
\ingroup num_geometries
\details \details_calc{num_geometries, number of geometries}.
\tparam Geometry \tparam_geometry
\param geometry \param_geometry
\return \return_calc{number of geometries}

\qbk{behavior,__single__:[qbk_ret 1]}
\qbk{behavior,__multi__:Returns boost::size(geometry); the input is considered as a range}
\qbk{compliance,__ogc__}
\qbk{complexity,Constant}
*/
template <typename Geometry>
inline std::size_t num_geometries(Geometry const& geometry)
{
    concept::check<Geometry const>();

    return core_dispatch::num_geometries
        <
            typename tag<Geometry>::type,
            is_multi<Geometry>::type::value,
            Geometry
        >::apply(geometry);
}


}} // namespace boost::geometry


#endif // BOOST_GEOMETRY_NUM_GEOMETRIES_HPP
