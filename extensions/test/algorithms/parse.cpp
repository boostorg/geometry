// Boost.Geometry (aka GGL, Generic Geometry Library)
// Unit Test

// Copyright (c) 2021 Joni Kerkelä, Oulu, Finland.

// Use, modification and distribution is subject to the Boost Software License,
// Version 1.0. (See accompanying file LICENSE_1_0.txt or copy at
// http://www.boost.org/LICENSE_1_0.txt)

#include <geometry_test_common.hpp>

#include <boost/geometry/core/coordinate_type.hpp>
#include <boost/geometry/extensions/algorithms/parse.hpp>
#include <boost/geometry/geometries/point.hpp>
#include <boost/geometry/util/math.hpp>
#include <boost/geometry/srs/projections/impl/dms_parser.hpp>

#include <test_common/test_point.hpp>

using namespace boost::geometry;

template <typename P>
void test_parse(std::string const& coordinate_1, std::string const& coordinate_2,
	double expected_result_coord_1, double expected_result_coord_2)
{
	auto geo_point_result = parse<P>(coordinate_1, coordinate_2);
	BOOST_CHECK_CLOSE(((double)bg::get<0>(geo_point_result)), expected_result_coord_1, 1.0e-6);
	BOOST_CHECK_CLOSE(((double)bg::get<1>(geo_point_result)), expected_result_coord_2, 1.0e-6);

	P p;
	parse(p, coordinate_1, coordinate_2);
	BOOST_CHECK_CLOSE(((double)bg::get<0>(p)), expected_result_coord_1, 1.0e-6);
	BOOST_CHECK_CLOSE(((double)bg::get<1>(p)), expected_result_coord_2, 1.0e-6);
}

template <typename P, typename S>
void test_parse_with_parse_strategy(std::string const& coordinate_1, std::string const& coordinate_2,
	double expected_result_coord_1, double expected_result_coord_2, S const& strategy)
{
	P p;
	parse<P, S>(p, coordinate_1, coordinate_2, strategy);
	BOOST_CHECK_CLOSE(((double)bg::get<0>(p)), expected_result_coord_1, 1.0e-6);
	BOOST_CHECK_CLOSE(((double)bg::get<1>(p)), expected_result_coord_2, 1.0e-6);
}

template <typename P, typename T>
void test_parse_with_point_and_parse_strategy(std::string const& coordinate_1, std::string const& coordinate_2,
	double expected_result_coord_1, double expected_result_coord_2, T const& parser)
{
	test_parse_with_parse_strategy<P, T>(coordinate_1,
		coordinate_2, expected_result_coord_1, expected_result_coord_2, parser);
}

template <typename P, bool as_radian>
void test_all_complex_dms_strategy()
{
	typedef typename coordinate_type<P>::type coord_t;
	typedef typename projections::detail::dms_parser<coord_t, as_radian, 'w', 'e', 'n', 's'> parser_t;

	// with minutes
	double expected_dms_result1 = as_radian ? 5.166666666 * bg::math::d2r<double>() : 5.166666666;
	double expected_dms_result2 = as_radian ? -5.166666666 * bg::math::d2r<double>() : -5.166666666;
	test_parse_with_point_and_parse_strategy<P, parser_t>(std::string("5d10E"), std::string("5d10N"),
		expected_dms_result1, expected_dms_result2, parser_t());

	// with minutes and indicator
	double expected_dms_result3 = as_radian ? (5.16666666 * bg::math::d2r<double>()) : 5.16666666;
	double expected_dms_result4 = as_radian ? -(5.16666666 * bg::math::d2r<double>()) : -5.16666666;
	test_parse_with_point_and_parse_strategy<P, parser_t>(std::string("5d10'E"), std::string("5d10'N"),
		expected_dms_result3, expected_dms_result4, parser_t());

	// with minutes/seconds
	double expected_dms_result5 = as_radian ? (5.1680555555 * bg::math::d2r<double>()) : 5.1680555555;
	double expected_dms_result6 = as_radian ? -(5.16805555555 * bg::math::d2r<double>()) : -5.16805555555;
	test_parse_with_point_and_parse_strategy<P, parser_t>(std::string("5d10'05E"), std::string("5d10'05N"),
		expected_dms_result5, expected_dms_result6, parser_t());

	// with seconds only
	double sec = 1 / 3600.0;
	double expected_dms_result7 = as_radian ? ((5 + sec) * bg::math::d2r<double>()) : (5 + sec);
	double expected_dms_result8 = as_radian ? -((5 + sec) * bg::math::d2r<double>()) : -(5 + sec);
	test_parse_with_point_and_parse_strategy<P, parser_t>(std::string("5d1\"E"), std::string("5d1\"N"),
		expected_dms_result7, expected_dms_result8, parser_t());

	// with spaces and signs
	test_parse_with_point_and_parse_strategy<P, parser_t>(std::string("5d 1\" E"), std::string("5d 1\" N"),
		expected_dms_result7, expected_dms_result8, parser_t());

	test_parse_with_point_and_parse_strategy<P, parser_t>(std::string("+5d 1\" E"), std::string("+5d 1\" N"),
		expected_dms_result7, expected_dms_result8, parser_t());

	test_parse_with_point_and_parse_strategy<P, parser_t>(std::string("-5d 1\" E"), std::string("-5d 1\" N"),
		-expected_dms_result7, -expected_dms_result8, parser_t());

	test_parse_with_point_and_parse_strategy<P, parser_t>(std::string("5d 10' 05 E"), std::string("5d 10' 05 N"),
		expected_dms_result5, expected_dms_result6, parser_t());

	// in radians
	double expected_dms_result9 = as_radian ? 5.15 : (5.15 * bg::math::r2d<double>());
	double expected_dms_result10 = as_radian ? -5.15 : -(5.15 * bg::math::r2d<double>());
	test_parse_with_point_and_parse_strategy<P, parser_t>(std::string("5.15RE"), std::string("5.15RN"),
		expected_dms_result9, expected_dms_result10, parser_t());

	test_parse_with_point_and_parse_strategy<P, parser_t>(std::string("5.15R E"), std::string("5.15R N"),
		expected_dms_result9, expected_dms_result10, parser_t());
}

template <typename P, bool as_radian>
void test_all_with_simple_dms_strategy()
{
	typedef typename coordinate_type<P>::type coord_t;

	double generic_expected_result1 = as_radian ? 2 * bg::math::d2r<double>() : 2;
	double generic_expected_result2 = as_radian ? -4 * bg::math::d2r<double>() : -4;

	typedef typename projections::detail::dms_parser<coord_t, as_radian, 'w', 'e', 'n', 's'> parser_t1;
	test_parse_with_point_and_parse_strategy<P, parser_t1>(std::string("2e"), std::string("4n"),
		generic_expected_result1, generic_expected_result2, parser_t1());

	test_parse_with_point_and_parse_strategy<P, parser_t1>(std::string("2w"), std::string("4s"),
		generic_expected_result2, generic_expected_result1, parser_t1());

	test_parse_with_point_and_parse_strategy<P, parser_t1>(std::string("2w"), std::string("4e"),
		2 * generic_expected_result1, generic_expected_result1, parser_t1());

	test_parse_with_point_and_parse_strategy<P, parser_t1>(std::string("2s"), std::string("4n"),
		-(generic_expected_result1), generic_expected_result2, parser_t1());

	test_parse_with_point_and_parse_strategy<P, parser_t1>(std::string("4n"), std::string("2s"),
		-(generic_expected_result1), generic_expected_result2, parser_t1());

	typedef typename projections::detail::dms_parser<coord_t, as_radian, 'n', 'o', 'z', 'w'> parser_t2;
	test_parse_with_point_and_parse_strategy<P, parser_t2>(std::string("2o"), std::string("4z"),
		generic_expected_result1, generic_expected_result2, parser_t2());

	typedef typename projections::detail::dms_parser<coord_t, as_radian, 'W', 'E', 'N', 'S'> parser_t3;
	test_parse_with_point_and_parse_strategy<P, parser_t3>(std::string("4N"), std::string("2S"),
		-(generic_expected_result1), generic_expected_result2, parser_t3());

	test_parse_with_point_and_parse_strategy<P, parser_t3>(std::string("4n"), std::string("2s"),
		-(generic_expected_result1), generic_expected_result2, parser_t3());
}


void test_all_without_strategy()
{
	test_parse<bg::model::point<double, 2, bg::cs::geographic<bg::radian> > >(std::string("1E"),
		std::string("2N"), 1 * bg::math::d2r<double>(), 2 * bg::math::d2r<double>());
	test_parse<bg::model::point<double, 2, bg::cs::geographic<bg::radian> > >(std::string("1W"),
		std::string("2S"), -1 * bg::math::d2r<double>(), -2 * bg::math::d2r<double>());
	test_parse<bg::model::point<double, 2, bg::cs::geographic<bg::degree> > >(std::string("1E"),
		std::string("2N"), 1 * bg::math::d2r<double>(), 2 * bg::math::d2r<double>());
	test_parse<bg::model::point<double, 2, bg::cs::geographic<bg::degree> > >(std::string("1W"),
		std::string("2S"), -1 * bg::math::d2r<double>(), -2 * bg::math::d2r<double>());
	test_parse<bg::model::point<double, 2, bg::cs::spherical<bg::radian> > >(std::string("1W"),
		std::string("2S"), -1 * bg::math::d2r<double>(), -2 * bg::math::d2r<double>());
	test_parse<bg::model::point<double, 2, bg::cs::spherical_equatorial<bg::radian> > >(std::string("1W"),
		std::string("2S"), -1 * bg::math::d2r<double>(), -2 * bg::math::d2r<double>());
	test_parse<bg::model::point<double, 2, bg::cs::undefined> >(std::string("1W"),
		std::string("2S"), -1 * bg::math::d2r<double>(), -2 * bg::math::d2r<double>());
	test_parse<bg::model::point<double, 2, bg::cs::cartesian> >(std::string("1W"),
		std::string("2S"), -1 * bg::math::d2r<double>(), -2 * bg::math::d2r<double>());
}

void test_all()
{
	test_all_without_strategy();
	test_all_with_simple_dms_strategy<bg::model::point<double, 2, bg::cs::geographic<bg::degree> >, false>();
	test_all_with_simple_dms_strategy<bg::model::point<double, 2, bg::cs::geographic<bg::radian> >, true>();
	test_all_complex_dms_strategy<bg::model::point<double, 2, bg::cs::geographic<bg::degree> >, false>();
	test_all_complex_dms_strategy<bg::model::point<double, 2, bg::cs::geographic<bg::radian> >, true>();
}

int test_main(int, char* [])
{
	test_all();
	return 0;
}
