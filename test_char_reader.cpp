#include "json_validator.hpp"
#include <iostream>

// voici un simple programme qui fonctionne comme un rudimentaire `cat` sous Unix.
// Le programme copie l'entrée standard vers la sortie standard, un caractère à
// la fois.
int main(int, char*[])
{
    JsonCharReader reader([&](char& c) {
        return bool(std::cin.get(c));
    });

    while (reader.advance()) {
        std::cout << reader.current();
    }
    std::cout << std::flush;
}
