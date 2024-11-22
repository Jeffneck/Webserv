#include "templates.hpp"
#include <iostream>

int main() {

    {
        std::cout << "---- SUBJECT MAIN ----\n" << std::endl;
        int a = 2;
        int b = 3;
        ::swap( a, b );
        std::cout << "a = " << a << ", b = " << b << std::endl;
        std::cout << "min( a, b ) = " << ::min( a, b ) << std::endl;
        std::cout << "max( a, b ) = " << ::max( a, b ) << std::endl;
        std::string c = "chaine1";
        std::string d = "chaine2";
        ::swap(c, d);
        std::cout << "c = " << c << ", d = " << d << std::endl;
        std::cout << "min( c, d ) = " << ::min( c, d ) << std::endl;
        std::cout << "max( c, d ) = " << ::max( c, d ) << std::endl;
    }

    {
        std::cout << "\n---- IMPROVED MAIN ----\n" << std::endl;

        std::cout << "----Test avec des entiers" << std::endl;
        int a = 10;
        int b = 20;

        std::cout << "Avant swap : a = " << a << ", b = " << b << std::endl;
        std::cout << "Max(a, b) = " << ::max(a, b) << std::endl;
        std::cout << "Min(a, b) = " << ::min(a, b) << std::endl;

        ::swap(a, b);
        std::cout << "Après swap : a = " << a << ", b = " << b << std::endl;

        std::cout << "\n----Test avec des doubles" << std::endl;
        double x = 3.14;
        double y = 2.71;

        std::cout << "Avant swap : x = " << x << ", y = " << y << std::endl;
        std::cout << "Max(x, y) = " << ::max(x, y) << std::endl;
        std::cout << "Min(x, y) = " << ::min(x, y) << std::endl;

        ::swap(x, y);
        std::cout << "Après swap : x = " << x << ", y = " << y << std::endl;

        std::cout << "\n----Test avec des chaînes de caractères" << std::endl;
        std::string str1 = "apple";
        std::string str2 = "banana";

        std::cout << "Avant swap : str1 = " << str1 << ", str2 = " << str2 << std::endl;
        std::cout << "Max(str1, str2) = " << ::max(str1, str2) << std::endl;
        std::cout << "Min(str1, str2) = " << ::min(str1, str2) << std::endl;

        ::swap(str1, str2);
        std::cout << "Après swap : str1 = " << str1 << ", str2 = " << str2 << std::endl;
    }

    return 0;
}

