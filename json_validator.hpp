#ifndef JSON_VALIDATOR_HPP_INCLUDED
#define JSON_VALIDATOR_HPP_INCLUDED

#include <functional>
#include <string>

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
    JsonCharReader(std::function<bool (char&)> input) :
    supply(input), value('\0'), stored(false) {}

    char current() {
        return value;
    }

    void storeLast() {
        stored = true;
    }

    bool advance() {
        if (!stored)
            return supply(value);
        stored = false;
        return true;
    }

private:
    std::function<bool (char&)> supply;
    char value;
    bool stored;
};


// C'est à ce niveau que se fera l'analyse lexicale.  En étant muni d'un
// entrée de caractères, il faudra discriminer le token que nous sommes en
// train de lire.  À première vue, le premier caractère est suffisant pour
// cela.
struct JsonTokenReader {
    JsonCharReader& charReader;

    JsonToken nextToken() {
        if (!charReader.advance())
            return { JsonToken::END_OF_STREAM };

        switch (charReader.current()) {
        case '{': return { JsonToken::OBJECT_START };
        case '}': return { JsonToken::OBJECT_END };
        case '[': return { JsonToken::ARRAY_START };
        case ']': return { JsonToken::ARRAY_END };
        case ':': return { JsonToken::COLON };
        case ',': return { JsonToken::COMMA };

        case 't':
            charReader.storeLast();
            if (readLitteral("true"))
                return { JsonToken::BOOLEAN_VALUE };
            return { JsonToken::UNEXPECTED };

        case 'f':
            charReader.storeLast();
            if (readLitteral("false"))
                return { JsonToken::BOOLEAN_VALUE };
            return { JsonToken::UNEXPECTED };

        case 'n':
            charReader.storeLast();
            if (readLitteral("null"))
                return { JsonToken::NULL_VALUE };
            return { JsonToken::UNEXPECTED };

        case ' ':
        case '\t':
        case '\r':
        case '\n':
            consumeWhitespace();
            return { JsonToken::WHITESPACE };

            // TODO: énumération incomplète, il reste encore à détecter et à valider
            // les types de tokens suivants:
            // - string
            // - number
        }

        return { JsonToken::UNEXPECTED };
    }

private:
    bool readLitteral(std::string_view litteral) {
        for (char character : litteral) {
            if (!charReader.advance())
                return false;
            if (charReader.current() != character)
                return false;
        }
        return true;
    }
    void consumeWhitespace() {
        // tant que l'on peut lire des caractères blanc, on poursuivra
        // la lecture.  On s'arrête si le flux est épuisé, ou si le
        // dernier caractère lu n'est pas blanc.  Dans ce cas on garde
        // ce caractère pour une lecture ultérieur; on ne le consomme pas.
        while (charReader.advance()) {
            char const c = charReader.current();
            if (c == ' ' || c == '\t' || c == '\r' || c == '\n')
                continue;

            charReader.storeLast();
            break;
        }
    }
};

#endif  // JSON_VALIDATOR_HPP_INCLUDED
