#ifndef FRUIT_HPP
#define FRUIT_HPP


#include <iostream>
#include <string>

/*!
\defgroup eat eat: Eating function
*/

namespace fruit
{
/*!
\brief An apple
\details The apple is the pomaceous fruit of the apple tree,
    species Malus domestica in the rose family (Rosaceae)
\tparam String the string-type (string,wstring,utf8-string,etc)

\qbk{before.synopsis,
[heading Model of]
Fruit Concept
}
*/
template <typename String = std::string>
class apple
{
    String sort;

public :
    /// constructor
    apple(String const& s) : sort(s) {}

    /// Get the name
    // (more doxygen e.g. @return, etc)
    String const& name() const { return sort; }
};


/*!
\brief Eat it
\ingroup eat
\details Eat the fruit
\param fruit the fruit
\tparam T the fruit type

\qbk{
[heading Example]
[apple]
[apple_output]
}
*/
template <typename T>
void eat(T const& fruit)
{
    std::cout << fruit.name() << std::endl;
}

}


#endif
