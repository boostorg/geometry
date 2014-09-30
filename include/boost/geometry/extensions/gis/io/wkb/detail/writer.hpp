#ifndef BOOST_GEOMETRY_IO_WKB_DETAIL_WRITER_HPP
#define BOOST_GEOMETRY_IO_WKB_DETAIL_WRITER_HPP

#include <cassert>
#include <cstddef>
#include <algorithm>
#include <iterator>
#include <limits>

#include <boost/concept_check.hpp>
#include <boost/cstdint.hpp>
#include <boost/type_traits/is_integral.hpp>
#include <boost/type_traits/is_same.hpp>
#include <boost/static_assert.hpp>

#include <boost/geometry/core/access.hpp>
#include <boost/geometry/core/coordinate_dimension.hpp>
#include <boost/geometry/core/coordinate_type.hpp>
#include <boost/geometry/core/exterior_ring.hpp>
#include <boost/geometry/core/interior_rings.hpp>
#include <boost/geometry/extensions/gis/io/wkb/detail/endian.hpp>
#include <boost/geometry/extensions/gis/io/wkb/detail/ogc.hpp>

#include <boost/range.hpp>
#include <boost/range/algorithm/for_each.hpp>

namespace boost { namespace geometry
{

#ifndef DOXYGEN_NO_DETAIL
namespace detail { namespace wkb
{

    template <typename T>
    struct value_writer
    {
        typedef T value_type;

        template <typename OutputIterator>
        static bool write(const T& value, OutputIterator& iter, byte_order_type::enum_t byte_order)
        {
            endian::endian_value<T> parsed_value(value);

// 				if (byte_order_type::xdr == byte_order)
// 				{
// 					parsed_value.template store<endian::big_endian_tag>(iter);
// 				}
// 				else if (byte_order_type::ndr == byte_order)
// 				{
// 					parsed_value.template store<endian::little_endian_tag>(iter);
// 				}
// 				else
// 				{
                parsed_value.template store<endian::native_endian_tag>(iter);
// 				}

            return true;
        }
    };

    template <typename P, int I, int N>
    struct writer_assigner
    {
        template <typename OutputIterator>
        static void run(const P& point, OutputIterator& iter, byte_order_type::enum_t byte_order)
        {
            typedef typename coordinate_type<P>::type coordinate_type;

            value_writer<double>::write(boost::geometry::get<I>(point), iter, byte_order);

            writer_assigner<P, I+1, N>::run(point, iter, byte_order);
        }
    };

    template <typename P, int N>
    struct writer_assigner<P, N, N>
    {
        template <typename OutputIterator>
        static void run(const P& point, OutputIterator& iter, byte_order_type::enum_t byte_order)
        {
            // terminate
            boost::ignore_unused_variable_warning(point);
            boost::ignore_unused_variable_warning(iter);
            boost::ignore_unused_variable_warning(byte_order);
        }
    };

    template <typename P>
    struct point_writer
    {
        template <typename OutputIterator>
        static bool write(const P& point, OutputIterator& iter,
            byte_order_type::enum_t byte_order)
        {
            // write endian type
            value_writer<uint8_t>::write(byte_order, iter, byte_order);

            // write geometry type
            uint32_t type = geometry_type<P>::get();
            value_writer<uint32_t>::write(type, iter, byte_order);

            // write point's x, y, z
            writer_assigner<P, 0, dimension<P>::value>::run(point, iter, byte_order);

            return true;
        }
    };

    template <typename L>
    struct linestring_writer
    {
        template <typename OutputIterator>
        static bool write(const L& linestring, OutputIterator& iter,
            byte_order_type::enum_t byte_order)
        {
            // write endian type
            value_writer<uint8_t>::write(byte_order, iter, byte_order);

            // write geometry type
            uint32_t type = geometry_type<L>::get();
            value_writer<uint32_t>::write(type, iter, byte_order);

            // write num points
            uint32_t num_points = boost::size(linestring);
            value_writer<uint32_t>::write(num_points, iter, byte_order);

            typedef typename point_type<L>::type point_type;

            for(boost::range_iterator<const L>::type point_iter = boost::begin(linestring);
                point_iter != boost::end(linestring);
                ++point_iter)
            {
                // write point's x, y, z
                writer_assigner<point_type, 0, dimension<point_type>::value>::run(*point_iter, iter, byte_order);
            }

            return true;
        }
    };

    template <typename P>
    struct polygon_writer
    {
        template <typename OutputIterator>
        static bool write(const P& polygon, OutputIterator& iter,
            byte_order_type::enum_t byte_order)
        {
            // write endian type
            value_writer<uint8_t>::write(byte_order, iter, byte_order);

            // write geometry type
            uint32_t type = geometry_type<P>::get();
            value_writer<uint32_t>::write(type, iter, byte_order);

            // write num rings
            uint32_t num_rings = 1 + boost::geometry::num_interior_rings(polygon);
            value_writer<uint32_t>::write(num_rings, iter, byte_order);

            typedef typename point_type<P>::type point_type;

            // write exterior ring
            typedef typename boost::geometry::ring_type<const P>::type ring_type;
            ring_type exterior_ring = boost::geometry::exterior_ring(polygon);

            uint32_t num_exterior_ring_points = boost::geometry::num_points(exterior_ring);
            value_writer<uint32_t>::write(num_exterior_ring_points, iter, byte_order);

            for(boost::range_iterator<const ring_type>::type point_iter = boost::begin(exterior_ring); 
                point_iter != boost::end(exterior_ring);
                ++point_iter)
            {
                // write point's x, y, z
                writer_assigner<point_type, 0, dimension<point_type>::value>::run(*point_iter, iter, byte_order);
            }

            // write interor rings
            typedef typename boost::geometry::interior_type<const P>::type interior_rings_type;

            interior_rings_type interior_rings = boost::geometry::interior_rings(polygon);

            for(boost::range_iterator<const interior_rings_type>::type interior_ring_iter = boost::begin(interior_rings); 
                interior_ring_iter != boost::end(interior_rings);
                ++interior_ring_iter)
            {
                uint32_t num_interior_ring_points = boost::geometry::num_points(*interior_ring_iter);
                value_writer<uint32_t>::write(num_interior_ring_points, iter, byte_order);

                for(boost::range_iterator<const ring_type>::type point_iter = boost::begin(*interior_ring_iter); 
                    point_iter != boost::end(*interior_ring_iter); 
                    ++point_iter)
                {
                    // write point's x, y, z
                    writer_assigner<point_type, 0, dimension<point_type>::value>::run(*point_iter, iter, byte_order);
                }
            }

            return true;
        }
    };

}} // namespace detail::wkb
#endif // DOXYGEN_NO_IMPL

}} // namespace boost::geometry
#endif // BOOST_GEOMETRY_IO_WKB_DETAIL_WRITER_HPP
