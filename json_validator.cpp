#include "json_validator.hpp"
#include <iostream>

int main(int, char*[])
{
    JsonCharReader charReader([&](char& c) {
        return bool(std::cin.get(c));
    });
    JsonTokenReader tokenReader(charReader);
    JsonParser parser(tokenReader);

    bool const validated = parser.validate();
    std::cout << (validated ? "vrai" : "faux") << "\n";
}
