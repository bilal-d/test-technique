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

    // on pourra au besoin rajouter ici un champs std::string
    // qui servira au tokens qui embarquent une valeur, comme
    // les STRING_VALUE, NUMBER_VALUE ou BOOLEAN_VALUE.
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

        case '"':
            if (readString())
                return { JsonToken::STRING_VALUE };
            return { JsonToken::UNEXPECTED };

        case '0': case '1': case '2': case '3':
        case '4': case '5': case '6': case '7':
        case '8': case '9': case '+': case '-':
            charReader.storeLast();
            if (readNumber())
                return { JsonToken::NUMBER_VALUE };
            return {JsonToken::UNEXPECTED };

            // TODO: énumération incomplète, il reste encore à détecter et à valider
            // les types de tokens suivants:
            // - string
            // - number
            // Par manque de temps, je me limite à une syntaxe très réduite, que l'on peut
            // bien sûr étendre pour parser du vrai JSON (les modifications se feront sur
            // l'analyseur lexical seulement).
            //
            // Pour les strings: je ne prends pas en charge les séquences d'échappement,
            // je commence et je m'arrête au symbol « " »
            // Pour les numbers: je prends en charge que les entiers, munis optionellement
            // d'un signe.
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

    bool readString() {
        while (charReader.advance()) {
            if (charReader.current() == '\"')
                return true;
        }
        return false;
    }

    bool readNumber() {
        if (readNumberSign()) {
            // TODO: le nombre commence par un signe, le conserver pour le conversion
            // ultérieurement
        }

        // au moins un digit doit suivre
        if (!readNumberDigit())
            return false;

        while (readNumberDigit()) {
            // TODO: stocker les digits en vue de les convertir en nombre
            // pour un usage ultérieur, et d'effectuer des vérifications supplémentaires.
        }
        return true;
    }

    bool readNumberSign() {
        if (!charReader.advance())
            return false;

        if (charReader.current() == '+' || charReader.current() == '-')
            return true;

        charReader.storeLast();
        return false;
    }

    bool readNumberDigit() {
        while (!charReader.advance())
            return false;

        if (isDigit(charReader.current()))
            return true;

        charReader.storeLast();
        return false;
    }

    void consumeWhitespace() {
        // tant que l'on peut lire des caractères blanc, on poursuivra
        // la lecture.  On s'arrête si le flux est épuisé, ou si le
        // dernier caractère lu n'est pas blanc.  Dans ce cas on garde
        // ce caractère pour une lecture ultérieur; on ne le consomme pas.
        while (charReader.advance()) {
            if (isWhiteSpace(charReader.current()))
                continue;

            charReader.storeLast();
            break;
        }
    }

    static bool isWhiteSpace(char c) {
        return c == ' ' || c == '\t' || c == '\r' || c == '\n';
    }
    static bool isDigit(char c) {
        return '0' <= c && c <= '9';
    }
};


// Je propose ici un parseur (simplement validant, et ne créant pas un DOM).
// De la même façon que pour la classe JsonTokenReader, on lira les tokens
// et on procédera à la validation du JSON.  On écartera les JsonToken::WHITESPACE.
//
struct JsonParser
{
    JsonParser(JsonTokenReader& input)
    : tokenReader(input), value(), stored(false) {}

    bool validate() {
        if (nextToken().type == JsonToken::END_OF_STREAM)
            return true;

        storeLast();
        return parseValue();
    }

private:
    bool parseValue() {
        switch (nextToken().type) {
            case JsonToken::NULL_VALUE:
            case JsonToken::BOOLEAN_VALUE:
            case JsonToken::NUMBER_VALUE:
            case JsonToken::STRING_VALUE:
                return true;

            case JsonToken::ARRAY_START:
                storeLast();
                return parseArray();

            case JsonToken::OBJECT_START:
                storeLast();
                return parseObject();

            default:
                return false;
        }
    }

    bool parseArray() {
        if (nextToken().type != JsonToken::ARRAY_START)
            return false;

        while (nextToken().type != JsonToken::ARRAY_END) {
            storeLast();

            do {
                if (!parseValue())
                    return false;
            } while (nextToken().type == JsonToken::COMMA);
            storeLast();
        }
        return true;
    }

    bool parseObject() {
        if (nextToken().type != JsonToken::OBJECT_START)
            return false;

        while (nextToken().type != JsonToken::OBJECT_END) {
            storeLast();

            do {
                if (!parseKeyValue())
                    return false;
            } while (nextToken().type == JsonToken::COMMA);
            storeLast();
        }
        return true;
    }

    bool parseKeyValue() {
        if (nextToken().type != JsonToken::STRING_VALUE)
            return false;
        if (nextToken().type != JsonToken::COLON)
            return false;
        if (!parseValue())
            return false;

        return true;
    }

private:
    JsonToken nextToken() {
        if (stored) {
            stored = false;
            return value;
        }
        while (true) {
            value = tokenReader.nextToken();
            if (value.type != JsonToken::WHITESPACE)
                break;
        }
        return value;
    }

    void storeLast() {
        stored = true;
    }

    JsonTokenReader& tokenReader;
    JsonToken value;
    bool stored;
};

#endif  // JSON_VALIDATOR_HPP_INCLUDED
