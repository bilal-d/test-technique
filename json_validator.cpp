#include <iostream>
#include <functional>

// J'aurais besoin de pouvoir représenter un token JSON.
// Concernant la validation, seul le type du token nous intéresse.
// On peut donc penser à une énumération, mais pour permettre la
// possibilité d'étendre l'usage en incluant la valeur parsée,
// ou une métadonnée comme le numéro de ligne/numéro de colonne
// où a été observé le token dans le flux d'entrée, je choisis
// une structure simple munie d'une énumération.
//
// TODO: les valeurs sont temporaires, pour me faire une idée.

struct JsonToken
{
    enum Type {
        UNEXPECTED = 0,
        END_OF_STREAM,
        WHITESPACE,
        OBJECT_START,
        OBJECT_END,
        ARRAY_START,
        ARRAY_END,
        COMMA,
        COLON,
        NULL_VALUE,
        BOOLEAN_VALUE,
        NUMBER_VALUE,
        STRING_VALUE,
    };

    Type type;
};


// J'inclus ici une partie permettant la lecture d'un flux de caractère,
// avec mise en mémoire d'un caractère.  L'usage d'une std::function apporte
// un coût en termes de performances (à cause de l'indirection), cela devra
// être mesuré et optimisé au besoin.
// L'intérêt de cette légère classe est de permettre l'ajout d'un décodeur
// UTF-8 ou autre si nécessaire.
// Il est possible d'utiliser les fonctionnalités des streams C++ (avec les
// méthodes std::istream::peek et std::istream::putback), mais je préfère
// découpler les choses à ce niveau (car l'entrée ne sera pas toujours un
// std::istream, cela peut être un contenu en mémoire ou un flux en
// provenance d'une connexion réseau).
//
struct JsonCharReader {
    std::function<bool (char&)> supply;
    char current;

    bool advance() {
        return supply(current);
    }
};


// C'est à ce niveau que se fera l'analyse lexicale.  En étant muni d'un
// entrée de caractères, il faudra discriminer le token que nous sommes en
// train de lire.  À première vue, le premier caractère est suffisant pour
// cela.
struct JsonTokenReader {
    JsonCharReader& charReader;

    JsonToken nextToken() {
        // TODO
        return { JsonToken::UNEXPECTED };
    }
};


int main(int, char*[])
{
    std::cout << "not implemented\n";
    return 0;
}
