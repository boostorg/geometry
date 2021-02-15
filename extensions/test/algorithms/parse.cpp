// Boost.Geometry (aka GGL, Generic Geometry Library)
// Unit Test

// Copyright (c) 2021 Joni Kerkelä, Oulu, Finland.
// Copyright (c) 2021 Ayush Gupta, Gujarat, India

// Use, modification and distribution is subject to the Boost Software License,
// Version 1.0. (See accompanying file LICENSE_1_0.txt or copy at
// http://www.boost.org/LICENSE_1_0.txt)

#include <string>
#include <geometry_test_common.hpp>

#include <boost/geometry/core/coordinate_type.hpp>
#include <boost/geometry/extensions/algorithms/parse.hpp>
#include <boost/geometry/geometries/point.hpp>
#include <boost/geometry/util/math.hpp>

#include <test_common/test_point.hpp>

using namespace boost::geometry;

template <typename Geometry, typename T>
void test_parse(std::string const& coordinate_1,
	std::string const& coordinate_2,
	T expected_result_coord_1,
	T expected_result_coord_2)
{
	Geometry geometry;
	geometry = parse<Geometry>(coordinate_1, coordinate_2);
	BOOST_CHECK_CLOSE(((double)bg::get<0>(geometry)), expected_result_coord_1, 1.0e-4);
	BOOST_CHECK_CLOSE(((double)bg::get<1>(geometry)), expected_result_coord_2, 1.0e-4);

	parse(geometry, coordinate_1, coordinate_2);
	BOOST_CHECK_CLOSE(((double)bg::get<0>(geometry)), expected_result_coord_1, 1.0e-4);
	BOOST_CHECK_CLOSE(((double)bg::get<1>(geometry)), expected_result_coord_2, 1.0e-4);
}

template <typename P, typename S>
void test_parse_with_parse_strategy(std::string const& coordinate_1, std::string const& coordinate_2,
	double expected_result_coord_1, double expected_result_coord_2, S const& strategy)
{
	P p;
	parse<P, S>(p, coordinate_1, coordinate_2, strategy);
	BOOST_CHECK_CLOSE(((double)bg::get<0>(p)), expected_result_coord_1, 1.0e-4);
	BOOST_CHECK_CLOSE(((double)bg::get<1>(p)), expected_result_coord_2, 1.0e-4);
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
	using parser_t = projections::detail::dms_parser<coord_t, as_radian, 'w', 'e', 'n', 's'>;

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

	// test with random characters in strategy(for generality check)
	using parser_t1 = bg::projections::detail::dms_parser<coord_t, true, 'A', 'B', 'C', 'D', 'm', 's', 'o', 'p'>;


	test_parse_with_point_and_parse_strategy<P, parser_t1>(std::string("45o 30m 30s A"), std::string("120o 30m 45s D"),
		-2.1033408163, 0.7942705942, parser_t1());

	test_parse_with_point_and_parse_strategy<P, parser_t1>(std::string("1.75p c"), std::string("5o 40m 55s b"),
		0.0991686810, -1.7500000000, parser_t1());

	// creating another strategy with returning value as degree
	using parser_t2 = bg::projections::detail::dms_parser<coord_t, false>; // return value in degree and rest parameters as default value.


	test_parse_with_point_and_parse_strategy<P, parser_t2>(std::string("80d 45' 30\"S"), std::string("150d 30' 30\"E"),
		150.5083333333, -80.7583333333, parser_t2());
	test_parse_with_point_and_parse_strategy<P, parser_t2>(std::string("15d 10' 20\"N"), std::string("2.75r W"),
		-157.5633261332, 15.1722222222, parser_t2());

}

template <typename P, bool as_radian>
void test_all_with_simple_dms_strategy()
{
	using coord_t = typename coordinate_type<P>::type;

	double generic_expected_result1 = as_radian ? 2 * bg::math::d2r<double>() : 2;
	double generic_expected_result2 = as_radian ? -4 * bg::math::d2r<double>() : -4;

	using parser_t1 = projections::detail::dms_parser<coord_t, as_radian, 'w', 'e', 'n', 's'>;
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

	using parser_t2 = projections::detail::dms_parser<coord_t, as_radian, 'n', 'o', 'z', 'w'>;
	test_parse_with_point_and_parse_strategy<P, parser_t2>(std::string("2o"), std::string("4z"),
		generic_expected_result1, generic_expected_result2, parser_t2());

	using parser_t3 = projections::detail::dms_parser<coord_t, as_radian, 'W', 'E', 'N', 'S'>;
	test_parse_with_point_and_parse_strategy<P, parser_t3>(std::string("4N"), std::string("2S"),
		-(generic_expected_result1), generic_expected_result2, parser_t3());

	test_parse_with_point_and_parse_strategy<P, parser_t3>(std::string("4n"), std::string("2s"),
		-(generic_expected_result1), generic_expected_result2, parser_t3());

}

template<typename Geometry>
void test_all_without_strategy()
{
	using T = typename bg::coordinate_type<Geometry>::type;
	T const d2r = math::d2r<T>();

	test_parse<Geometry, T>(std::string("1E"), std::string("2N"), 1 * d2r, 2 * d2r);
	test_parse<Geometry, T>(std::string("1W"), std::string("2S"), -1 * d2r, -2 * d2r);
	test_parse<Geometry, T>(std::string("1E"), std::string("2N"), 1 * d2r, 2 * d2r);
	test_parse<Geometry, T>(std::string("1W"), std::string("2S"), -1 * d2r, -2 * d2r);
	test_parse<Geometry, T>(std::string("1W"), std::string("2S"), -1 * d2r, -2 * d2r);
	test_parse<Geometry, T>(std::string("1W"), std::string("2S"), -1 * d2r, -2 * d2r);
	test_parse<Geometry, T>(std::string("1W"), std::string("2S"), -1 * d2r, -2 * d2r);
	test_parse<Geometry, T>(std::string("1W"), std::string("2S"), -1 * d2r, -2 * d2r);

	// tests for dms strings, values returned in radian(default strategy).
	test_parse<Geometry, T>(std::string("45d 30' 30\" N"), std::string("120D 30'45\"W"), -2.1033408163, 0.7942705942);
	test_parse<Geometry, T>(std::string("32D45' 57\"n"), std::string("170d 10' 25\"E"), 2.9700910868, 0.5718719189);
	test_parse<Geometry, T>(std::string("5d38'40\"S"), std::string("168D 10' 20\"w"), -2.9351602461, -0.0985141822);
	test_parse<Geometry, T>(std::string("72D 20'45\"s"), std::string("5d 40' 55\"e"), 0.0991686810, -1.2626735329);

	// test for radian and dms strings combined returning values in degree.
	test_parse<Geometry, T>(std::string("2.5r N"), std::string("0.75r E"), 0.7500000000, 2.5000000000);
	test_parse<Geometry, T>(std::string("1.25r S"), std::string("120D 30' 45\"w"), -2.1033408163, -1.2500000000);
}

void test_without_strategy()
{
	test_all_without_strategy<bg::model::point<double, 2, bg::cs::spherical<bg::degree>>>();
	test_all_without_strategy<bg::model::point<double, 2, bg::cs::spherical<bg::radian>>>();
	test_all_without_strategy<bg::model::point<double, 2, bg::cs::spherical_equatorial<bg::degree>>>();
	test_all_without_strategy<bg::model::point<double, 2, bg::cs::spherical_equatorial<bg::radian>>>();
	test_all_without_strategy<bg::model::point<double, 2, bg::cs::geographic<bg::degree>>>();
	test_all_without_strategy<bg::model::point<double, 2, bg::cs::geographic<bg::radian>>>();
	test_all_without_strategy<bg::model::point<double, 2, bg::cs::undefined>>();
	test_all_without_strategy<bg::model::point<double, 2, bg::cs::cartesian>>();
}
void test_all()
{
	test_without_strategy();
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

