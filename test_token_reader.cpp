#include "json_validator.hpp"
#include <iostream>

std::ostream& operator<<(std::ostream& out, JsonToken const& token) {
    switch (token.type) {
    case JsonToken::UNEXPECTED:     out << "UNEXPECTED";    break;
    case JsonToken::END_OF_STREAM:  out << "END_OF_STREAM"; break;
    case JsonToken::WHITESPACE:     out << "WHITESPACE";    break;
    case JsonToken::OBJECT_START:   out << "OBJECT_START";  break;
    case JsonToken::OBJECT_END:     out << "OBJECT_END";    break;
    case JsonToken::ARRAY_START:    out << "ARRAY_START";   break;
    case JsonToken::ARRAY_END:      out << "ARRAY_END";     break;
    case JsonToken::COMMA:          out << "COMMA";         break;
    case JsonToken::COLON:          out << "COLON";         break;
    case JsonToken::NULL_VALUE:     out << "NULL_VALUE";    break;
    case JsonToken::BOOLEAN_VALUE:  out << "BOOLEAN_VALUE"; break;
    case JsonToken::NUMBER_VALUE:   out << "NUMBER_VALUE";  break;
    case JsonToken::STRING_VALUE:   out << "STRING_VALUE";  break;
    default:                        out << "#UNKNOWN#";     break;
    }
    return out;
}

int main(int, char*[])
{
    JsonCharReader charReader([&](char& c) {
        return bool(std::cin.get(c));
    });

    JsonTokenReader tokenReader(charReader);
    while (true) {
        auto const token = tokenReader.nextToken();
        std::cout << token << std::endl;
        if (token.type == JsonToken::UNEXPECTED || token.type == JsonToken::END_OF_STREAM)
            break;
    }
}
