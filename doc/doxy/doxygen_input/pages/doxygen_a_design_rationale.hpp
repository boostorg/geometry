// Boost.Geometry (aka GGL, Generic Geometry Library)
//
// Copyright Barend Gehrels 2007-2009, Geodan, Amsterdam, the Netherlands
// Copyright Bruno Lalande 2008, 2009
// Use, modification and distribution is subject to the Boost Software License,
// Version 1.0. (See accompanying file LICENSE_1_0.txt or copy at
// http://www.boost.org/LICENSE_1_0.txt)

#ifndef _DOXYGEN_DESIGN_RATIONALE_HPP
#define _DOXYGEN_DESIGN_RATIONALE_HPP


//---------------------------------------------------------------------------------------------------

/*!
\page design Design rationale




\section par1 Introduction

Suppose you need C++ software to calculate the distance between two points. You might define a struct:

\code
struct mypoint
{
    double x, y;
};
\endcode

And a function, containing the algorithm:
\code
double distance(mypoint const& a, mypoint const& b)
{
    double dx = a.x - b.x;
    double dy = a.y - b.y;
    return sqrt(dx * dx + dy * dy);
}
\endcode

Quite simple, and it is usable, but not generic. For a library it has to be designed way further. The design above can only be used for 2D points, for the struct \b mypoint (and no other struct), in a Cartesian coordinate system.
A generic library should be able to calculate the distance:
- for any point class or struct, not on just this \b mypoint type
- in more than two dimensions
- for other coordinate systems, e.g. over the earth or on a sphere
- between a point and a line or between other geometry combinations
- in other (e.g. higher) precision than ‘double’
- avoiding the square root: often we don’t want to calculate the square because it is a relatively expensive function, and for comparing distances it is not necessary

In this page we will make the design step by step more generic.




\section par2 Using templates
The distance function can be changed into a template function. This is trivial and allows calculating the distance between other point types than just \b mypoint. We add two template parameters, allowing input of two different point types.

\code
template <typename P1, typename P2>
double distance(P1 const& a, P2 const& b)
{
    double dx = a.x - b.x;
    double dy = a.y - b.y;
    return sqrt(dx * dx + dy * dy);
}
\endcode

This template version is slightly better, but not much. Consider a C++ class where member variables are protected… Such a class does not allow to access .x and .y directly. So this paragraph is short and we just go on.




\section par3 Using traits
We need to take a generic approach, to allow any point type as input to the distance function. Instead of accessing .x and .y, we will add a few levels of indirection, using a traits system.
The distance function then becomes:
\code
template <typename P1, typename P2>
double distance(P1 const& a, P2 const& b)
{
    double dx = get<0>(a) - get<0>(b);
    double dy = get<1>(a) - get<1>(b);
    return sqrt(dx * dx + dy * dy);
}
\endcode

This adapted distance function uses a generic get-function, with dimension as a template parameter, to access the coordinates of a point. This get-function forwards to a traits system, defined as following:
\code
namespace traits
{
    template <typename P, int D>
    struct access {};
}
\endcode

which is then specialized for our \b mypoint type, implementing a static method called ‘get’:
\code
namespace traits
{
    template <>
    struct access<mypoint, 0>
    {
        static double get(mypoint const& p)
        {
            return p.x;
        }
    };
    // same for 1: p.y
    ...
}
\endcode

Calling traits::access<mypoint, 0>::get(a) now returns us our x-coordinate. Nice? It is quite verbose for a function like this, used so often in the library. We can shorten the syntax by adding an extra \b free \b function:
\code
template <int D, typename P>
inline double get(P const& p)
{
    return traits::access<P, D>::get(p);
}
\endcode

This enables us to call get<0>(a), for any point having the traits::access specialization, as shown in the distance algorithm at the start of this paragraph.
So we wanted to enable classes with methods like .x(), and they are supported as long as there is a specialization of the access struct with a static get function returning .x() for dimension 0, and similar for 1 and .y().

Alternatively we could implement, in the traits class, the dimension as a template parameter in a member template function:

\code
template <>
struct access<mypoint>
{
    template <int D>
    static double get(mypoint const& p)
    // either return x/y using an if-clause
    // or call a detail-struct specialized
    //   per dimension
};
\endcode

This alternative gives in the end the same functionality. It however is either using an if-clause (which is not compile-time, so not preferred; it is slightly slower), or adding another level of indirection (a sub-structure specialized on dimension). Therefore the form with the dimension as a template parameter of the struct is preferred.






\section par4 Dimension agnosticism
Now we can calculate the distance between points in 2D, points of any structure or class. However, we wanted to have 3D as well. So we have to make it dimension agnostic.
This complicates our distance function. We can use a for-loop to walk through dimensions, but for loops have another performance than the straightforward coordinate addition which was there originally. However, we can make more usage of templates and implement the distance algorithm as following; it is more complex but attractive for template fans:
\code
template <typename P1, typename P2, int D>
struct pythagoras
{
    static double apply(P1 const& a, P2 const& b)
    {
        double d = get<D-1>(a) - get<D-1>(b);
        return d * d + pythagoras<P1, P2, D-1>::apply(a, b);
   }
};

template <typename P1, typename P2 >
struct pythagoras<P1, P2, 0>
{
    static double apply(P1 const&, P2 const&)
    {
        return 0;
    }
};
\endcode

The distance function is calling that pythagoras struct, specifying the number of dimensions:
\code
template <typename P1, typename P2>
double distance(P1 const& a, P2 const& b)
{
    BOOST_STATIC_ASSERT(( dimension<P1>::value == dimension<P2>::value ));

    return sqrt(pythagoras<P1, P2, dimension<P1>::value>::apply(a, b));
}
\endcode

The dimension which is referred to is defined using another traits class:
\code
namespace traits
{
    template <typename P>
    struct dimension {};
}
\endcode

which has to be specialized again for the struct \b mypoint. Because it only has to publish a value, it can be conveniently derived from the Boost Meta-Programming Library (MPL) class boost::mpl::int_:
\code
namespace traits
{
    template <>
    struct dimension<mypoint>
        : boost::mpl::int_<2>
    {};
}
\endcode

Like the free function \b get, the library also contains a metafunction \b dimension.
\code
template <typename P>
struct dimension : traits::dimension<P>
{};
\endcode

The extra declaration is convenient to not call the traits namespace. But there is a more important reason why it is useful. This is explained below.
Now we have agnosticism in the number of dimensions. Our more generic distance function now accepts points of three or more dimensions. The compile-time assertion will prevent point \b a having two dimension and point \b b having three dimensions.







\section par5 Coordinate type
We assumed double above. What if our points are in integer? We can easily add a traits class, and we will do that. However, the distance between two integer coordinates can still be an irrational value. Besides that, a design goal was to avoid square roots. We handle these cases below, in another paragraph. For the moment we keep returning double, but we allow integer coordinates for our point types.
To define the coordinate type, we add another traits class, coordinate_type, which should be specialized by the library user:
\code
namespace traits
{
    template <typename P>
    struct coordinate_type{};

    // specialization for our mypoint
    template <>
    struct coordinate_type<mypoint>
    {
        typedef double type;
    };
}
\endcode

Like the access function, where we had a free get function, we add a proxy here as well. A longer version is presented later on, the short function would look like this:

\code
template <typename P>
struct coordinate_type :    traits::coordinate_type<P> {};
\endcode

We now can modify our distance algorithm again. Because it still returns double, we only modify the Pythagoras computation class.  It should return the coordinate type of its input. But... it has two, possibly different, input point types. They might also differ in their coordinate types. Not that that is very likely, but we’re designing a generic library and we should handle those strange cases.
We have to choose one of the coordinate types and of course we select the one with the highest precision. This is not worked out here, it would be too long, and it is not related to geometry.
We just assume that there is a metafunction \b select_most_precise selecting the best type.
So our computation class becomes:
\code
template <typename P1, typename P2, int D>
struct pythagoras
{
    typedef typename select_most_precise
        <
            typename coordinate_type<P1>::type,
            typename coordinate_type<P2>::type
        >::type computation_type;

    static computation_type apply(P1 const& a, P2 const& b)
    {
        computation_type d = get<D-1>(a) - get<D-1>(b);
        return d * d + pythagoras <P1, P2, D-1> ::apply(a, b);
   }
};
\endcode





\section par6 Different geometries
At this point, we’ve designed a dimension agnostic system supporting any point type of any coordinate type. There are still some tweaks but they will be worked out later.
Now we will see how we calculate the distance between a point and a polygon, or between a point and a line-segment. These formulae are more complex, and the influence on design is even larger.
We don’t want to add a function with another name:
\code
template <typename P, typename S>
double distance_point_segment(P const& p, S const& s)
\endcode

We want to be generic, the distance function has to be called from code not knowing the type of geometry it handles, so it has to be named distance. We also cannot create an overload because that would be ambiguous, having the same template signature.
There are two solutions to avoid this ambiguity:
- tag dispatching
- SFINAE

We select tag dispatching because it fits into the traits system, and also because SFINAE has several drawbacks, listed in another paragraph.
With tag dispatching the distance algorithm inspects the type of geometry of the input parameters. The distance function is changed into:
\code
template <typename G1, typename G2>
double distance(G1 const& g1, G2 const& g2)
{
    return dispatch::distance
        <
            typename tag<G1>::type,
            typename tag<G2>::type,
            G1, G2
        >::apply(g1, g2);
}
\endcode

It is referring to a metafunction called \b tag and forwarding the call to the method \b apply of a dispatch::distance structure. The tag metafunction is another traits class, and should be specialized for per point type, both shown here:
\code
namespace traits
{
    template <typename G>
    struct tag {};

    // specialization
    template <>
    struct tag<mypoint>
    {
        typedef point_tag type;
    };
}
\endcode

There is again a free metafunction, like we did for \b coordinate_system and \b get:
\code
template <typename G>
struct tag : traits::tag<G> {};
\endcode

Tags (point_tag, segment_tag, etc) are empty structures with the sole purpose to specialize a dispatch class.
The dispatch class for distance, and its specializations, are all defined in a separate namespace and look like the following:

\code
namespace dispatch {
template < typename Tag1, typename Tag2, typename G1, typename G2 >
struct distance
{};

template <typename P1, typename P2>
struct distance < point_tag, point_tag, P1, P2 >
{
    static double apply(P1 const& a, P2 const& b)
    {
        // here we call pythagoras
        // exactly like we did before
        ...
    }
};

template <typename P, typename S>
struct distance
<
    point_tag, segment_tag, P, S
>
{
    static double apply(P const& p, S const& s)
    {
        // here we refer to another function
        // implementing point-segment
        // calculations in 2 or 3
        // dimensions...
        ...
    }
};
// here we might have many more
// specializations,
// for point-polygon, box-circle, etc.

} // namespace
\endcode

So yes, it is possible; the distance algorithm is generic now in the sense that it also supports different geometry types. One drawback: we have to define two dispatch specializations for point - segment and for segment - point separately. That will also be solved, in the paragraph reversibility below.
The example below shows where we are now: different point types, geometry types, dimensions.
\code
point a(1,1);
point b(2,2);
std::cout << distance(a,b) << std::endl;
segment s1(0,0,5,3);
std::cout << distance(a, s1) << std::endl;
rgb red(255, 0, 0);
rbc orange(255, 128, 0);
std::cout << "color distance: " << distance(red, orange) << std::endl;
\endcode





\section par7 Kernel revisited

We described above that we had a traits class coordinate_type, defined in namespace traits, and defined a separate coordinate_type class as well. This was actually not really necessary before, because the only difference was the namespace clause. But now that we have another geometry type, a segment in this case, it is essential. We can call the coordinate_type metafunction for any geometry type, point, segment, polygon, etc, implemented again by tag dispatching:

\code
template <typename G>
struct coordinate_type
{
    typedef typename dispatch::coordinate_type
        <
            typename tag<G>::type, G
        >::type type;
};
\endcode

Inside the dispatch namespace this metafunction is implemented twice: a generic version and one specialization for points. The specialization for points calls the traits class. The generic version calls the point specialization, as a sort of recursive metafunction definition:

\code
namespace dispatch
{

// Version for any geometry:
template <typename GeometryTag, typename G>
struct coordinate_type
{
    typedef typename point_type
        <
            GeometryTag, G
        >::type point_type;

    // Call specialization on point-tag
    typedef typename coordinate_type < point_tag, point_type >::type type;
};

// Specialization for point-type:
template <typename P>
struct coordinate_type<point_tag, P>
{
    typedef typename
        traits::coordinate_type<P>::type
        type;
};
\endcode


So it calls another metafunction point_type. This is not elaborated in here but realize that it is available for all geometry types, and typedefs the point type which makes up the geometry, calling it type.

The same applies for the metafunction dimension and for the upcoming metafunction coordinate_system.





\section par8 Coordinate system
Until here we assumed a Cartesian system. But we know that the Earth is not flat. Calculating a distance between two GPS-points with the system above would result in nonsense. So we again extend our design. We define for each point type a coordinate system type, using the traits system again. Then we make the calculation dependant on that coordinate system.

Coordinate system is similar to coordinate type, a metafunction, calling a dispatch function to have it for any geometry-type, forwarding to its point specialization, and finally calling a traits class, defining a typedef type with a coordinate system. We don’t show that all here again. We only show the definition of a few coordinate systems:
\code
struct cartesian {};

template<typename DegreeOrRadian>
struct geographic
{
    typedef DegreeOrRadian units;
};
\endcode

So Cartesian is simple; for geographic we can also select if its coordinates are stored in degrees or in radians.

The distance function will now change: it will select the computation method for the corresponding coordinate system and then call the dispatch struct for distance. We call the computation method specialized for coordinate systems a \b strategy. So the new version of the distance function is:

\code
template <typename G1, typename G2>
double distance(G1 const& g1, G2 const& g2)
{
    typedef typename default_distance_strategy
        <
            typename point_type<G1>::type,
            typename point_type<G2>::type,
            typename coordinate_system<G1>::type,
            typename coordinate_system<G2>::type,
            dimension<G1>::value
        >::type strategy;

    return dispatch::distance
        <
            typename tag<G1>::type,
            typename tag<G2>::type,
            G1, G2, strategy
        >::apply(g1, g2, strategy());
}
\endcode

The default_distance_strategy mentioned here is a metafunction with specializations for different coordinate systems.

\code
template <typename P1, typename P2, typename T1, typename T2, int D>
struct default_distance_strategy
{
    // undefined; type is not defined
};

template <typename P1, typename P2, int D>
struct default_distance_strategy<P1, P2, cartesian, cartesian, D>
{
    typedef pythagoras<P1, P2, D> type;
};
\endcode

So here is our Pythagoras again, now defined as a strategy. The distance dispatch function just calls its apply method.

So this is an important step: for spherical or geographical coordinate systems, another strategy (computation method) can be implemented. For spherical coordinate systems we have the haversine formula. So the dispatching traits struct is specialized like this:
\code
template <typename P1, typename P2, int D = 2>
struct default_distance_strategy<P1, P2, spherical, spherical, D>
{
    typedef haversine<P1, P2> type;
};

// struct haversine with apply function
// is omitted here
\endcode

For geography, we have some alternatives for distance calculation. There is the Andoyer method, fast and precise, and there is the Vincenty method, slower and more precise, and there are some less precise approaches as well.

Per coordinate system, one strategy is defined as the default strategy.
To be able to use another strategy as well, we modify our design again and add an overload for the distance algorithm, taking a strategy object as a third parameter.

This new overload distance function also has the advantage that the strategy can be constructed outside the distance function. Because it was constructed inside above, it could not have construction parameters. But for Andoyer or Vincenty, or the haversine formula, it certainly makes sense to have a constructor taking the radius of the earth as a parameter.
So the distance overloaded function is:

\code
template <typename G1, typename G2, typename S>
double distance(G1 const& g1, G2 const& g2, S const& strategy)
{
    return dispatch::distance
        <
            typename tag<G1>::type,
            typename tag<G2>::type,
            G1, G2, S
        >::apply(g1, g2, strategy);
}
\endcode

The strategy has to have a method \b apply taking two points as arguments (for points). It is not required that it is a static method. A strategy
might define a constructor, where a configuration value is passed and stored as a member variable. In those cases a static
method would be inconvenient. It can be implemented as a normal method (with the const qualifier).

We do not list all implementations here, Vincenty would cover half a page of mathematics, but you will understand the idea. We can call distance like this:
\code
distance(c1, c2)
\endcode

where c1,c2 are Cartesian points, or like this:

\code
distance(g1, g2)
\endcode

where g1,g2 are Geographic points, calling the default strategy for Geographic points (e.g. Andoyer), and like this:

\code
distance(g1, g2, vincenty<G1, G2>(6275))
\endcode

where a strategy is specified explicitly and constructed with a radius.










\section par9 Point concept
The five traits classes mentioned in the paragraphs above form together the Point Concept. Any point type for which specializations are implemented in the traits namespace should be handled correctly by the library. So the Point concept consists of:
- a specialization for traits::tag
- a specialization for traits::coordinate_system
- a specialization for traits::coordinate_type
- a specialization for traits::dimension
- a specialization for traits::access

The last one is a class, containing the method \b get and the (optional) method \b set, the first four are metafunctions, either defining \b type or declaring \b value (conform MPL conventions).
So we now have agnosticism for the number of dimensions, agnosticism for coordinate systems, our design can handle any coordinate type, and it can handle different geometry types.
Furthermore we can specify our own strategies, the code will not compile in case of two points with different dimensions (because of the assertion), and it will not compile for two points with different coordinate systems (because there is no specialization).
We still have the distance function returning double. Next paragraph will solve that.



\section par10 Return type
We promised that calling sqrt was not always necessary. So we define a distance result struct that contains the squared value and is convertible to a double value.
This, however, only has to be done for Pythagoras. The spherical distance functions do not take the square root so for them it is not necessary to avoid the expensive square root call; they can just return their distance.
So the distance result struct is dependant on strategy, therefore made a member type of the strategy. The result struct looks like this:
\code
template<typename T = double>
struct cartesian_distance
{
    T sq;
    explicit cartesian_distance(T const& v) : sq (v) {}

    inline operator T() const
    {
        return std::sqrt(sq);
    }
};
\endcode

It also has operators defined to compare itself to other results without taking the square root.

Each strategy should define its return type, within the strategy class, e.g.:

\code
    typedef cartesian_distance<T> return_type;
\endcode
or:

\code
    typedef double return_type
\endcode

for Pythagoras and spherical, respectively.

Again our distance function will be modified, as expected, to reflect the new return type. For the overload with a strategy it is not complex:
\code
template < typename G1, typename G2, typename Strategy >
typename Strategy::return_type distance( G1 const& G1 , G2 const& G2 , S const& strategy)
\endcode

But for the one without strategy we have to select strategy, coordinate type, etc. It would be spacious to do it in one line so we add a separate metafunction:
\code
template <typename G1, typename G2 = G1>
struct distance_result
{
    typedef typename point_type<G1>::type P1;
    typedef typename point_type<G2>::type P2;
    typedef typename default_distance_strategy
        <
            P1, P2,
            typename cs_tag<P1>::type,
            typename cs_tag<P2>::type
        >::type S;

    typedef typename S::return_type type;
};
\endcode

and modify our distance function:
\code
template <typename G1, typename G2>
inline typename distance_result<G1, G2>::type distance(G1 const& g1, G2 const& g2)
{ ... }
\endcode

Of course also the apply functions in the dispatch specializations will return a result like this. They have a strategy as a template parameter everywhere, making the less verbose version possible.




\section par11 Reversibility
Our dispatch::distance class was specialized for pairs of geometries, like <point_tag, point_tag> or <point_tag, segment_tag>. But library users can also call the distance function with a segment and a point, in that order. Actually, there are many geometry types (polygon, box, linestring), how to implement all combinations without duplicating all tag dispatching functions?
The answer is that we automatically reverse the arguments, if appropriate. For distance it is appropriate because distance is a commutative function.
We add a metafunction geometry_id, which has specializations for each geometry type, just derived from boost::mpl::int_, such that it can be ordered. Point is 1, segment is e.g. 2.

Then we add a metafunction reverse_dispatch:
\code
template <typename G1, typename G2>
struct reverse_dispatch : detail::reverse_dispatch
    <
        geometry_id<G1>::type::value,
        geometry_id<G2>::type::value
    >
{};
\endcode

Because of the order in geometry_id, we can arrange (template) parameters in that order, in specializations. So the detail structure looks like:
\code
namespace detail
{
    template <int Id1, int Id2>
    struct reverse_dispatch : boost::mpl::if_c
        <
            (Id1 > Id2),
            boost::true_type,
            boost::false_type
        >
    {};
\endcode

And our distance function will be modified again with some template meta-programming:

\code
return boost::mpl::if_c
    <
        boost::geometry::reverse_dispatch <G1, G2>::type::value,
        detail::distance_reversed
            <
                typename tag<G1>::type,
                typename tag<G2>::type,
                G1, G2,
                // strategy
            >,
        dispatch::distance
            <
                typename tag<G1>::type,
                typename tag<G2>::type,
                G1, G2,
                // strategy
            >
        >
    >::type::apply(g1, g2, s);
\endcode

Where the detail::distance_reversed is a specific struct, forwarding its call to dispatch::distance, reversing all its template arguments and function arguments.


\section par13 Multi-geometries
Consider Hawaii as a multi_polygon: it is a set of islands (polygons), forming together one entity. The shortest distance from a ship to Hawaii is the shortest distance to the closest island. That algorithm also holds for the shortest distance from a point to a multi-line or a multi-point.
So we don't want to implement that functionality three times. Instead we add a metafunction, is_multi, which is always true for a multi-geometry (multi-point, multi-linestring, multi-polygon) and always false for the single versions.

\code
template <typename Geometry>
struct is_multi : core_dispatch::is_multi<typename tag<Geometry>::type>
{};


namespace dispatch
{

template <typename GeometryTag>
struct is_multi : boost::false_type {};

template <>
struct is_multi<multi_point_tag> : boost::true_type {};


template <>
struct is_multi<multi_linestring_tag> : boost::true_type {};


template <>
struct is_multi<multi_polygon_tag> : boost::true_type {};

} // namespace dispatch
\endcode


Now we can specialize on is_multi, so we add two boolean IsMulti's, one for each geometry type, to our distance dispatch struct and make the call to them in the distance function:

\code
// specialization:
template <typename GeometryTag1, typename GeometryTag2, typename G1, typename G2, typename Strategy>
struct distance
    <
        GeometryTag1, GeometryTag2, G1, G2, Strategy,
        false, // G1 is_multi == false, so dispatch a 'single' geometry
        true   // G2 is_multi == true, so dispatch a multi-geometry
    >
   : detail::distance::distance_single_to_multi<G1, G2, Strategy>
{};
\endcode




If that relatively simple change is done, we have to do one thing: implement the single-to-multi distance implementation structure

\code
template<typename Geometry, typename MultiGeometry, typename Strategy>
struct distance_single_to_multi
{
   typedef typename Strategy::return_type return_type;

   static return_type apply(Geometry const& geometry,
               MultiGeometry const& multi,
               Strategy const& strategy)
   {
       using namespace boost;

       return_type mindist = make_distance_result<return_type>(
                       numeric::bounds<typename select_coordinate_type<Geometry, MultiGeometry>::type>::highest());

       typedef typename range_const_iterator<MultiGeometry>::type iterator;
       for(iterator it = begin(multi); it != end(multi); ++it)
       {
           return_type dist = boost::geometry::distance(geometry, *it);
           if (dist < mindist)
           {
               mindist = dist;
           }
       }

       return mindist;
   }
};
\endcode



which iterates over a collection of multi-shapes, and returns the shortest distance. Another function can implement the multi-to-multi function. Note that because of reversibility we don't have to implement the multi-to-single version (as long as multi's always have an higher ID than single's).



\section par14 SFINAE
Instead of tag dispatching we alternatively could have chosen for SFINAE, mentioned above. With SFINAE (Substitution Failure Is Not An Error) we add optional parameters to the distance function, which sole use is to make an overload invalid for other geometry types than specified. So like:
\code
template <typename P1, typename P2>
inline double distance(P1 const& p1, P2 const& p2
    , typename boost::enable_if <is_point<P1>   >::type* = 0
    , typename boost::enable_if <is_point<P2>   >::type* = 0
    )
    {
        return impl::distance::point_to_point(p1, p2);
    }
\endcode

There would then be overloads for point-segment, point-polygon, etc.
This SFINAE:
- gives often compiler troubles and headaches: if a user makes an error somewhere, the compiler will not select any of the methods, and/or it will give completely incomprehensible error listings, just because of this SFINAE.
Stated otherwise (pasted from an answer on the list): With SFINAE the real error is hidden behind the phrase "Failed to specialize". The compiler discards all overloads, because of an error somewhere, and you get this error with no clue how to go on. What you get is the error that it is just failing. All overloads are gone, the compiler is not wrong, there is an error somewhere, but the only visible error message which makes sense for the compiler to give is something like "failed to specialize" or "no matching function call". With tag dispatching you get the real error message. That can also be difficult, but the message(s), sometimes a whole list, give at least a clue of what's wrong. In this case: add the banana-tag or add an implementation for banana. The usage of concepts should reduce the length of the list and give a clearer error message.
- So the essence is: compiler errors in code based on tag dispatching are easier to find than compiler errors in code based on SFINAE, because the SFINAE-case is based on discarding overloads and <i>meaningful error messages are discarded as well</i>
- the combination of SFINAE and the BCCL using boost-concept-requires has been quite difficult, or impossible
- does not support partial specializations because it is a function. The tag-dispatching function is of course also not supporting that, but it forwards its call to the dispatch struct where partial specializations (and member template functions) are possible. The SFINAE could do that as well but then: why not just add one tag more and have tag dispatching instead?
- is a trick to deceive the compiler. “As a language behavior it was designed to avoid programs becoming ill-formed” (http://en.wikipedia.org/wiki/Substitution_failure_is_not_an_error), while tag dispatching is based on specialization, a core feature of C++
- is more verbose (tag dispatching makes the main free function declarations shorter)
- several Boost reviewers appreciated the tag dispatching approach and prefered them over SFINAE


*/

#endif // _DOXYGEN_DESIGN_RATIONALE_HPP
