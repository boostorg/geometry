#include <iostream>
#include <string>

#include "fruit.hpp"


int main()
{
    fruit::apple<> a("my apple");
    eat(a);

    return 0;
}

