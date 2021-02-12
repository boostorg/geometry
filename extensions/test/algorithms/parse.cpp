// Boost.Geometry (aka GGL, Generic Geometry Library)
// Unit Test

// Copyright (c) 2007-2013 Barend Gehrels, Amsterdam, the Netherlands.
// Copyright (c) 2008-2013 Bruno Lalande, Paris, France.
// Copyright (c) 2009-2013 Mateusz Loskot, London, UK.
// Copyright (c) 2013 Adam Wulkiewicz, Lodz, Poland.

// Use, modification and distribution is subject to the Boost Software License,
// Version 1.0. (See accompanying file LICENSE_1_0.txt or copy at
// http://www.boost.org/LICENSE_1_0.txt)

#include<string>

#include<geometry_test_common.hpp>

#include<boost/geometry/geometries/geometries.hpp>
#include<boost/geometry/extensions/algorithms/parse.hpp>


template<typename T, typename Geometry>
void check_parse(Geometry point, T expected_latitude, T expected_longitude)
{
	BOOST_CHECK_CLOSE(bg::get<0>(point), expected_longitude, 0.0001);
	BOOST_CHECK_CLOSE(bg::get<1>(point), expected_latitude, 0.0001);
}

template<typename T, typename Geometry>
void test_parse(std::string const& s1,
	std::string const& s2,
	T expected_latitude,
	T expected_longitude)
{
	Geometry geometry;

	// check both the overloaded versions of parse function without strategy

	bg::parse<Geometry>(geometry, s1, s2);
	check_parse<T, Geometry>(geometry, expected_latitude, expected_longitude);
	bg::parse<Geometry>(geometry, s2, s1);  // reversed order
	check_parse<T, Geometry>(geometry, expected_latitude, expected_longitude);

	geometry = bg::parse<Geometry>(s1, s2);
	check_parse<T, Geometry>(geometry, expected_latitude, expected_longitude);
	geometry = bg::parse<Geometry>(s2, s1);  // reversed order
	check_parse<T, Geometry>(geometry, expected_latitude, expected_longitude);
}

template<typename T, typename Geometry, typename S>
void test_parse(std::string const& s1,
	std::string const& s2,
	T expected_latitude,
	T expected_longitude,
	S const& strategy)
{
	Geometry geometry;
	bg::parse<Geometry, S>(geometry, s1, s2, strategy);
	check_parse<T, Geometry>(geometry, expected_latitude, expected_longitude);

	// checking reversed order of strings
	bg::parse<Geometry, S>(geometry, s2, s1, strategy);
	check_parse<T, Geometry>(geometry, expected_latitude, expected_longitude);
}

template<typename Geometry>
void test_parse_without_strategy()
{
	typedef typename bg::coordinate_type<Geometry>::type coord_t;

	// test for degree coodinates 
	test_parse<coord_t, Geometry>("45d 30' 30\" N", "120D 30'45\"W", 0.7942705942, -2.1033408163);
	test_parse<coord_t, Geometry>("32D45' 57\"n", "170d 10' 25\"E", 0.5718719189, 2.9700910868);
	test_parse<coord_t, Geometry>("5d38'40\"S", "168D 10' 20\"w", -0.0985141822, -2.9351602461);
	test_parse<coord_t, Geometry>("72D 20'45\"s", "5d 40' 55\"e", -1.2626735329, 0.0991686810);

	// test for degree and radian coordinates
	test_parse<coord_t, Geometry>("2.5r N", "0.75r E", 2.5000000000, 0.7500000000);
	test_parse<coord_t, Geometry>("1.25r S", "120D 30' 45\"w", -1.2500000000, -2.1033408163);
}

template<typename Geometry>
void test_parse_with_strategy()
{
	typedef typename bg::coordinate_type<Geometry>::type coord_t;
	// creating a strategy with random characters to check generality of the dms_parser
	typedef bg::projections::detail::dms_parser
		<coord_t, // coordinate type of geometry
		true,     // whether the returned value should be in radian
		'A',      // for positive latitude
		'B',      // for positive longitude
		'C',      // for negative latitude
		'D',      // for negative longitude 
		'm',      // symbol denoting minute
		's',      // symbol denoting seconds
		'o',      // symbol denoting degree 
		'p'>  dms_parser_t1;

	// checking parser with random symbols assigned above
	test_parse<coord_t, Geometry>("45o 30m 30s A", "120o 30m 45s D", 0.7942705942, -2.1033408163, dms_parser_t1());
	test_parse<coord_t, Geometry>("1.75p c", "5o 40m 55s b", -1.7500000000, 0.0991686810, dms_parser_t1());

	// creating another strategy with returning value as degree
	typedef bg::projections::detail::dms_parser
		<coord_t,
		false>  dms_parser_t2; // return value in degree and rest parameters as default value.

	// cheking parser with dms_parser_t2 strategy
	test_parse<coord_t, Geometry>("80d 45' 30\"S", "150d 30' 30\"E", -80.7583333333, 150.5083333333, dms_parser_t2());
	test_parse<coord_t, Geometry>("15d 10' 20\"N", "2.75r W", 15.1722222222, -157.5633261332, dms_parser_t2());
}

int test_main(int, char* [])
{

	typedef bg::model::point<float, 2, bg::cs::geographic<bg::degree>> geod;
	typedef bg::model::point<float, 2, bg::cs::geographic<bg::radian>> geor;
	typedef bg::model::point<float, 2, bg::cs::spherical<bg::degree>> sphd;
	typedef bg::model::point<float, 2, bg::cs::spherical<bg::radian>> sphr;
	typedef bg::model::point<float, 2, bg::cs::spherical_equatorial<bg::degree>> spheqd;
	typedef bg::model::point<float, 2, bg::cs::spherical_equatorial<bg::radian>> spheqr;
	// test with default strategy
	test_parse_without_strategy<geod>();
	test_parse_without_strategy<geor>();
	test_parse_without_strategy<sphd>();
	test_parse_without_strategy<sphr>();
	test_parse_without_strategy<spheqd>();
	test_parse_without_strategy<spheqr>();

	// test with strategy
	test_parse_with_strategy<geod>();
	test_parse_with_strategy<geor>();
	test_parse_with_strategy<sphd>();
	test_parse_with_strategy<sphr>();
	test_parse_with_strategy<spheqd>();
	test_parse_with_strategy<spheqr>();
	return 0;
}